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

#if !defined (TRIGGEROBJ_H)
#define TRIGGEROBJ_H

#include "System.h"
#include "GCNamedObject.h"
#include "DDBInputInterface.h"
#include "GAM.h"


///
class Saturation{
    /** */
    bool    status;

    /** */
    float   upperSaturationLevel;

    /** */
    float   lowerSaturationLevel;

public:

    /** */
    Saturation(){ status = False; }

    /** */
    void SetUpperLevel(float value){
        this->upperSaturationLevel = value;
    }

    /** */
    void SetLowerLevel(float value){
        this->lowerSaturationLevel = value;
    }

    /** */
    float GetUpperLevel(){ return upperSaturationLevel; }

    /** */
    float GetLowerLevel(){ return lowerSaturationLevel; }

    /** */
    bool IsSaturated(){ return status; }

    /** */
    void Update(float value){
        if( value > upperSaturationLevel )      status = True;
        else if( value < lowerSaturationLevel ) status = True;
        else Reset();
    }

    /** */
    void Reset(){ status = False; }

};

class TriggerObj;


extern "C" {
    bool TriggerObjAddInterfaces(TriggerObj &tobj,GCRTemplate<GAM> gam);
}

class TriggerObj: public GCNamedObject{

protected:

    /** */
    FString                             signalTriggerName;

    /** Integer output flag */
    bool                                isIntTriggerSignal;

    /** */
    Saturation                          saturation;

    /** */
    Saturation                          saturationHysteresis;

    /** */
    bool                                isTriggered;

    /** */
    bool                                resetTrigger;

    /** */
    int32                               triggerTimeUsec;

    /** */
    int32                               delayResetTime;

/*******************************************/

    /** */
    GCRTemplate<GAM>                    gamLink;

    /** */
    DDBInputInterface                   *ddbInputInterface;

public:

    /** */
    TriggerObj();

    /** */
    ~TriggerObj();

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    void Update(int32 usecTime);

    /** */
    bool IsTriggered(){ return isTriggered; }

    /** */
    void Reset(){ isTriggered = False; }

    /** */
    friend bool TriggerObjAddInterfaces(TriggerObj &wfgc, GCRTemplate<GAM> gam);

    /** Adds those Interfaces that have been initialized */
    bool AddInterfaces(GCRTemplate<GAM> &gam){return TriggerObjAddInterfaces(*this,gam);}

};

#endif
