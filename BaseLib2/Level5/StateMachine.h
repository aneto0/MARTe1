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
 * A generic state machine that reacts to messages and acts by sending messages
 */
#if !defined(_STATE_MACHINE_)
#define _STATE_MACHINE_

#include "MessageHandler.h"
#include "GCReferenceContainer.h"
#include "StateMachineState.h"
#include "SXMemory.h"
#include "ConfigurationDataBase.h"
#include "HttpInterface.h"

class StateMachine;

extern "C"{

    bool SMObjectLoadSetup(StateMachine &sm,ConfigurationDataBase &info,StreamInterface *err);

    bool SMObjectSaveSetup(StateMachine &sm,ConfigurationDataBase &info,StreamInterface *err);

    bool SMProcessMessage(StateMachine &sm,GCRTemplate<MessageEnvelope> envelope);

    bool SMProcessHttpMessage(StateMachine &sm,HttpStream &hStream);

}


OBJECT_DLL(StateMachine)

/** This is a container of StateMachineState and a MessageHandler
    When it receives a message it matches it to the StateMachineEvent
      contained in the currentState
    If code matches or if (code==0) content matches than it sends all
      the messages contained in StateMachineEvent.
    A reply to the original message is sent with the parameters
      (State_code,"State_label") if the original message required manual reply
    ErrorState is the state the machine reaches if the specified
      state cannot be found. By default its value is ERROR but if ERROR
      does not exist then the state will not change.
    See StateMachineState for the ENTER EXIT action objects and for the Events.
      If any of the ENTER or EXIT actions fail the state machine will reach ErrorState
    DEFAULT is a state whose Events and ENTER/EXIT actions are appended to those of the
      current state
    if the next state is specified as SAMESTATE this means that not change of state
      will occurr
        If a MessageEnvelope or MessageDeliveryRequest with the first
        object contained being a Message  called SENDSTATE is found than
        the current state is used as content for that Message.
    In this case the MessageEnvelope and MDR are duplicated
    Note that the current state not the final state are sent
*/
class StateMachine: public GCReferenceContainer, public MessageHandler,public HttpInterface{

OBJECT_DLL_STUFF(StateMachine)

    friend bool SMObjectLoadSetup(StateMachine &sm,ConfigurationDataBase &info,StreamInterface *err);

    friend bool SMObjectSaveSetup(StateMachine &sm,ConfigurationDataBase &info,StreamInterface *err);

    friend bool SMProcessMessage(StateMachine &sm,GCRTemplate<MessageEnvelope> envelope);

    friend bool SMProcessHttpMessage(StateMachine &sm,HttpStream &hStream);

    /** refrence to the currently active state: initialised with the first state */
    GCRTemplate<StateMachineState>  currentState;

    /** refrence to the default error state: last state or the one called ERROR */
    GCRTemplate<StateMachineState>  errorState;

    /** Reference to a state called DEFAULT.
        It ENTER and EXIT messages are executed before and after the state specific ones
        at every state change */
    GCRTemplate<StateMachineState>  defaultState;

    /** The name of the error state. bu default ERROR*/
    BString                         errorStateName;

    /** verboseLevel = 0 means no diagnostics 1 shows all the state changes */
    int32                           verboseLevel;

    /** implement a change of state
        if newStateName = SAMESTATE will not change state
        returns True if the state has changed */
    bool ChangeState(const char *newStateName);

    /** Process the @param code or @param value to determine the next state */
    bool Trigger(int code,const char *value);

public:

    /** The message handling routine */
    virtual     bool                ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
        return SMProcessMessage(*this,envelope);
    }

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream){
        return SMProcessHttpMessage(*this,hStream);
    }


    /* Creates a Menu Tree creating a stream first then a CDB and
        then using ObjectLoadSetup  */
                bool                Init(
                        const char *                    configuration,
                        Streamable *                    err             = NULL)
    {
        SXMemory config((char *)configuration,strlen(configuration));
        ConfigurationDataBase cdb;
        if (!cdb->ReadFromStream(config,err)){
            AssertErrorCondition(ParametersError,"Init: cdb.ReadFromStream failed");
            return False;
        }
        return ObjectLoadSetup(cdb,err);
    }

    /**  initialise an object from a set of configs
        The syntax is the same as GCReferenceContainer
        The container content should be only of class StateMachinState
        OPtional parameter is ErrorStateName which allows selecting the
        ErrorState
        If NextState is SAMESTATE no change will occur
        VerboseLevel {integer} if set >0 will cause diagnostic messages to appear
            >=1 shows status changes
            >=2 shows actions performed
            >=10 shows message processing details
        The object name is taken from the current node name  */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return SMObjectLoadSetup(*this,info,err);
    }

    /**  save an object to a set of configs
        The syntax is the same as GCReferenceContainer
        The container content should be only of class StateMachinState
        The object name is taken from the current node name  */
           virtual bool             ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return SMObjectSaveSetup(*this,info,err);
    }


    /** */


};


#endif



