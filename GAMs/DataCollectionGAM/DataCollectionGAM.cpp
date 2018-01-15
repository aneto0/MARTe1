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


#include "DataCollectionGAM.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"
#include "GCNString.h"
#include "MenuEntry.h"

DataCollectionGAM::DataCollectionGAM(){
    usecTime          = NULL;
    fastTrigger       = NULL;
    jpfData           = NULL;
    hasTriggerSignal  = False;
    acceptingMessages = False;
}

bool DataCollectionGAM::Initialise(ConfigurationDataBase& cdbData){

    acceptingMessages = False;
    CDBExtended cdb(cdbData);

    //////////////////////////
    // Add Time BaseSignal //
    //////////////////////////

    if(!AddInputInterface(usecTime,"UsecTimeInterface")){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s failed to add input interface InputInterface UsecTimeInterface",Name());
        return False;
    }

    FString timeBase;
    if(!cdb.ReadFString(timeBase,"UsecTimeSignalName")){
        AssertErrorCondition(InitialisationError, "DataCollectionGAM::Initialise: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }

    FString timeBaseType;
    if(!cdb.ReadFString(timeBaseType,"TimeSignalType","int32")){
        AssertErrorCondition(Warning, "DataCollectionGAM::Initialise: %s does not specify a TimeSignalType. Assuming int32", Name());
    }

    if(!usecTime->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),timeBase.Buffer());
        return False;
    }

    /////////////////////////////////////
    // Add Trigger Signal to Interface //
    /////////////////////////////////////

    if(cdb->Exists("TriggerSignalName")){

        if(!AddIOInterface(fastTrigger,"FastTriggerSignal",DDB_ReadMode|DDB_WriteMode)){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s failed to add input interface InputInterface FastTriggerSignal",Name());
            return False;
        }

        FString triggerSignal;
        if(!cdb.ReadFString(triggerSignal,"TriggerSignalName")){
            AssertErrorCondition(InitialisationError, "DataCollectionGAM::Initialise: %s does not specify a TriggerSignalName", Name());
            return False;
        }

        if(!fastTrigger->AddSignal(triggerSignal.Buffer(), "int32")){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),triggerSignal.Buffer());
            return False;
        }

        hasTriggerSignal = True;
    }

    //////////////////////////////////////
    // Add JPF Signal List to Interface //
    //////////////////////////////////////

    if(!AddInputInterface(jpfData,"JPFSignalList")){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s failed to add input interface InputInterface JPFSignalList",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    if(!jpfData->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),jpfData->InterfaceName());
        return False;
    }

    cdb->MoveToFather();


    /////////////////////////////////////
    // Init RTDataCollector Parameters //
    /////////////////////////////////////

    if(!dataCollector.ObjectLoadSetup(cdb,NULL, jpfData)){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s: dataCollector ObjectLoadSetup Failed",Name());
        return False;
    }

    /////////////////////////////
    // Prepare Menu Interfaces //
    /////////////////////////////

    /** Menu System for Time Base setting*/
    GCRTemplate <MenuEntry>            dataCollectionTimeBaseMenu(GCFT_Create);
    if(!dataCollectionTimeBaseMenu.IsValid()){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::Initialise: %s: Failed creating menu interface for setting the time base.",Name());
        return False;
    }

    dataCollectionTimeBaseMenu->SetTitle("Modify Collection Time Base");
    dataCollectionTimeBaseMenu->SetUp(ModifyTimeBase,NULL,NULL,&dataCollector);
    menu->Insert(dataCollectionTimeBaseMenu);

    acceptingMessages = True;
    return True;
}

bool DataCollectionGAM::Execute(GAM_FunctionNumbers functionNumber){

    // Read Cycle Time
    usecTime->Read();
    int32 *timePointer   = (int32 *)usecTime->Buffer();
    int32 usecTimeSample = timePointer[0];

    // Get the Trigger Request
    bool  fastTriggerRequested = False;
    if(hasTriggerSignal){
        fastTrigger->Read();
        int32 fastTriggerSignal =  *((int32 *)fastTrigger->Buffer());
        fastTriggerRequested = (fastTriggerSignal != 0);
    }

    // Read Jpf Data
    jpfData->Read();
    uint32 *jpfDataBuffer = (uint32 *)(jpfData->Buffer());

    switch(functionNumber){
        case GAMOnline:{
            if(!dataCollector.StoreData(jpfDataBuffer, usecTimeSample, fastTriggerRequested)){
                AssertErrorCondition(FatalError,"DataCollectionGAM::%s: Execute(GAMOffline | GAMOnline) Failed", Name());
                return False;
            }
        }break;

        case GAMCheck:{
            if(!dataCollector.PrepareForNextPulse()){
                AssertErrorCondition(FatalError,"DataCollectionGAM::%s: Execute(GAMPrepulse) Failed", Name());
                return False;
            }
        }break;

        case GAMPrepulse:{
            // Disable Handling of the Messages
            if(acceptingMessages) acceptingMessages = False;
        }break;

        case GAMPostpulse:{
            // Enable Handling of the Messages
            if(!acceptingMessages) acceptingMessages = True;

            if(!dataCollector.CompleteDataCollection()){
                AssertErrorCondition(FatalError,"DataCollectionGAM::%s: Execute(GAMPostpulse) Failed", Name());
                return False;
            }
        }break;
    };


    // Clear Trigger Request
    if(fastTriggerRequested){
        int32 *fastTriggerSignal = (int32 *)fastTrigger->Buffer();
        *fastTriggerSignal       = 0;
        fastTrigger->Write();
    }

    return True;

};

GCRTemplate<SignalInterface>  DataCollectionGAM::GetSignal(const FString &signalName){
    return dataCollector.GetSignalData(signalName);
};

bool DataCollectionGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){

    if(!acceptingMessages){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: DataCollectionGAM is not accepting messages yet", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (!message.IsValid()){
        AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: Received invalid Message", Name());
        return False;
    }

    FString messageContent = message->Content();
    FString messageSender  = envelope->Sender();

    // GAP Message Request
    if(messageContent == "LISTSIGNALS" || messageContent == "LISTSIGNALSDDB"){

        GCRTemplate<Message> addSignals(GCFT_Create);
        if(!addSignals.IsValid()){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: Failed creating Message", Name());
            return False;
        }

        addSignals->Init(0,"ADDSIGNAL");

        CDBExtended cdb;
        if(!dataCollector.ObjectSaveSetup(cdb)){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: Failed gathering information about data collection", Name());
            return False;
        }

        int32 nOfAcquiredSignals = 0;
        cdb.ReadInt32(nOfAcquiredSignals, "NOfChannels");

        if(!cdb->Move("Signals")){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: Signals entry not available in the data base", Name());
            return False;
        }

        for(int nSignals = 0; nSignals < nOfAcquiredSignals; nSignals++){
            GCRTemplate<GCNString> signalDescription(GCFT_Create);
            if(!signalDescription.IsValid()){
                AssertErrorCondition(FatalError,"DataCollectionGAM::ProcessMessage: %s: Failed creating GCNString", Name());
                return False;
            }

            cdb->MoveToChildren(nSignals);
            FString signalName;
	    if(messageContent == "LISTSIGNALS") {
	      cdb.ReadFString(signalName, "Name");
	    } else if(messageContent == "LISTSIGNALSDDB") {
	      cdb.ReadFString(signalName, "DDBName");
	    }
            signalDescription->SetObjectName(signalName.Buffer());
            cdb->WriteToStream(*(signalDescription.operator->()));
            signalDescription->Seek(0);
            addSignals->Insert(signalDescription);
            cdb->MoveToFather();
            //AssertErrorCondition(Information,"DataCollectionGAM::ProcessMessage: %s: Adding Signal %s to the collection list", Name(), signalName.Buffer());
        }


        GCRTemplate<MessageEnvelope> addSignalsEnvelope(GCFT_Create);
        if(!addSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"DataCollectionGAM::ProcessMessage: %s: Failed creating MessageEnvelope", Name());
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
            AssertErrorCondition(FatalError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating Message", Name());
            return False;
        }

        sendSignal->Init(0,"SIGNAL");

        GCRTemplate<GCNString>  errorMessage(GCFT_Create);
        if(!errorMessage.IsValid()){
            AssertErrorCondition(FatalError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating GCNString", Name());
            return False;
        }

        errorMessage->SetObjectName("ERROR");

        GCRTemplate<MessageEnvelope> sendSignalsEnvelope(GCFT_Create);
        if(!sendSignalsEnvelope.IsValid()){
            AssertErrorCondition(FatalError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: Failed creating MessageEnvelope", Name());
            return False;
        }

        sendSignalsEnvelope->PrepareReply(envelope, sendSignal);

        GCRTemplate<GCNString>  signalName = message->Find(0);
        if(!signalName.IsValid()){
            errorMessage->Printf("Missing Signal Name");
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: No valid signalName has been specified ", Name());
            return False;
        }

        GCRTemplate<SignalInterface>  signal = GetSignal(*(signalName.operator->()));

        if (!signal.IsValid()){
            errorMessage->Printf("Signal %s not found in GAM %s", signalName->Buffer(), Name());
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: %s ", Name(), errorMessage->Buffer());
            return False;
        }

        sendSignal->Insert(signal);
        MessageHandler::SendMessage(sendSignalsEnvelope);
        return True;
    }

    return False;
}

bool DataCollectionGAM::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf("<html><head><title>DataCollectionGAM - %s</title></head><body>", Name());

    dataCollector.HTMLInfo(hStream);

    hStream.Printf("</table></body></html>");
    return True;
}

int DataCollectionGAM::GetTotalSamplesCollected(){
  return dataCollector.GetTotalSamplesCollected();
}

OBJECTLOADREGISTER(DataCollectionGAM,"$Id$")
