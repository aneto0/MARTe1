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
#include "StorageGAM.h"
#include "CDBExtended.h"

bool StorageGAM::Initialise(ConfigurationDataBase& cdbData) {

    acceptingMessages = False;

    CDBExtended cdb(cdbData);

    /// Import and check TriggerWindowInterface objects
    numberOfEventTriggerObjects = Size();
    if(numberOfEventTriggerObjects == 0) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: no trigger objects found", Name());
        return False;
    }

    if((eventTriggerObject = new GCRTemplate<EventTrigger>[numberOfEventTriggerObjects]) == NULL) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: unable to allocate memory to hold TriggerWindowInterface objects", Name());
        return False;        
    }
    for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
        eventTriggerObject[i] = Find(i);
        if(!eventTriggerObject[i].IsValid()) {
            AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: child object %d is not of the TriggerWindowInterface type", Name(), i);
            return False;
        }
    }

    /// Add usec time signal
    FString timeBase;
    timeBase.SetSize(0);
    if(!cdb.ReadFString(timeBase, "UsecTimeSignalName")) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }
    FString timeBaseType;
    timeBaseType.SetSize(0);
    if(!cdb.ReadFString(timeBaseType, "UsecTimeSignalType")) {
        AssertErrorCondition(Warning, "StorageGAM::Initialise: %s does not specify a UsecTimeSignalType", Name());
        return False;
    }
    if(!AddInputInterface(usecTimeInput, "UsecTimeInterface")) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s failed to add input interface InputInterface UsecTimeInterface", Name());
        return False;
    }
    if(!usecTimeInput->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s failed to add input Signal %s to interface InputInterface", Name(), timeBase.Buffer());
        return False;
    }

    for(int j = 0 ; j < numberOfEventTriggerObjects ; j++) {
        if(!eventTriggerObject[j]->AddSignal(timeBase, usecTimeInput)) {
            AssertErrorCondition(InitialisationError,"StorageGAM::Initialise: %s: unable to add usecTime signal to event trigger", Name());
            return False;
        }
    }

    /// Move to the list of signals to store
    if(!cdb->Move("SignalList")) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: cannot move to SignalList", Name());
        return False;        
    }

    /// Check and allocate memory for this amount of signals to store
    numberOfSignals2Store = cdb->NumberOfChildren();
    if(numberOfSignals2Store == 0) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: no signals specified for storage", Name());
        return False;        
    }

    if((input = (DDBInputInterface **)malloc(numberOfSignals2Store*sizeof(DDBInputInterface *))) == NULL) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: unable to allocate memory to hold input interfaces", Name());
        return False;        
    } else {
        /// Initialise all DDBInputInterface pointers to NULL
        /// or else all hell's gonna break lose
        for(int i = 0 ; i < numberOfSignals2Store ; i++) {
            input[i] = NULL;
        }
    }

    for(int i = 0 ; i < numberOfSignals2Store ; i++) {
        cdb->MoveToChildren(i);

        FString ddbSignalNameAux;
        ddbSignalNameAux.SetSize(0);
        if(!cdb.ReadFString(ddbSignalNameAux, "DDBSignalName")) {
            AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: DDBSignalName not found", Name());
            return False;        
        }
        FString externalSignalNameAux;
        externalSignalNameAux.SetSize(0);
        if(!cdb.ReadFString(externalSignalNameAux, "Alias")) {
            AssertErrorCondition(Warning, "StorageGAM::Initialise: %s: Alias not found", Name());
        }
        FString signalTypeAux;
        signalTypeAux.SetSize(0);
        if(!cdb.ReadFString(signalTypeAux, "SignalType")) {
            AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: SignalType not found", Name());
            return False;        
        }

        FString inputInterfaceName;
        inputInterfaceName.SetSize(0);
        inputInterfaceName.Printf("%sInputInterface", ddbSignalNameAux.Buffer());
        if(!AddInputInterface(input[i], inputInterfaceName.Buffer())){
            AssertErrorCondition(InitialisationError,"StorageGAM::Initialise: %s: failed to add input interface: %s",Name(), inputInterfaceName.Buffer());
            return False;
        }
        if(!input[i]->AddSignal(ddbSignalNameAux.Buffer(), signalTypeAux.Buffer())){
            AssertErrorCondition(InitialisationError,"StorageGAM::Initialise: %s: failed to add %s to %s", Name(), ddbSignalNameAux.Buffer(), input[i]->InterfaceName());
            return False;
        }

        bool ret = True;
        for(int j = 0 ; j < numberOfEventTriggerObjects ; j++) {
            ret &= eventTriggerObject[j]->AddSignal(externalSignalNameAux, input[i]);
        }
        if(!ret) {
            AssertErrorCondition(InitialisationError,"StorageGAM::Initialise: %s: unable to add signals to event trigger", Name());
            return False;
        }

        cdb->MoveToFather();
    }

    /** Get references of all SignalInfoContainers */
    numberOfSignalInfoContainers = 0;
    for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
        numberOfSignalInfoContainers += eventTriggerObject[i]->GetNumberOfAcquisitionTriggers();
    }

    if((signalInfoContainer = (SignalInfoContainer **)malloc(numberOfSignalInfoContainers*sizeof(SignalInfoContainer *))) == NULL) {
        AssertErrorCondition(InitialisationError, "StorageGAM::Initialise: %s: cannot allocate memory for SignalInfoContainer reference holder", Name());
        return False;        
    }

    /// Get pointer to all signal info containers (except the master one)
    uint32 counter = 0;
    for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
        for(int j = 0 ; j < eventTriggerObject[i]->GetNumberOfAcquisitionTriggers() ; j++) {
            signalInfoContainer[counter] = eventTriggerObject[i]->GetAcquisitionTriggerObject(j)->GetSignalInfoContainer();
            counter++;
        }
    }

    /// Setup the master signal info container
    for(int i = 1 ; i <= numberOfSignals2Store ; i++) {
        uint32 maxNumberOfCycles = 0;
        FString aliasName = ((SignalInformation *)(signalInfoContainer[0]->signalInfo.ListPeek(i)))->GetAlias();
        for(int j = 0 ; j < numberOfSignalInfoContainers ; j++) {
            maxNumberOfCycles += ((SignalInformation *)(signalInfoContainer[j]->signalInfo.ListPeek(i)))->GetTotalNumberOfCycles();
        }
        master.AddSignal(aliasName, maxNumberOfCycles, 0, input[i-1]);
    }
    
    acceptingMessages = True;
    
    return True;
}

void __thread_decl DataDiscombobulation(void* data){
    if (data == NULL) {
        CStaticAssertErrorCondition(InitialisationError,"StorageGAM::DataDiscombobulation: data is void!");
        return;
    }
    
    StorageGAM *storage = (StorageGAM *)data;
    
    // Set high priority for this thread
    // Threads::SetRealTimeClass();
    // Threads::SetPriorityLevel(32);
    
    // Signal correct initialization to ObjectLoadSetup
    storage->StartStopSem.Post();
    
    /// Discombobulation goes here
    storage->Discombobulate();
    
    storage->acceptingMessages = True;
}

bool StorageGAM::Discombobulate() {
    
    /// Reset all buffer read pointers
    for(int i = 0 ; i < numberOfSignalInfoContainers ; i++) {
        ((SignalInformation *)(signalInfoContainer[i]->signalInfo.ListPeek(0)))->acqBuffManager.linearBuffer.SetReadPointerPosition(0);
    }

    /// Reset all master buffer write pointers
    for(int i = 0 ; i < numberOfSignals2Store ; i++) {
        ((SignalInformation *)(master.signalInfo.ListPeek(i)))->acqBuffManager.linearBuffer.SetWritePointerPosition(0);
    }
    
    /// 
    uint32 newTime;
    uint32 currentTime;
    uint32 lastTime = 0;
    uint32 counter  = 0;
    bool   ret;
    while(1) {
        uint32 idx = 0;
        ret = False;
        for(int i = 0 ; i < numberOfSignalInfoContainers ; i++) {
            newTime     = 0xFFFFFFFF;
            currentTime = newTime;
            ret |= ((SignalInformation *)(signalInfoContainer[i]->signalInfo.ListPeek(0)))->acqBuffManager.linearBuffer.RetrieveAndAdvancePointer(&newTime, ((DDBSignalDescriptor *)(usecTimeInput->SignalsList()))->SignalTypeCode().ByteSize());
            if((lastTime == 0) && (counter == 0)) {
                if(newTime >= lastTime) {
                    if(newTime < currentTime) {
                        currentTime = newTime;
                        idx = i;
                    }
                }
            } else {
                if(newTime > lastTime) {
                    if(newTime < currentTime) {
                        currentTime = newTime;
                        idx = i;
                    }
                }
            }
        }
        if((!ret) || (currentTime == 0xFFFFFFFF)) {
            break;
        }
        /// Align read pointer of all signals with the time signal
        signalInfoContainer[idx]->SynchroniseInternalSignals(-1);
        /// Copy data here!!!
        for(int j = 1 ; j <= numberOfSignals2Store ; j++) {
            ((SignalInformation *)(master.signalInfo.ListPeek(j-1)))->acqBuffManager.linearBuffer.StoreAndAdvancePointer(((SignalInformation *)(signalInfoContainer[idx]->signalInfo.ListPeek(j)))->acqBuffManager.linearBuffer.ReadPtr(), ((SignalInformation *)(master.signalInfo.ListPeek(j-1)))->acqBuffManager.bytesPerCycle);
        }
        lastTime = currentTime;
        counter++;
    }

    return True;
}

bool StorageGAM::Execute(GAM_FunctionNumbers functionNumber) {
    
    /** Read usecTime */
    usecTimeInput->Read();
    uint32 usecTime = (uint32)(*((uint32 *)(usecTimeInput->Buffer())));

    switch(functionNumber) {
        case GAMPrepulse: {
            acceptingMessages = False;
            /** Read signals to store */
            for(int i = 0 ; i < numberOfSignals2Store ; i++) {
                input[i]->Read();
            }
            /** Reset pointers and acquisition buffers */
            for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
                eventTriggerObject[i]->Reset();
            }
            /** Update circular buffers */
            for(int i = 0 ; i < numberOfSignalInfoContainers ; i++) {
                signalInfoContainer[i]->UpdateData();
            }
            for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
                eventTriggerObject[i]->Evaluate(usecTime);
            }
        } break;

        case GAMOnline: {
            /** Read signals to store */
            for(int i = 0 ; i < numberOfSignals2Store ; i++) {
                input[i]->Read();
            }
            /** Update circular buffers */
            for(int i = 0 ; i < numberOfSignalInfoContainers ; i++) {
                signalInfoContainer[i]->UpdateData();
            }
            for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
                eventTriggerObject[i]->Evaluate(usecTime);
            }
        } break;

        case GAMPostpulse: {
            /** Read signals to store */
            for(int i = 0 ; i < numberOfSignals2Store ; i++) {
                input[i]->Read();
            }
            /** Update circular buffers */
            for(int i = 0 ; i < numberOfSignalInfoContainers ; i++) {
                signalInfoContainer[i]->UpdateData();
            }
            for(int i = 0 ; i < numberOfEventTriggerObjects ; i++) {
                eventTriggerObject[i]->Evaluate(usecTime);
                eventTriggerObject[i]->Flush();
            }
            /// Begin thread to have data ready for servicing
            TID tid = Threads::BeginThread(DataDiscombobulation, this, THREADS_DEFAULT_STACKSIZE, "DataDiscombobulation");
            // Wait for the thread to start
            StartStopSem.Wait();
        } break;
    };

    return True;
}

bool StorageGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope) {

    if(!acceptingMessages){
        AssertErrorCondition(InitialisationError,"StorageGAM::ProcessMessage: %s: StorageGAM is not accepting messages yet", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (!message.IsValid()){
        AssertErrorCondition(InitialisationError,"StorageGAM::ProcessMessage: %s: Received invalid Message", Name());
        return False;
    }

    FString messageContent = message->Content();

    // GAP Message Request
    if(messageContent == "LISTSIGNALS" || messageContent == "LISTSIGNALSDDB"){

        GCRTemplate<Message> addSignals(GCFT_Create);
        if(!addSignals.IsValid()){
            AssertErrorCondition(InitialisationError,"StorageGAM::ProcessMessage: %s: Failed creating Message", Name());
            return False;
        }

        addSignals->Init(0,"ADDSIGNAL");

        for(int nSignals = 0 ; nSignals < numberOfSignals2Store ; nSignals++){
            GCRTemplate<GCNString> signalDescription(GCFT_Create);
            if(!signalDescription.IsValid()){
                AssertErrorCondition(FatalError,"StorageGAM::ProcessMessage: %s: Failed creating GCNString", Name());
                return False;
            }
            
            FString signalName;
            signalName.SetSize(0);
            if(messageContent == "LISTSIGNALS") {
                signalName = ((SignalInformation *)(master.signalInfo.ListPeek(nSignals)))->GetAlias();
            } else if(messageContent == "LISTSIGNALSDDB") {
                signalName = ((SignalInformation *)(master.signalInfo.ListPeek(nSignals)))->GetDDBSignalName();
            }
            signalDescription->SetObjectName(signalName.Buffer());
            signalDescription->Seek(0);
            addSignals->Insert(signalDescription);
        }


        GCRTemplate<MessageEnvelope> addSignalsEnvelope(GCFT_Create);
        if(!addSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"StorageGAM::ProcessMessage: %s: Failed creating MessageEnvelope", Name());
            return False;
        }

        addSignalsEnvelope->PrepareReply(envelope, addSignals);
        MessageHandler::SendMessage(addSignalsEnvelope);

        return True;
    }else if(messageContent == "INITSIGNAL"){

        return True;
    }else if(messageContent == "GETSIGNAL"){

        GCRTemplate<Message> sendSignal(GCFT_Create);
        if(!sendSignal.IsValid()){
            AssertErrorCondition(FatalError,"StorageGAM::ProcessMessage: %s: GETSIGNALS: Failed creating Message", Name());
            return False;
        }
        
        sendSignal->Init(0,"SIGNAL");
        
        GCRTemplate<GCNString>  errorMessage(GCFT_Create);
        if(!errorMessage.IsValid()){
            AssertErrorCondition(FatalError,"StorageGAM::ProcessMessage: %s: GETSIGNALS: Failed creating GCNString", Name());
            return False;
        }
        
        errorMessage->SetObjectName("ERROR");
        
        GCRTemplate<MessageEnvelope> sendSignalsEnvelope(GCFT_Create);
        if(!sendSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"StorageGAM::ProcessMessage: %s: GETSIGNALS: Failed creating MessageEnvelope", Name());
            return False;
        }

        sendSignalsEnvelope->PrepareReply(envelope, sendSignal);
        
        GCRTemplate<GCNString>  signalName = message->Find(0);
        if(!signalName.IsValid()){
            errorMessage->Printf("Missing Signal Name");
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"StorageGAM::ProcessMessage: %s: GETSIGNALS: No valid signalName has been specified ", Name());
            return False;
        }

        GCRTemplate<Signal> signal(GCFT_Create);
        if(!RetrieveSignal(*(signalName.operator->()), signal)) {
            AssertErrorCondition(InitialisationError,"StorageGAM::ProcessMessage: %s: RetrieveSignal() failed for %s", Name(), signalName->Buffer());
            return False;
        }

        if (!signal.IsValid()){
            errorMessage->Printf("Signal %s not found in GAM %s", signalName->Buffer(), Name());
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"StorageGAM::ProcessMessage: %s: GETSIGNALS: %s ", Name(), errorMessage->Buffer());
            return False;
        }

        sendSignal->Insert(signal);
        MessageHandler::SendMessage(sendSignalsEnvelope);
        return True;
    }

    return False;
}

bool StorageGAM::RetrieveSignal(const FString &sigName, GCRTemplate<Signal> &sig, MemoryAllocationFlags allocFlags) {
    int32 signalIndex = 0;
    while(signalIndex < numberOfSignals2Store) {
        /// Check if signal exists
        if(sigName == ((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->GetAlias().Buffer()) {
            break;
        }
        if(sigName == ((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->GetDDBSignalName().Buffer()) {
            break;
        }
        signalIndex++;
    }
    if(signalIndex >= numberOfSignals2Store) {
        // Signal not found ... remove the reference to invalidate the signal
        AssertErrorCondition(FatalError, "StorageGAM::RetrieveSignal: %s: %s not found", Name(), sigName.Buffer());
        sig.RemoveReference();
        return False;
    } else {
        // Signal found ... copying data
        BasicTypeDescriptor btd = ((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->GetBasicTypeDescriptor();
        
        uint32 startPointer = (uint32)(((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->acqBuffManager.linearBuffer.DataBuffer());
        uint32 endPointer = (uint32)(((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->acqBuffManager.linearBuffer.WritePtr());
        uint32 byteSize = ((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->GetBasicTypeDescriptor().ByteSize();
        uint32 numberOfSamples = (endPointer-startPointer)/byteSize;
        
        if(!sig->CopyData(btd, (int32)numberOfSamples, ((SignalInformation *)(master.signalInfo.ListPeek(signalIndex)))->acqBuffManager.linearBuffer.DataBuffer(), allocFlags)) {
            AssertErrorCondition(FatalError,"StorageGAM::RetrieveSignal: %s: cannot allocate memory for signal object %s", Name(), sigName.Buffer());
            sig.RemoveReference();
            return False;
        }
    }        

    /// All is good ... baptize the signal
    sig->SetObjectName(sigName.Buffer());
    
    return True;
}

bool StorageGAM::ProcessHttpMessage(HttpStream &hStream) {
    // hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    // hStream.keepAlive = False;
    // //copy to the client
    // hStream.WriteReplyHeader(False);

    // hStream.Printf("<html><head><title>StorageGAM - %s</title></head><body>", Name());

    // dataCollector.HTMLInfo(hStream);

    // hStream.Printf("</table></body></html>");
    return True;
}
OBJECTLOADREGISTER(StorageGAM,"$Id$")
