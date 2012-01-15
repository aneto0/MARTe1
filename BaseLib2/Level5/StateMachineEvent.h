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
 * An event contained(referred to) within a state
 */
#if !defined(_STATE_MACHINE_EVENTS)
#define _STATE_MACHINE_EVENTS

#include "GCReferenceContainer.h"
#include "FString.h"

class StateMachine;

class StateMachineEvent;

extern "C"{

    bool SMEObjectLoadSetup(StateMachineEvent &sme,ConfigurationDataBase &info,StreamInterface *err);

    bool SMEObjectSaveSetup(StateMachineEvent &sme,ConfigurationDataBase &info,StreamInterface *err);

}

class StateMachineState;

OBJECT_DLL(StateMachineEvent)

/** This is a container of MessageDeliveryRequest
It als+o can contains a set of StateMachineStateRefere*/
class StateMachineEvent: public GCReferenceContainer{

    friend bool SMEObjectLoadSetup(StateMachineEvent &sme,ConfigurationDataBase &info,StreamInterface *err);

    friend bool SMEObjectSaveSetup(StateMachineEvent &sme,ConfigurationDataBase &info,StreamInterface *err);

OBJECT_DLL_STUFF(StateMachineEvent)

    /** the code associated with this event 0 means use the value*/
    int32               code;

    /** the value associated with this event */
    BString             value;

    /** when this event occurs what is the next state */
    BString             nextState;

    /** if the actions fail what is the next state */
    BString             errorState;

public:

    /** Performs the action associate with a trigger */
    bool                            Trigger(
                                StateMachine &                  sm,
                                int32                           code,
                                const char *                    value,
                                StateMachineState &             currentState,
                                FString &                       newStateName,
                                int32                           verboseLevel);

    /**  initialise an object from a set of configs
        The syntax is the same as GCReferenceContainer
        The container content should be only of class MessageEnvelope or MessageDeliveryRequest
        important parameters are
            Code (an integer) or UserCode (an integer)
            Value (a string) that identify the trigger
            NextState (a string) is the next state and
            ErrorState (a string) is the next state on error
        The object name is taken from the current node name.
        If the object is called DEFAULT than it matches any message code
        If a MessageEnvelope or MessageDeliveryRequest called SENDSTATE
        is found than the current state is added as content.
        In this case the MessageEnvelope and MDR are duplicated.
        Note that the current state not the final state are sent
    */
    virtual     bool                ObjectLoadSetup(
                                ConfigurationDataBase &         info,
                                StreamInterface *               err)
    {
        return SMEObjectLoadSetup(*this,info,err);
    }

    /**  save an object to a set of configs
        See ObjectLoadSetup  */
           virtual bool             ObjectSaveSetup(
                                ConfigurationDataBase &         info,
                                StreamInterface *               err)
    {
        return SMEObjectSaveSetup(*this,info,err);
    }


    /** the code associated with this event 0 means use the value*/
    int32                           Code()
    {
        return code;
    }

    /** the value associated with this event */
    const char *                    Value()
    {
        return value.Buffer();
    }

    /** when this event occurs what is the next state */
    const char *                    NextState()
    {
        return nextState.Buffer();
    }

    /** if the actions fail what is the next state */
    const char *                    ErrorState()
    {
        return errorState.Buffer();
    }

};


#endif

