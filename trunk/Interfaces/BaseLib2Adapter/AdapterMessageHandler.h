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

#ifndef ADAPTER_MESSAGE_HANDLER_H_
#define ADAPTER_MESSAGE_HANDLER_H_

#include "GCRTemplate.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"

/**
 * @brief Forwards messages to the Adapter::ReceiveMessageFromBaseLib2
 */
OBJECT_DLL(AdapterMessageHandler)
class AdapterMessageHandler : public GCNamedObject, public MessageHandler {
OBJECT_DLL_STUFF(AdapterMessageHandler)
public:
    /**
     * @brief Constructor. NOOP.
     */
    AdapterMessageHandler();

    /**
     * @brief Desstructor. NOOP.
     */
    virtual ~AdapterMessageHandler();

    /**
     * @brief Forwards the message to the Adapter::ReceiveMessageFromBaseLib2 (replies are currently not supported).
     * @details The message content shall be coded as DESTINATION::FUNCTION, where DESTINATION is the name of the 
     * Object to be called in the remote system and FUNCTION is the method to be called in the remote system.
     * @param[in] envelope the message to be forwarded.
     * @return true if the message was successfully forwarded.
     */
    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

};

#endif

