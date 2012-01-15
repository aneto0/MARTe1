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
 * A state within the state machine
 */ 
#if !defined(_STATE_MACHINE_STATE)
#define _STATE_MACHINE_STATE

#include "GCReferenceContainer.h"
#include "MessageDispatcher.h"
#include "FString.h"
#include "StateMachineEvent.h"
#include "CDBExtended.h"

class StateMachineState;

extern "C"{

    bool SMSObjectLoadSetup(StateMachineState &sms,ConfigurationDataBase &info,StreamInterface *err);

    bool SMSObjectSaveSetup(StateMachineState &sms,ConfigurationDataBase &info,StreamInterface *err);

}

/** */
enum SMS_ActOnResults{

    SMS_NotFound    = 0,

    SMS_Ok          = 1,

    SMS_Error       = -1,

} ;

OBJECT_DLL(StateMachineState)

/** This is a container of StateMachineEvents or MessageDeliveryRequest
    There can be only 2 MessageDeliveryRequest one named ENTER and one EXIT
    These are messages that are delivered on entering or exiting the state
  */
class StateMachineState: public GCReferenceContainer{

    friend bool SMSObjectLoadSetup(StateMachineState &sms,ConfigurationDataBase &info,StreamInterface *err);

    friend bool SMSObjectSaveSetup(StateMachineState &sms,ConfigurationDataBase &info,StreamInterface *err);


OBJECT_DLL_STUFF(StateMachineState)

private:

    /** a code associated with this state */
    int32               stateCode;

    /** Perform the activity called @param name
        returns false if the action was not found or in case of errors */
    SMS_ActOnResults                ActOn(
                        StateMachine &                  sm,
                        const char *                    name);
public:

    /** constructor */
                                    StateMachineState()
    {
        stateCode = 0;
    }


    /** destructor */
    virtual                         ~StateMachineState()
    {
    }

    /** Find this trigger and execute the associated actions
        returns the new state */
    bool                            Trigger(
                        StateMachine &                  sm,
                        int32                           code,
                        const char *                    value,
                        FString &                       newStateName,
                        int32                           verboseLevel);

    /** */
    inline SMS_ActOnResults          Enter(
                        StateMachine &                  sm)
    {
        return ActOn(sm,"ENTER");
    }

    /** */
    inline SMS_ActOnResults          Exit(
                        StateMachine &                  sm)
    {
        return ActOn(sm,"EXIT");
    }

    /**  initialise an object from a set of configs
        The syntax is the same as GCReferenceContainer
        One shall also define a code using StateCode = nnn
        The main container content should be only of class StateMachineEvent
        If an object called ENTER is added of type MessageDeliveryRequest or
        MessageEnvelope than a message is sent on entry
        If an object called EXIT is added of type MessageDeliveryRequest or
        MessageEnvelope than a message is sent on exit
        If a StateMachineEvent called DEFAULT is encountered while matching the trigger
        then a match is always automatically found
        The object name is taken from the current node name
        If a MessageEnvelope or MessageDeliveryRequest with the first
        object contained being a Message  called SENDSTATE is found than
        the current state is used as content for that Message.
        In this case the MessageEnvelope and MDR are duplicated
        Note that the current state not the final state are sent
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        bool ret = SMSObjectLoadSetup(*this,info,err);

        CDBExtended cdbx(info);
        if (!cdbx.ReadInt32(stateCode,"StateCode",0)){
            AssertErrorCondition(Warning,"ObjectLoadSetup: StateCode undefined using code 0x%x",this);
            stateCode = (int32)(size_t)this;
        }

        return ret;
    }

    /**  save an object to a set of configs
        The syntax is the same as GCReferenceContainer
        The container content should be only of class StateMachinState
        The object name is taken from the current node name  */
           virtual bool             ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        bool ret = SMSObjectSaveSetup(*this,info,err);

        CDBExtended cdbx(info);
        cdbx.WriteInt32(stateCode,"StateCode");

        return ret;
    }

    /** retrieve the code associated with the state */
    inline int32                        StateCode()
    {
        return stateCode;
    }

};

#endif

