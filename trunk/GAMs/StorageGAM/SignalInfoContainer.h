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
#include "LinkedListHolder.h"
#include "SignalInformation.h"

#if !defined(_SIGNAL_INFO_CONTAINER_)
#define _SIGNAL_INFO_CONTAINER_

OBJECT_DLL(SignalInfoContainer)
class SignalInfoContainer : public GCNamedObject {
OBJECT_DLL_STUFF(SignalInfoContainer)

private:

    /// Number of signals to acquire
    uint32                    numberOfSignals;

public:

    /// Linked list with pointers to signal info objects
    LinkedListHolder          signalInfo;

public:

    /// Constructor
    SignalInfoContainer() {
        numberOfSignals = 0;
    };

    /// Destructor
    ~SignalInfoContainer() {
        signalInfo.CleanUp();
    };

    /// Initialise object
    bool ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err);

    /// Add a new signal to the list
    bool AddSignal(FString alias, uint32 maxNumberOfCycles2Acquire, uint32 cyclesPerAcqTrig, DDBInputInterface *ddbii);

    /// Reset Proxy
    void Reset();

    /// UpdateData proxy
    void UpdateData();

    /// Mark proxy
    void Mark(int32 cycleOffset);

    /// Store proxy
    void Store(int32 cycleOffset);

    /// Flush proxy
    void Flush();

    /// Synch the linear buffer read pointer
    /// with the time array read pointer
    void SynchroniseInternalSignals(int32 cycleOffset);
};

#endif
