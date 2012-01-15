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

#ifndef _MESSAGE_DELIVERY_REQUEST
#define _MESSAGE_DELIVERY_REQUEST

/** 
 * @file
 * @brief MessageDeliveryRequest implementation
 * 
 * A Message Delivery Request is a container that will contain a message and associated data.
 * In addition it will contain a sender reference and a list of recipients.
 */
#include "GCNamedObject.h"
#include "GCReferenceContainer.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"
#include "FString.h"
#include "Message.h"
#include "MDRFlags.h"


OBJECT_DLL(MessageDeliveryRequest)

extern "C"{

    bool MDRObjectLoadSetup(MessageDeliveryRequest &mdr,ConfigurationDataBase &info,StreamInterface *err);

    bool MDRObjectSaveSetup(MessageDeliveryRequest &mdr,ConfigurationDataBase &info,StreamInterface *err);

    bool MDRPrepareMDR(MessageDeliveryRequest &mdr,GCRTemplate<Message>message,const char * destinations,MDRFlags flags,GCNamedObject *source = NULL);

}

/** a form containing information where to deliver a message and the message itself
    it contains also the mechanism to synchronise a sender with the return receipts from all its
    recipients  */
class MessageDeliveryRequest: public GCNamedObject {

OBJECT_DLL_STUFF(MessageDeliveryRequest)

    friend bool MDRObjectLoadSetup(MessageDeliveryRequest &mdr,ConfigurationDataBase &info,StreamInterface *err);

    friend bool MDRObjectSaveSetup(MessageDeliveryRequest &mdr,ConfigurationDataBase &info,StreamInterface *err);

    friend bool MDRPrepareMDR(MessageDeliveryRequest &mdr,GCRTemplate<Message>message,const char * destinations,MDRFlags flags,GCNamedObject *source);

protected:

    /** the message */
    GCRTemplate<Message>            message;

    /** comma/space separated list of destinations */
    BString                         destinations;

    /** who made the request */
    BString                         sender;

    /** to wait for reply */
    EventSem                        event;

    /** number of valid destinations
        created when sending messages by ProcessMDR and
        used by */
    uint32                          numberOfDestinations;

    /** options */
    MDRFlags                        flags;

    /** how long to wait for reply */
    TimeoutType                     msecTimeout;

public:
    /** build empty request */
    MessageDeliveryRequest()
    {
        flags                   = MDRF_None;
        numberOfDestinations    = 0;
        msecTimeout             = TTInfiniteWait;
        sender = "None";
        event.Create();
    }

    ~MessageDeliveryRequest()
    {        
        event.Close();
    }

    /** set the specified object as sender of the message.
    */
    inline void                     SetSender(
                        GCNamedObject &                 sender)
    {
        sender.GetUniqueName(this->sender);
    }

    /** send the message @param message to the object @param destination
        if destination starts with :: the name search scope is the global object container
        otherwise it is the message reciever container */
    inline bool                     PrepareMDR(
                        GCRTemplate<Message>            message,
                        const char *                    destinations,
                        MDRFlags                        flags       = MDRF_None,
                        GCNamedObject *                 source      = NULL,
                        TimeoutType                     msecTimeout = TTInfiniteWait)
    {
        this->msecTimeout = msecTimeout;
        return MDRPrepareMDR(*this,message,destinations,flags,source);
    }

    /** Prepares Envelopes and delivers them */
    bool                            ProcessMDR(MessageDispatcher *md);

    /** count down the number of messages to acknowledge.
    returns True only all acks received */
    bool                            Acknowledge();

    /**  initialise an object from a set of configs
        The syntax is
        Sender = <any text (within " if includes spaces) >
        Destinations = <any text (within " if includes spaces)
                        NB multiple destinations are space or comma separated >
        MsecTimeOut = how msec to wait for reply
        Flags       = [ EarlyAutomaticReply LateAutomaticReply NoReply ManualReply ]
        Message = {
            Class = Message
            <See Message syntax>
        }

        The object name is taken from the current node name
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MDRObjectLoadSetup(*this,info,err);
    }

    /**  save an object to a set of configs
        The syntax is
        Sender = <any text (within " if includes spaces) >
        Destinations = <any text (within " if includes spaces) >
        Message = {
           Class = Message
            <See Message syntax>
        }
    */
           virtual bool             ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MDRObjectSaveSetup(*this,info,err);
    }

    /** the task will wait here if necessary */
    inline EventSem &               Event()
    {
        return event;
    }

    /** return the destinations string */
    inline const char *             Destinations()      const
    {
        return destinations.Buffer();
    }

    /** the message */
    inline GCRTemplate<Message>     GetMessage()        const
    {
        return message;
    }

    /** True if must wait for reply */
    inline bool                     ReplyExpected() const{
        return ((flags.flags & MDRF_ReplyExpected.flags) != 0);
    }

    /** How long to wait for a reply */
    inline TimeoutType              MsecTimeout()       const 
    {
        return msecTimeout;
    }

    /**  */
    inline MDRFlags                 Flags()             const
    {
        return flags;
    }

};


#endif
