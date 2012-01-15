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
#if !defined(LOGGER_MESSAGE_QUEUE)
#define LOGGER_MESSAGE_QUEUE

/** @file
    A synchronous FIFO queue which allows to add LoggerMessage  */

#include "GCReferenceContainer.h"
#include "LoggerMessage.h"
#include "EventSem.h"
#include "TimeoutType.h"


class LoggerMessageQueue : public GCReferenceContainer {

private:
    EventSem      waitEventSem;
    
public:
    
    /** constructor */
    LoggerMessageQueue(){
        waitEventSem.Create();
        waitEventSem.Reset();
    }

    /** Destructor */
    virtual ~LoggerMessageQueue()
    {
    }
    
    /** Adds at one side of the Queue */
    bool Add(GCRTemplate<LoggerMessage> message){        
        if(!mux.Lock()){
            AssertErrorCondition(FatalError, "Mux failed");
            return False;
        }
        int size = Size();
        if(!Insert(message)){
            AssertErrorCondition(FatalError, "Add failed");
            mux.UnLock();
            return False;
        }
        if(size == 0){
            waitEventSem.Post();
        }
        mux.UnLock();        
        return True;
    }

    /** Takes one element from the other side */
    GCRTemplate<LoggerMessage> Get(){
        GCRTemplate<LoggerMessage> lm;

        mux.Lock();
        if (Size() == 0){
            waitEventSem.Reset();
            mux.UnLock();
            if(!waitEventSem.Wait(msecTimeout)){
                return lm;
            }
            mux.Lock();
        }
        lm = Remove((int)0);
        mux.UnLock();
                
        return lm;
    }
    
    /** Returns the element at i if it exists. Unblocking method */
    GCRTemplate<LoggerMessage> Peek(uint32 loc){
        return Find(loc);
    }
    
    /** Safely closes the queue */
    void Close(){
        mux.Lock();
        waitEventSem.Reset();
        waitEventSem.Close();
        mux.UnLock();        
    }
};


#endif
