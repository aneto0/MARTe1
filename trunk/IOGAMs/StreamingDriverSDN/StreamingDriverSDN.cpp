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
#include "StreamingDriverSDN.h"
#include "CDBExtended.h"

void StreamingDriverSDNUDPConsumerThread(void *args){
    StreamingDriverSDN *streamingDriver    = (StreamingDriverSDN *)args;
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
                    CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not write ready packet in socket!");
                }

				//Publish SDN data
                if(!streamingDriver->publisherSDNSocket.publish((void *)(streamingDriver->transferBuffer - sizeof(double)))){
                    CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not write ready packet in SDN!");
                }

                memTransferCounter = 0;
            }

            //Mark as free
            if(Atomic::Exchange(&(streamingDriver->bufferFreeStates[idxCounter]), streamingDriver->Free) != streamingDriver->Full){
//                CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not mark buffer index: %d as free!", idxCounter);
            }

            idxCounter++;
            if(idxCounter == streamingDriver->numberOfBuffers){
                idxCounter = 0;
            }
            //Full loop and still here? Is this a problem? To be decided
            if(idxCounter == startIdxCounter && (streamingDriver->numberOfTransferBuffers > 1)){
                CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. The consumer is likely under heavy stress. A full consuming loop was performed, and no empty buffers were found. idxCounter=%d", idxCounter);
                idxCounter = streamingDriver->lastUnlockedBufferIndex;
                break;
            }
        }

        if(streamingDriver->consumerThreadIsAlive){
            streamingDriver->synchEventSem.ResetWait();
        }
    }
    streamingDriver->consumerThreadIsAlive = True;
    CStaticAssertErrorCondition(Information, "StreamingDriverSDN::StreamingDriverSDNConsumerThread is exiting");
}

void StreamingDriverUDPConsumerThread(void *args){
    StreamingDriverSDN *streamingDriver    = (StreamingDriverSDN *)args;
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
                    CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not write ready packet in socket!");
                }

                memTransferCounter = 0;
            }

            //Mark as free
            if(Atomic::Exchange(&(streamingDriver->bufferFreeStates[idxCounter]), streamingDriver->Free) != streamingDriver->Full){
//////////////////                CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not mark buffer index: %d as free!", idxCounter);
            }

            idxCounter++;
            if(idxCounter == streamingDriver->numberOfBuffers){
                idxCounter = 0;
            }
            //Full loop and still here? Is this a problem? To be decided
            if(idxCounter == startIdxCounter){
                CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. The consumer is likely under heavy stress. A full consuming loop was performed, and no empty buffers were found. idxCounter=%d", idxCounter);
                idxCounter = streamingDriver->lastUnlockedBufferIndex;
                break;
            }
        }

        if(streamingDriver->consumerThreadIsAlive){
            streamingDriver->synchEventSem.ResetWait();
        }
    }
    streamingDriver->consumerThreadIsAlive = True;
    CStaticAssertErrorCondition(Information, "StreamingDriverSDN::StreamingDriverConsumerThread is exiting");
}

void StreamingDriverSDNConsumerThread(void *args){
    StreamingDriverSDN *streamingDriver    = (StreamingDriverSDN *)args;
    int32            idxCounter         = 0;
    int32            startIdxCounter    = 0;
    int32            memTransferCounter = 0;
    uint32           transferPacketSize = 0;

    double now  = 0;
    double last = 0;
    double send = 0; 

    Threads::SetRealTimeClass();
	int64 start;
    while(streamingDriver->consumerThreadIsAlive){
        startIdxCounter = idxCounter;
	
        while(streamingDriver->bufferLockStates[idxCounter] == streamingDriver->Unlocked && streamingDriver->bufferFreeStates[idxCounter] == streamingDriver->Full){
		//printf("Last time here was @ %e seconds ago\n", ((HRT::HRTCounter() - start)* HRT::HRTPeriod()));
		start = HRT::HRTCounter();
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

		//Publish SDN data
                if(!streamingDriver->publisherSDNSocket.publish((void *)(streamingDriver->transferBuffer - sizeof(double)))){
                    CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not write ready packet in SDN!");
                }
                memTransferCounter = 0;
            }

            //Mark as free
            if(Atomic::Exchange(&(streamingDriver->bufferFreeStates[idxCounter]), streamingDriver->Free) != streamingDriver->Full){
                CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. Could not mark buffer index: %d as free!", idxCounter);
            }

            idxCounter++;
            if(idxCounter == streamingDriver->numberOfBuffers){
                idxCounter = 0;
            }
            //Full loop and still here? Is this a problem? To be decided
            if(idxCounter == startIdxCounter && (streamingDriver->numberOfBuffers > 1)){
                CStaticAssertErrorCondition(Warning, "StreamingDriverSDN::StreamingDriverConsumerThread. The consumer is likely under heavy stress. A full consuming loop was performed, and no empty buffers were found. idxCounter=%d", idxCounter);
                idxCounter = streamingDriver->lastUnlockedBufferIndex;
                break;
            }
        }

        if(streamingDriver->consumerThreadIsAlive){
            streamingDriver->synchEventSem.Wait();
            streamingDriver->synchEventSem.Reset();
        }
    }
    streamingDriver->consumerThreadIsAlive = True;
    CStaticAssertErrorCondition(Information, "StreamingDriverSDN::StreamingDriverConsumerThread is exiting");
}

int StreamingDriverSDN::GetNextUnlockedBufferIndexAndLock(){
    lastUnlockedBufferIndex++;
    if(lastUnlockedBufferIndex == numberOfBuffers){
        lastUnlockedBufferIndex = 0;
    }
    Atomic::Exchange(&bufferLockStates[lastUnlockedBufferIndex], Locked);
    return lastUnlockedBufferIndex;
}

bool StreamingDriverSDN::WriteData(uint32 usecTime, const int32 *toWriteBuffer){
    // Get the last time mark
    lastUsecTime = usecTime;
    
    //A free buffer index and lock the buffer
    int32 unlockedBufferIndex = GetNextUnlockedBufferIndexAndLock();
    if(unlockedBufferIndex == -1){
        //No free buffer was found. Return True just not to stop the realtime thread
        AssertErrorCondition(Warning,"StreamingDriverSDN::WriteData: No free (unlocked) buffer was found. The number of configured buffers is: %d", numberOfBuffers);
        return True;
    }

    if(bufferFreeStates[unlockedBufferIndex] == Full){
        lostBuffers++;
        //We are running too fast and just overlaped the consumer thread
        if((HRT::HRTCounter() - lastWarningCounter) * HRT::HRTPeriod() > 1){
            AssertErrorCondition(Warning, "StreamingDriverSDN::WriteData: The system seems to be underdimensioned. Wrote over full buffers in the last second. Conf. buffers: %d Wrote over: %d", numberOfBuffers, lostBuffers);
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
        AssertErrorCondition(Warning, "StreamingDriverSDN::WriteData: Could not mark buffer index: %d as full!", unlockedBufferIndex);
    }
    //Unlock the buffer
    if(Atomic::Exchange(&bufferLockStates[unlockedBufferIndex], Unlocked) != Locked){
        AssertErrorCondition(FatalError, "StreamingDriverSDN::WriteData: Could not unlock buffer index: %d which was locked by me!", unlockedBufferIndex);
    }
    synchEventSem.Post();

    /** Trigger External Activities */
    for(int activity = 0; activity < nOfTriggeringServices; activity++){
        triggerService[activity].Trigger();
    }
    
    return True;
}

int32 StreamingDriverSDN::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber){
    
    return 1;
}

bool StreamingDriverSDN::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    // Common initializations
    if(!GenericAcqModule::ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Generic initialization failed",Name());
        return False;
    }
   
    if(!cdb.ReadInt32(numberOfBuffers, "NumberOfBuffers", 20000)){
        AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s NumberOfBuffers has not been specified. Using default value: %d", Name(), numberOfBuffers);
    }        
 
    if(!cdb.ReadInt32(numberOfTransferBuffers, "NumberOfTransferBuffers", 50)){
        AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s NumberOfTransferBuffers has not been specified. Using default value: %d", Name(), numberOfTransferBuffers);
    }

    if(!cdb.ReadInt32(outputType, "OutputType", 0)){
        AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s outputType has not been specified. Using default value: %d", Name(), outputType);
    }  

    //UDP Config
    if (outputType == 0 || outputType == 2){
        if(!cdb.ReadInt32(receiverUDPPort, "ReceiverUDPPort", 14500)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s ReceiverUDPPort has not been specified. Using default value: %d", Name(), receiverUDPPort);
        }

        if(!cdb.ReadBString(receiverUDPAddress, "ReceiverUDPAddress", "localhost")){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s ReceiverUDPAddress has not been specified. Using default value: %s", Name(), receiverUDPAddress.Buffer());
        }

        if(!cdb.ReadInt32(cpuMask, "CpuMask", 0x2)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s CpuMask has not been specified. Using default value: 0x%x", Name(), cpuMask);
        }


        if(!senderSocket.Open()){
            AssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Could not open streaming socket", Name());
            return False;
        }

        if(!senderSocket.Connect(receiverUDPAddress.Buffer(), receiverUDPPort)){
            AssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Could not connect streaming socket to address: %s , port: %d", Name(), receiverUDPAddress.Buffer(), receiverUDPPort);
            return False;
        }

        AssertErrorCondition(Information, "StreamingDriverSDN::ObjectLoadSetup: %s Connected streaming socket to address: %s , port: %d", Name(), receiverUDPAddress.Buffer(), receiverUDPPort);
    }

    //SDN Config
    if (outputType == 1 || outputType == 2){
	int readInt = 0;
	if(!cdb.ReadInt32(readInt, "ReceiverSDNPort", 50000)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s receiverSDNPort has not been specified. Using default value: %d", Name(), readInt);
    	}else{
	    publisherSDNSocket.setPort(readInt);
	}  

    	if(!cdb.ReadInt32(readInt, "ReceiverSDNRemoteNode", 1)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s receiverSDNRemoteNode has not been specified. Using default value: %d", Name(), readInt);
	}else{
	    publisherSDNSocket.setRemoteNode(readInt);
	}  

    	if(!cdb.ReadInt32(readInt, "ConnectTimeout", 0)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s connectTimeout has not been specified. Using default value: %d", Name(), readInt);
	}else{
	    publisherSDNSocket.setConnectTimeout(readInt);
	}  

    	if(!cdb.ReadInt32(readInt, "WaitTimeout", 0)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s waitTimeout has not been specified. Using default value: %d", Name(), readInt);
	}else{
	    publisherSDNSocket.setWaitTimeout(readInt);
	}

        FString readString;
	if(!cdb.ReadBString(readString, "ReceiverSDNAddress", "224.0.1.10")){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s receiverSDNAddress has not been specified. Using default value: %s", Name(), readString.Buffer());
    	}else{
	    publisherSDNSocket.setAddress((char *)readString.Buffer());
	}

        if(!cdb.ReadBString(readString, "ReceiverSDNNodes", "4,8,12")){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s receiverSDNNodes has not been specified. Using default value: %s", Name(), readString.Buffer());
        }else{
	    publisherSDNSocket.setNodes((char *)readString.Buffer());
	}

        if(!cdb.ReadBString(readString, "ReceiverSDNSegmentID", "1")){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s receiverSDNSegmentID has not been specified. Using default value: %s", Name(), readString.Buffer());
        }else{
	    publisherSDNSocket.setSegmentID((char *)readString.Buffer());
	}

        if(!cdb.ReadInt32(cpuMask, "CpuMask", 0x2)){
            AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s CpuMask has not been specified. Using default value: 0x%x", Name(), cpuMask);
        }

    }

    if(numberOfTransferBuffers > numberOfBuffers){
        AssertErrorCondition(InitialisationError, "StreamingDriverSDN::ObjectLoadSetup: NumberOfTransferBuffers must no be higher than NumberOfBuffers: %d > %d", numberOfTransferBuffers, numberOfBuffers);
        return False;
    }

    if(memoryBuffer != NULL) free((void *&)memoryBuffer);
    memBufferSize    = NumberOfOutputs() * sizeof(float) + sizeof(int32);
    memoryBuffer     = (char *)malloc(memBufferSize * numberOfBuffers);
    if(memoryBuffer == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Failed allocating %d bytes for storing data",Name(), memBufferSize * numberOfBuffers);
        return False;
    }    
    memset(memoryBuffer, 0, memBufferSize * numberOfBuffers);
    AssertErrorCondition(Information,"StreamingDriverSDN::ObjectLoadSetup: %s Successfully allocated %d bytes for data storing divided in %d buffers",Name(), memBufferSize * numberOfBuffers, numberOfBuffers);


    if(transferBuffer != NULL){
        transferBuffer -= sizeof(double);
        free((void *&)transferBuffer);        
    }
    transferBuffer     = (char *)malloc(memBufferSize * numberOfTransferBuffers + sizeof(double));
    if(transferBuffer  == NULL){
        CStaticAssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Could not allocate: %d bytes for the transfer buffer. The consumer thread will NOT start", Name(), memBufferSize * numberOfTransferBuffers);
    }
    memset(transferBuffer, 0, memBufferSize * numberOfTransferBuffers + sizeof(double));
    transferBuffer += sizeof(double);
    AssertErrorCondition(Information,"StreamingDriverSDN::ObjectLoadSetup: %s Successfully allocated %d bytes for data streaming divided in %d buffers",Name(), memBufferSize * numberOfTransferBuffers, numberOfTransferBuffers);


    if(bufferLockStates != NULL) free((void *&)bufferLockStates);
    bufferLockStates = (int32 *)malloc(numberOfBuffers * sizeof(int32));
    if(bufferLockStates == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Failed allocating %d bytes for the buffer locking semaphores" ,Name(), numberOfBuffers * sizeof(int32));
        return False;
    }
    memset(bufferLockStates, 0, numberOfBuffers * sizeof(int32));

    if(bufferFreeStates != NULL) free((void *&)bufferFreeStates);
    bufferFreeStates = (int32 *)malloc(numberOfBuffers * sizeof(int32));
    if(bufferFreeStates == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriverSDN::ObjectLoadSetup: %s Failed allocating %d bytes for the buffer locking semaphores" ,Name(), numberOfBuffers * sizeof(int32));
        return False;
    }
    memset(bufferFreeStates, 0, numberOfBuffers * sizeof(int32));

    /** Begin the consumer thread*/
    lastUnlockedBufferIndex = -1;
    
    //SDN publish init

    int segmentSize = memBufferSize * numberOfTransferBuffers + sizeof(double);
    publisherSDNSocket.setSegmentSize(segmentSize);

    if (outputType != 0){
        publisherSDNSocket.setSegmentSize(segmentSize);

        if(!publisherSDNSocket.publisher_init()){
            AssertErrorCondition(InitialisationError,"StreamingDriverSDN::publish_init Error");
            return False;
        }else{
	    AssertErrorCondition(Information, "StreamingDriverSDN::publish_init: SDN Publisher Initialized: Version: %s", sdn_version());	        
        }
    }  

    //0 - UDP, 1 - SDN, 2 - Both
    switch(outputType){
	case 0:
		consumerThreadID = Threads::BeginThread((ThreadFunctionType)StreamingDriverUDPConsumerThread, this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
	        break;
	case 1:
		consumerThreadID = Threads::BeginThread((ThreadFunctionType)StreamingDriverSDNConsumerThread, this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
	        break;
	case 2:
		consumerThreadID = Threads::BeginThread((ThreadFunctionType)StreamingDriverSDNUDPConsumerThread, this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
	        break;
    }

    return True;
}

StreamingDriverSDN::~StreamingDriverSDN(){
    if(memoryBuffer     != NULL) free((void *&)memoryBuffer);
    if(bufferLockStates != NULL) free((void *&)bufferLockStates);
    if(bufferFreeStates != NULL) free((void *&)bufferFreeStates);
    if(transferBuffer != NULL){
        transferBuffer -= sizeof(double);
        free((void *&)transferBuffer);        
    }
    //Change to SDN
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
        AssertErrorCondition(InitialisationError,"StreamingDriverSDN::~StreamingDriver(): Consumer thread did not exited in 1 seconds %s", Name());
    }
}

StreamingDriverSDN::StreamingDriverSDN(){

    memoryBuffer            = NULL;
    transferBuffer          = NULL;
    bufferLockStates        = NULL;
    bufferFreeStates        = NULL;
    
    memBufferSize           = 0;
    lastUsecTime            = 0;
    numberOfBuffers         = 0;
    numberOfTransferBuffers = 0;
    //Change to SDN
    receiverUDPPort         = 0;
    cpuMask                 = 0;

    lastUnlockedBufferIndex = -1;

    lostBuffers             = 0;
    lastWarningCounter      = 0;

    consumerThreadIsAlive   = True;
    consumerThreadID        = 0;

    //Change to SDN
    receiverUDPAddress.SetSize(0);

    synchEventSem.Create();
    synchEventSem.Reset();

}


OBJECTLOADREGISTER(StreamingDriverSDN,"$Id: StreamingDriverSDN.cpp,v 1.8 2009/06/09 11:04:51 aneto Exp $")
