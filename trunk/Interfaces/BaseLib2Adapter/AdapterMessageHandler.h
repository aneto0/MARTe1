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

#ifndef PLUMBER_MESSAGE_HANDLER_H_
#define PLUMBER_MESSAGE_HANDLER_H_

#include "GCRTemplate.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"

/**
 * @brief TODO
 */
OBJECT_DLL(AdapterMessageHandler)
class AdapterMessageHandler : public GCNamedObject, public MessageHandler {
OBJECT_DLL_STUFF(AdapterMessageHandler)
public:
    /**
     * @brief TODO
     */
    AdapterMessageHandler();

    /**
     * @brief TODO
     */
    ~AdapterMessageHandler();

    /**
     * @brief TODO
     */
    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

};

#endif

