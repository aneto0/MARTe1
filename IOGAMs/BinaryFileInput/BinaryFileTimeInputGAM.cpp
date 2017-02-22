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
 * $Id: BinaryFileTimeInputGAM.cpp 462 2016-04-22 12:38:30Z aneto $
 *
**/


#include "BinaryFileTimeInputGAM.h"
#include "GlobalObjectDataBase.h"

#include "CDBExtended.h"
#include "HRT.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "InterruptDrivenTTS.h"

BinaryFileTimeInputGAM::~BinaryFileTimeInputGAM(){}

bool BinaryFileTimeInputGAM::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);

    ///////////////////////////
    // Sinchronizing Service //
    ///////////////////////////

    FString ttsName;
    if(!cdb.ReadFString(ttsName,"TriggeringServiceName")){
        AssertErrorCondition(InitialisationError, "BinaryFileTimeInputGAM::Initialise: %s does not specify a TriggeringServiceName", Name());
        return False;
    }

    GCReference ttsModule = GetGlobalObjectDataBase()->Find(ttsName.Buffer(),GCFT_Recurse);
    if(!ttsModule.IsValid()){
        AssertErrorCondition(InitialisationError, "BinaryFileTimeInputGAM::Initialise: %s: Could not find module %s in Object Containers", Name(),ttsName.Buffer());
        return False;
    }
    
    trigger = ttsModule;
    if(!trigger.IsValid()){
        AssertErrorCondition(InitialisationError, "BinaryFileTimeInputGAM::Initialise: %s: %s is not of TimeTriggeringServiceInterface Type", Name(),ttsName.Buffer());
        return False;
    }

    if(!BinaryFileTimeInputGAM::Initialise(cdb)){
        AssertErrorCondition(InitialisationError, "BinaryFileTimeInputGAM::Initialise: %s: InputGAM::Initialize failed", Name());
        return False;
    }
    
    AssertErrorCondition(Warning,"BinaryFileTimeInputGAM::Initialise: %s: Module %s Has been Successfully Loaded", Name(),inputModule->Name());

    return True;
}

bool BinaryFileTimeInputGAM::Execute(GAM_FunctionNumbers functionNumber){

    //////////////////////////////////////////
    // Synchronise with the real time cycle //
    //////////////////////////////////////////
    
    if(!trigger->Synchronise()){
        AssertErrorCondition(FatalError,"BinaryFileTimeInputGAM::Execute: Timeout on Execute");
    }
    return BinaryFileTimeInputGAM::Execute(functionNumber);
}

bool BinaryFileTimeInputGAM::ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){

    // Dump Interface to CDB
    CDBExtended cdb(info);

    FString moduleName;
    moduleName = Name();

    cdb.WriteString(ClassName(),"Class");
    BinaryFileTimeInputGAM::ObjectSaveSetup(cdb,err);
    
    return True;
}


OBJECTLOADREGISTER(BinaryFileTimeInputGAM,"$Id: BinaryFileTimeInputGAM.cpp 462 2016-04-22 12:38:30Z aneto $")
