/**
 * @file CircularBufferSynchDrv.h
 * @brief Header file for class CircularBufferSynchDrv
 * @date 21/04/2016
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

#ifndef CIRCULAR_BUFFER_SYNCH_DRV
#define CIRCULAR_BUFFER_SYNCH_DRV

#include "System.h"
#include "GenericAcqModule.h"
#include "File.h"
#include "EventSem.h"
#include "FastPollingMutexSem.h"
#include "MessageHandler.h"

OBJECT_DLL(CircularBufferSynchDrv)
class CircularBufferSynchDrv:public GenericAcqModule{
OBJECT_DLL_STUFF(CircularBufferSynchDrv)

private:
    /**
     * The number of buffers of the circular buffer
     */
	int32 numberOfBuffers;
    int32 currentBuffer;
    int32 currentReadBuffer;
    EventSem synchSem;

    /**
     * The shared buffer. The WriteData puts data here and the GetData reads data from here
     */
    int32 **sharedBuffer;
   
    /**
     * Current number of free buffers
     */
    int32 numberOfFreeBuffers;
        
public:
    CircularBufferSynchDrv(){
        numberOfBuffers = 0;
        currentBuffer = 0;
        currentReadBuffer = 0;
        sharedBuffer = NULL;
        synchSem.Create();
        synchSem.Reset();
    }

    virtual ~CircularBufferSynchDrv(){
        uint32 i = 0;
        if(sharedBuffer != NULL){
            for(i=0; i<numberOfBuffers; i++){
                if(sharedBuffer[i] != NULL){
                    delete []sharedBuffer[i];
                }
            }
            delete sharedBuffer;
        }
        synchSem.Close();
    }
    
    /**
     * Reset the internal counters 
     */
    bool PulseStart(){
        return True;
    }

    /**
     * Display the status of the shared buffer in real-time
     */
    bool ProcessHttpMessage(HttpStream &hStream);

    /** 
     * Gets the data at the sharedBuffer[currentBuffer] location 
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
     * Writes to the circular buffer and trigger
     */
    bool WriteData(uint32 usecTime, const int32* buffer);

    /**
     * Returns as soon as there is new data available in the shared buffer
     */
    virtual bool Poll();
    
};

#endif
