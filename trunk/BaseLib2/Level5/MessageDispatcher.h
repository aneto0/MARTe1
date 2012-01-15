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

#if !defined(_MESSAGE_DISPATCHER_)
#define _MESSAGE_DISPATCHER_

/** 
 * @file
 * A special global object that handles sending messages for third parties 
 */
#include "GCReferenceContainer.h"
#include "EventSem.h"
#include "MessageDeliveryRequest.h"
#include "GCNamedObject.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"
#include "FastPollingMutexSem.h"


class MessageDispatcher;

extern "C"{

    /** */
    void                            MessageDispatcherThreadFN(void *arg);

    /** The main thread function */
    bool                            MDProcessRequestQueue(MessageDispatcher &md);

    /** To handle replies mainly */
    bool                            MDProcessMessage(MessageDispatcher &md,GCRTemplate<MessageEnvelope> envelope);

    /** Retrieves the global instance of message dispatcher.
        If not allocated it will allocate it and place it in the
        GlobalObjectContainer as the ovject MessageDispatcher*/
    MessageDispatcher &             GlobalMessageDispatcher();

}



OBJECT_DLL(MessageDispatcher)


/** Class that provide message sending to single/multiple recipients.
    Provides also wait for acknowledge service from single/multiple destinations  */
class MessageDispatcher: public GCNamedObject, public MessageHandler{

OBJECT_DLL_STUFF(MessageDispatcher)

    friend  void        MessageHandlerThreadFN(void *arg);

    friend  bool        MDProcessRequestQueue(MessageDispatcher &md);

    friend  bool        MDProcessMessage(MessageDispatcher &md,GCRTemplate<MessageEnvelope> envelope);

    friend  void        MessageDispatcherThreadFN(void *arg);

protected:

    /** protects access to the requestsQueue */
            FastPollingMutexSem                         requestsMux;

    /** the message Q */
            GCReferenceContainer                        requestsQueue;

    /** the wait for Acknowledge Q */
            GCReferenceContainer                        waitQueue;
//            GCRContainerTemplate<MessageDeliveryRequest> waitQueue;

    /** to mark a new request in the list */
            EventSem                                    requestsEvent;

    /** to mark the start of the handling thread */
            EventSem                                    threadStartEvent;

    /** to request the end of the processing thread */
            bool                                        threadToContinue;

    /** The thread ID. if not 0 it means a thread is running
        the variable is protected */
            TID                                         mDThreadID;

public:
    /** constructor */
                        MessageDispatcher()
    {

        mDThreadID  = (TID)0;


        requestsEvent.Create();
        threadStartEvent.Create();

        // will be marked True by the thread itself when starting
        threadToContinue = False;

        // remove events
        requestsEvent.Reset();
        threadStartEvent.Reset();

        // label this thread
        FString threadName;
        threadName.Printf("MessageDispatcher:%s",Name());

        // create a new one
        mDThreadID = Threads::BeginThread(MessageDispatcherThreadFN,this,THREADS_DEFAULT_STACKSIZE,threadName.Buffer());

        // wait for thread start notification
        if (!threadStartEvent.Wait(globalTimeout)){
            AssertErrorCondition(Timeout,"MessageDispatcher: Request Handling Thread did not start within %i msecs",globalTimeout.msecTimeout);
        }

        requestsEvent.Reset();

    }

    /** Destructor */
    virtual             ~MessageDispatcher()
    {
        threadToContinue = False;

        // wait for thread start notification
        if (!threadStartEvent.Wait(globalTimeout)){
            AssertErrorCondition(Timeout,"MessageDispatcher: Request Handling Thread did not stop within %i msecs",globalTimeout.msecTimeout);
        }

        if (mDThreadID != (TID)0){
            SleepSec(0.1);
        }
        if (mDThreadID != (TID)0){
            Threads::Kill(mDThreadID);
            CStaticAssertErrorCondition(Timeout,"~MessageDispatcher Killed thread");
        }

    }

    /** Process requests on the request Queue */
    inline bool         ProcessRequestQueue()
    {
        return MDProcessRequestQueue(*this);
    }

    /** the virtual function needed by MessageHandlerInterface */
    virtual bool        ProcessMessage(GCRTemplate<MessageEnvelope> envelope)
    {
        return MDProcessMessage(*this,envelope);
    }

    /** queues the request and makes the task wait if acknowledge is requested
        @param waitForReply True will wait for reply if any of the relevant MDRF_ flags are set */
    inline bool         GMDSendMessageRequest(
                        GCRTemplate<MessageDeliveryRequest> messageRequest,
                        TimeoutType                         msecTimeout,
                        bool                                waitForReply )
    {
        if (messageRequest->ReplyExpected()){
            messageRequest->Event().Reset();
        }


        if (requestsMux.FastLock(msecTimeout)){
            requestsQueue.Insert(messageRequest);
            requestsEvent.Post();
            requestsMux.FastUnLock();
        } else {
            AssertErrorCondition(Timeout,"GMDSendMessageRequest timeout on queueing");
            return False;
        }

        if (waitForReply) return WaitForReply(messageRequest,msecTimeout);

        return True;

    }

    /** will wait for reply if MDRF_AutomaticReply is set  */
    inline bool         WaitForReply(
                        GCRTemplate<MessageDeliveryRequest> messageRequest,
                        TimeoutType                         msecTimeout = TTInfiniteWait
     ){
        if (messageRequest->ReplyExpected()){
            TimeoutType msecTimeout_ = messageRequest->MsecTimeout();
            if (msecTimeout_ == TTInfiniteWait) msecTimeout_ = msecTimeout;
            if (!messageRequest->Event().Wait(msecTimeout_)){
                AssertErrorCondition(Timeout,"GMDSendMessageRequest Reply wait timeout");
                return False;
            }
        }

        return True;
    }

    /** queues the request  */
    inline bool         GMDSendMessageRequest(
                        GCRTemplate<MessageEnvelope>        messageRequest,
                        TimeoutType                         msecTimeout)
    {
        if (requestsMux.FastLock(msecTimeout)){
            requestsQueue.Insert(messageRequest);
            requestsEvent.Post();
            requestsMux.FastUnLock();
        } else {
            AssertErrorCondition(Timeout,"GMDSendMessageRequest timeout on queueing");
            return False;
        }

        return True;
    }
};

/** Function to dispatch messages to many destinations */
static inline bool      GMDSendMessageDeliveryRequest(
                        GCRTemplate<MessageDeliveryRequest> messageRequest,
                        TimeoutType                         msecTimeout = TTInfiniteWait,
                        bool                                waitForReply = True)
{
    MessageDispatcher & gmd = GlobalMessageDispatcher();
    return gmd.GMDSendMessageRequest(messageRequest,msecTimeout,waitForReply);
}

/** Function to wait for replies */
static inline bool      GMDWaitForReply(
                        GCRTemplate<MessageDeliveryRequest> messageRequest,
                        TimeoutType                         msecTimeout = TTInfiniteWait)
{
    MessageDispatcher & gmd = GlobalMessageDispatcher();
    return gmd.WaitForReply(messageRequest,msecTimeout);
}

/** Function to dispatch a message in an envelope */
static inline bool      GMDSendMessageEnvelope(
                        GCRTemplate<MessageEnvelope>        messageRequest)
{
    MessageDispatcher & gmd = GlobalMessageDispatcher();
    return gmd.GMDSendMessageRequest(messageRequest,TTInfiniteWait);
}


#endif
