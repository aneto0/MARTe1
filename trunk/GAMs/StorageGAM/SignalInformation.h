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
#include "DDBInputInterface.h"
#include "AcquisitionBufferManager.h"
#include "GCReference.h"

#if !defined(_SIGNAL_INFORMATION_)
#define _SIGNAL_INFORMATION_

OBJECT_DLL(SignalInformation)
class SignalInformation : public GCNamedObject, public LinkedListable {
OBJECT_DLL_STUFF(SignalInformation)

private:

    /// Pointer to the DDB interface
    const DDBInputInterface         *ddbip;

    /// Signal descriptor pointer
    const DDBSignalDescriptor       *ddbsd;

    /// Signal basic type
    BasicTypeDescriptor              btd;

    /// Signal name in the DDB
    FString                          ddbSignalName;

    /// Signal name alias
    /// This is usually the name the signals
    /// is known to the outside world (i.e. outside MARTe)
    FString                          alias;

    /// Number of signal samples per cycle
    uint32                           samplesPerCycle;

    /// Number of bytes per signal sample
    uint32                           bytesPerSample;

    /// Number of signal bytes per cycle
    uint32                           bytesPerCycle;

    /// Total number of cycles
    uint32                           totalNumberOfCycles;

    /// Total number of bytes
    uint32                           totalNumberOfBytes;

public:

    /// The objects that manages the data acquisition buffers
    AcquisitionBufferManager         acqBuffManager;


public:

    /// Constructor
    SignalInformation();
    
    /// Initialise the object and configure the acquisiton buffer manager
    bool           Initialise(FString alias, uint32 maxNumberOfCycles2Acquire, uint32 cyclesPerStoreOperation, DDBInputInterface *ddbInterfacePtr);

    /// Reset proxy
    void           Reset();

    /// UpdateData proxy
    void           UpdateData();
    
    /// Mark proxy
    void           Mark(int32 cycleOffset);
    
    /// Store proxy
    void           Store(int32 cycleOffset);

    /// Return the total number of cycles
    uint32         GetTotalNumberOfCycles();

    /// Return the signal alias
    const FString &GetAlias();

    /// Return the signal's DDB name
    const FString &GetDDBSignalName();

    /// Flush proxy
    void           Flush();

    /// Return the signal's basic type
    const BasicTypeDescriptor &GetBasicTypeDescriptor();
};
#endif
