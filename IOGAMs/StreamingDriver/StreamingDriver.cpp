/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

#include "System.h"
#include "StreamingDriver.h"
#include "CDBExtended.h"

void StreamingDriverConsumerThread(void *args){
    StreamingDriver *streamingDriver    = (StreamingDriver *)args;
    int32            idxCounter         = 0;
    int32            startIdxCounter    = 0;
    int32            memTransferCounter = 0;
    uint32           transferPacketSize = 0;

    double now  = 0;
    double last = 0;
    double send = 0; 

    Threads::SetRealTimeClass();

    while(streamingDriver->consumerThreadIsAlive){
        startIdxCounter = idxCounter;
        while(streamingDriver->bufferLockStates[idxCounter] == streamingDriver->Unlocked && streamingDriver->bufferFreeStates[idxCounter] == streamingDriver->Full){
            //Copy the data to the transfer buffer
            memcpy(streamingDriver->transferBuffer + memTransferCounter * streamingDriver->memBufferSize, streamingDriver->memoryBuffer + idxCounter * streamingDriver->memBufferSize, streamingDriver->memBufferSize);
            memTransferCounter++;
            //packet is ready to go
            if(memTransferCounter == streamingDriver->numberOfTransferBuffers){                
                now  = HRT::HRTCounter() * HRT::HRTPeriod();
                send = now - last;
                last = now;
                memcpy(streamingDriver->transferBuffer - sizeof(double), &send, sizeof(double));
                //Send packet
                transferPacketSize = streamingDriver->memBufferSize * streamingDriver->numberOfTransferBuffers + sizeof(double);
                if(!streamingDriver->senderSocket.Write((const void *)(streamingDriver->transferBuffer - sizeof(double)), transferPacketSize)){
                    CStaticAssertErrorCondition(Warning, "StreamingDriver::StreamingDriverConsumerThread. Could not write ready packet in socket!");
                }
                memTransferCounter = 0;
            }

            //Mark as free
            if(Atomic::Exchange(&(streamingDriver->bufferFreeStates[idxCounter]), streamingDriver->Free) != streamingDriver->Full){
                CStaticAssertErrorCondition(Warning, "StreamingDriver::StreamingDriverConsumerThread. Could not mark buffer index: %d as free!", idxCounter);
            }

            idxCounter++;
            if(idxCounter == streamingDriver->numberOfBuffers){
                idxCounter = 0;
            }
            //Full loop and still here? Is this a problem? To be decided
            if((idxCounter == startIdxCounter) && (streamingDriver->numberOfBuffers > 1)){
                CStaticAssertErrorCondition(Warning, "StreamingDriver::StreamingDriverConsumerThread. The consumer is likely under heavy stress. A full consuming loop was performed, and no empty buffers were found. idxCounter=%d", idxCounter);
                idxCounter = streamingDriver->lastUnlockedBufferIndex;
                break;
            }
        }

        if(streamingDriver->consumerThreadIsAlive){
            streamingDriver->synchEventSem.ResetWait();
        }
    }
    streamingDriver->consumerThreadIsAlive = True;
    CStaticAssertErrorCondition(Information, "StreamingDriver::StreamingDriverConsumerThread is exiting");
}

int StreamingDriver::GetNextUnlockedBufferIndexAndLock(){
    lastUnlockedBufferIndex++;
    if(lastUnlockedBufferIndex == numberOfBuffers){
        lastUnlockedBufferIndex = 0;
    }
    Atomic::Exchange(&bufferLockStates[lastUnlockedBufferIndex], Locked);
    return lastUnlockedBufferIndex;
}


bool StreamingDriver::WriteData(uint32 usecTime, const int32 *toWriteBuffer){
    // Get the last time mark
    lastUsecTime = usecTime;
    
    //A free buffer index and lock the buffer
    int32 unlockedBufferIndex = GetNextUnlockedBufferIndexAndLock();
    if(unlockedBufferIndex == -1){
        //No free buffer was found. Return True just not to stop the realtime thread
        AssertErrorCondition(Warning,"StreamingDriver::WriteData: No free (unlocked) buffer was found. The number of configured buffers is: %d", numberOfBuffers);
        return True;
    }

    if(bufferFreeStates[unlockedBufferIndex] == Full){
        lostBuffers++;
        //We are running too fast and just overlaped the consumer thread
        if((HRT::HRTCounter() - lastWarningCounter) * HRT::HRTPeriod() > 1){
            AssertErrorCondition(Warning, "StreamingDriver::WriteData: The system seems to be underdimensioned. Wrote over full buffers in the last second. Conf. buffers: %d Wrote over: %d", numberOfBuffers, lostBuffers);
            lostBuffers = 0;
            lastWarningCounter = HRT::HRTCounter();
        }
        //Mark as free
        Atomic::Exchange(&bufferFreeStates[unlockedBufferIndex], Free);
    }

    //Local copy of the buffer
    uint32 *copyBuffer = (uint32 *)(memoryBuffer + unlockedBufferIndex * memBufferSize);

    //Copy the time header    
    *copyBuffer = usecTime;
    copyBuffer++;
    //Copy the data
    memcpy(copyBuffer, toWriteBuffer, memBufferSize - sizeof(int32));
   
    //Mark as full
    if(Atomic::Exchange(&bufferFreeStates[unlockedBufferIndex], Full) != Free){
        AssertErrorCondition(Warning, "StreamingDriver::WriteData: Could not mark buffer index: %d as full!", unlockedBufferIndex);
    }
    //Unlock the buffer
    if(Atomic::Exchange(&bufferLockStates[unlockedBufferIndex], Unlocked) != Locked){
        AssertErrorCondition(FatalError, "StreamingDriver::WriteData: Could not unlock buffer index: %d which was locked by me!", unlockedBufferIndex);
    }
    synchEventSem.Post();

    /** Trigger External Activities */
    for(int activity = 0; activity < nOfTriggeringServices; activity++){
        triggerService[activity].Trigger();
    }
    
    return True;
}

int32 StreamingDriver::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber){
    
    return 1;
}

bool StreamingDriver::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    // Common initializations
    if(!GenericAcqModule::ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Generic initialization failed",Name());
        return False;
    }
   
    if(!cdb.ReadInt32(numberOfBuffers, "NumberOfBuffers", 20000)){
        AssertErrorCondition(Warning,"StreamingDriver::ObjectLoadSetup: %s NumberOfBuffers has not been specified. Using default value: %d", Name(), numberOfBuffers);
    }        
 
    if(!cdb.ReadInt32(numberOfTransferBuffers, "NumberOfTransferBuffers", 50)){
        AssertErrorCondition(Warning,"StreamingDriver::ObjectLoadSetup: %s NumberOfTransferBuffers has not been specified. Using default value: %d", Name(), numberOfTransferBuffers);
    }

    if(!cdb.ReadInt32(receiverUDPPort, "ReceiverUDPPort", 14500)){
        AssertErrorCondition(Warning,"StreamingDriver::ObjectLoadSetup: %s ReceiverUDPPort has not been specified. Using default value: %d", Name(), receiverUDPPort);
    }

    if(!cdb.ReadBString(receiverUDPAddress, "ReceiverUDPAddress", "localhost")){
        AssertErrorCondition(Warning,"StreamingDriver::ObjectLoadSetup: %s ReceiverUDPAddress has not been specified. Using default value: %s", Name(), receiverUDPAddress.Buffer());
    }

    if(!cdb.ReadInt32(cpuMask, "CpuMask", 0x2)){
        AssertErrorCondition(Warning,"StreamingDriver::ObjectLoadSetup: %s CpuMask has not been specified. Using default value: 0x%x", Name(), cpuMask);
    }

    if(!senderSocket.Open()){
        AssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Could not open streaming socket", Name());
        return False;
    }

    if(!senderSocket.Connect(receiverUDPAddress.Buffer(), receiverUDPPort)){
        AssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Could not connect streaming socket to address: %s , port: %d", Name(), receiverUDPAddress.Buffer(), receiverUDPPort);
        return False;
    }

    AssertErrorCondition(Information, "StreamingDriver::ObjectLoadSetup: %s Connected streaming socket to address: %s , port: %d", Name(), receiverUDPAddress.Buffer(), receiverUDPPort);

    if(numberOfTransferBuffers > numberOfBuffers){
        AssertErrorCondition(InitialisationError, "StreamingDriver::ObjectLoadSetup: NumberOfTransferBuffers must no be higher than NumberOfBuffers: %d > %d", numberOfTransferBuffers, numberOfBuffers);
        return False;
    }

    if(memoryBuffer != NULL) free((void *&)memoryBuffer);
    memBufferSize    = NumberOfOutputs() * sizeof(float) + sizeof(int32);
    memoryBuffer     = (char *)malloc(memBufferSize * numberOfBuffers);
    if(memoryBuffer == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Failed allocating %d bytes for storing data",Name(), memBufferSize * numberOfBuffers);
        return False;
    }    
    memset(memoryBuffer, 0, memBufferSize * numberOfBuffers);
    AssertErrorCondition(Information,"StreamingDriver::ObjectLoadSetup: %s Successfully allocated %d bytes for data storing divided in %d buffers",Name(), memBufferSize * numberOfBuffers, numberOfBuffers);


    if(transferBuffer != NULL){
        transferBuffer -= sizeof(double);
        free((void *&)transferBuffer);        
    }
    transferBuffer     = (char *)malloc(memBufferSize * numberOfTransferBuffers + sizeof(double));
    if(transferBuffer  == NULL){
        CStaticAssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Could not allocate: %d bytes for the transfer buffer. The consumer thread will NOT start", Name(), memBufferSize * numberOfTransferBuffers);
    }
    memset(transferBuffer, 0, memBufferSize * numberOfTransferBuffers + sizeof(double));
    transferBuffer += sizeof(double);
    AssertErrorCondition(Information,"StreamingDriver::ObjectLoadSetup: %s Successfully allocated %d bytes for data streaming divided in %d buffers",Name(), memBufferSize * numberOfTransferBuffers, numberOfTransferBuffers);


    if(bufferLockStates != NULL) free((void *&)bufferLockStates);
    bufferLockStates = (int32 *)malloc(numberOfBuffers * sizeof(int32));
    if(bufferLockStates == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Failed allocating %d bytes for the buffer locking semaphores" ,Name(), numberOfBuffers * sizeof(int32));
        return False;
    }
    memset(bufferLockStates, 0, numberOfBuffers * sizeof(int32));


    if(bufferFreeStates != NULL) free((void *&)bufferFreeStates);
    bufferFreeStates = (int32 *)malloc(numberOfBuffers * sizeof(int32));
    if(bufferFreeStates == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriver::ObjectLoadSetup: %s Failed allocating %d bytes for the buffer locking semaphores" ,Name(), numberOfBuffers * sizeof(int32));
        return False;
    }
    memset(bufferFreeStates, 0, numberOfBuffers * sizeof(int32));

    /** Begin the consumer thread*/
    lastUnlockedBufferIndex = -1;
    consumerThreadID = Threads::BeginThread((ThreadFunctionType)StreamingDriverConsumerThread, this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);

    return True;
}

StreamingDriver::~StreamingDriver(){
    if(memoryBuffer     != NULL) free((void *&)memoryBuffer);
    if(bufferLockStates != NULL) free((void *&)bufferLockStates);
    if(bufferFreeStates != NULL) free((void *&)bufferFreeStates);
    if(transferBuffer != NULL){
        transferBuffer -= sizeof(double);
        free((void *&)transferBuffer);        
    }
    senderSocket.Close();
    consumerThreadIsAlive = False;
    synchEventSem.Post();
    int resetCounter = 0;
    while(!consumerThreadIsAlive){
        if(resetCounter > 10){
            break;
        }
        resetCounter++;
        SleepMsec(100);
    }

    if(!consumerThreadIsAlive){
        AssertErrorCondition(InitialisationError,"StreamingDriver::~StreamingDriver(): Consumer thread did not exited in 1 seconds %s", Name());
    }
}

StreamingDriver::StreamingDriver(){

    memoryBuffer            = NULL;
    transferBuffer          = NULL;
    bufferLockStates        = NULL;
    bufferFreeStates        = NULL;
    
    memBufferSize           = 0;
    lastUsecTime            = 0;
    numberOfBuffers         = 0;
    numberOfTransferBuffers = 0;
    receiverUDPPort         = 0;
    cpuMask                 = 0;

    lastUnlockedBufferIndex = -1;

    lostBuffers             = 0;
    lastWarningCounter      = 0;

    consumerThreadIsAlive   = True;
    consumerThreadID        = 0;

    receiverUDPAddress.SetSize(0);

    synchEventSem.Create();
    synchEventSem.Reset();
}


OBJECTLOADREGISTER(StreamingDriver,"$Id: StreamingDriver.cpp,v 1.9 2011/09/07 16:02:52 aneto Exp $")

