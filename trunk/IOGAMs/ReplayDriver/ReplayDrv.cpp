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
 * $Id: ReplayDrv.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/

#include "ReplayDrv.h"
#include "GlobalObjectDataBase.h"

bool ReplayDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    AssertErrorCondition(Information, "ReplayDrv::ObjectLoadSetup: %s Loading driver ", Name());

    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"ReplayDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }

    FString filename;
    if(!cdb.ReadFString(filename, "Filename")){
        AssertErrorCondition(InitialisationError,"ReplayDrv::ObjectLoadSetup: %s Filename has to be specified", Name());
        return False;
    }

    dataFile.SetOpeningModes(openCreate | accessModeRW);
    if(!dataFile.Open(filename.Buffer())){
        AssertErrorCondition(InitialisationError,"ReplayDrv::ObjectLoadSetup: %s Filename has to be specified", Name());
        return False;
    }
    dataFile.Seek(0);
    if(numberOfInputChannels > 0){
        bufferSize = numberOfInputChannels * sizeof(int32);
    }
    if(numberOfOutputChannels > 0){
        bufferSize = numberOfOutputChannels * sizeof(int32);
    }
    return True;
}

/**
 * GetData
 */
int32 ReplayDrv::GetData(uint32 usecTime, int32 *ibuffer, int32 bufferNumber){
    int32  readTime = 0;
    uint32 size     = sizeof(int32);
    if(!dataFile.Read(&readTime, size)){
        return -1;
    }
    if(readTime > usecTime){
        size     = sizeof(int32);
        dataFile.Seek(dataFile.Position() - sizeof(int32));
        return 0;
    }

    size = bufferSize;
    if(!dataFile.Read(ibuffer, size)){
        return -1;
    }
    while(readTime < usecTime){
        size     = sizeof(int32);
        if(!dataFile.Read(&readTime, size)){
            return -1;
        }
        size = bufferSize;
        if(!dataFile.Read(ibuffer, size)){
            return -1;
        }
    }
    return 1;
}

bool ReplayDrv::WriteData(uint32 usecTime, const int32* buffer){
    //This is needed to discard transients while MARTe is changing state
    if(lastUsecTime > usecTime){
        dataFile.Seek(0);
    }
    uint32 size  = sizeof(int32);
    dataFile.Write(&usecTime, size);
    size         = bufferSize;
    dataFile.Write(buffer, size);
    lastUsecTime = usecTime;
    return True;
}

OBJECTLOADREGISTER(ReplayDrv, "$Id: ReplayDrv.cpp 3 2012-01-15 16:26:07Z aneto $")

