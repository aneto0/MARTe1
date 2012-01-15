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


#include "TimeFilteredInputGAM.h"
#include "GlobalObjectDataBase.h"
#include "CDBExtended.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"


bool TimeFilteredInputGAM::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);

    ///////////////////////////
    // Sinchronizing Service //
    ///////////////////////////

    FString ttsName;
    if(!cdb.ReadFString(ttsName,"TriggeringServiceName")){
        AssertErrorCondition(InitialisationError, "TimeFilteredInputGAM::Initialise: %s does not specify a TriggeringServiceName", Name());
        return False;
    }

    GCReference ttsModule = GetGlobalObjectDataBase()->Find(ttsName.Buffer(),GCFT_Recurse);
    if(!ttsModule.IsValid()){
        AssertErrorCondition(InitialisationError, "TimeFilteredInputGAM::Initialise: %s: Could not find module %s in Object Containers", Name(),ttsName.Buffer());
        return False;
    }
    
    trigger = ttsModule;
    if(!trigger.IsValid()){
        AssertErrorCondition(InitialisationError, "TimeFilteredInputGAM::Initialise: %s: %s is not of TimeTriggeringServiceInterface Type", Name(),ttsName.Buffer());
        return False;
    }

    if(!FilteredInputGAM::Initialise(cdb)){
        AssertErrorCondition(InitialisationError, "TimeFilteredInputGAM::Initialise: %s: FilteredInputGAM::Initialize failed", Name());
        return False;
    }
    
    AssertErrorCondition(Warning,"TimeFilteredInputGAM::Initialise: %s: Module %s Has been Successfully Loaded", Name(),inputModule->Name());

    return True;
}

bool TimeFilteredInputGAM::Execute(GAM_FunctionNumbers functionNumber){

    //////////////////////////////////////////
    // Synchronise with the real time cycle //
    //////////////////////////////////////////
    
    if(!trigger->Synchronise()){
        AssertErrorCondition(FatalError,"TimeFilteredInputGAM::Execute: Timeout on Execute");
    }
    
    return FilteredInputGAM::Execute(functionNumber);
}

bool TimeFilteredInputGAM::ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){

    // Dump Interface to CDB
    CDBExtended cdb(info);

    FString moduleName;
    moduleName = Name();

    cdb.WriteString(ClassName(),"Class");
    FilteredInputGAM::ObjectSaveSetup(cdb,err);
    
    return True;
}


OBJECTLOADREGISTER(TimeFilteredInputGAM,"$Id: TimeFilteredInputGAM.cpp,v 1.3 2008/12/09 16:20:53 fpiccolo Exp $")
