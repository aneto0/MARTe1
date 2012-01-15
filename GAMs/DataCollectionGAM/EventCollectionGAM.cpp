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

#include "EventCollectionGAM.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"
#include "GCNString.h"
#include "MenuEntry.h"

EventCollectionGAM::EventCollectionGAM(){
    usecTime           = NULL;
    jpfData            = NULL;
    acceptingMessages  = False;
    inputState         = NULL;
    triggerAcquisition = False;
    inputStateByteSize = 0;
}

bool EventCollectionGAM::Initialise(ConfigurationDataBase& cdbData){

    acceptingMessages = False;
    CDBExtended cdb(cdbData);

    //////////////////////////
    // Add Time BaseSignal //
    //////////////////////////

    if(!AddInputInterface(usecTime,"UsecTimeInterface")){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s failed to add input interface InputInterface UsecTimeInterface",Name());
        return False;
    }

    FString timeBase;
    if(!cdb.ReadFString(timeBase,"UsecTimeSignalName")){
        AssertErrorCondition(InitialisationError, "EventCollectionGAM::Initialise: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }

    FString timeBaseType;
    if(!cdb.ReadFString(timeBaseType,"TimeSignalType","int32")){
        AssertErrorCondition(Warning, "EventCollectionGAM::Initialise: %s does not specify a TimeSignalType. Assuming int32", Name());
    }

    if(!usecTime->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),timeBase.Buffer());
        return False;
    }

    //////////////////////////////////////
    // Add JPF Signal List to Interface //
    //////////////////////////////////////

    if(!AddInputInterface(jpfData,"JPFSignalList")){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s failed to add input interface InputInterface JPFSignalList",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    if(!jpfData->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),jpfData->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    inputStateByteSize = sizeof(uint32) * jpfData->BufferWordSize(); 
    inputState         =  (uint32 *)malloc(inputStateByteSize);
    if(inputState == NULL){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s: Failed allocating space for the input state of interface %s",Name(),jpfData->InterfaceName());
        return False;
    }
    
    /////////////////////////////////////
    // Init RTDataCollector Parameters //
    /////////////////////////////////////

    if(!dataCollector.ObjectLoadSetup(cdb,NULL, jpfData)){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::Initialise: %s: dataCollector ObjectLoadSetup Failed",Name());
        return False;
    }

    acceptingMessages = True;
    return True;
}

bool EventCollectionGAM::Execute(GAM_FunctionNumbers functionNumber){

    // Read Cycle Time
    usecTime->Read();
    int32 *timePointer   = (int32 *)usecTime->Buffer();
    int32 usecTimeSample = timePointer[0];

    // Read Jpf Data
    jpfData->Read();
    uint32 *jpfDataBuffer = (uint32 *)(jpfData->Buffer());

    // Check if data acquisition is needed (skip the first signal which is the time base)
    for(int i = 1; (i < (inputStateByteSize/sizeof(int32))) && (!triggerAcquisition); i++){
        if(jpfDataBuffer[i] != inputState[i]) triggerAcquisition = True;
    }
    
    switch(functionNumber){
        case GAMOnline:{
            if(!dataCollector.StoreData(jpfDataBuffer, usecTimeSample, triggerAcquisition)){
                AssertErrorCondition(FatalError,"EventCollectionGAM::%s: Execute(GAMOffline | GAMOnline) Failed", Name());
                return False;
            }
        }break;

        case GAMPrepulse:{
            // Disable Handling of the Messages
            if(acceptingMessages) acceptingMessages = False;

            if(!dataCollector.PrepareForNextPulse()){
                AssertErrorCondition(FatalError,"EventCollectionGAM::%s: Execute(GAMPrepulse) Failed", Name());
                return False;
            }

            triggerAcquisition = True;
            
            return True;
        }break;

        case GAMPostpulse:{
            // Enable Handling of the Messages
            if(!acceptingMessages) acceptingMessages = True;

            if(!dataCollector.CompleteDataCollection()){
                AssertErrorCondition(FatalError,"EventCollectionGAM::%s: Execute(GAMPostpulse) Failed", Name());
                return False;
            }
        }break;
    };

    // Copy the present inputs 
    memcpy(inputState, jpfDataBuffer ,inputStateByteSize);
    triggerAcquisition = False;
    return True;

};

GCRTemplate<SignalInterface>  EventCollectionGAM::GetSignal(const FString &signalName){
    return dataCollector.GetSignalData(signalName);
};

bool EventCollectionGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){

    if(!acceptingMessages){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: State Machine is not a valid type", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (!message.IsValid()){
        AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: Received invalid Message", Name());
        return False;
    }

    FString messageContent = message->Content();
    FString messageSender  = envelope->Sender();

    // GAP Message Request
    if(messageContent == "LISTSIGNALS"){

        GCRTemplate<Message> addSignals(GCFT_Create);
        if(!addSignals.IsValid()){
            AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: Failed creating Message", Name());
            return False;
        }

        addSignals->Init(0,"ADDSIGNAL");

        CDBExtended cdb;
        if(!dataCollector.ObjectSaveSetup(cdb)){
            AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: Failed gathering information about data collection", Name());
            return False;
        }

        int32 nOfAcquiredSignals = 0;
        cdb.ReadInt32(nOfAcquiredSignals, "NOfChannels");

        if(!cdb->Move("Signals")){
            AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: Signals entry not available in the data base", Name());
            return False;
        }

        for(int nSignals = 0; nSignals < nOfAcquiredSignals; nSignals++){
            GCRTemplate<GCNString> signalDescription(GCFT_Create);
            if(!signalDescription.IsValid()){
                AssertErrorCondition(FatalError,"EventCollectionGAM::ProcessMessage: %s: Failed creating GCNString", Name());
                return False;
            }
            
            cdb->MoveToChildren(nSignals);
            FString signalName;
            cdb.ReadFString(signalName, "Name");
            signalDescription->SetObjectName(signalName.Buffer());
            cdb->WriteToStream(*(signalDescription.operator->()));
            signalDescription->Seek(0);
            addSignals->Insert(signalDescription);
            cdb->MoveToFather();
            //AssertErrorCondition(Information,"EventCollectionGAM::ProcessMessage: %s: Adding Signal %s to the collection list", Name(), signalName.Buffer());
        }


        GCRTemplate<MessageEnvelope> addSignalsEnvelope(GCFT_Create);
        if(!addSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"EventCollectionGAM::ProcessMessage: %s: Failed creating MessageEnvelope", Name());
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
            AssertErrorCondition(FatalError,"EventCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating Message", Name());
            return False;
        }

        sendSignal->Init(0,"SIGNAL");

        GCRTemplate<GCNString>  errorMessage(GCFT_Create);
        if(!errorMessage.IsValid()){
            AssertErrorCondition(FatalError,"EventCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating GCNString", Name());
            return False;
        }

        errorMessage->SetObjectName("ERROR");

        GCRTemplate<MessageEnvelope> sendSignalsEnvelope(GCFT_Create);
        if(!sendSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"EventCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating MessageEnvelope", Name());
            return False;
        }

        sendSignalsEnvelope->PrepareReply(envelope, sendSignal);

        GCRTemplate<GCNString>  signalName = message->Find(0);
        if(!signalName.IsValid()){
            errorMessage->Printf("Missing Signal Name");
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: GETSIGNALS: No valid signalName has been specified ", Name());
            return False;
        }

        GCRTemplate<SignalInterface>  signal = GetSignal(*(signalName.operator->()));
        if (!signal.IsValid()){
            errorMessage->Printf("Signal %s not found in GAM %s", signalName->Buffer(), Name());
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"EventCollectionGAM::ProcessMessage: %s: GETSIGNALS: %s ", Name(), errorMessage->Buffer());
            return False;
        }

        sendSignal->Insert(signal);
        MessageHandler::SendMessage(sendSignalsEnvelope);
        return True;
    }

    return False;
}

OBJECTLOADREGISTER(EventCollectionGAM,"$Id: EventCollectionGAM.cpp,v 1.2 2009/01/14 13:09:21 fpiccolo Exp $")
