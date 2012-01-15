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

#include "SharedMemoryDrv.h"
#include "GlobalObjectDataBase.h"

bool SharedMemoryDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    AssertErrorCondition(Information, "SharedMemoryDrv::ObjectLoadSetup: %s Loading signals", Name());

    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"SharedMemoryDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }

    cdb.ReadInt32(readMemKey,  "ReadMemKey", -1);
    cdb.ReadInt32(writeMemKey, "WriteMemKey", -1);

    if(readMemKey == -1 && writeMemKey == -1){
        AssertErrorCondition(InitialisationError,"SharedMemoryDrv::ObjectLoadSetup: either a read or a write key must be specified. Both are -1",Name());
        return False;
    }

    if(readMemKey != -1){
        readMemSize = NumberOfInputs() *  sizeof(int32);
        readMem     = (char *)SharedMemoryAlloc(readMemKey, readMemSize);
        if(readMem == NULL){
            AssertErrorCondition(InitialisationError,"SharedMemoryDrv::ObjectLoadSetup: failed to allocated %d bytes for the read memory",Name(),readMemSize);
            return False;
        }
    }

    if(writeMemKey != -1){
        writeMemSize = NumberOfOutputs() *  sizeof(int32);
        writeMem     = (char *)SharedMemoryAlloc(writeMemKey, writeMemSize);
        if(writeMem == NULL){
            AssertErrorCondition(InitialisationError,"SharedMemoryDrv::ObjectLoadSetup: failed to allocated %d bytes for the write memory",Name(),writeMemSize);
            return False;
        }
    }

    return True;
}

/**
 * GetData
 */
int32 SharedMemoryDrv::GetData(uint32 usecTime, int32 *ibuffer, int32 bufferNumber){
    lastUsecTime = *ibuffer;
    memcpy(ibuffer, readMem, readMemSize);
    return 1;
}

bool SharedMemoryDrv::WriteData(uint32 usecTime, const int32* buffer){
    memcpy(writeMem, buffer, writeMemSize);
    return True;
}

OBJECTLOADREGISTER(SharedMemoryDrv, "$Id: SharedMemoryDrv.cpp,v 1.1 2011/09/16 16:42:43 aneto Exp $")

