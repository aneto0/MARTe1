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

#include "SignalInfoContainer.h"

bool SignalInfoContainer::ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err) {
    return True;
}

void SignalInfoContainer::Flush() {
    for(int i = 0 ; i < numberOfSignals ; i++) {
        ((SignalInformation *)(signalInfo.ListPeek(i)))->Flush();
    }
}

void SignalInfoContainer::Reset() {
    for(int i = 0 ; i < numberOfSignals ; i++) {
        ((SignalInformation *)(signalInfo.ListPeek(i)))->Reset();
    }
}

void SignalInfoContainer::Mark(int32 cycleOffset) {
    for(int i = 0 ; i < numberOfSignals ; i++) {
        ((SignalInformation *)(signalInfo.ListPeek(i)))->Mark(cycleOffset);
    }
}

void SignalInfoContainer::Store(int32 cycleOffset) {
    for(int i = 0 ; i < numberOfSignals ; i++) {
        ((SignalInformation *)(signalInfo.ListPeek(i)))->Store(cycleOffset);
    }
}

void SignalInfoContainer::UpdateData() {
    for(int i = 0 ; i < numberOfSignals ; i++) {
        ((SignalInformation *)(signalInfo.ListPeek(i)))->UpdateData();
    }
}

void SignalInfoContainer::SynchroniseInternalSignals(int32 cycleOffset) {

    char  *timeReadPtr = (char *)((SignalInformation *)(signalInfo.ListPeek(0)))->acqBuffManager.linearBuffer.ReadPtr();
    char  *timeDataPtr = (char *)((SignalInformation *)(signalInfo.ListPeek(0)))->acqBuffManager.linearBuffer.DataBuffer();
    uint32 bytesPerCycle = ((SignalInformation *)(signalInfo.ListPeek(0)))->acqBuffManager.bytesPerCycle;
    
    uint32 numberOfCycles = (timeReadPtr+cycleOffset*bytesPerCycle-timeDataPtr)/bytesPerCycle;

    for(int i = 1 ; i < numberOfSignals ; i++) {
        ((SignalInformation *)(signalInfo.ListPeek(i)))->acqBuffManager.linearBuffer.SetReadPointerPosition((((SignalInformation *)(signalInfo.ListPeek(i)))->acqBuffManager.bytesPerCycle)*numberOfCycles);
    }
}

bool SignalInfoContainer::AddSignal(FString alias, uint32 maxNumberOfCycles2Acquire, uint32 cyclesPerAcqTrig, DDBInputInterface *ddbii) {
    if(ddbii == NULL) {
        AssertErrorCondition(FatalError, "SignalInfoContainer::AddSignal: %s: DDB input interface pointer = null", Name());
        return False;
    }
    
    SignalInformation *signalInfoObject = new SignalInformation;
    if(!signalInfoObject) {
        AssertErrorCondition(FatalError, "SignalInfoContainer::AddSignal: %s: unable to allocate memory for signal info object", Name());
        return False;
    }
    
    if(!signalInfoObject->Initialise(alias, maxNumberOfCycles2Acquire, cyclesPerAcqTrig, ddbii)) {
        AssertErrorCondition(FatalError, "SignalInfoContainer::AddSignal: %s: Initialise of signal info object failed", Name());
        return False;
    }
    signalInfo.ListAdd(signalInfoObject);
    
    numberOfSignals++;
    
    return True;
}
OBJECTLOADREGISTER(SignalInfoContainer, "$Id$")
