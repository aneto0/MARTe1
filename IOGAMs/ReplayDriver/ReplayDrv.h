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
 * $Id: ReplayDrv.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

#if !defined (REPLAY_DRV)
#define REPLAY_DRV

/**
 * @file
 * A driver which can be used to replay MARTe runs.
 * When connected to an OutputGAM it will sink the data to a file
 * which can be used then to feed an InputGAM in another MARTe instance
 * This is particular useful to replay experiments or to simulate offline
 * data acquired with any physical apparatus
 * The first 4 bytes store the time in microseconds
 */

#include "System.h"
#include "GenericAcqModule.h"
#include "File.h"

OBJECT_DLL(ReplayDrv)
class ReplayDrv:public GenericAcqModule{
OBJECT_DLL_STUFF(ReplayDrv)

private:
    /**
     * The file with the data
     */
    File dataFile;

    /**
     * Stores the number of bytes to read/write
     */
    uint32 bufferSize;

    /**
     * The last read usec time
     */
    uint32 lastUsecTime;
public:
    ReplayDrv(){
        bufferSize   = 0;
        lastUsecTime = 0;
    }

    virtual ~ReplayDrv(){
        dataFile.Close(); 
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
