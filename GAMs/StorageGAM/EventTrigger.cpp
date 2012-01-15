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

#include "EventTrigger.h"

bool EventTrigger::ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err) {
    CDBExtended cdb(info);

    /// Read time configuration parameters
    int32 usecPeriodAux;
    if(!cdb.ReadInt32(usecPeriodAux, "UsecPeriod")) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: cannot find UsecPeriod cfg parameter", Name());
        return False;
    } else {
        if(usecPeriodAux <= 0) {
            AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: UsecPeriod <= 0", Name());
            return False;
        } else {
            usecPeriod = (uint32)usecPeriodAux;
        }
    }

    if(!cdb.ReadFloat(startSec, "StartSec")) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s StartSec cfg parameter not found", Name());
        return False;
    }
    if(!cdb.ReadFloat(endSec, "EndSec")) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s EndSec cfg parameter not found", Name());
        return False;
    }
    if(startSec > endSec) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s StartSec > EndSec", Name());
        return False;
    }

    startUsec = (uint32)round(startSec * 1e6);
    endUsec   = (uint32)round(endSec   * 1e6);

    /// Load child objects
    if(!GCReferenceContainer::ObjectLoadSetup(cdb, err)) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: failed executing father's ObjectLoadSetup", Name());
        return False;
    }

    /// Check child object types
    for(int i = 0 ; i < Size() ; i++) {
        GCRTemplate<AcquisitionTrigger> acqTrigAux   = Find(i);
        GCRTemplate<OperationCondition> conditionAux = Find(i);
        if(acqTrigAux.IsValid()) {
             numberOfAcqTriggers++;
        } else if(conditionAux.IsValid()) {
            if(hasCondition) {
                AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: only one OperationCondition object allowed", Name());
                return False;
            } else {
                condition    = conditionAux;
                hasCondition = True;
            }
        } else {
            AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: object type not allowed in this context", Name());
            return False;
        }
    }
    
    int32 periodicEventTriggerEveryNCyclesAux;
    if(hasCondition) {
        if(cdb.ReadInt32(periodicEventTriggerEveryNCyclesAux, "PeriodicEventTriggerEveryNCycles")) {
            AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: simultaneously conditional and periodic event triggering", Name());
            return False;
        }
    } else {
        cdb.ReadInt32(periodicEventTriggerEveryNCyclesAux, "PeriodicEventTriggerEveryNCycles", 1);
        periodicEventTriggerEveryNCycles = (uint32)periodicEventTriggerEveryNCyclesAux;
        //periodicEventCounter = periodicEventTriggerEveryNCycles;
        periodicEventCounter = 1;
    }
    
    /// Check if there is at least one acq trigger object
    if(numberOfAcqTriggers <= 0) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: At least one acq trigger object required", Name());
        return False;
    }

    /// Allocate memory for array of acq triggers
    if((acqTrigger = new GCRTemplate<AcquisitionTrigger>[numberOfAcqTriggers]) == NULL) {
        AssertErrorCondition(InitialisationError, "EventTrigger::ObjectLoadSetup: %s: unable to allocate memory for AcquisitionTrigger object array", Name());
        return False;
    }
    /// Store acq trigger objects in array
    uint32 counter = 0;
    for(int i = 0 ; i < Size() ; i++) {
        GCRTemplate<AcquisitionTrigger> acqTrigAux = Find(i);
        if(acqTrigAux.IsValid()) {
            acqTrigger[counter] = acqTrigAux;
            counter++;
        }
    }

    /// Estimate maximum number of events
    numberOfRTCycles = (endUsec-startUsec)/usecPeriod;
    if(hasCondition) {
        maxNumberOfEvents = numberOfRTCycles;
    } else {
        maxNumberOfEvents = numberOfRTCycles/periodicEventTriggerEveryNCycles;
    }
    
    /// Configure acq trigger objects with max number of events
    for(int i = 0 ; i < numberOfAcqTriggers ; i++) {
        acqTrigger[i]->SetMaxNumberOfCycles2Acquire(maxNumberOfEvents);
    }    

    return True;
}

void EventTrigger::Reset() {
    for(int i = 0 ; i < numberOfAcqTriggers ; i++) {
        acqTrigger[i]->Reset();
    }

    if(hasCondition) {
        condition->Reset();
    }
}

void EventTrigger::Evaluate(uint32 usecTime) {
    bool event = False;
    if((usecTime >= startUsec) && (usecTime <= endUsec)) {
        if(hasCondition) {
            event = condition->IsTrue();
        } else {
            event = EvaluatePeriodic();
        }
    }

    for(int i = 0 ; i < numberOfAcqTriggers ; i++) {
        acqTrigger[i]->Evaluate(event);
    }
}

bool EventTrigger::EvaluatePeriodic() {
    if(periodicEventCounter == periodicEventTriggerEveryNCycles) {
        periodicEventCounter = 1;
        return True;
    } else {
        periodicEventCounter++;
        return False;
    }
}

bool EventTrigger::AddSignal(FString alias, DDBInputInterface *ddbii) {
    bool ret = True;
    for(int i = 0 ; i < numberOfAcqTriggers ; i++)
        ret &= (acqTrigger[i]->AddSignal(alias, ddbii));
    
    return ret;
}

uint32 EventTrigger::GetNumberOfAcquisitionTriggers() {
    return numberOfAcqTriggers;
}

void EventTrigger::Flush() {
    for(int i = 0 ; i < numberOfAcqTriggers ; i++) {
        acqTrigger[i]->Flush();
    }
}
OBJECTLOADREGISTER(EventTrigger, "$Id: EventTrigger.cpp,v 1.4 2011/04/09 15:42:21 dalves Exp $")
