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

#include "MessageDispatcher.h"
#include "FastPollingMutexSem.h"
#include "GlobalObjectDataBase.h"

OBJECTLOADREGISTER(MessageDispatcher,"$Id: MessageDispatcher.cpp,v 1.10 2008/09/26 08:27:10 fpiccolo Exp $")


static FastPollingMutexSem mux;

GCRTemplate<MessageDispatcher> globalMessageDispatcher;


MessageDispatcher& GlobalMessageDispatcher(){
    mux.FastLock();
    if (!globalMessageDispatcher.IsValid()){
        GCRTemplate<MessageDispatcher> newMessageDispatcher(GCFT_Create);
        globalMessageDispatcher = newMessageDispatcher;
        globalMessageDispatcher->SetObjectName("GlobalMessageDispatcher");
        GetGlobalObjectDataBase()->Insert(globalMessageDispatcher);
        // This sleep is to allow the queue processing task to start before sending the message in GMDSendDeliveryRequest.
        SleepSec(0.01);
    }

    mux.FastUnLock();

    return *(globalMessageDispatcher.operator->());
}


void MessageDispatcherThreadFN(void *arg){

    Threads::SetPriorityLevel(0);
    MessageDispatcher *md = (MessageDispatcher *)arg;

    // calls the method
    if (md != NULL){

        // mark thread start!
        md->threadStartEvent.Post();

        // let the main task continue and Reset the semaphore
//        SleepSec(0.1);

        // do the task
        md->ProcessRequestQueue();

        // mark thread end!
        md->mDThreadID = (TID)0;

        // wake other task
        md->threadStartEvent.Post();

    }

}


bool MDProcessRequestQueue(MessageDispatcher &md){
    md.threadToContinue = True;

    // check for any request to abandon ship
    while(md.threadToContinue){
        // synchronise
        if (md.requestsEvent.Wait(md.globalTimeout)){
            md.requestsEvent.Reset();
            // check for any request to abandon ship
            bool toContinue = md.threadToContinue;
            while(toContinue){

                // access resources
                if (md.requestsMux.FastLock(md.globalTimeout)){
                    // check Q size
                    if (md.requestsQueue.Size() > 0){

                        GCReference request;
                        request = md.requestsQueue.Remove(0);

                        // process request here
                        GCRTemplate<MessageEnvelope> envelope;
                        envelope = request;

                        // try to see if it is a simple envelope
                        if (envelope.IsValid()){

                            // UnLock Queue
                            md.requestsMux.FastUnLock();
                            MessageHandler::SendMessage(envelope);
                        } else {

                            // try to see if it is a Message Delivery Request
                            GCRTemplate<MessageDeliveryRequest> delivery;
                            delivery = request;
                            if (delivery.IsValid()){

                                // add this delivery to the wait Q
                                if (delivery->ReplyExpected()){
                                    md.waitQueue.Insert(delivery);
                                }

                                // UnLock Queue after possibly having added
                                // a new element to the waitQueue
                                md.requestsMux.FastUnLock();

                                // delegate the job to the Message Delivery Request object
                                delivery->ProcessMDR(&md);

                            } else { // try to cast to Object simply

                                // UnLock Queue
                                md.requestsMux.FastUnLock();

                                GCRTemplate<Object> object;
                                object = request;
                                if (object.IsValid()){
                                    md.AssertErrorCondition(FatalError,"ProcessQueue:Object of class %s discarded",object->ClassName());
                                } else {
                                    md.AssertErrorCondition(FatalError,"ProcessQueue:Unknown object in queue discarded");
                                }

                            } // !delivery.IsValid()

                        } // !envelope.IsValid()

                    } else {
                        md.requestsMux.FastUnLock();
                        toContinue = False;
                    } // if md.requestQueue.Size() > 0

                } // if (requestsMux.FastLock(timeout))

                // check for any request to abandon ship
                toContinue = toContinue && md.threadToContinue;

            } // while(toContinue)

        } // eventSem Wait

    } // while(threadToContinue)

    return True;

} // end MDProcessRequestQueue


bool MDProcessMessage(
            MessageDispatcher &             md,
            GCRTemplate<MessageEnvelope>    envelope){

    GCRTemplate<Message> message;
    if (envelope->AutomaticReply()) {
        message = envelope->GetMessage();
    } else {
        message = envelope->GetOriginal();
    }
    if (message.IsValid()){

        if (md.requestsMux.FastLock(md.globalTimeout)){

            for (int i = 0;i<md.waitQueue.Size();i++){
                GCRTemplate<MessageDeliveryRequest> mdr;
                mdr = md.waitQueue.Find(i);
                if (mdr->GetMessage() == message){
                    if (mdr->Acknowledge()){
                        md.waitQueue.Remove(i);
                        md.requestsMux.FastUnLock();
                        return True;
                    }
                }
            }
            md.requestsMux.FastUnLock();
        } else {
            md.AssertErrorCondition(Timeout,"MDProcessMessage: timeout accessing waitQueue");
        }

    }
    return True;
}

