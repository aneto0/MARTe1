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
 * A special object that allows two programs communicate seamlessly.
 */
#if !defined(_MESSAGE_SERVER_)
#define _MESSAGE_SERVER_

#include "GCReferenceContainer.h"
#include "MessageDeliveryRequest.h"
#include "GCNamedObject.h"
#include "MessageEnvelope.h"
#include "StartStopMessageHandlerInterface.h"


class MessageServer;

extern "C"{

    /** */
    void                            MessageServerThreadFN(void *arg);

    /** initialise object */
    bool                            MSObjectLoadSetup(MessageServer &ms,ConfigurationDataBase &info,StreamInterface *err);

    /** download settings */
    bool                            MSObjectSaveSetup(MessageServer &ms,ConfigurationDataBase &info,StreamInterface *err);

    /** the UDP port used by the message server */
    int32                           MSGetServerPort();

}



OBJECT_DLL(MessageServer)


/** This is the message incoming server  */
class MessageServer: public GCNamedObject,public StartStopMessageHandlerInterface{

OBJECT_DLL_STUFF(MessageServer)

    friend  void        MSProcessIncomingMessages(MessageServer &ms);

    friend  void        MessageServerThreadFN(void *arg);

    friend  bool        MSObjectLoadSetup(MessageServer &ms,ConfigurationDataBase &info,StreamInterface *err);

    friend  bool        MSObjectSaveSetup(MessageServer &ms,ConfigurationDataBase &info,StreamInterface *err);

protected:

    /** the UDP port to receive from */
            int32                                       serverPort;

    /** to mark the start of the handling thread */
            EventSem                                    threadStartEvent;

    /** to request the end of the processing thread */
            bool                                        threadToContinue;

    /** The thread ID. if not 0 it means a thread is running
        the variable is protected */
            TID                                         msThreadID;

    /** timeout in use: wait for messages for this amount then check for closure condition  */
            TimeoutType                                 globalTimeout;

public:
    /** constructor */
                                    MessageServer()
    {

        msThreadID  = (TID)0;

        globalTimeout = 1000;

        threadStartEvent.Create();

        // will be marked True by the thread itself when starting
        threadToContinue = False;

        serverPort = 0;

    }

    /** Destructor */
    virtual                         ~MessageServer()
    {
        Stop();
    }

    /** Implements the Start Function */
    virtual bool Start(){
        if (!Stop()){
            AssertErrorCondition(FatalError,"Start: Cannot assess/obtain a thraed stopped condition");
            return False;
        }

        if (serverPort == 0){
            AssertErrorCondition(FatalError,"Start: object not initialised: serverPort = 0 ");
            return False;
        }

        // will be marked True by the thread itself when starting
        threadToContinue = False;

        // remove events
        threadStartEvent.Reset();

        // label this thread
        FString threadName;
        threadName.Printf("MessageServer:%s",Name());

        // create a new one
        msThreadID = Threads::BeginThread(MessageServerThreadFN,this,THREADS_DEFAULT_STACKSIZE,threadName.Buffer());

        // wait for thread start notification
        if (!threadStartEvent.Wait(globalTimeout)){
            AssertErrorCondition(Timeout,"Start: Request Handling Thread did not start within %i msecs",globalTimeout.msecTimeout);
        }
        return True;
    }

    /** Implements the Stop Function */
    virtual bool Stop(){
        if (msThreadID == (TID)0 ) return True;

        threadToContinue = False;

        // wait for thread start notification
        if (!threadStartEvent.Wait(globalTimeout)){
            AssertErrorCondition(Timeout,"Stop: Request Handling Thread did not stop within %i msecs",globalTimeout.msecTimeout);
        }

        if (msThreadID != (TID)0){
            SleepSec(0.1);
        }
        if (msThreadID != (TID)0){
            Threads::Kill(msThreadID);
            AssertErrorCondition(Timeout,"Stop:  Killed thread");
            msThreadID = (TID)0;
        }
        return True;
    }


    /** initialise object */
    virtual     bool                ObjectLoadSetup(
                ConfigurationDataBase &         info,
                StreamInterface *               err){

        return MSObjectLoadSetup(*this,info,err);
    }

    /** save settings */
    virtual     bool                ObjectSaveSetup(
                ConfigurationDataBase &         info,
                StreamInterface *               err){

        return MSObjectSaveSetup(*this,info,err);
    }

};



#endif
