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

#include "TriggerObj.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"


///
bool TriggerObjAddInterfaces(TriggerObj &tobj, GCRTemplate<GAM> gam) {

  if( tobj.ddbInputInterface  != NULL ) {
    if( !gam->AddInputInterface(tobj.ddbInputInterface,NULL) ){
      tobj.AssertErrorCondition(InitialisationError,"TriggerObj::AddInterfaces: AddInputInterface Failed for module %s", tobj.Name());
      return False;
    }
  }else{
    tobj.AssertErrorCondition(Information,"TriggerObj::AddInterfaces: Input interface already added %s", tobj.Name());
  }

  return True;
}


///
TriggerObj::TriggerObj(){
    isIntTriggerSignal = False;
    isTriggered        = False;
    resetTrigger       = False;
    ddbInputInterface  = NULL;
    triggerTimeUsec    = 0;
    delayResetTime     = 0;
    saturation.Reset();
    saturationHysteresis.Reset();
}

///
TriggerObj::~TriggerObj(){
}

///
void TriggerObj::Update(int32 usecTime){

    ddbInputInterface->Read();

    float inputTrigger;

    if( isIntTriggerSignal ){
        inputTrigger = (float)(*((int32*)ddbInputInterface->Buffer()));
    }else{
        inputTrigger = *( (float*)ddbInputInterface->Buffer() );
    }

    saturation.Update(inputTrigger);
    saturationHysteresis.Update(inputTrigger);

    if( !resetTrigger && isTriggered ) return;

    bool newStateTrigger = saturation.IsSaturated() || (saturationHysteresis.IsSaturated() &&  isTriggered);

    if( isTriggered ){
        if( delayResetTime > 0 ){
            if( delayResetTime <= (usecTime-triggerTimeUsec) ){
                saturationHysteresis.Reset();
                saturation.Reset();
                Reset();
            }
        }else{
            isTriggered = newStateTrigger;
        }
    }else{
        if( newStateTrigger ){
            isTriggered = True;
            triggerTimeUsec = usecTime;
        }
    }
}


///
bool TriggerObj::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    AssertErrorCondition(FatalError,"TriggerObj::Initialize Chiamata");

    CDBExtended cdb(cdbData);

    if( !cdb.ReadFString(signalTriggerName,"TriggerSignalName") ){
        AssertErrorCondition(InitialisationError,"TriggerObj::Initialize TriggerSignalName not found");
        return False;
    }

    FString signalTriggerType;
    if( !cdb.ReadFString(signalTriggerType,"TriggerSignalType") ){
        AssertErrorCondition(InitialisationError,"TriggerObj::Initialize TriggerSignalType not found");
        return False;
    }

    // set Thresholds
    float triggerLevel;
    if( !cdb.ReadFloat(triggerLevel,"UpperTriggerLevel",1.0e10) ){
        AssertErrorCondition(Warning,"TriggerObj::Initialize UpperTriggerLevel not found. Set to 1.0e10");
    }
    saturation.SetUpperLevel(triggerLevel);

    if( !cdb.ReadFloat(triggerLevel,"LowerTriggerLevel",-1.0e10) ){
        AssertErrorCondition(Warning,"TriggerObj::Initialize LowerTriggerLevel not found. Set to -1.0e10");
        return False;
    }
    saturation.SetLowerLevel(triggerLevel);


    float hystDelta;
    cdb.ReadFloat(hystDelta,"Hysteresis",0.0);
    if( hystDelta < 0.0 ){
        AssertErrorCondition(InitialisationError,"TriggerObj::Initialize Hysteresis >= 0.0");
        return False;
    }
    AssertErrorCondition(Information,"TriggerObj::Initialize Hysteresis set to %f",hystDelta);

    saturationHysteresis.SetUpperLevel(saturation.GetUpperLevel()-hystDelta);
    saturationHysteresis.SetLowerLevel(saturation.GetLowerLevel()+hystDelta);

    float delayResetTimeSec;
    cdb.ReadFloat(delayResetTimeSec,"DelayReset",0.0);
    delayResetTime = (int32)(delayResetTimeSec*1000000);

    FString gamLinkName;
    if( !cdb.ReadFString(gamLinkName,"GAMlink") ){
        AssertErrorCondition(InitialisationError,"TriggerObj::Initialize GAMlink not found");
        return False;
    }

    GCReference gam = GetGlobalObjectDataBase()->Find(gamLinkName.Buffer(),GCFT_Recurse);
    if( !gam.IsValid() ){
        AssertErrorCondition(InitialisationError, "TriggerObj::ObjectLoadSetup: %s: Could not find module in Object Containers", Name());
        return False;
    }
    gamLink = gam;
    if( !gamLink.IsValid() ){
        AssertErrorCondition(InitialisationError, "TriggerObj::ObjectLoadSetup: %s: is not GAM", Name());
        return False;
    }

    FString dummy;
    dummy = Name();
    dummy += "InputInterface";

    ddbInputInterface = new DDBInputInterface(Name(),dummy.Buffer(),DDB_ReadMode);

    if( ddbInputInterface == NULL ){
        AssertErrorCondition(InitialisationError,"TriggerObj::Init error allocating memory for the DDB interface gain");
        return False;
    }

    if( !ddbInputInterface->AddSignal(signalTriggerName.Buffer(),signalTriggerType.Buffer())  ){
        AssertErrorCondition(InitialisationError,"TriggerObj::Init error loading %s signal from DynamicDataBuffer",Name());
        return False;
    }

    if( !AddInterfaces(gamLink) ){
        AssertErrorCondition(InitialisationError, "TriggerObj::ObjectLoadSetup: %s: error adding interface", Name());
        return False;
    }

    if( strcmp(signalTriggerType.Buffer(),"float") == 0 ) isIntTriggerSignal = False;
    else                                                  isIntTriggerSignal = True;

    resetTrigger = True;
    FString resetFlag;
    cdb.ReadFString(resetFlag,"ResetTrigger","ON");
    if( strcmp(resetFlag.Buffer(),"OFF") == 0 ) resetTrigger = False;

    return True;
}

OBJECTLOADREGISTER(TriggerObj,"$Id: TriggerObj.cpp,v 1.2 2009/10/28 16:10:56 lzabeo Exp $")


