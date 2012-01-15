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

#include "AcquisitionTrigger.h"

bool AcquisitionTrigger::ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err) {
    CDBExtended cdb(info);

    /// Read max number of acq triggers (optional)
    int32 maxAcquisitionTriggersAux;
    if(cdb.ReadInt32(maxAcquisitionTriggersAux, "MaxAcquisitionTriggers", HARD_MAX_ACQ_TRIGGERS)) {
        if(maxAcquisitionTriggersAux <= 0) {
            AssertErrorCondition(InitialisationError, "AcquisitionTrigger::ObjectLoadSetup: %s: MaxAcquisitionTriggers <= 0", Name());
            return False;
        }
    }
    maxAcquisitionTriggers = (uint32)maxAcquisitionTriggersAux;


    cdb.ReadInt32(acqStartCycleOffsetRelativeToEvent, "AcqStartCycleOffsetRelativeToEvent", 0);
    cdb.ReadInt32(acqEndCycleOffsetRelativeToEvent  , "AcqEndCycleOffsetRelativeToEvent"  , 0);
    if(acqEndCycleOffsetRelativeToEvent < acqStartCycleOffsetRelativeToEvent) {
        AssertErrorCondition(InitialisationError, "AcquisitionTrigger::ObjectLoadSetup: %s: AcqEndCycleOffsetRelativeToEvent < AcqStartCycleOffsetRelativeToEvent", Name());
        return False;
    }
    cyclesPerAcqTrigger = acqEndCycleOffsetRelativeToEvent-acqStartCycleOffsetRelativeToEvent+1;


    int32 periodicAcquisitionTriggerEveryNEventsAux;
    if(cdb.ReadInt32(periodicAcquisitionTriggerEveryNEventsAux, "PeriodicAcquisitionTriggerEveryNEvents", 1)) {
        if(periodicAcquisitionTriggerEveryNEventsAux <= 0) {
            AssertErrorCondition(InitialisationError, "AcquisitionTrigger::ObjectLoadSetup: %s: PeriodicAcquisitionTriggerEveryNEvents <= 0", Name());
            return False;
        }
    }
    periodicAcquisitionTriggerEveryNEvents = (uint32)periodicAcquisitionTriggerEveryNEventsAux;

    Reset();

    return True;
}

void AcquisitionTrigger::Reset() {
    periodicEventCounter      = 1;
    acquisitionTriggerCounter = 0;
    dealingWithEvent          = False;

    signalInfoContainer.Reset();
}

void AcquisitionTrigger::Flush() {
    signalInfoContainer.Flush();
}

void AcquisitionTrigger::Evaluate(bool isEvent) {
    if((isEvent) && (acquisitionTriggerCounter < maxAcquisitionTriggers)) {
        if(periodicEventCounter == periodicAcquisitionTriggerEveryNEvents) {
            periodicEventCounter = 1;
            counterToTriggerAcqFromEvent = acqEndCycleOffsetRelativeToEvent;
            if(!dealingWithEvent) {
                /// Mark the beggining of the acquisition
                signalInfoContainer.Mark(acqStartCycleOffsetRelativeToEvent-1);
            }
            dealingWithEvent = True;
        } else {
            periodicEventCounter++;
        }
    }
    
    if(dealingWithEvent) {
        if((--counterToTriggerAcqFromEvent < 0) && (acquisitionTriggerCounter < maxAcquisitionTriggers)) {
            /// Order the data storage
            if(acqEndCycleOffsetRelativeToEvent < 0) {
                signalInfoContainer.Store(acqEndCycleOffsetRelativeToEvent);
            } else {
                signalInfoContainer.Store(0);
            }
            acquisitionTriggerCounter++;
            dealingWithEvent = False;
        }
    }
}

void AcquisitionTrigger::SetMaxNumberOfCycles2Acquire(uint32 maxEvents) {
    maxNumberOfCycles2Acquire = cyclesPerAcqTrigger*maxEvents/periodicAcquisitionTriggerEveryNEvents;
}

bool AcquisitionTrigger::AddSignal(FString alias, DDBInputInterface *ddbii) {
    return (signalInfoContainer.AddSignal(alias, maxNumberOfCycles2Acquire, cyclesPerAcqTrigger, ddbii));
}

OBJECTLOADREGISTER(AcquisitionTrigger, "$Id: AcquisitionTrigger.cpp,v 1.6 2011/04/11 10:51:46 dalves Exp $")
