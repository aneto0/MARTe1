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


#include "MessageEnvelope.h"
#include "CDBExtended.h"

OBJECTLOADREGISTER(MessageEnvelope,"$Id$");


bool
MSGEObjectLoadSetup(
                MessageEnvelope &               msg,
                ConfigurationDataBase &         info,
                StreamInterface *               err){

    CDBExtended cdbx(info);

/*
    if (!msg.GCNamedObject::ObjectLoadSetup(info,err)){
        msg.AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot retrieve name of node");
        return False;
    }
*/
    if (!cdbx.ReadBString(msg.sender,"Sender","None")){
        msg.AssertErrorCondition(Warning,"ObjectLoadSetup:Cannot find Sender for this MessageEnvelope");
    }

    if (!cdbx.ReadBString(msg.destination,"Destination","None")){
        msg.AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot find Destination for this MessageEnvelope");
        return False;
    }

    BString flags;
    cdbx.ReadBString(flags,"Flags","NoReply");
    msg.flags = MDRFFromString(flags.Buffer());

    int32 isReply;
    cdbx.ReadInt32(isReply,"Reply",0);
    if (isReply){
        msg.flags = msg.flags | MDRF_Reply;
    }

    if (!msg.GCReferenceContainer::ObjectLoadSetup(info,err)){
        msg.AssertErrorCondition(FatalError,"ObjectLoadSetup: GCReferenceContainer::ObjectLoadSetup failed");
        return False;
    }



    return True;
}

bool
MSGEObjectSaveSetup(
                MessageEnvelope &               msg,
                ConfigurationDataBase &         info,
                StreamInterface *               err){

    CDBExtended cdbx(info);

    if (!cdbx.WriteString(msg.sender.Buffer(),"Sender")){
        msg.AssertErrorCondition(Warning,"ObjectSaveSetup:Cannot write Sender for this MessageEnvelope");
    }

    if (!cdbx.WriteString(msg.destination.Buffer(),"Destination")){
        msg.AssertErrorCondition(FatalError,"ObjectSaveSetup:Cannot write Destination for this MessageEnvelope");
        return False;
    }

    const char *flags = MDRFToString(msg.flags);
    cdbx.WriteString(flags,"Flags");

    if ((msg.flags & MDRF_Reply)!=(MDRFlags)0){
        int32 isReply= 1;
        cdbx.WriteInt32(isReply,"Reply");
    }

    bool ret = True;
    ret = msg.GCReferenceContainer::ObjectSaveSetup(info,err);

    if (!ret){
        msg.AssertErrorCondition(FatalError,"ObjectSaveSetup: GCReferenceContainer::ObjectSaveSetup failed");
    }

    return ret;

}


/** send the message @param message to the object @param destination
if destination starts with :: the name search scope is the global object container
otherwise it is the message reciever container */
bool
MSGEPrepareMessageEnvelope(
                MessageEnvelope &               msg,
                GCRTemplate<Message>            message,
                MDRFlags                        flags,
                const char *                    destination,
                const char *                    source)
{


    msg.sender        = source;
    msg.destination   = destination;
    msg.flags         = flags;

    // remove old messages
    msg.GCReferenceContainer::CleanUp();

    if (message.IsValid()){
        // mark as the message
        if (!msg.Insert(message)){
           msg.AssertErrorCondition(FatalError,"MSGEPrepareMessageEnvelope: failed adding Message to envelope");
           return False;
        }
    } else {
        msg.AssertErrorCondition(Warning,"MSGEPrepareMessageEnvelope: empty envelope");
    }
    return True;
}

bool     MSGEPrepareReply(
                MessageEnvelope &               msg,
                GCRTemplate<MessageEnvelope>    messageEnvelope,
                GCRTemplate<Message>            replyMessage,
                MDRFlags                        flags,
                int                             maxHistory)
{

    msg.sender          = messageEnvelope->destination;
    msg.destination     = messageEnvelope->sender;
    msg.flags           = flags;

    if (replyMessage.IsValid()){
        // Save the new message
        if (!msg.Insert(replyMessage,0)){
           msg.AssertErrorCondition(FatalError,"MSGEPrepareReply: failed adding Message to envelope");
           return False;
        }
    } else {
        if (flags != MDRF_Reply){
            msg.AssertErrorCondition(Warning,"MSGEPrepareReply: empty reply");
        }
    }

    // copy history
    int index = 0;
    while(msg.Size() < maxHistory){
        if (messageEnvelope->Size() > index){
            GCReference gc = messageEnvelope->Find(index);
            if (gc.IsValid()) msg.Insert(gc);
            else break;
        } else break;
        index++;
    }

    return True;

}

