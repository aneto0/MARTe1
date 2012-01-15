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

#if !defined(_MESSAGE_ENVELOPE_)
#define _MESSAGE_ENVELOPE_

/** 
 * @file
 * The folder containing the message
 * the address, the sender address and
 * possibly the mail it was replying to
 */
#include "GCReferenceContainer.h"
#include "BString.h"
#include "GCReference.h"
#include "Message.h"
#include "MDRFlags.h"
#include "MuxLock.h"


OBJECT_DLL(MessageEnvelope)

#if !defined(_CINT)

class MessageEnvelope;

extern "C"{
    bool MSGEObjectLoadSetup(MessageEnvelope &msg,ConfigurationDataBase &info,StreamInterface *err);

    bool MSGEObjectSaveSetup(MessageEnvelope &msg,ConfigurationDataBase &info,StreamInterface *err);

    bool MSGEPrepareMessageEnvelope(MessageEnvelope &msg,GCRTemplate<Message>message,MDRFlags flags,const char * destination,const char *source);

    /** Convert a MessageEnvelope into a reply envelope. Swaps sender and destination.
        Adds reply. */
    bool            MSGEPrepareReply(
                        MessageEnvelope &               msg,
                        GCRTemplate<MessageEnvelope>    messageEnvelope,
                        GCRTemplate<Message>            replyMessage,
                        MDRFlags                        flags,
                        int                             maxHistory);

}

#endif

/** it is a code a string and an object reference
    it also contain a stack messages
    typically:
        top ) the last message
        ... ) a history of message-exchanges
    */
class MessageEnvelope: public GCReferenceContainer{
OBJECT_DLL_STUFF(MessageEnvelope)

#if !defined(_CINT)

    friend bool MSGEObjectLoadSetup(MessageEnvelope &msg,ConfigurationDataBase &info,StreamInterface *err);

    friend bool MSGEObjectSaveSetup(MessageEnvelope &msg,ConfigurationDataBase &info,StreamInterface *err);

    friend bool MSGEPrepareMessageEnvelope(MessageEnvelope &msg,GCRTemplate<Message>message,MDRFlags flags,const char * destination,const char *source);

    friend bool     MSGEPrepareReply(
                        MessageEnvelope &               msg,
                        GCRTemplate<MessageEnvelope>    messageEnvelope,
                        GCRTemplate<Message>            replyMessage,
                        MDRFlags                        flags,
                        int                             maxHistory);

    friend class MessageHandler;
#endif  //CINT

protected:
    /** the name of the object sending the message */
    BString                         sender;

    /** the name of the object destination of the message */
    BString                         destination;

    /** options */
    MDRFlags                        flags;

public:

#if !defined(_CINT)

    /** constructor to build an empty envelope */
                                    MessageEnvelope()
    {
        sender = "None";
        destination = "None";
    }

    /** destructor */
    virtual                        ~MessageEnvelope()
    {
    }

    /** retrieve the message sender */
    inline const char *             Sender()
    {
        return sender.Buffer();
    }

    /** retrieve the message destination */
    inline const char *             Destination()
    {
        return destination.Buffer();
    }

    /** set the specified object as sender of the message.
    */
    inline void                     SetSender(
                        GCNamedObject &                 sender)
    {
        if(strcmp(this->sender.Buffer(),"None")==0){
            sender.GetUniqueName(this->sender);
        }
    }

    /** set the specified object as sender of the message.
    */
    inline void                     SetDestination(
                        const char *                    destination)
    {
        this->destination = destination;
    }

    /** send the message @param message to the object @param destination
        if destination starts with :: the name search scope is the global object container
        otherwise it is the message reciever container
        use @param flags to request a reply.
    */
    inline bool                     PrepareMessageEnvelope(
                        GCRTemplate<Message>            message,
                        const char *                    destination,
                        MDRFlags                        flags       = MDRF_None,
                        GCNamedObject *                 source      = NULL)
    {
        // unlocks automatically on exit from function
        MuxLock muxLock;
        if (!muxLock.Lock(mux, msecTimeout)){
           AssertErrorCondition(Timeout,"PrepareMessageEnvelope: timeout on resource sharing ");
           return False;
        }
        BString sourceName;
        if(strcmp(sender.Buffer(),"None") == 0){
            if (source != NULL){
                source->GetUniqueName(sourceName);
            } else {
                sourceName = "";
            }
        }else{
            sourceName = sender;
        }
        return MSGEPrepareMessageEnvelope(*this,message,flags,destination,sourceName.Buffer());
    }

    /** Creates a MessageEnvelope as a reply to the @param messageEnvelope.
        Swaps sender and destination and copies all content
        Adds reply on top. @param maxHistory regulates the number of old-messages
        left in the envelope */
    inline bool                     PrepareReply(
                        GCRTemplate<MessageEnvelope>    messageEnvelope,
                        GCRTemplate<Message>            replyMessage,
                        MDRFlags                        flags       = MDRF_None,
                        int                             maxHistory  =   2)
    {
        // unlocks automatically on exit from function
        MuxLock muxLock;
        if (!muxLock.Lock(mux, msecTimeout)){
           AssertErrorCondition(Timeout,"PrepepareReply: timeout on resource sharing ");
           return False;
        }
        return MSGEPrepareReply(*this,messageEnvelope,replyMessage,flags,maxHistory);
    }

    /** This is the acknowledge to a message reception.
        It is automatically generated from the original enevelope */
    inline bool                     PrepareAutomaticReply(
                        GCRTemplate<MessageEnvelope>    envelope)
    {
        GCRTemplate<Message> nullMessage;
        return PrepareReply(envelope,nullMessage,MDRF_Reply);
    }

    /** Get the message */
    inline GCRTemplate<Message>     GetMessage()
    {
        if (Size() > 0) return Find(0);
        GCRTemplate<Message> gcrtm;
        return gcrtm;
    }

    /** Get the original message if this is a reply
        @param index default 1 is the original message
        any higher number is the history of messages*/
    inline GCRTemplate<Message>     GetOriginal(int index = 1)
    {
        if (Size() > index) return Find(index);
        GCRTemplate<Message> gcrtm;
        return gcrtm;
    }

    /**  initialise an object from a set of configs
        The syntax is
        Sender = <any text (within " if includes spaces) >
        Destination = <any text (within " if includes spaces) >
        <Use syantax of GCReferenceContainer>
        Add = {
            MessageName = {
                <See Message syntax>
            }
            [ReplyName = {
                <See Message syntax>
            }]
        }
    */
    virtual bool                    ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MSGEObjectLoadSetup(*this,info,err);
    }

    /**  save an object to a set of configs
        The syntax is
        Sender = <any text (within " if includes spaces) >
        Destination = <any text (within " if includes spaces) >
        Content = {
            <Use syantax of GCReferenceContainer>
            Add = {
             Message = {
                 <See Message syntax>
             }
             [Reply = {
                 <See Message syntax>
             }]
            }
             ... (other message names are possible)
        }
    */
    virtual bool                    ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MSGEObjectSaveSetup(*this,info,err);
    }

    /** True if an automatic reply shall be generated after processing */
    inline bool                     LateAutomaticReplyExpected() const
    {
        return ((flags & MDRF_ReplyExpected) == MDRF_LateAutomaticReply) ;
    }

    /** True if an automatic reply shall be generated */
    inline bool                     EarlyAutomaticReplyExpected() const
    {
        return ((flags & MDRF_ReplyExpected) == MDRF_EarlyAutomaticReply) ;
    }

    /** True if a manual reply shall be generated by the user */
    inline bool                     ManualReplyExpected() const
    {
        return ((flags & MDRF_ReplyExpected) == MDRF_ManualReply) ;
    }

    /** True if a manual reply shall be generated by the user */
    inline bool                     ReplyExpected() const
    {
        return ((flags & MDRF_ReplyExpected) != 0) ;
    }

    /** True if an automatic reply */
    inline bool                     AutomaticReply() const
    {
        return ((flags & MDRF_Reply) != 0);
    }

#endif //CINT
};


#endif
