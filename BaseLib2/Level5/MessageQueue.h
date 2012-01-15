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
 * A GCRContainerTemplate<MessageEnvelope> with a FIFO only policy and an
 * EventSem to synchronise tasks to new data arrival.
 */
#if !defined(_MESSAGE_QUEUE_)
#define _MESSAGE_QUEUE_

#include "GCReferenceContainer.h"
#include "MessageEnvelope.h"
#include "EventSem.h"


/** a GCRContainerTemplate<MessageEnvelope> with a FIFO only policy  */
class MessageQueue: public GCReferenceContainer {
private:
    /** a new message in the Q */
    EventSem        newMessage;

    /** */
    bool Lock(TimeoutType tt = TTInfiniteWait){
        return mux.Lock(tt);
    }

    /** */
    bool UnLock(){
        return mux.UnLock();
    }

    /** Bool to know whether the message handler destructor has been called */
    bool isDying;

public:
    /** */
    MessageQueue(){
        isDying = False;
        newMessage.Create();
    }

    /** */
    void SetObjectName(const char *name){
        GCReferenceContainer::SetObjectName(name);
    }

    /** */
    virtual ~MessageQueue(){
        newMessage.Close();
    }

    /** Adds at one side of the Queue */
    bool SendMessage(GCRTemplate<MessageEnvelope> envelope, TimeoutType tt = TTInfiniteWait){
        Lock(tt);
        bool ret = Insert(envelope);
        newMessage.Post();
        UnLock();
        return ret;
    }

    /** */
    GCRTemplate<MessageEnvelope> GetMessage(TimeoutType tt = TTInfiniteWait){
        GCRTemplate<MessageEnvelope> answer;
        if (Lock(tt)){
            while (Size() ==0) {
                newMessage.Reset();
                UnLock();

                /* timeout waiting for data */
                if (!newMessage.Wait(tt)) return answer;
		if (isDying){
		    return answer;
		}

                /* timout locking Q */
                if (!Lock(tt)) return answer;

            }

            answer = Remove((int) 0);

            UnLock();
        }
        return answer;
    }

    /** Number of messages in the queue*/
    int32 Size(){
        return GCReferenceContainer::Size();
    }

    /** Post the synchronisation semaphore. */
    void Reset(){
        isDying = True;
        newMessage.Post();
    }
};


#endif
