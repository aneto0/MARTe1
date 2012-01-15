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

#include "HttpService.h"


OBJECTLOADREGISTER(HttpService,"$Id: HttpService.cpp,v 1.2 2008/05/08 13:24:53 fpiccolo Exp $")

/** the function to be implemented by the user application */
bool HttpService::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){

    if (verboseLevel >= 10) AssertErrorCondition(Information,"%s Received Message from %s: ",Name(),envelope->Sender());

    GCRTemplate<Message> message = envelope->GetMessage();
    if (message.IsValid()){
        if (verboseLevel >= 10) AssertErrorCondition(Information,"%s:Message[(code)%i,(id)%i]  is %s",Name(),message->GetMessageCode().Code(),message->Id(),message->Content());
    } else {
        AssertErrorCondition(CommunicationError,"%s:Invalid Message\n",Name());
        return False;
    }

    if (strncasecmp(message->Content(),"START",5)==0){
        Start();
    } else
    if (strncasecmp(message->Content(),"STOP",4)==0){
        Stop();
    } else {
        AssertErrorCondition(CommunicationError,"%s: Invalid Message Content %s",Name(),message->Content());
        return False;
    }

    if (envelope->ManualReplyExpected()){
        AssertErrorCondition(Information,"%s Sending Manual reply to %s",Name(),envelope->Sender());
        GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
        GCRTemplate<Message> gcrtm(GCFT_Create);
        gcrtm->Init(message->GetMessageCode().Code(),"ACK");
        gcrtme->PrepareReply(envelope,gcrtm);
        MessageHandler::SendMessage(gcrtme);
    }

    if (verboseLevel >= 10) AssertErrorCondition(Information,"%s Successfully handled Message from %s: ",Name(),envelope->Sender());
    return True;

}
