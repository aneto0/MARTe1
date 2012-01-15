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

/**
 * @file
 * @brief Start and stop services 
 *
 * Classes implementing this interface will be able to be 
 * automatically started/stopped with messages
 */
#if !defined _START_STOP_MESSAGE_HANDLER_INTERFACE_
#define _START_STOP_MESSAGE_HANDLER_INTERFACE_

#include "System.h"
#include "MessageHandler.h"

class StartStopMessageHandlerInterface;

extern "C" {
    bool SSMHIProcessMessage(StartStopMessageHandlerInterface &sshi, GCRTemplate<MessageEnvelope> envelope);
}



class StartStopMessageHandlerInterface:public MessageHandler{

private:

    friend bool SSMHIProcessMessage(StartStopMessageHandlerInterface &sshi, GCRTemplate<MessageEnvelope> envelope);

private:

    /** Process the message 
        If the Message Content is Start or Stop it calls the 
        Start or Stop function accordingly.
        If the Message Content is not Start or Stop it calls
        the ProcessMessage2 function.
     */
    virtual bool        ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
        return SSMHIProcessMessage(*this, envelope);
    }

protected:

    /** Implements the Start Function */
    virtual bool Start() = 0;

    /** Implements the Stop Function */
    virtual bool Stop() = 0;

public:

    /** Handles Message Content which are not Start nor Stop */
    virtual bool        ProcessMessage2(GCRTemplate<MessageEnvelope> envelope){ return False; }
    
    /** Constructor */
    StartStopMessageHandlerInterface(){};

    /** Destructor */
    virtual ~StartStopMessageHandlerInterface(){};

};

#endif

