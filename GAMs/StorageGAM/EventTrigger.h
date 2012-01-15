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
#include "GCReferenceContainer.h"
#include "CDBExtended.h"
#include "OperationCondition.h"
#include "AcquisitionTrigger.h"

#if !defined(_EVENT_TRIGGER_)
#define _EVENT_TRIGGER_

OBJECT_DLL(EventTrigger)
class EventTrigger : public GCReferenceContainer {
OBJECT_DLL_STUFF(EventTrigger)

private:

    /// Cycle time in micro seconds
    uint32                              usecPeriod;

    /// Start time in seconds
    float                               startSec;

    /// Start time in micro seconds
    uint32                              startUsec;

    /// End time in seconds
    float                               endSec;

    /// End time in micro seconds
    uint32                              endUsec;

    /// Maximum number of cycles
    uint32                              numberOfRTCycles;

    /// Condition object
    GCRTemplate<OperationCondition>     condition;

    /// Flag for condition object existence
    bool                                hasCondition;

    /// Array of acq trigger objects
    GCRTemplate<AcquisitionTrigger>    *acqTrigger;

    /// Total number of acq trigger objects
    uint32                              numberOfAcqTriggers;

    /// Used for fixed periodic events that do not require operation condition
    uint32                              periodicEventTriggerEveryNCycles;

    /// Counter used in the case of periodic event triggering
    uint32                              periodicEventCounter;

    /// The estimated maximum number of events
    uint32                              maxNumberOfEvents;

public:

    /// Construcor
    EventTrigger() {
        acqTrigger          = NULL;
        numberOfAcqTriggers = 0;
        hasCondition        = False;
    };

    /// Destructor
    ~EventTrigger() {
        if(acqTrigger != NULL) {
            delete [] acqTrigger;
            acqTrigger = NULL;
        }
    };

    /// Initialise object
    bool   ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err);

    /// Reset object and proxy
    void   Reset();
        
    /// Evaluate event state
    void   Evaluate(uint32 usecTime);

    /// AddSignal proxy
    bool   AddSignal(FString alias, DDBInputInterface *ddbii);

    /// Retrieve number of acquisition trigger objects
    uint32 GetNumberOfAcquisitionTriggers();

    /// Retrieve reference to acquisition trigger object
    GCRTemplate<AcquisitionTrigger> &GetAcquisitionTriggerObject(uint32 idx) {
        return acqTrigger[idx];
    }

    /// Order flush
    void Flush();

private:

    /// Performance event evaluation in the case of
    /// periodic configuration
    bool EvaluatePeriodic();

};

#endif
