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
 * $Id: FileWriterDrv.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

#if !defined (FILE_WRITE_DRV)
#define FILE_WRITE_DRV

#include "System.h"
#include "GenericAcqModule.h"
#include "File.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
#include "MessageHandler.h"

OBJECT_DLL(FileWriterDrv)
class FileWriterDrv:public GenericAcqModule, public MessageHandler{
OBJECT_DLL_STUFF(FileWriterDrv)

private:
    /**
     * Files where the data is stored
     */
    File *outputFiles;

    /**
     * Name of the files
     */
    FString *outputFilenames;
    
    /**
     * Number of files
     */
    int32 numberOfOutputFiles;

    /**
     * The shared buffer. The WriteData puts data here
     * and a lower priority thread consumes from this buffer and stores in the disk
     * The shared buffer is composed by N numberOfBuffers. The first element of each buffer
     * can be either 0 or 1. When 0 the buffer has no information to be consumed. When 1 the lower priority
     * thread should consume this information and put it to 0. The remained of the buffer contains 
     * numberOfOutputFiles * numberOfBytesPerSignal bytes, so that numberOfBytesPerSignal will be 
     * synched to each output file
     */
    int32 **sharedBuffer;
    
    /**
     * Number of buffers in the shared buffer
     */
    int32 numberOfBuffers;

    /**
     * Number of bytes per signal (or outputFile)
     */
    int32 numberOfBytesPerSignal;

    /**
     * The same as numberOfBytesPerSignal but in words (int32)
     */
    int32 numberOfWordsPerSignal;

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
     * The writer thread is stopped by the message
     * This has to be called by the state machine to properly close all the files...
     */
    #define MSG_STOP "STOP"

public:
    FileWriterDrv(){
        outputFiles = NULL;
        outputFilenames = NULL;
        sharedBuffer = NULL;
        sharedBufferMux.Create();
        sharedBufferSem.Create();
        numberOfOutputFiles = 0;
        numberOfBuffers = 0;
        numberOfBytesPerSignal = 0;
        numberOfWordsPerSignal = 0;
        running = False;
        lastWriteIdx = 0;
        cpuMask = 1;
        threadID = 0;
        threadPriority = 0;
    }

    virtual ~FileWriterDrv(){
        if(running){
            AssertErrorCondition(FatalError, "%s: Trying to destroy the FileWriterDrv while running. Memory leaking as opposed to crashing...", Name());
            return;
        }
        sharedBufferMux.FastUnLock();
        sharedBufferMux.Close();
        sharedBufferSem.Post();
        sharedBufferSem.Close();
        if(outputFiles != NULL){
            delete []outputFiles;
        }
        if(outputFilenames != NULL){
            delete []outputFilenames;
        }
        int32 i=0;
        for(i=0; i<numberOfBuffers; i++){
            delete []sharedBuffer[i];
        }
    }
    
    /**
     * Thread callback function to write the data to disk
     */
    void WriteDataToDisk();

    /**
     * Reset the internal counters 
     */
    bool PulseStart(){
        return True;
    }

    /**
     * Required to receive the STOP message which will synch data to the disk
     */
    bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

    /**
     * Stops the thread that writes data to disk
     */
    bool StopWriteDataToDiskThread();

    /**
     * Display the status of the shared buffer in real-time
     */
    bool ProcessHttpMessage(HttpStream &hStream);

    /** 
     * NOOP
     */
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0){
        AssertErrorCondition(FatalError, "%s: GetData not supported", Name());
        return -1;
    }

    /**
     * Load and configure object parameters
     * @param info the configuration database
     * @param err the error stream
     * @return True if no errors are found during object configuration
     */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

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
     * Not supported
     */
    bool WriteData(uint32 usecTime, const int32* buffer);
};

#endif
