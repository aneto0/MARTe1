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
#include "CDBExtended.h"
#include "SignalInfoContainer.h"

#if !defined(_ACQUISITION_TRIGGER_)
#define _ACQUISITION_TRIGGER_

static const int32 HARD_MAX_ACQ_TRIGGERS = 0xFFFFFFFF;

OBJECT_DLL(AcquisitionTrigger)
class AcquisitionTrigger : public GCNamedObject {
OBJECT_DLL_STUFF(AcquisitionTrigger)

private:

    /// Signal info container object
    SignalInfoContainer    signalInfoContainer;

    /// The start cycle with respect to the event
    int32                               acqStartCycleOffsetRelativeToEvent;

    /// The end cycle with respect to the event
    int32                               acqEndCycleOffsetRelativeToEvent;

    /// A counter to keep track of when to give the trigger order with respect
    /// to the occurence of the event
    int32                              counterToTriggerAcqFromEvent;

    /// Number of cycles to acquire per acquisition trigger
    uint32                              cyclesPerAcqTrigger;

    /// Used for fixed periodic acquisition triggers
    uint32                              periodicAcquisitionTriggerEveryNEvents;

    /// Counter used in the case of periodic acquisition triggering
    uint32                              periodicEventCounter;

    /// Maximum number of acquisition triggers
    uint32                              maxAcquisitionTriggers;

    /// The maximum number of cycles to acquire
    uint32                              maxNumberOfCycles2Acquire;

    /// Acquisition trigger counter
    uint32                              acquisitionTriggerCounter;

    /// A flag to inform whether or not an event is already being dealt with
    bool                                dealingWithEvent;

public:

    /// Initialise object
    bool ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err);

    /// Reset object and proxy
    void Reset();

    /// Evaluate the acquisition trigger action
    void Evaluate(bool isEvent);

    /// Set the maximum number of cycles
    void SetMaxNumberOfCycles2Acquire(uint32 maxEvents);

    /// Function being proxied up to SignalInfoContainer
    /// to add a signal to the signal info list
    bool AddSignal(FString alias, DDBInputInterface *ddbii);

    /// Return a pointer to the signal info container object
    SignalInfoContainer *GetSignalInfoContainer() {
        return (&signalInfoContainer);
    }

    /// Flush proxy
    void Flush();
};

#endif
