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

#include "InputModulesService.h"
#include "GlobalObjectDataBase.h"
#include "CDBExtended.h"

bool InputModulesService::Trigger(int64 usecTime){

    if(inputsModules == NULL)return False;
    for(int i = 0; i < numberOfInputModules; i++){
        if(!inputsModules[i]->StartSingleAcquisition()){
            AssertErrorCondition(FatalError,"InputModuleService::Trigger: %s: StartSingleAcquisition Failed",Name());
            return False;
        }
    }
    return True;
}

/** */
bool InputModulesService::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    if(!TimeServiceActivity::ObjectLoadSetup(cdb,err)){
        AssertErrorCondition(FatalError,"InputModuleService::ObjectLoadSetup: %s: TimeServiceActivity::ObjectLoadSetup Failed",Name());
        return False;
    }

    if(!cdb->Exists("InputModules")){
        AssertErrorCondition(FatalError,"InputModuleService::ObjectLoadSetup: %s: Entry InputModules has not been specified",Name());
        return False;
    }

    int size[1] = {0};
    int maxDims = 1;

    if(!cdb->GetArrayDims(size,maxDims,"InputModules")){
        AssertErrorCondition(FatalError,"InputModuleService::ObjectLoadSetup: %s: GetArrayDims Failed for entry InputModules",Name());
        return False;
    }

    numberOfInputModules = size[0];

    FString *modulesNames = new FString[numberOfInputModules];
    if(modulesNames == NULL){
        AssertErrorCondition(FatalError,"InputModuleService::ObjectLoadSetup: %s: Failed allocating space for %d FString",Name(),numberOfInputModules);
        return False;
    }


    if(!cdb.ReadFStringArray(modulesNames, size,maxDims,"InputModules")){
        AssertErrorCondition(FatalError,"InputModuleService::ObjectLoadSetup: %s: ReadFStringArray Failed for entry InputModules",Name());
        delete[] modulesNames;
        return False;
    }

    inputsModules = new GCRTemplate<GenericAcqModule>[numberOfInputModules];
    if(inputsModules == NULL){
        AssertErrorCondition(FatalError,"InputModuleService::ObjectLoadSetup: %s: Failed allocating space for %d GCRTemplate<GenericAcqModule>",Name(),numberOfInputModules);
        delete[] modulesNames;
        return False;
    }


    for(int i = 0;i < numberOfInputModules;i++){
        GCReference module = GetGlobalObjectDataBase()->Find(modulesNames[i].Buffer());
        if(!module.IsValid()){
            AssertErrorCondition(InitialisationError,"InputModuleService::ObjectLoadSetup: %s: Failed retrieving reference for %s in GlobalContainer", Name(),modulesNames[i].Buffer());
            delete[] modulesNames;
            return False;
        }

        inputsModules[i] = module;
        if(!inputsModules[i].IsValid()){
            AssertErrorCondition(InitialisationError,"InputModuleService::ObjectLoadSetup: %s: The object %s is not of GenericAcqModule Type", Name(),modulesNames[i].Buffer());
            delete[] modulesNames;
            return False;
        }
    }

    delete[] modulesNames;
    return True;
}

OBJECTLOADREGISTER(InputModulesService,"$Id: InputModulesService.cpp,v 1.3 2011/05/20 15:02:15 aneto Exp $")
