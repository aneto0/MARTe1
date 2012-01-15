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

#if !defined (SHARED_MEMORY_DRV)
#define SHARED_MEMORY_DRV

/**
 * @file
 * A driver which reads/writes values to a shared memory area.
 * If used as a time input source, it assumes that the first
 * 4 bytes are the time in microseconds
 */

#include "System.h"
#include "GenericAcqModule.h"

OBJECT_DLL(SharedMemoryDrv)
class SharedMemoryDrv:public GenericAcqModule{
OBJECT_DLL_STUFF(SharedMemoryDrv)

private:
    /**
     * lastUsecTime;
     */
    int32 lastUsecTime;

    /**
     * The shared memory area where data is read from
     */
    char *readMem;

    /**
     * The shared memory area where data is written to
     */
    char *writeMem;

    /**
     * Shared memory key identifications (uid of the shared memory)
     * -1 means don't use
     */
    int32 readMemKey;
    int32 writeMemKey;

    /**
     * Memory size in bytes
     */
    int32 readMemSize;
    int32 writeMemSize;

public:
    SharedMemoryDrv(){
        readMem      = NULL;
        writeMem     = NULL;
        readMemKey   = -1;
        writeMemKey  = -1;
        readMemSize  = 0;
        writeMemSize = 0;
        lastUsecTime = 0;
    }

    virtual ~SharedMemoryDrv(){
        if(readMem != NULL){
            SharedMemoryFree(readMem);
        }
        if(writeMem != NULL){
            SharedMemoryFree(writeMem);
        }
    }

    /**
     * Reset the internal counters 
     */
    bool PulseStart(){
        lastUsecTime = 0;
        return True;
    }


    /** 
     * Gets Data from the shared memory to the caller IOGAM
     * @param usecTime Microseconds Time
     * @return -1 on Error, 1 on success
     */
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);

    /**
     * Writes data from the IOGAM to the shared memory
     * @param usecTime the time in microseconds
     * @param buffer the buffer of data to write
     * @return True if successful
     */
    bool WriteData(uint32 usecTime, const int32* buffer);

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
};

#endif
