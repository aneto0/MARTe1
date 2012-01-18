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
#include "StreamingDriverSDNReceiver.h"
#include "CDBExtended.h"

void StreamingDriverReceiverThread(void *args){
    StreamingDriverSDNReceiver *streamingDriver    = (StreamingDriverSDNReceiver *)args;
    int32                    idxCounter         = 0;
    int32                    startIdxCounter    = 0;
    int32                    memTransferCounter = 0;
    uint32                   transferPacketSize = 0;

    Threads::SetHighClass();

    while(streamingDriver->receiverThreadIsAlive){
        //Wait for new packets
        transferPacketSize = streamingDriver->memBufferSize * streamingDriver->numberOfTransferBuffers + sizeof(double);
        if(!streamingDriver->receiverSocket.Read(streamingDriver->transferBuffer, transferPacketSize)){
            CStaticAssertErrorCondition(Warning, "StreamingDriverReceiver::StreamingDriverReceiverThread. Could not read ready packet in socket!");
            continue;
        }
        if(!streamingDriver->receiverThreadIsAlive){
            break;
        }

        int i = 0;

        for(i=0; i<streamingDriver->numberOfTransferBuffers; i++){
            //A free buffer index and lock the buffer
            int32 unlockedBufferIndex = streamingDriver->GetNextUnlockedBufferIndexAndLock();
            if(streamingDriver->bufferFreeStates[unlockedBufferIndex] == streamingDriver->Full){
                //We are running too fast and just overlaped the consumer thread
                streamingDriver->lostBuffers++;
                if((HRT::HRTCounter() - streamingDriver->lastWarningCounter) * HRT::HRTPeriod() > 1){
                    CStaticAssertErrorCondition(Warning, "StreamingDriverReceiver::StreamingDriverReceiverThread. The system seems to be underdimensioned. Wrote over full buffers in the last second. Conf. buffers: %d Wrote over: %d", streamingDriver->numberOfBuffers, streamingDriver->lostBuffers);
                    streamingDriver->lastWarningCounter = HRT::HRTCounter();
                    streamingDriver->lostBuffers = 0;
                }
                //Mark as free
                Atomic::Exchange(&streamingDriver->bufferFreeStates[unlockedBufferIndex], streamingDriver->Free);
                streamingDriver->lastReadIdx = unlockedBufferIndex;
            }
        
            //Local copy of the buffer
            uint32 *copyBuffer = (uint32 *)(streamingDriver->memoryBuffer + unlockedBufferIndex * streamingDriver->memBufferSize);
            //Copy the data
            memcpy(copyBuffer, streamingDriver->transferBuffer + sizeof(double) + i * streamingDriver->memBufferSize, streamingDriver->memBufferSize);
            streamingDriver->lastUsecTime = *copyBuffer;
            //Mark as full
            if(Atomic::Exchange(&streamingDriver->bufferFreeStates[unlockedBufferIndex], streamingDriver->Full) != streamingDriver->Free){
                CStaticAssertErrorCondition(Warning, "StreamingDriverReceiver::StreamingDriverReceiverThread Could not mark buffer index: %d as full!", unlockedBufferIndex);
            }
            //Unlock the buffer
            if(Atomic::Exchange(&streamingDriver->bufferLockStates[unlockedBufferIndex], streamingDriver->Unlocked) != streamingDriver->Locked){
                CStaticAssertErrorCondition(FatalError, "StreamingDriverReceiver::WriteData: Could not unlock buffer index: %d which was locked by me!", unlockedBufferIndex);
            }
            //Signal new buffer available
            streamingDriver->synchEventSem.Post();
        }
        //Signal new buffer available
        //streamingDriver->synchEventSem.Post();
    }
    streamingDriver->receiverThreadIsAlive = True;
    CStaticAssertErrorCondition(Information, "StreamingDriverReceiver::StreamingDriverReceiverThread is exiting");
}

void StreamingDriverSDNReceiverThread(void *args){
    StreamingDriverSDNReceiver *streamingDriver    = (StreamingDriverSDNReceiver *)args;
    int32                    idxCounter         = 0;
    int32                    startIdxCounter    = 0;
    int32                    memTransferCounter = 0;
    uint32                   transferPacketSize = 0;

    Threads::SetHighClass();
    while(streamingDriver->receiverThreadIsAlive){
         //Wait for new packets
        transferPacketSize = streamingDriver->memBufferSize * streamingDriver->numberOfTransferBuffers + sizeof(double);

	//printf("streamingDriver->numberOfTransferBuffers: %d\n", streamingDriver->numberOfTransferBuffers);
	volatile const void * data;
	data = streamingDriver->receiverSDNSocket.receive();
	//printf("Received new data\n");
        if (data !=NULL)
        {
		//printf("Coyping data\n");
	    memcpy(streamingDriver->transferBuffer, (char *) data, transferPacketSize);
        }
        else{
		//printf("NULL data\n");
	    CStaticAssertErrorCondition(Warning, "StreamingDriverSDNReceiver::StreamingDriverSDNReceiverThread. NULL package received");
	    continue;
        }

        if(!streamingDriver->receiverThreadIsAlive){
            break;
        }

        int i = 0;
        for(i=0; i<streamingDriver->numberOfTransferBuffers; i++){
            //A free buffer index and lock the buffer
            int32 unlockedBufferIndex = streamingDriver->GetNextUnlockedBufferIndexAndLock();
            if(streamingDriver->bufferFreeStates[unlockedBufferIndex] == streamingDriver->Full){
                //We are running too fast and just overlaped the consumer thread
                streamingDriver->lostBuffers++;
                if((HRT::HRTCounter() - streamingDriver->lastWarningCounter) * HRT::HRTPeriod() > 1){
//                    CStaticAssertErrorCondition(Warning, "StreamingDriverReceiver::StreamingDriverReceiverThread. The system seems to be underdimensioned. Wrote over full buffers in the last second. Conf. buffers: %d Wrote over: %d", streamingDriver->numberOfBuffers, streamingDriver->lostBuffers);
                    streamingDriver->lastWarningCounter = HRT::HRTCounter();
                    streamingDriver->lostBuffers = 0;
                }
                //Mark as free
                Atomic::Exchange(&streamingDriver->bufferFreeStates[unlockedBufferIndex], streamingDriver->Free);
                streamingDriver->lastReadIdx = unlockedBufferIndex;
            }
        
            //Local copy of the buffer
            uint32 *copyBuffer = (uint32 *)(streamingDriver->memoryBuffer + unlockedBufferIndex * streamingDriver->memBufferSize);
            
            //Copy the data
            memcpy(copyBuffer, streamingDriver->transferBuffer + sizeof(double) + i * streamingDriver->memBufferSize, streamingDriver->memBufferSize);
            streamingDriver->lastUsecTime = *copyBuffer;
		//printf("%lld\n", streamingDriver->lastUsecTime);

		//printf("Received: %d\n", *copyBuffer);	
            //Mark as full
            if(Atomic::Exchange(&streamingDriver->bufferFreeStates[unlockedBufferIndex], streamingDriver->Full) != streamingDriver->Free){
                CStaticAssertErrorCondition(Warning, "StreamingDriverReceiver::StreamingDriverReceiverThread Could not mark buffer index: %d as full!", unlockedBufferIndex);
            }
            //Unlock the buffer
            if(Atomic::Exchange(&streamingDriver->bufferLockStates[unlockedBufferIndex], streamingDriver->Unlocked) != streamingDriver->Locked){
                CStaticAssertErrorCondition(FatalError, "StreamingDriverReceiver::WriteData: Could not unlock buffer index: %d which was locked by me!", unlockedBufferIndex);
            }
            //Signal new buffer available
            streamingDriver->synchEventSem.Post();
        }
        //Signal new buffer available
        //streamingDriver->synchEventSem.Post();
    }
    streamingDriver->receiverThreadIsAlive = True;
    CStaticAssertErrorCondition(Information, "StreamingDriverReceiver::StreamingDriverReceiverThread is exiting");
}

int StreamingDriverSDNReceiver::GetNextUnlockedBufferIndexAndLock(){

    lastUnlockedBufferIndex++;
    if(lastUnlockedBufferIndex == numberOfBuffers){
        lastUnlockedBufferIndex = 0;
    }
    Atomic::Exchange(&bufferLockStates[lastUnlockedBufferIndex], Locked);
    return lastUnlockedBufferIndex;
}

int32 StreamingDriverSDNReceiver::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber){
    int32 *lbuffer = (int32 *)(memoryBuffer + memBufferSize * lastReadIdx);
    int32 *rbuffer = buffer;
/*    if(synchronizing){
        rbuffer[0] = *lbuffer;
        rbuffer[1] = buffer[0];

        // Skip the packet sample number and sample time
        rbuffer += 2;
    }*/

    memcpy(rbuffer, lbuffer + 1, memBufferSize - sizeof(int32));
//    if(Atomic::Exchange(&bufferFreeStates[lastReadIdx], Free) != Full){
//        AssertErrorCondition(Warning, "StreamingDriver Could not mark buffer index: %d as free!", lastReadIdx);
//    }

    lastReadIdx++;
    if(lastReadIdx == numberOfBuffers){
    	lastReadIdx = 0;
    }
    return True;
}

bool StreamingDriverSDNReceiver::Poll(){
    if(bufferLockStates[lastReadIdx] == Unlocked && bufferFreeStates[lastReadIdx] == Full){
        /** Trigger External Activities */
        for(int activity = 0; activity < nOfTriggeringServices; activity++){
            triggerService[activity].Trigger();
        }
        Atomic::Exchange(&bufferFreeStates[lastReadIdx], Free);
	synchEventSem.Reset();
        return True;
    }
    if(synchEventSem.Wait(synchronizingMSTimeout)){
        return Poll();
    }

    return False;
}

bool StreamingDriverSDNReceiver::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    // Common initializations
    if(!GenericAcqModule::ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Generic initialization failed",Name());
        return False;
    }
   
    if(!cdb.ReadInt32(numberOfBuffers, "NumberOfBuffers", 20000)){
        AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s NumberOfBuffers has not been specified. Using default value: %d", Name(), numberOfBuffers);
    }        

    if(!cdb.ReadInt32(numberOfTransferBuffers, "NumberOfTransferBuffers", 50)){
        AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s NumberOfTransferBuffers has not been specified. Using default value: %d", Name(), numberOfTransferBuffers);
    }

    if(!cdb.ReadInt32(receiverUDPPort, "ReceiverUDPPort", 14500)){
        AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s ReceiverUDPPort has not been specified. Using default value: %d", Name(), receiverUDPPort);
    }

    if(!cdb.ReadInt32(cpuMask, "CpuMask", 0x2)){
        AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s CpuMask has not been specified. Using default value: 0x%x", Name(), cpuMask);
    }

    FString syncMethod;
    if(!cdb.ReadFString(syncMethod, "SynchronizationMethod")){
        CStaticAssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: SynchronizationMethod has not been specified.");
        return False;
    }
    if (syncMethod == "GetLatest") synchronizing = False;
    else synchronizing = True;
    
    if(!cdb.ReadInt32(synchronizingMSTimeout, "SynchronizingMSTimeout", 100)){
        if(synchronizing){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s SynchronizingMSTimeout has not been specified. Using default value: %d", Name(), synchronizingMSTimeout);
        }
    }
 
    //Type Config
    if(!cdb.ReadInt32(inputType, "InputType", 0)){
        AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s inputType has not been specified. Using default value: %d", Name(), inputType);
    }

    //UDP Config
    if (inputType == 0){
        if(!receiverSocket.Open()){
            AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Could not open streaming socket", Name());
            return False;
        }

        if(!receiverSocket.SetBlocking(True)){
            AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Could not set socket blocking = True", Name());
        }

        if(!receiverSocket.Listen(receiverUDPPort)){
            AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Streaming socket cannot listen in port: %d", Name(), receiverUDPPort);
            return False;
        }

        if(!receiverSocket.SetBlocking(True)){
            AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Could not set socket blocking = True", Name());
        }

        AssertErrorCondition(Information, "StreamingDriverReceiver::ObjectLoadSetup: %s Streaming socket listening in port: %d", Name(), receiverUDPPort);
    }
 
    //SDN Config
    if (inputType == 1){
	int readInt = 0;
	if(!cdb.ReadInt32(readInt, "ReceiverSDNPort", 50000)){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s receiverSDNPort has not been specified. Using default value: %d", Name(), readInt);
    	}else{
	    receiverSDNSocket.setPort(readInt);
	}  

    	if(!cdb.ReadInt32(readInt, "ReceiverSDNRemoteNode", 1)){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s receiverSDNRemoteNode has not been specified. Using default value: %d", Name(), readInt);
	}else{
	    receiverSDNSocket.setRemoteNode(readInt);
	}  

    	if(!cdb.ReadInt32(readInt, "ConnectTimeout", 0)){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s connectTimeout has not been specified. Using default value: %d", Name(), readInt);
	}else{
	    receiverSDNSocket.setConnectTimeout(readInt);
	}  

    	if(!cdb.ReadInt32(readInt, "WaitTimeout", 0)){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s waitTimeout has not been specified. Using default value: %d", Name(), readInt);
	}else{
	    receiverSDNSocket.setWaitTimeout(readInt);
	}

        FString readString;
	if(!cdb.ReadBString(readString, "ReceiverSDNAddress", "224.0.1.10")){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s receiverSDNAddress has not been specified. Using default value: %s", Name(), readString.Buffer());
    	}else{
	    receiverSDNSocket.setAddress((char *)readString.Buffer());
	}

        if(!cdb.ReadBString(readString, "ReceiverSDNNodes", "4,8,12")){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s receiverSDNNodes has not been specified. Using default value: %s", Name(), readString.Buffer());
        }else{
	    receiverSDNSocket.setNodes((char *)readString.Buffer());
	}

        if(!cdb.ReadBString(readString, "ReceiverSDNSegmentID", "1")){
            AssertErrorCondition(Warning,"StreamingDriverReceiver::ObjectLoadSetup: %s receiverSDNSegmentID has not been specified. Using default value: %s", Name(), readString.Buffer());
        }else{
	    receiverSDNSocket.setSegmentID((char *)readString.Buffer());
	}
    }

     if(!cdb.ReadInt32(cpuMask, "CpuMask", 0x2)){
          AssertErrorCondition(Warning,"StreamingDriverSDN::ObjectLoadSetup: %s CpuMask has not been specified. Using default value: 0x%x", Name(), cpuMask);
     }

    if(numberOfTransferBuffers > numberOfBuffers){
        AssertErrorCondition(InitialisationError, "StreamingDriverReceiver::ObjectLoadSetup: NumberOfTransferBuffers must no be higher than NumberOfBuffers: %d > %d", numberOfTransferBuffers, numberOfBuffers);
        return False;
    }

    if(memoryBuffer != NULL) free((void *&)memoryBuffer);
/*    if(synchronizing){
        //Discard the packet counter and usectime . This is not arriving as signal
        memBufferSize    = (NumberOfInputs() - 2) * sizeof(float) + sizeof(int32);
    }
    else{*/
        memBufferSize    = NumberOfInputs() * sizeof(float) + sizeof(int32);
//    }
    memoryBuffer     = (char *)malloc(memBufferSize * numberOfBuffers);
    if(memoryBuffer == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Failed allocating %d bytes for storing data",Name(), memBufferSize * numberOfBuffers);
        return False;
    }    
    memset(memoryBuffer, 0, memBufferSize * numberOfBuffers);
    AssertErrorCondition(Information,"StreamingDriverReceiver::ObjectLoadSetup: %s Successfully allocated %d bytes for data storing divided in %d buffers",Name(), memBufferSize * numberOfBuffers, numberOfBuffers);


    if(transferBuffer != NULL) free((void *&)transferBuffer);
    transferBuffer     = (char *)malloc(memBufferSize * numberOfTransferBuffers);
    if(transferBuffer  == NULL){
        CStaticAssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Could not allocate: %d bytes for the transfer buffer. The consumer thread will NOT start", Name(), memBufferSize * numberOfTransferBuffers);
    }
    memset(transferBuffer, 0, memBufferSize * numberOfTransferBuffers);
    AssertErrorCondition(Information,"StreamingDriverReceiver::ObjectLoadSetup: %s Successfully allocated %d bytes for data streaming divided in %d buffers",Name(), memBufferSize * numberOfTransferBuffers, numberOfTransferBuffers);

    if(bufferLockStates != NULL) free((void *&)bufferLockStates);
    bufferLockStates = (int32 *)malloc(numberOfBuffers * sizeof(int32));
    if(bufferLockStates == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Failed allocating %d bytes for the buffer locking semaphores" ,Name(), numberOfBuffers * sizeof(int32));
        return False;
    }
    memset(bufferLockStates, 0, numberOfBuffers * sizeof(int32));


    if(bufferFreeStates != NULL) free((void *&)bufferFreeStates);
    bufferFreeStates = (int32 *)malloc(numberOfBuffers * sizeof(int32));
    if(bufferFreeStates == NULL){
        AssertErrorCondition(InitialisationError,"StreamingDriverReceiver::ObjectLoadSetup: %s Failed allocating %d bytes for the buffer locking semaphores" ,Name(), numberOfBuffers * sizeof(int32));
        return False;
    }
    memset(bufferFreeStates, 0, numberOfBuffers * sizeof(int32));

    /** Begin the consumer thread*/
    lastUnlockedBufferIndex = -1;

    int segmentSize = memBufferSize * numberOfTransferBuffers + sizeof(double);
    receiverSDNSocket.setSegmentSize(segmentSize);

    if (inputType==0){
	receiverThreadID = Threads::BeginThread((ThreadFunctionType)StreamingDriverReceiverThread, this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);

    }else{
        //SDN publish init
        if(receiverSDNSocket.receiver_init()){
            AssertErrorCondition(Information,"StreamingDriver::receiver_init SDN Receiver Initialized: Version: %s", sdn_version());            
        }else{
	     AssertErrorCondition(InitialisationError, "StreamingDriver::receiver_init error: Version: %s", sdn_version());	        
	     return False;
        }
	receiverSDNThreadID = Threads::BeginThread((ThreadFunctionType)StreamingDriverSDNReceiverThread, this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);

    }    

    lastReadIdx = 0;
    
    return True;
}

StreamingDriverSDNReceiver::~StreamingDriverSDNReceiver(){
    char        unlock[1];
    receiverThreadIsAlive  = False;
    uint32      unlockSize = 1;
    UDPSocket   unlockSocket;
    unlockSocket.Open();
    unlockSocket.Connect("localhost", receiverUDPPort);
    unlockSocket.Write((const void *)unlock, unlockSize);
    receiverSocket.Close();
    unlockSocket.Close();

    synchEventSem.Post();
    int resetCounter = 0;
    while(!receiverThreadIsAlive){
        if(resetCounter > 10){
            break;
        }
        resetCounter++;
        SleepMsec(100);
    }

    receiverSDNSocket.close();
    if(!receiverThreadIsAlive){
        AssertErrorCondition(FatalError,"StreamingDriverReceiver::~StreamingDriverReceiver(): Consumer thread did not exited in 1 seconds %s", Name());
    }
    else{
        if(memoryBuffer     != NULL) free((void *&)memoryBuffer);
        if(bufferLockStates != NULL) free((void *&)bufferLockStates);
        if(bufferFreeStates != NULL) free((void *&)bufferFreeStates);
        if(transferBuffer   != NULL) free((void *&)transferBuffer);
    }
}

StreamingDriverSDNReceiver::StreamingDriverSDNReceiver(){

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
    lastReadIdx             = 0;

    lostBuffers             = 0;
    lastWarningCounter      = 0;
    
    lastUnlockedBufferIndex = -1;

    synchronizing           = False;
    synchronizingMSTimeout  = 0;

    receiverThreadIsAlive   = True;
    receiverThreadID        = 0;

    synchEventSem.Create();
}

OBJECTLOADREGISTER(StreamingDriverSDNReceiver, "$Id: StreamingDriverSDNReceiver.cpp,v 1.13 2011/02/17 17:33:41 aneto Exp $")

