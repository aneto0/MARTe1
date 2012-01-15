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

#include "SignalInformation.h"

SignalInformation::SignalInformation() {
    ddbip               = NULL;
    ddbsd               = NULL;
    btd                 = BTDTNone;
    ddbSignalName       = "";
    alias               = "";
    samplesPerCycle     = 0;
    bytesPerSample      = 0;
    bytesPerCycle       = 0;
    totalNumberOfCycles = 0;
    totalNumberOfBytes  = 0;
};

bool SignalInformation::Initialise(FString alias, uint32 maxNumberOfCycles2Acquire, uint32 cyclesPerStoreOperation, DDBInputInterface *ddbInterfacePtr) {

    if(ddbInterfacePtr == NULL) {
        AssertErrorCondition(InitialisationError, "SignalInformation::Initialise: %s: DDB interface pointer = null", Name());
        return False;
    }
    
    totalNumberOfCycles = maxNumberOfCycles2Acquire;

    this->alias         = alias;
    
    ddbip               = ddbInterfacePtr;
    ddbsd               = ddbInterfacePtr->SignalsList();

    btd                 = ddbsd->SignalTypeCode();

    ddbSignalName       = ddbsd->SignalName();
    samplesPerCycle     = ddbsd->SignalSize();
    bytesPerSample      = ddbsd->SignalTypeCode().ByteSize();
    bytesPerCycle       = samplesPerCycle * bytesPerSample;
    totalNumberOfBytes  = maxNumberOfCycles2Acquire * bytesPerCycle;
    
    if(!acqBuffManager.Config(ddbInterfacePtr, bytesPerCycle, cyclesPerStoreOperation, totalNumberOfBytes)) {
        AssertErrorCondition(InitialisationError, "SignalInformation::Initialise: %s: error configuring the acquisition buffer manager object", Name());
        return False;
    }
    
    return True;
};

void SignalInformation::Flush() {
    acqBuffManager.Flush();        
};

void SignalInformation::Reset() {
    acqBuffManager.Reset();        
};

void SignalInformation::UpdateData() {
    acqBuffManager.UpdateData();
};

void SignalInformation::Mark(int32 cycleOffset) {
    acqBuffManager.Mark(cycleOffset);
};

void SignalInformation::Store(int32 cycleOffset) {
    acqBuffManager.Store(cycleOffset);
};

uint32 SignalInformation::GetTotalNumberOfCycles() {
    return totalNumberOfCycles;
};
const FString &SignalInformation::GetAlias() {
    return alias;
};
const FString &SignalInformation::GetDDBSignalName() {
    return ddbSignalName;
};
const BasicTypeDescriptor &SignalInformation::GetBasicTypeDescriptor() {
    return btd;
};
OBJECTLOADREGISTER(SignalInformation, "$Id$")
