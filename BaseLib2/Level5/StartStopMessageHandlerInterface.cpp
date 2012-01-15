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

#include "StartStopMessageHandlerInterface.h"
#include "FString.h"

bool SSMHIProcessMessage(StartStopMessageHandlerInterface &sshi, GCRTemplate<MessageEnvelope> envelope){

    GCRTemplate<Message> message       = envelope->GetMessage();
    FString              messageSender = envelope->Sender();
    if (!message.IsValid()){
        CStaticAssertErrorCondition(InitialisationError,"StartStopMessageHandlerInterface::ProcessMessage: Received invalid Message from %s", messageSender.Buffer());
        return False;
    }

    FString messageContent = message->Content();
    CStaticAssertErrorCondition(Information,"StartStopMessageHandlerInterface::ProcessMessage: Processing message %s from %s", messageSender.Buffer(), messageContent.Buffer());

    bool handled = False;
    if (messageContent == "START"){
        if(!sshi.Start()){
            CStaticAssertErrorCondition(InitialisationError,"StartStopMessageHandlerInterface::ProcessMessage: Start Failed ");
        }
        handled = True;
    }

    if (messageContent == "STOP"){
        if(!sshi.Stop()){
            CStaticAssertErrorCondition(InitialisationError,"StartStopMessageHandlerInterface::ProcessMessage: Stop Failed ");
        }
        handled = True;
    }

    if (handled){

        if (envelope->ManualReplyExpected()){
            CStaticAssertErrorCondition(Information,"SSMHIProcessMessage:Sending Manual reply to %s\n",envelope->Sender());

            GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
            GCRTemplate<Message> gcrtm(GCFT_Create);
            gcrtm->Init(FinishedMessage,"REPLY MESSAGE");
            gcrtme->PrepareReply(envelope,gcrtm);
            MessageHandler::SendMessage(gcrtme);
        }
        return True;
    }
    return sshi.ProcessMessage2(envelope);
}
