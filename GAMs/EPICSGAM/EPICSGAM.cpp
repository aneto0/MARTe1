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
/*
EPICS GAM is a data proxy from/to RTThread and EPICSLib
 
 secondo me bisogna farla a tipo GAM con la sincronizzazione 
 come visto c'è da risolvere il problema del postEvent che viene 
 gratis sull'epics thread ma non in MARTe
 
 
config parameters
subsampling to epics
thresholds 
bandwidth/deadbands
*/


#include "EPICSGAM.h"

#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"

#include "GCNString.h"
#include "MenuEntry.h"

EPICSGAM::EPICSGAM(){
    usecTime          = NULL;
    fastTrigger       = NULL;
    jpfData           = NULL;
    hasTriggerSignal  = False;
    acceptingMessages = False;
}

/* in GAMs usually instead of calling ObjectLoadSetup of the single GAM you can
 * call the super object ObjectLoadSetup that will call the redefined Initialize
 * method, so ObjectLoadSetup is not required
 */
//ObjectLoadSetup() {
bool EPICSGAM::Initialise(ConfigurationDataBase& cdbData) {

    acceptingMessages = False;
    CDBExtended cdb(cdbData);

    //////////////////////////
    // Add Time BaseSignal  //
    //////////////////////////
    
    if(!AddInputInterface(usecTime,"UsecTimeInterface")){
        AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s failed to add input interface InputInterface UsecTimeInterface",Name());
        return False;
    }

    FString timeBase;
    if(!cdb.ReadFString(timeBase,"UsecTimeSignalName")){
        AssertErrorCondition(InitialisationError, "EPICSGAM::Initialise: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }

    FString timeBaseType;
    if(!cdb.ReadFString(timeBaseType,"TimeSignalType","int32")){
        AssertErrorCondition(Warning, "EPICSGAM::Initialise: %s does not specify a TimeSignalType. Assuming int32", Name());
    }

    if(!usecTime->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())){
        AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),timeBase.Buffer());
        return False;
    }

    /////////////////////////////////////
    // Add Trigger Signal to Interface //
    /////////////////////////////////////

    if(cdb->Exists("TriggerSignalName")){

        if(!AddIOInterface(fastTrigger,"FastTriggerSignal",DDB_ReadMode|DDB_WriteMode)){
            AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s failed to add input interface InputInterface FastTriggerSignal",Name());
            return False;
        }

        FString triggerSignal;
        if(!cdb.ReadFString(triggerSignal,"TriggerSignalName")){
            AssertErrorCondition(InitialisationError, "EPICSGAM::Initialise: %s does not specify a TriggerSignalName", Name());
            return False;
        }

        if(!fastTrigger->AddSignal(triggerSignal.Buffer(), "int32")){
            AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),triggerSignal.Buffer());
            return False;
        }

        hasTriggerSignal = True;
    }

    //////////////////////////////////////
    // Add EPICS Signal List to Interface //
    //////////////////////////////////////

    if(!AddInputInterface(jpfData,"EPICSSignalList")){
        AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s failed to add input interface InputInterface JPFSignalList",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }
//jpfData is DDBInterface
    if(!jpfData->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),jpfData->InterfaceName());
        return False;
    }
    cdb->MoveToFather();

    // we move this initialization there to let the signal Table fetching "SignalServer" parameter
    if(!signalTable.Initialize(*jpfData, cdb)) {
        AssertErrorCondition(InitialisationError,"EPICSGAM::Initialise: %s: Failed to initialize signal Table",Name());
        return False;
    }
    
    acceptingMessages = True;
    return True;
} //--------------------------------------------------------------------------- EIPCSGAM::Initialize

bool EPICSGAM::Execute(GAM_FunctionNumbers functionNumber) {

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
    // Update the signals table
    signalTable.UpdateSignals( (char *)jpfData->Buffer(), usecTimeSample);
    
    switch(functionNumber){
        case GAMOnline:{
        	break;
        }
        case GAMPrepulse:{
            // Disable Handling of the Messages
            if(acceptingMessages) acceptingMessages = False;
            break;
        }
        case GAMPostpulse:{
            // Enable Handling of the Messages
            if(!acceptingMessages) acceptingMessages = True;
            break;
        }
    };

    // Clear Trigger Request
    if(fastTriggerRequested){
        int32 *fastTriggerSignal = (int32 *)fastTrigger->Buffer();
        *fastTriggerSignal       = 0;
        fastTrigger->Write();
    }

    return True;

}
//----------------------------------------------------------------------------- end Execute

/*
GCRTemplate<SignalInterface>  EPICSGAM::GetSignal(const FString &signalName){
    return dataCollector.GetSignalData(signalName);
};
*/

// still to do..
bool EPICSGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){

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
    if(messageContent == "LISTSIGNALS"){

        GCRTemplate<Message> addSignals(GCFT_Create);
        if(!addSignals.IsValid()){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: Failed creating Message", Name());
            return False;
        }

        addSignals->Init(0,"ADDSIGNAL");

        CDBExtended cdb;
/*        if(!dataCollector.ObjectSaveSetup(cdb)){
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: Failed gathering information about data collection", Name());
            return False;
        }
*/
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
            cdb.ReadFString(signalName, "Name");
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
    }
    else if ( messageContent == "INITSIGNAL" ) {

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
        if (!signalName.IsValid()) {
            errorMessage->Printf("Missing Signal Name");
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: No valid signalName has been specified ", Name());
            return False;
        }
/*
        GCRTemplate<SignalInterface>  signal = GetSignal(*(signalName.operator->()));
        if (!signal.IsValid()) {
            errorMessage->Printf("Signal %s not found in GAM %s", signalName->Buffer(), Name());
            sendSignal->Insert(errorMessage);
            MessageHandler::SendMessage(sendSignalsEnvelope);
            AssertErrorCondition(InitialisationError,"DataCollectionGAM::ProcessMessage: %s: GETSIGNALS: %s ", Name(), errorMessage->Buffer());
            return False;
        }

        sendSignal->Insert(signal);
        MessageHandler::SendMessage(sendSignalsEnvelope);
*/
        return True;
    }

    return False;
}
//----------------------------------------------------------------------------- end ProcessMessage

const char* EPICSGAM::css = "table.bltable {"
	"margin: 1em 1em 1em 2em;"
	"background: whitesmoke;"
	"border-collapse: collapse;"
	"}"
	"table.bltable th, table.bltable td {"
	"border: 1px silver solid;"
	"padding: 0.2em;"
	"}"
	"table.bltable th {"
	"background: gainsboro;"
	"text-align: left;"
	"}"
	"table.bltable caption {"
	"margin-left: inherit;"
	"margin-right: inherit;"
	"}";
// ----------------------------------------------------------------------------


#define TABLE_NEWROW hStream.Printf("<tr>\n")
#define TABLE_ENDROW hStream.Printf("</tr>\n")

bool EPICSGAM::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style></head><body>\n" );

    
    hStream.Printf("<h1> EPICSGAM EPICSSignalTable dump</h1>\n");
    hStream.Printf("<table class=\"bltable\">\n");
    EPICSSignal * ptrSignal = dynamic_cast<EPICSSignal*>( this->signalTable.List() );
    TABLE_NEWROW;
    hStream.Printf("<td>DDB name</td> <td>DDB offset</td> <td>DDB size</td> <td>DDB type</td>\n"
        			"<td>EPICS name</td> <td>EPICS id</td> <td>EPICS subsamples</td> <td>EPICS counter</td>\n");
    TABLE_ENDROW;
    while (ptrSignal) {
    	TABLE_NEWROW;
    	BString bsbuf;
    	hStream.Printf("<td>%s</td> <td>%d</td> <td>%d</td> <td>%s</td>\n"
    			"<td>%s</td> <td>%d</td> <td>%d</td> <td>%d</td>\n",
    			ptrSignal->GetDDBName(), ptrSignal->GetDDBOffset(),
    			ptrSignal->GetDDBSize(), (ptrSignal->GetDDBType()).ConvertToString(bsbuf),
    			
    			ptrSignal->GetEPICSName(), ptrSignal->GetEPICSIndex(),
    			ptrSignal->GetEPICSSubSample(), ptrSignal->counter);
    	TABLE_ENDROW;
    	
    	ptrSignal =dynamic_cast<EPICSSignal*>( ptrSignal->Next() );
    }
	hStream.Printf("</table>\n");
    hStream.Printf("<BR>\n");
   
    
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}
//----------------------------------------------------------------------------- end ProcessHttpMessage

OBJECTLOADREGISTER(EPICSGAM,"$Id: EPICSGAM.cpp,v 1.22 2011/06/26 10:10:10 abarb Exp $")
