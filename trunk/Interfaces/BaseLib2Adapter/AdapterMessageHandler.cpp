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
 * $Id: $
 *
**/
#include "GenDefs.h"
#include "Adapter.h"
#include "AdapterMessageHandler.h"
#include "FString.h"

using namespace BaseLib2;
AdapterMessageHandler::AdapterMessageHandler() : GCNamedObject(), MessageHandler() {
}

AdapterMessageHandler::~AdapterMessageHandler() {
}

bool AdapterMessageHandler::ProcessMessage(GCRTemplate<MessageEnvelope> envelope) {
    GCRTemplate<Message> msg = envelope->GetMessage();
    bool ok = msg.IsValid();
    FString destination;
    FString content;
    FString function;
    Adapter *adapter = Adapter::Instance();
    if (ok) {
        function = msg->Content();
        ok = function.Seek(0ULL);
    }
    if (ok) {
        ok = function.GetToken(destination, "::");
    }
    if (ok) {
        ok = function.GetToken(content, "::");
    }
    if (ok) {
        ok = adapter->ReceiveMessageFromBaseLib2(destination.Buffer(), content.Buffer(), msg->GetMessageCode().Code());
    }
    return ok;
}

OBJECTLOADREGISTER(AdapterMessageHandler,"$Id: $")

