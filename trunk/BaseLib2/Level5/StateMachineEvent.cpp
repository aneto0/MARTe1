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

#include "StateMachine.h"
#include "StateMachineEvent.h"
#include "StateMachineState.h"
#include "MessageEnvelope.h"
#include "MessageDispatcher.h"
#include "CDBExtended.h"

OBJECTLOADREGISTER(StateMachineEvent,"$Id$")


bool SMEObjectLoadSetup(StateMachineEvent &sme,ConfigurationDataBase &info,StreamInterface *err){

    bool ret = sme.GCReferenceContainer::ObjectLoadSetup(info,err);

    CDBExtended cdbx(info);

    bool ret1 = cdbx.ReadBString(sme.value,"Value","");
    bool ret2 = cdbx.ReadInt32 (sme.code,"Code",0);
    // if code is not specified then check for UserCode
    if (!ret2){
        if (cdbx.ReadInt32 (sme.code,"UserCode",0)){
            ret2 = True;
            sme.code += UserMessageCode.Code();
        }
    }
    if (!(ret1 | ret2)){
        sme.AssertErrorCondition(ParametersError,"ObjectLoadSetup: Either Value or Code MUST be specified");
        ret = False;
    }

    if (!cdbx.ReadBString(sme.nextState,"NextState","ERROR")){
        sme.AssertErrorCondition(ParametersError,"ObjectLoadSetup: NextState MUST be specified");
        ret = False;
    }
    if (!cdbx.ReadBString(sme.errorState,"ErrorState","ERROR")){
        sme.AssertErrorCondition(Warning,"ObjectLoadSetup: ErrorState is not specified assuming ERROR");
    }

    return ret;

}

bool SMEObjectSaveSetup(StateMachineEvent &sme,ConfigurationDataBase &info,StreamInterface *err){

    return sme.GCReferenceContainer::ObjectSaveSetup(info,err);

}

/** Performs the action associate with a trigger */
bool StateMachineEvent::Trigger(
                                StateMachine &                  sm,
                                int32                           code,
                                const char *                    value,
                                StateMachineState &             currentState,
                                FString &                       newStateName,
                                int32                           verboseLevel)
{
    // It is a DEFAULT action ,
    // matching code and !=0,
    // matching value and not null,
    if (((code == this->code) && (this->code != 0))                    ||
        ((value != NULL) && (value[0] != 0) && (this->value == value)) ||
        (strncasecmp(Name(),"DEFAULT",20)==0)
        ){

        // no content -> no action
        if (Size() == 0){
            if (verboseLevel > 2) {
                AssertErrorCondition(Information,"Trigger: event %s has no actions",Name());
            }
        }

        // for each element in the container
        for (int i = 0;i < Size(); i++){
            GCRTemplate<GCNamedObject> gcno = Find(i);
            if (gcno.IsValid()){

                GCRTemplate<MessageEnvelope> me = gcno;
                if (me.IsValid()){

                    //SENDSTATE
                    GCRTemplate<Message>gcrtm = me->Find("SENDSTATE");
                    if (gcrtm.IsValid()){
                        GCRTemplate<Message> gcrtm(GCFT_Create);

                        if (!gcrtm.IsValid()){
                            AssertErrorCondition(FatalError,"Trigger: Failed creating message for SENDSTATE");
                            return False;
                        }

                        if (sm.currentState.IsValid()){
                            gcrtm->Init(sm.currentState->StateCode(),sm.currentState->Name());
                        } else {
                            AssertErrorCondition(FatalError,"Trigger: sm->currentState is not valid! using provided state instead ");
                            gcrtm->Init(currentState.StateCode(),currentState.Name());
                        }

                        if (sm.verboseLevel >= 10) {
                            AssertErrorCondition(Information,"Trigger: sending state(%i,%s) to %s",gcrtm->GetMessageCode().Code(),gcrtm->Content(),me->Destination());
                        }

                        GCRTemplate<MessageEnvelope> mec(GCFT_Create);
                        if (!mec.IsValid()){
                            AssertErrorCondition(FatalError,"Trigger: Failed creating copy for SENDSTATE");
                            return False;
                        }
                        mec->PrepareMessageEnvelope(gcrtm,me->Destination(),MDRF_None,&sm);
                        // replace with copy
                        me = mec;
                    } else {
                        // associate sender
                        me->SetSender(sm);
                    }

                    MessageHandler::SendMessage(me);
//                    GMDSendMessageEnvelope(me);
                    if (verboseLevel > 2){
                        AssertErrorCondition(Information,"Trigger: executing %s request on event %s",me->Name() ,Name());
                    }
                } else {

                    GCRTemplate<MessageDeliveryRequest> mdr = gcno;
                    if (mdr.IsValid()){

                        //SENDSTATE
                        GCRTemplate<Message>gcrtm = mdr->GetMessage();
                        if (gcrtm.IsValid())
                        if (strcmp(gcrtm->Name(),"SENDSTATE")==0){
                            GCRTemplate<Message> gcrtm(GCFT_Create);

                            if (!gcrtm.IsValid()){
                                AssertErrorCondition(FatalError,"Trigger: Failed creating mdr message for SENDSTATE");
                                return False;
                            }

                            if (sm.currentState.IsValid()){
                                gcrtm->Init(sm.currentState->StateCode(),sm.currentState->Name());
                            } else {
                                AssertErrorCondition(FatalError,"Trigger: sm->currentState is not valid! using provided state instead ");
                                gcrtm->Init(currentState.StateCode(),currentState.Name());
                            }

                            if (sm.verboseLevel >= 10) {
                                AssertErrorCondition(Information,"Trigger: sending state(%i,%s) to %s",gcrtm->GetMessageCode().Code(),gcrtm->Content(),mdr->Destinations());
                            }

                            GCRTemplate<MessageDeliveryRequest> mdrc(GCFT_Create);
                            if (!mdrc.IsValid()){
                                AssertErrorCondition(FatalError,"Trigger: Failed creating mdr copy for SENDSTATE");
                                return False;
                            }
                            mdrc->PrepareMDR(gcrtm,mdr->Destinations(),mdr->Flags(),&sm,mdr->MsecTimeout());
                            // replace with copy
                            mdr = mdrc;
                        } else {
                            // associate sender
                            mdr->SetSender(sm);
                        }

                        if (!GMDSendMessageDeliveryRequest(mdr,1000)){
                            AssertErrorCondition(Timeout,"Trigger: timeout executing %s request on event %s",mdr->Name() ,Name());
                            newStateName = errorState.Buffer();
                            return True;
                        } else {
                            if (verboseLevel > 2){
                                AssertErrorCondition(Information,"Trigger: executing %s request on event %s",mdr->Name() ,Name());
                            }
                        }
                    } else {
                        AssertErrorCondition(ParametersError,"Trigger: event %s has action %s of unknown type %s",Name(),gcno->Name(),gcno->ClassName());
                    }
                }
            } else {
                AssertErrorCondition(ParametersError,"Trigger: event %s has action #%i of unknown name and type ",Name(),i);
            }
        }
        newStateName = nextState.Buffer();
        return True;
    }
    return False;
}


