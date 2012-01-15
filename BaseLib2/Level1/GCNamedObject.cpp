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

#include "GCNamedObject.h"
#include "ConfigurationDataBase.h"

OBJECTREGISTER(GCNamedObject,"$Id$")

bool GCNOObjectLoadSetup(
                    GCNamedObject &                 gcno,
                    ConfigurationDataBase &         info,
                    StreamInterface *               err){
    BString objectName;
    int size[1] = { 1 };

    int nDim = 1;
    if (info->ReadArray(&objectName,CDBTYPE_BString,size,nDim,"Name")){
        gcno.SetObjectName(objectName.Buffer());
    } else
    if (info->NodeName(objectName)){
        if (objectName[0] == '+'){
            gcno.SetObjectName(objectName.Buffer() + 1);
        } else {
            gcno.SetObjectName(objectName.Buffer());
        }
    } else {
        gcno.AssertErrorCondition(FatalError,"ObjectLoadSetup: cannot set name of object\n");
        return False;
    }

    BString classVersion;
    nDim = 1;
    if (info->ReadArray(&classVersion,CDBTYPE_BString,size,nDim,"ClassVersion")){
        if (!(classVersion == gcno.Version())){
            gcno.AssertErrorCondition(Warning,"Version mismatch! initialising object of version %s with data from object of version %s",gcno.Version(),classVersion.Buffer());
        }
    }

    return True;
}

bool GCNOObjectSaveSetup(
                    GCNamedObject &                 gcno,
                    ConfigurationDataBase &         info,
                    StreamInterface *               err){
    int size[1] = { 1 };
    int nDim = 1;
    info->WriteArray(&gcno.name,CDBTYPE_String,size,nDim,"Name");
    const char *v = gcno.Version();
    info->WriteArray(&v,CDBTYPE_String,size,nDim,"ClassVersion");

    return True;
}

