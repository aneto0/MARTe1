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
 * A simple Message Interface that can be used to create multiple message
 * handling objects.
 */
#if !defined(_MESSAGE_INTERFACE_)
#define _MESSAGE_INTERFACE_

#include "System.h"
#include "MessageEnvelope.h"

/** This interface alone cannot implement a MessageHandler.
    To implement an object with multiple message handling capabilities simply
    have each component class inherit from this class and then the final class inherit
    from all the base classes and MessageHandler.
    If each component class has registered itself with the MessageHandler
    then the standard message handling function will try all the interfaces */
class MessageInterface{

public:
    /** the function to be implemented by the user application */
    virtual bool        ProcessMessage(GCRTemplate<MessageEnvelope> envelope)=0;

    /** the function to be implemented by the user application */
    virtual bool        ProcessMessage2(GCRTemplate<MessageEnvelope> envelope,
                                        const char *subAddress){
        if (subAddress == NULL) return ProcessMessage(envelope);
        return False;
    }

};


#endif
