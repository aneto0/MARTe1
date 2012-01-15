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
 * @brief Send messages using a menu entry.
 */
#if !defined _SEND_MESSAGE_MENU_ENTRY
#define _SEND_MESSAGE_MENU_ENTRY

#include "System.h"
#include "MenuEntry.h"
#include "GCRTemplate.h"
#include "MessageEnvelope.h"
#include "CDBExtended.h"

class SendMessageMenuEntry;

extern "C"{
    bool SendMenuMessage(StreamInterface &in,StreamInterface &out,void *userData);
}

OBJECT_DLL(SendMessageMenuEntry)

class SendMessageMenuEntry: public MenuEntry{

private:

    /** Contains the Message Envelope used for sending the Message*/
    GCRTemplate<MessageEnvelope>    messageEnvelope;

    /** Contains the Menu Action */
    friend bool   SendMenuMessage(StreamInterface &in,StreamInterface &out,void *userData);

public:

    /** Constructor */
    SendMessageMenuEntry(){}

    /** Destructor */ 
    virtual ~SendMessageMenuEntry(){}


    /** Object Load Setup Function.
        Calls the MenuEntry Object Load Setup and then
        load the user specified MessageEnvelope according
        to the MessageEnvelope specifications. 
        The MessageEnvelope must be specified within a member
        named Envelope.
     */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {

        CDBExtended cdb(info);
        if(!MenuEntry::ObjectLoadSetup(cdb, err)){
            AssertErrorCondition(InitialisationError,"SendMessageMenuEntry::ObjectLoadSetup: MenuEntry ObjectLoadSetup Failed");
            return False;
        }
        
        if(!cdb->Move("Envelope")){
            AssertErrorCondition(InitialisationError,"SendMessageMenuEntry::ObjectLoadSetup: No Envelope has been specified");
            return False;
        }

        if(!messageEnvelope.ObjectLoadSetup(cdb,err)){
            AssertErrorCondition(InitialisationError,"SendMessageMenuEntry::ObjectLoadSetup: messageEnvelope ObjectLoadSetup failed");
            return False;
        }

        cdb->MoveToFather();

        SetUp(SendMenuMessage, NULL, NULL, this);

        return True;
    }

private:

OBJECT_DLL_STUFF(SendMessageMenuEntry)

};

#endif

