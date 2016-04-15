/**
 * @file MDSWriterNode.h
 * @brief Header file for class MDSWriterNode
 * @date 06/02/2016
 * @author LLorenc Capella 
 * @author Andre' Neto
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.
 */

#ifndef MDS_WRITER_DRV
#define MDS_WRITER_DRV

#include "System.h"
#include "GenericAcqModule.h"
#include "File.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
#include "MessageHandler.h"
#include "mdsobjects.h"
#include "MDSWriterNode.h"

OBJECT_DLL(MDSWriterDrv)
class MDSWriterDrv:public GenericAcqModule, public MessageHandler{
OBJECT_DLL_STUFF(MDSWriterDrv)

private:

    /**
     * Refresh the MDS tree every refreshTime seconds
     */
	int32 refreshTime;
	
    /**
     * Name of the event to trigger at the refreshTime period
     */
	FString eventName;	

    /**
     * Name of the MDSplus tree 
     */
	FString treeName;

    /**
     * MDSplus tree
     */	
	MDSplus::Tree *tree;

    /**
     * Pulse number in the MDSplus tree
     */
	int32 pulseNumber;

    /**
     * Number of MDSplus signals to store
     */	
	int numberOfOutputNodes;

    /**
     * Each node represents a signal to store in MDSplus
     */
    MDSWriterNode **nodes;

    /**
     * The shared buffer. The WriteData puts data here
     * and a lower priority thread consumes from this buffer and stores it in MDSplus
     * The shared buffer is composed by N numberOfBuffers. The first element of each buffer
     * can be either 0 or 1. When 0 the buffer has no information to be consumed. When 1 the lower priority
         * thread should consume this information and put it to 0. 
     */
    int32 **sharedBuffer;
    
    /**
     * Number of buffers in the shared buffer
     */
    int32 numberOfBuffers;

    /**
     * Semaphore which is posted everytime WriteData is called to warn that new data should be available
     */
    EventSem sharedBufferSem;

    /**
     * Protects the access to the first element of the buffer (which informs if there is data to be consumed or not)
     */
    FastPollingMutexSem sharedBufferMux;

    /**
     * True while the thread that writes to the disk is running
     */
    bool running;

    /**
     * Index incremented by WriteData
     */
    int32 lastWriteIdx;

    /** 
     * CPU mask of the consumer thread  (i.e. the one that sinks the data into the disk)
     */
    int32 cpuMask;

    /** 
     * The id of the consumer thread 
     */ 
    TID threadID;

    /** 
     * Receiver thread priority within real time class
     */
    int32 threadPriority;

    /**
     * Current number of free buffers
     */
    int32 numberOfFreeBuffers;
        
    /**
     * The total number of words used by each entry of the shared buffer
     */
    uint32 sharedBufferSizeWords;

public:
    MDSWriterDrv(){
		eventName = "";
		refreshTime = 0;
		treeName = "";
		tree = NULL;
        sharedBuffer = NULL;
        nodes = NULL;
        sharedBufferMux.Create();
        sharedBufferSem.Create();
        numberOfBuffers = 0;
        running = False;
        lastWriteIdx = 0;
        cpuMask = 1;
        threadID = 0;
        threadPriority = 0;
        numberOfFreeBuffers = 0;
        sharedBufferSizeWords = 0;
    }

    virtual ~MDSWriterDrv(){
        if(running){
            AssertErrorCondition(FatalError, "%s: Trying to destroy the MDSWriterDrv while running. Memory leaking as opposed to crashing...", Name());
            return;
        }
        sharedBufferMux.FastUnLock();
        sharedBufferMux.Close();
        sharedBufferSem.Post();
        sharedBufferSem.Close();
        int32 i=0;
        if(nodes != NULL){
			delete []nodes;
        }
        if(sharedBuffer != NULL){
            for(i=0; i<numberOfBuffers; i++){
                if(sharedBuffer[i] != NULL){
                    delete []sharedBuffer[i];
                }
            }
            delete sharedBuffer;
        }
        if(tree != NULL){
            delete tree;
        }
    }
    
    /**
     * Thread callback function to write the data to MDS
     */
    void WriteDataToMDS();

    /**
     * Reset the internal counters 
     */
    bool PulseStart(){
        return True;
    }

    /**
     * Stops the thread that writes data to disk
     */
    bool StopWriteDataToMDSThread();

    /**
     * Display the status of the shared buffer in real-time
     */
    bool ProcessHttpMessage(HttpStream &hStream);

    /** 
     * Not implemented.
     */
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);

    /**
     * Load and configure object parameters
     * @param info the configuration database
     * @param err the error stream
     * @return True if no errors are found during object configuration
     */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /**
     * TODO
     */
    bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

    /**
     * NOOP
     */
    bool ObjectDescription(StreamInterface &s,bool full,StreamInterface *er){
        return True;
    }

    /**
     * NOOP
     */
    bool SetInputBoardInUse(bool on){
        return True;
    }

    /**
     * NOOP
     */
    bool SetOutputBoardInUse(bool on){
        return True;
    }

    /**
     * Writes the buffer to the MDSWriterNodes
     */
    bool WriteData(uint32 usecTime, const int32* buffer);

    
};

#endif
