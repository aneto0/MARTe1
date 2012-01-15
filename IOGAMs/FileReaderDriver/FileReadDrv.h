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

#if !defined (FILE_READ_DRV)
#define FILE_READ_DRV

#include "System.h"
#include "GenericAcqModule.h"
#include "FileSignalList.h"

OBJECT_DLL(FileReadDrv)
class FileReadDrv:public GenericAcqModule{
OBJECT_DLL_STUFF(FileReadDrv)

private:
    /**
     * The signal list. Each list has a time vector associated to N signal vectors
     */
    FileSignalList **signalLists;
    /**
     * Number of signals for the list N
     */
    int32 numberOfSignalLists;

public:
    FileReadDrv(){
        signalLists         = NULL;
        numberOfSignalLists = 0;
    }

    virtual ~FileReadDrv(){
        if(signalLists != NULL){
            delete []signalLists;
        }
    }

    /**
     * Reset the internal counters 
     */
    bool PulseStart(){
        int i=0;
        for(i=0; i<numberOfSignalLists; i++){
            signalLists[i]->Reset();
        }
        return True;
    }


    /** 
     * Gets Data From the Module to the DDB
     * @param usecTime Microseconds Time
     * @return -1 on Error, 1 on success
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
     * Not supported
     */
    bool WriteData(uint32 usecTime, const int32* buffer){
        AssertErrorCondition(FatalError, "%s: WriteData not supported", Name());
        return False;
    }
};

#endif
