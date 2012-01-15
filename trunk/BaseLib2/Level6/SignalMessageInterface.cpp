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
#include "SignalMessageInterface.h"
#include "GAM.h"
#include "GCNString.h"

/**  Send message to all objects of a given criteria */
class SignalLister : public IteratorT<GCReference> {
    /** where to write to */
    SignalMessageInterface *source;

public:

    /**  */
    SignalLister(SignalMessageInterface *source = NULL) {
        this->source = source;
    }

    virtual ~SignalLister() {
    }

    /** actual function */
    virtual void Do(GCReference data) {
        GCRTemplate<GAM> gcrtgam;
        GCRTemplate<MessageHandler> gcrtmhi;
        gcrtgam = data;
        gcrtmhi = data;
        if (gcrtgam.IsValid() && gcrtmhi.IsValid()) {
            BString destinationName;
            gcrtgam->GetUniqueName(destinationName);

            GCRTemplate<Message> gcrtm(GCFT_Create);
            gcrtm->Init(0, "LISTSIGNALS");
            GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
            gcrtme->PrepareMessageEnvelope(gcrtm, destinationName.Buffer(), MDRF_None, source);
            MessageHandler::SendMessage(gcrtme);
        }
    }

    /** actual function */
    virtual void Do2(GCReference data, SFTestType mode) {
        if (mode == SFTTNull) {
            Do(data);
        }
    }
};

/** Handles the message requests*/
bool SMIProcessMessage(SignalMessageInterface &smi, GCRTemplate<MessageEnvelope> envelope) {
    if (!envelope.IsValid()) {
        smi.AssertErrorCondition(FatalError, "%s::ProcessMessage: envelope is not valid!!", smi.Name());
        return False;
    }

    GCRTemplate<Message> gcrtm = envelope->GetMessage();

    if (!gcrtm.IsValid()) {
        smi.AssertErrorCondition(FatalError, "%s::ProcessMessage: (Message)gcrtm is not valid!!", smi.Name());
        return False;
    }

    int32 code = gcrtm->GetMessageCode().Code();
    if (code != 0) {
        smi.AssertErrorCondition(CommunicationError, "%s::ProcessMessage: only code = 0 accepted [processing message 0x%x from %s]", smi.Name(), code, envelope->Sender());
        return False;
    }

    const char *requestedAction = gcrtm->Content();
    smi.AssertErrorCondition(Information, "%s::ProcessMessage: processing message %s from %s]", smi.Name(), requestedAction, envelope->Sender());

    if (strcmp(requestedAction, "AUTODETECT") == 0) {
        if (smi.cache.IsValid()) {
            // clear the cache
            if (smi.cache->Size() > 0) smi.cache->CleanUp();
        }

        CDBExtended cdbx(smi.signalDataBase);
        cdbx->AddChildAndMove("Signals");
        cdbx->CleanUp(CDBAM_SubTreeOnly);

        SignalLister gmhsl(&smi);

        //We might not want to ask ALL objects in the GlobalObjectDataBase but instead ask only a sub-tree
        GCRTemplate<GCReferenceContainer> nonGlobalRootSearch = GetGlobalObjectDataBase()->Find(smi.signalSearchRoot.Buffer());
        if(nonGlobalRootSearch.IsValid()){
            nonGlobalRootSearch->Iterate(&gmhsl, GCFT_Recurse);
        }
        else{
            GetGlobalObjectDataBase()->Iterate(&gmhsl, GCFT_Recurse);
        }
        if(envelope->ManualReplyExpected()){
            GCRTemplate<Message> msg(GCFT_Create);
            if(!msg.IsValid()){
                smi.AssertErrorCondition(FatalError, "SignalInterface::SMIProcessMessage: %s: Failed creating response message for AUTODETECT", smi.Name());
                return False;
            }
            GCRTemplate<MessageEnvelope> mec(GCFT_Create);
            if (!mec.IsValid()){
                smi.AssertErrorCondition(FatalError, "SignalInterface::SMIProcessMessage: %s: Failed creating response message envelope for AUTODETECT", smi.Name());
                return False;
            }
            msg->Init(0, "OK");
            mec->PrepareReply(envelope,gcrtm);
            MessageHandler::SendMessage(mec);
        }
        return True;
    }

    if (strcmp(requestedAction, "ADDSIGNAL") == 0) {

        int i = 0;
        for (i = 0; i < gcrtm->Size(); i++) {
            GCRTemplate<GCNString> gcrtgcns = gcrtm->Find(i);

            if (!gcrtgcns.IsValid()) {

                smi.AssertErrorCondition(Warning, "%s::ProcessMessage: processing message %s from %s but found no GCNString in position %i", smi.Name(), requestedAction, envelope->Sender(), i);

            } else { // a valid GCNString

                CDBExtended cdb(smi.signalDataBase);

                if (!cdb->AddChildAndMove("Signals")) {
                    smi.AssertErrorCondition(FatalError, "%s::ProcessMessage: cdb cannot move to Signals", smi.Name());
                    return False;
                }

                if (!cdb->AddChildAndMove(gcrtgcns->Name())) {
                    smi.AssertErrorCondition(FatalError, "%s::ProcessMessage: cdb cannot create and move to %s", smi.Name(), gcrtgcns->Name());
                    return False;
                }

                smi.AssertErrorCondition(Information, "%s::ProcessMessage: Processing Signal %s", smi.Name(), gcrtgcns->Name());

                FString errors;
                if (!cdb->ReadFromStream(*(gcrtgcns.operator->()), &errors)) {
                    smi.AssertErrorCondition(FatalError, "%s::ProcessMessage: cdb cannot initialise section: %s", smi.Name(), errors.Buffer());
                    return False;
                }

                if (!cdb.WriteString(envelope->Sender(), "Owner")) {
                    smi.AssertErrorCondition(FatalError, "%s::ProcessMessage: cdb cannot create field Owner", smi.Name());
                    return False;
                }

            } // end check validity as GCNString
        } // end for
        return True;
    }

    smi.AssertErrorCondition(FatalError, "%s::ProcessMessage:Cannot execute %s request from %s", smi.Name(), requestedAction, envelope->Sender());
    return False;
}

GCRTemplate<SignalInterface> SMIGetSignal(SignalMessageInterface &smi, const char *signalName) {
    GCRTemplate<SignalInterface> signal;

    smi.signalMux.Lock();
    if (smi.cache.IsValid()) {
        signal = smi.cache->Find(signalName);
    }

    if (signal.IsValid()) {
        smi.signalMux.UnLock();
        return signal;
    }

    CDBExtended cdbx(smi.signalDataBase);

    if (!cdbx->Move("Signals")) {
        smi.AssertErrorCondition(FatalError, "%s::GetSignal: failed moving cdbx to Signals", smi.Name());
        smi.signalMux.UnLock();
        return signal;
    }

    if (!cdbx->Move(signalName)) {
        smi.AssertErrorCondition(FatalError, "%s::GetSignal: signal %s not found in Signals list", smi.Name(), signalName);
        smi.signalMux.UnLock();
        return signal;
    }

    FString owner;
    if (!cdbx.ReadFString(owner, "Owner")) {
        smi.AssertErrorCondition(FatalError, "%s::GetSignal: signal %s found but owner not specified!!", smi.Name(), signalName);
        smi.signalMux.UnLock();
        return signal;
    }

    smi.AssertErrorCondition(Information, "%s::SendCollectedData: Requesting Signal %s to GAM %s", smi.Name(), signalName, owner.Buffer());
    //
    // Prepare and send message
    //
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0, "GETSIGNAL");

    GCRTemplate<GCNString> messageContent(GCFT_Create);
    messageContent->Printf("%s", signalName);

    gcrtm->Insert(messageContent);
    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
    gcrtme->PrepareMessageEnvelope(gcrtm, owner.Buffer(), MDRF_ManualReply, &smi);

    GCRTemplate<MessageEnvelope> replyEnvelope;
    smi.SendMessageAndWait(gcrtme, replyEnvelope, smi.timeout);
    if (!replyEnvelope.IsValid()) {
        smi.AssertErrorCondition(FatalError, "%s::GetSignal: failed SendMessageAndWait GETSIGNAL to %s", smi.Name(), owner.Buffer());
        smi.signalMux.UnLock();
        return signal;
    }

    GCRTemplate<Message> replyMessage = replyEnvelope->GetMessage();
    replyEnvelope->CleanUp();

    if (!replyMessage.IsValid()) {
        smi.AssertErrorCondition(FatalError, "%s::GetSignal: no message in the reply to GETSIGNAL from %s", smi.Name(), owner.Buffer());
        smi.signalMux.UnLock();
        return signal;
    }

    signal = replyMessage->Find(signalName);

    if (!signal.IsValid()) {
        smi.AssertErrorCondition(FatalError, "%s::GetSignal: in the reply to GETSIGNAL from %s there is no Signal called %s", smi.Name(), owner.Buffer(), signalName);
        smi.signalMux.UnLock();
        return signal;
    }

    if (smi.cache.IsValid()) {
        if (smi.cache->Size() >= smi.maxCacheSize) {
            smi.cache->Remove(0);
            if (smi.cache->Size() >= smi.maxCacheSize) {
                smi.AssertErrorCondition(Warning, "%s::GetSignal: could not trim cache size to %i: now size = %i", smi.Name(), smi.maxCacheSize, smi.cache->Size());
            }
        }

        if (!smi.cache->Insert(signal)) {
            smi.AssertErrorCondition(Warning, "%s::GetSignal: could not cache signal %s", smi.Name(), signalName);
        }
    }
    smi.signalMux.UnLock();
    return signal;
}

