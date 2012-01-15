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

#include "WaveformCollectionGAM.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"
#include "GCNString.h"
#include "MenuEntry.h"

WaveformCollectionGAM::WaveformCollectionGAM(){
    usecTime           = NULL;
    jpfData            = NULL;
    acceptingMessages  = False;
    inputState         = NULL;
    differencesState   = NULL;
    triggerAcquisition = False;
    inputStateByteSize = 0;
    lastSampleUsecTime = -1;
    sampleNumber       = 0;
    maxProportionalVariation = 1e-6;
    maxAbsoluteVariation     = 1e-12;

}

bool WaveformCollectionGAM::Initialise(ConfigurationDataBase& cdbData){

    acceptingMessages = False;
    CDBExtended cdb(cdbData);


    cdb.ReadDouble(maxProportionalVariation,"MaxProportionalVariation",1e-6);
    cdb.ReadDouble(maxAbsoluteVariation    ,"MaxAbsoluteVariation"    ,1e-12);

    //////////////////////////
    // Add Time BaseSignal //
    //////////////////////////

    if(!AddInputInterface(usecTime,"UsecTimeInterface")){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s failed to add input interface InputInterface UsecTimeInterface",Name());
        return False;
    }

    FString timeBase;
    if(!cdb.ReadFString(timeBase,"UsecTimeSignalName")){
        AssertErrorCondition(InitialisationError, "WaveformCollectionGAM::Initialise: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }

    FString timeBaseType;
    if(!cdb.ReadFString(timeBaseType,"TimeSignalType","int32")){
        AssertErrorCondition(Warning, "WaveformCollectionGAM::Initialise: %s does not specify a TimeSignalType. Assuming int32", Name());
    }

    if(!usecTime->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),timeBase.Buffer());
        return False;
    }

    //////////////////////////////////////
    // Add JPF Signal List to Interface //
    //////////////////////////////////////

    if(!AddInputInterface(jpfData,"JPFSignalList")){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s failed to add input interface InputInterface JPFSignalList",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    if(!jpfData->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),jpfData->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    inputStateByteSize = sizeof(uint32) * jpfData->BufferWordSize();
    inputState         =  (float *)malloc(inputStateByteSize);
    if(inputState == NULL){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s: Failed allocating space for the input state of interface %s",Name(),jpfData->InterfaceName());
        return False;
    }
    memset(inputState,0,inputStateByteSize);

    differencesState   =  (float *)malloc(inputStateByteSize);
    if(differencesState == NULL){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s: Failed allocating space for the differences state of interface %s",Name(),jpfData->InterfaceName());
        return False;
    }
    memset(differencesState,0,inputStateByteSize);

    /////////////////////////////////////
    // Init RTDataCollector Parameters //
    /////////////////////////////////////

    if(!dataCollector.ObjectLoadSetup(cdb,NULL, jpfData)){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::Initialise: %s: dataCollector ObjectLoadSetup Failed",Name());
        return False;
    }

    acceptingMessages = True;
    return True;
}

bool WaveformCollectionGAM::Execute(GAM_FunctionNumbers functionNumber){

    // Read Cycle Time
    usecTime->Read();
    int32 *timePointer   = (int32 *)usecTime->Buffer();
    int32 usecTimeSample = timePointer[0];

    // Read Jpf Data
    jpfData->Read();
    const float *jpfDataBuffer = (float *)(jpfData->Buffer());

    // Calculate current differences
    for(int i = 1; i < (inputStateByteSize/sizeof(int32)); i++){
        float currentDifference = jpfDataBuffer[i] - inputState[i];

        /* | x'(T) - x'ref| < maxPropVar |x'ref|+ maxVar; */
        if (fabs(currentDifference - differencesState[i]) >
            ( maxProportionalVariation * fabs(inputState[i]) + maxAbsoluteVariation )
           )
        { // If so we should acquire this sample
            triggerAcquisition = True;
        }
        differencesState[i] = currentDifference;
    }

    // Main loop
    switch(functionNumber){
        case GAMOnline:{
            switch (sampleNumber) {
                case 0:{
                    sampleNumber++;
                }break;

                case 1:{// Second sample: store first value
                    if(!dataCollector.StoreData((uint32*)inputState, lastSampleUsecTime, True)){
                        AssertErrorCondition(FatalError,"WaveformCollectionGAM::%s: Execute(GAMOffline | GAMOnline) Failed", Name());
                        return False;
                    }
                    sampleNumber++;
                }break;

                default:{//Otherwise use the standard logic
                    if(!dataCollector.StoreData((uint32*)inputState, lastSampleUsecTime, triggerAcquisition)){
                        AssertErrorCondition(FatalError,"WaveformCollectionGAM::%s: Execute(GAMOffline | GAMOnline) Failed", Name());
                        return False;
                    }
                }break;
            }
            lastSampleUsecTime = usecTimeSample;
        }break;

        case GAMPrepulse:{
            // Disable Handling of the Messages
            if(acceptingMessages) acceptingMessages = False;

            if(!dataCollector.PrepareForNextPulse()){
                AssertErrorCondition(FatalError,"WaveformCollectionGAM::%s: Execute(GAMPrepulse) Failed", Name());
                return False;
            }

            // This way the first sample will always be acquired
            triggerAcquisition = True;
            lastSampleUsecTime = 0;
            sampleNumber       = 0;
            return True;
        }break;

        case GAMPostpulse:{
            // Write last point
            if(!dataCollector.StoreData((const uint32*)inputState, usecTimeSample, True)){
                AssertErrorCondition(FatalError,"WaveformCollectionGAM::%s: Error writing last sample at postpulse", Name());
                return False;
            }

            // Enable Handling of the Messages
            if(!acceptingMessages) acceptingMessages = True;

            if(!dataCollector.CompleteDataCollection()){
                AssertErrorCondition(FatalError,"WaveformCollectionGAM::%s: Execute(GAMPostpulse) Failed", Name());
                return False;
            }

        }break;
    };

    // Copy the present inputs
    memcpy(inputState, jpfDataBuffer ,inputStateByteSize);
    triggerAcquisition = False;
    return True;

};

GCRTemplate<SignalInterface>  WaveformCollectionGAM::GetSignal(const FString &signalName){
    return dataCollector.GetSignalData(signalName);
};

bool WaveformCollectionGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){

    if(!acceptingMessages){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: State Machine is not a valid type", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (!message.IsValid()){
        AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: Received invalid Message", Name());
        return False;
    }

    FString messageContent = message->Content();
    FString messageSender  = envelope->Sender();

    // GAP Message Request
    if(messageContent == "LISTSIGNALS"){

        GCRTemplate<Message> addSignals(GCFT_Create);
        if(!addSignals.IsValid()){
            AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: Failed creating Message", Name());
            return False;
        }

        addSignals->Init(0,"ADDSIGNAL");

        CDBExtended cdb;
        if(!dataCollector.ObjectSaveSetup(cdb)){
            AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: Failed gathering information about data collection", Name());
            return False;
        }

        int32 nOfAcquiredSignals = 0;
        cdb.ReadInt32(nOfAcquiredSignals, "NOfChannels");

        if(!cdb->Move("Signals")){
            AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: Signals entry not available in the data base", Name());
            return False;
        }

        for(int nSignals = 0; nSignals < nOfAcquiredSignals; nSignals++){
            GCRTemplate<GCNString> signalDescription(GCFT_Create);
            if(!signalDescription.IsValid()){
                AssertErrorCondition(FatalError,"WaveformCollectionGAM::ProcessMessage: %s: Failed creating GCNString", Name());
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
            //AssertErrorCondition(Information,"WaveformCollectionGAM::ProcessMessage: %s: Adding Signal %s to the collection list", Name(), signalName.Buffer());
        }


        GCRTemplate<MessageEnvelope> addSignalsEnvelope(GCFT_Create);
        if(!addSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"WaveformCollectionGAM::ProcessMessage: %s: Failed creating MessageEnvelope", Name());
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
            AssertErrorCondition(FatalError,"WaveformCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating Message", Name());
            return False;
        }

        sendSignal->Init(0,"SIGNAL");

        GCRTemplate<GCNString>  errorMessage(GCFT_Create);
        if(!errorMessage.IsValid()){
            AssertErrorCondition(FatalError,"WaveformCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating GCNString", Name());
            return False;
        }

        errorMessage->SetObjectName("ERROR");

        GCRTemplate<MessageEnvelope> sendSignalsEnvelope(GCFT_Create);
        if(!sendSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"WaveformCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating MessageEnvelope", Name());
            return False;
        }

        sendSignalsEnvelope->PrepareReply(envelope, sendSignal);

        GCRTemplate<GCNString>  signalName = message->Find(0);
        if(!signalName.IsValid()){
            errorMessage->Printf("Missing Signal Name");
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: GETSIGNALS: No valid signalName has been specified ", Name());
            return False;
        }

        GCRTemplate<SignalInterface>  signal = GetSignal(*(signalName.operator->()));
        if (!signal.IsValid()){
            errorMessage->Printf("Signal %s not found in GAM %s", signalName->Buffer(), Name());
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"WaveformCollectionGAM::ProcessMessage: %s: GETSIGNALS: %s ", Name(), errorMessage->Buffer());
            return False;
        }

        sendSignal->Insert(signal);
        MessageHandler::SendMessage(sendSignalsEnvelope);
        return True;
    }

    return False;
}

OBJECTLOADREGISTER(WaveformCollectionGAM,"$Id: WaveformCollectionGAM.cpp,v 1.4 2009/06/29 17:50:04 rvitelli Exp $")
