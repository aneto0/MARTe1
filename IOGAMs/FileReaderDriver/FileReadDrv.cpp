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

#include "FileReadDrv.h"
#include "GlobalObjectDataBase.h"

bool FileReadDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    AssertErrorCondition(Information, "FileReadDrv::ObjectLoadSetup: %s Loading signals", Name());

    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"FileReadDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }

    numberOfSignalLists = Size();
    if(numberOfSignalLists < 1){
        AssertErrorCondition(InitialisationError,"FileReadDrv::ObjectLoadSetup: %s at least 1 FileSignalList must be specified.",Name());
        return False;
    }

    signalLists = new FileSignalList*[numberOfSignalLists];
    if(signalLists == NULL){
        AssertErrorCondition(InitialisationError,"FileReadDrv::ObjectLoadSetup: %s failed to allocate %d pointer for FileSignalList.",Name(), numberOfSignalLists);
        return False;
    }

    int32 i=0;
    for(i=0; i<numberOfSignalLists; i++){
        signalLists[i] = (FileSignalList *)Find(i).operator->();
    }

    return True;
}

/**
 * GetData
 */
int32 FileReadDrv::GetData(uint32 usecTime, int32 *ibuffer, int32 bufferNumber){
    int32 i      = 0;
    char *buffer = (char *)ibuffer;
    for(i=0; i<numberOfSignalLists; i++){
        void *samples = signalLists[i]->GetNextSample(usecTime);
        if(samples != NULL){
            memcpy(buffer, samples, signalLists[i]->signalType.ByteSize() * signalLists[i]->numberOfSignals);
            buffer += signalLists[i]->signalType.ByteSize() * signalLists[i]->numberOfSignals;
        }
    }
    return 1;
}

OBJECTLOADREGISTER(FileReadDrv,"$Id$")

