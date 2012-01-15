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
 * The interface of the objects that are able to handle messages
 */
#if !defined(_MESSAGE_HANDLER_)
#define _MESSAGE_HANDLER_

#include "MessageInterface.h"
#include "MessageQueue.h"
#include "EventSem.h"

class MessageHandler;
class MessageEnvelope;

extern "C"{

    /** */
    void    MessageHandlerThreadFN(
                        void *                          arg);

    /** */
    bool    MHHandleMessage(
                        MessageHandler &                mh,
                        GCRTemplate<MessageEnvelope>    envelope,
                        const char *                    subAddress = NULL);

    /** */
    bool    MHConstructor(
                        MessageHandler &                mh);

    /** */
    bool    MHSendMessage(
                        GCRTemplate<MessageEnvelope>    gcrtme);

    /** */
    bool    MHSendMessageAndWait(
                        MessageHandler &                mh,
                        GCRTemplate<MessageEnvelope>    message,
                        GCRTemplate<MessageEnvelope> &  reply,
                        TimeoutType                     timeout
                    );

    /** send messages to server */
    bool    MHSendMessageRemotely(
                        GCRTemplate<MessageEnvelope>    envelope,
                        const char *                    serverAddress,
                        int                             serverPort
                    );


}

/** class interface for objects that can handle messages.
    If the flag @param immediate is True it will handle the message immediately
    or it will put on a Queue and spawn a thread to handle it later   */
class MessageHandler: protected MessageInterface{

    friend void MessageHandlerThreadFN(
                        void *                          arg);

    friend bool MHHandleMessage(
                        MessageHandler &                mh,
                        GCRTemplate<MessageEnvelope>    envelope,
                        const char *                    subAddress);

    friend bool MHConstructor(
                        MessageHandler &                mh);

    friend bool MHSendMessageAndWait(
                        MessageHandler &                mh,
                        GCRTemplate<MessageEnvelope>    message,
                        GCRTemplate<MessageEnvelope> &  reply,
                        TimeoutType                     timeout
                    );

    /** it is the actual body of the managing thread */
            void        ProcessMessageQueue();

protected:
    /** True means that the message is processed immediately
        Note that the user handler function must be able to
        handle parallel requests */
            bool                    immediate;

    /** to keep the thread alive */
            bool                    keepAlive;

    /** the pools of Queues */
            GCReferenceContainer    messageQueues;

    /** the local message Q */
            GCRTemplate<MessageQueue> messageQueue;

    /** The thread ID. if not 0 it means a thread is running
        the variable is protected */
            TID                     threadID;

    /** The level at which the handler task operates */
            uint32                  threadPriorityLevel;

    /** The level at which the handler task operates
        0= Idle 1=Regular 2= Server 3 = RT */
            uint32                  threadPriorityClass;

    /** To wait for the task start/stop events */
            EventSem                event;

    /** To prevent concurrent access to threadId */
            MutexSem                threadIdMutex;

    /** How long to wait for single action */
            TimeoutType             globalTimeout;

    /** How long to wait for thread destruction */
            TimeoutType             globalDestructionTimeout;

    /** To be overridden in descendent clases. @aparam envelope is a valid reference to a
        MessageEnvelope
        Note that if immediate is True the user handler function must be able to
        handle parallel requests
        @return True if the message is considered to have been consumed
                False if the handler does not recognise the message as its own */
    virtual bool        ProcessMessage(
                            GCRTemplate<MessageEnvelope>    envelope)
    {
        return False;
    }

    /** If the envelope has no reply mechanism specified */
    void EnableReplyOnEnvelope(GCRTemplate<MessageEnvelope> envelope){
        envelope->flags = MDRF_EarlyAutomaticReply | envelope->flags;
    }


public:
    /** */
                        MessageHandler()
    {
        MHConstructor(*this);
    }

    /** */
    virtual             ~MessageHandler()
    {
        if (threadID != (TID)0){
            event.Reset();
            keepAlive = False;
            messageQueue->Reset();
            if (!event.Wait(globalDestructionTimeout)){
	        CStaticAssertErrorCondition(Timeout, "~MessageHandler() exited with timeout: ResetWait(TO = %i, Handle = 0x%x) = False", globalDestructionTimeout.msecTimeout, event.Handle());
                Threads::Kill(threadID);
            }
        }
    }

    /** the function to be called externally */
    inline  bool        HandleMessage(
                            GCRTemplate<MessageEnvelope>    envelope,
                            const char *                    subAddress = NULL)
    {
        // allow treating empty string and NULL ptr in the same way from now on.
        if (subAddress){
            int l = strlen(subAddress);
            if (l == 0) subAddress = NULL;
        }
        return MHHandleMessage(*this,envelope,subAddress);
    }

    /** send message and wait for reply! */
    inline bool         SendMessageAndWait(
                            GCRTemplate<MessageEnvelope>    envelope,
                            GCRTemplate<MessageEnvelope> &  reply,
                            TimeoutType                     timeout = TTInfiniteWait
                        )
    {
        return MHSendMessageAndWait(
                        *this,
                        envelope,
                        reply,
                        timeout
                    );
    }

    /** function to send message */
    static inline bool  SendMessage(
                            GCRTemplate<MessageEnvelope>    gcrtme
                        )
    {
        return MHSendMessage(gcrtme);
    }

};


#endif
