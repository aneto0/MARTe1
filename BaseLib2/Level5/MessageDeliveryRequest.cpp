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

#include "GCReferenceContainer.h"
#include "MessageDeliveryRequest.h"
#include "MessageDispatcher.h"
#include "CDBExtended.h"
#include "MDRFlags.h"


OBJECTLOADREGISTER(MessageDeliveryRequest,"$Id$");


bool
MDRObjectLoadSetup(
            MessageDeliveryRequest &    mdr,
            ConfigurationDataBase &     info,
            StreamInterface *           err){

    CDBExtended cdbx(info);
    if (!mdr.GCNamedObject::ObjectLoadSetup(info,err)){
        mdr.AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot retrieve name of node");
        return False;
    }

    if (!cdbx.ReadBString(mdr.sender,"Sender","None")){
        mdr.AssertErrorCondition(Warning,"ObjectLoadSetup:Cannot find Sender for this MessageDeliveryRequest");
    }

    if (!cdbx.ReadBString(mdr.destinations,"Destinations","None")){
        mdr.AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot find Destinations for this MessageDeliveryRequest");
        return False;
    }

    int32 timeout;
    if (!cdbx.ReadInt32(timeout,"MsecTimeOut",TTInfiniteWait.msecTimeout)){
        mdr.AssertErrorCondition(Warning,"ObjectLoadSetup:Cannot find MsecTimeOut for this MessageDeliveryRequest");
    } else {
        mdr.msecTimeout = timeout;
    }

    const char *idList[5] ={
        "EarlyAutomaticReply",
        "LateAutomaticReply",
        "NoReply",
        "ManualReply",
        NULL
    };
    const int32 idValues[5] ={
        MDRF_EarlyAutomaticReply.flags,
        MDRF_LateAutomaticReply.flags,
        MDRF_NoReply.flags,
        MDRF_ManualReply.flags,
        NULL
    };

    int32 value;
    if (cdbx.ReadOptions(value,"Flags",idList,idValues,MDRF_NoReply.flags)){
        mdr.flags = MDRFlags(value);
    } else {
        mdr.AssertErrorCondition(Warning,"ObjectLoadSetup:Cannot find Flags for this MessageDeliveryRequest: assuming NoReply");
        mdr.flags = MDRF_NoReply;
    }

    bool ret = True;
    if (info->Move("Message")){

        ret = mdr.message.ObjectLoadSetup(info,err);

        info->MoveToFather();
    }

    if (!ret){
        mdr.AssertErrorCondition(FatalError,"ObjectLoadSetup: Messgae::ObjectLoadSetup failed");
    }

    return ret;
}

bool
MDRObjectSaveSetup(
            MessageDeliveryRequest &    mdr,
            ConfigurationDataBase &     info,
            StreamInterface *           err){

    CDBExtended cdbx(info);

    if (!cdbx.WriteString(mdr.sender.Buffer(),"Sender")){
        mdr.AssertErrorCondition(Warning,"ObjectSaveSetup:Cannot write Sender for this MessageEnvelope");
    }

    if (!cdbx.WriteString(mdr.sender.Buffer(),"Destination")){
        mdr.AssertErrorCondition(FatalError,"ObjectSaveSetup:Cannot write Destination for this MessageEnvelope");
        return False;
    }

    bool ret = True;
    if (info->AddChildAndMove("Message")){

        ret = mdr.message.ObjectSaveSetup(info,err);

        info->MoveToFather();
    }

    if (!ret){
        mdr.AssertErrorCondition(FatalError,"ObjectSaveSetup: GCReferenceContainer::ObjectSaveSetup failed");
    }

    return ret;

}


/** send the message @param message to the object @param destination
    if destination starts with :: the name search scope is the global object container
    otherwise it is the message reciever container */
bool MDRPrepareMDR(
                        MessageDeliveryRequest &        mdr,
                        GCRTemplate<Message>            message,
                        const char *                    destinations,
                        MDRFlags                        flags,
                        GCNamedObject *                 source)

{
    // Do not replace the user specified sender
    if(strcmp(mdr.sender.Buffer(), "None") == 0){
        if (source != NULL){
            source->GetUniqueName(mdr.sender);
        } else {
            mdr.sender = "Unknown";
        }
    }

    mdr.message = message;

    mdr.destinations = destinations;

    mdr.flags = flags;

    return True;

}

/** Prepares Envelopes and delivers them */
bool MessageDeliveryRequest::ProcessMDR(MessageDispatcher *md)
{
    // container for the letters
//    GCRContainerTemplate<MessageEnvelope> envelopes;
    GCReferenceContainer envelopes;

    FString destinations;
    destinations = this->destinations.Buffer();

    // prepare all the envlopes
    numberOfDestinations = 0;
    FString token;
    while (destinations.GetToken(token,", \n\t")){

        GCRTemplate<MessageEnvelope> envelope(GCFT_Create);

        if (envelope->PrepareMessageEnvelope(message,token.Buffer(),flags,md)){
            envelopes.Insert(envelope);
            numberOfDestinations++;
        } else {
            AssertErrorCondition(FatalError,"ProcessMDR: cannot create Envelope to %s",token.Buffer());
        }

        token.SetSize(0);
    }

    // send all the letters
    while (envelopes.Size() > 0){
        GCRTemplate<MessageEnvelope> envelope;
        envelope = envelopes.Remove(0);
        if (envelope.IsValid()){

            MessageHandler::SendMessage(envelope);
        }
    }

    return True;
}

/** count down the number of messages to acknowledge.
returns True only all acks received */
bool MessageDeliveryRequest::Acknowledge(){
    if (numberOfDestinations > 0){
        numberOfDestinations--;
        if (numberOfDestinations == 0){
            event.Post();
            return True;
        }
        return False;
    }
    return True;
}


