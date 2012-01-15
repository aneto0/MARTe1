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

#include "StateMachineState.h"
#include "StateMachine.h"

OBJECTLOADREGISTER(StateMachineState,"$Id$")

bool SMSObjectLoadSetup(StateMachineState &sms,ConfigurationDataBase &info,StreamInterface *err){

    if (!sms.GCReferenceContainer::ObjectLoadSetup(info,err)) return False;

    return True;

}

bool SMSObjectSaveSetup(StateMachineState &sms,ConfigurationDataBase &info,StreamInterface *err){

    return sms.GCReferenceContainer::ObjectSaveSetup(info,err);

}

/* Perform the activity called @param name */
SMS_ActOnResults StateMachineState::ActOn(
                                StateMachine &                  sm,
                                const char *                    name)
{
    GCRTemplate<GCNamedObject> enterInstruction = Find(name);
    // no enter instruction
    if (!enterInstruction.IsValid()) return SMS_NotFound;

    GCRTemplate<MessageEnvelope> me = enterInstruction;
    if (me.IsValid()){

        //SENDSTATE
        GCRTemplate<Message>gcrtm = me->Find("SENDSTATE");
        if (gcrtm.IsValid()){
            GCRTemplate<Message> gcrtm(GCFT_Create);
            if (!gcrtm.IsValid()){
                AssertErrorCondition(FatalError,"ActOn: Failed creating message for SENDSTATE");
                return SMS_Error;
            }
            if (sm.currentState.IsValid()){
                gcrtm->Init(sm.currentState->StateCode(),sm.currentState->Name());
            } else {
                AssertErrorCondition(FatalError,"ActOn: sm->currentState is not valid! using this state instead ");
                gcrtm->Init(this->StateCode(),this->Name());
            }

            if (sm.verboseLevel >= 10) {
                AssertErrorCondition(Information,"ActOn: sending state(%i,%s) to %s",gcrtm->GetMessageCode().Code(),gcrtm->Content(),me->Destination());
            }

            GCRTemplate<MessageEnvelope> mec(GCFT_Create);
            if (!mec.IsValid()){
                AssertErrorCondition(FatalError,"ActOn: Failed creating copy for SENDSTATE");
                return SMS_Error;
            }
            mec->PrepareMessageEnvelope(gcrtm,me->Destination(),MDRF_None,&sm);
            // replace with copy
            me = mec;
        } else {
            // associate sender
            me->SetSender(sm);
        }

        MessageHandler::SendMessage(me);
        return SMS_Ok;
    }

    GCRTemplate<MessageDeliveryRequest> mdr = enterInstruction;
    if (mdr.IsValid()){

        //SENDSTATE
        GCRTemplate<Message>gcrtm = mdr->GetMessage();
        if (gcrtm.IsValid()) {
            if (strcmp(gcrtm->Name(),"SENDSTATE")==0){
                GCRTemplate<Message> gcrtm(GCFT_Create);

                if (!gcrtm.IsValid()){
                    AssertErrorCondition(FatalError,"Trigger: Failed creating mdr message for SENDSTATE");
                    return SMS_Error;
                }

                if (sm.currentState.IsValid()){
                    gcrtm->Init(sm.currentState->StateCode(),sm.currentState->Name());
                } else {
                    AssertErrorCondition(FatalError,"ActOn: sm->currentState is not valid! using this state instead ");
                    gcrtm->Init(this->StateCode(),this->Name());
                }

                if (sm.verboseLevel >= 10) {
                    AssertErrorCondition(Information,"ActOn: sending state(%i,%s) to %s",gcrtm->GetMessageCode().Code(),gcrtm->Content(),mdr->Destinations());
                }

                GCRTemplate<MessageDeliveryRequest> mdrc(GCFT_Create);
                if (!mdrc.IsValid()){
                    AssertErrorCondition(FatalError,"Trigger: Failed creating mdr copy for SENDSTATE");
                    return SMS_Error;
                }
                mdrc->PrepareMDR(gcrtm,mdr->Destinations(),mdr->Flags(),&sm,mdr->MsecTimeout());
                // replace with copy
                mdr = mdrc;
            } else {
                // associate sender
                mdr->SetSender(sm);
            }
        }

        if (GMDSendMessageDeliveryRequest(mdr)){
            return SMS_Ok;
        } else {
            return SMS_Error;
        }
    }

    AssertErrorCondition(FatalError,"ActOn: %s object is of type %s (not MessageEnvelope or MDR)", name,enterInstruction->ClassName());
    return SMS_Error;

}

/** Find this trigger and execute the associated actions
    returns the new state */
bool StateMachineState::Trigger(
                                StateMachine &                  sm,
                                int32                           code,
                                const char *                    value,
                                FString &                       newStateName,
                                int32                           verboseLevel)
{
    for (int i = 0;i < Size(); i++){
        GCRTemplate<StateMachineEvent> sme = Find(i);

        if (sme.IsValid()){
            bool ret = sme->Trigger(sm,code,value,*this,newStateName,verboseLevel);
            if (ret) {
                return True;
            }
        }
    }

    AssertErrorCondition(CommunicationError,"Trigger: State %s Event (%i %s) unmatched", Name(),code,value);

    return False;
}

