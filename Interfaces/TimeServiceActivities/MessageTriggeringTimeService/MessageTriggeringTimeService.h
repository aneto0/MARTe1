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

#ifndef MESSAGE_TRIGGERING_TIME_SERVICE
#define MESSAGE_TRIGGERING_TIME_SERVICE

#include "TimeServiceActivity.h"
#include "GenericAcqModule.h"
#include "Threads.h"
#include "EventSem.h"
#include "MessageHandler.h"
#include "MessageDeliveryRequest.h"
#include "MessageTriggeringMask.h"

/**
 * @file 
 * This TimeServiceActivity will compare data from a driver against user configured
 * masks. If the mask is valid it will trigger the send of one or more messages.
 */

OBJECT_DLL(MessageTriggeringTimeService)

class MessageTriggeringTimeService: public TimeServiceActivity, public MessageHandler{

OBJECT_DLL_STUFF(MessageTriggeringTimeService)

private:

    /**
     * Messages are sent using a different thread in order to avoid blocking the trigger 
     */
    friend void MessageTriggerFn(MessageTriggeringTimeService &mtts);

    /**
     * The driver where data is read from
     */
    GCRTemplate<GenericAcqModule> driver;

    /**
     * Number of bytes retrieved in each acquistion channel of the driver. This is typically
     * 4 in a 32 bit system and 8 in a 64 bit system
     */
    int32 numberOfBytesPerChannel;

    /**
     * Buffer where data read from the driver is stored
     */
    char *dataBuffer;

    /**
     * List of all the registered MessageTriggeringMask objects
     */
    GCRTemplate<MessageTriggeringMask> *maskList;

    /**
     * Number of registered MessageTriggeringMask objects
     */
    int32 nMaskObjs;

    /**
     * Semaphore to wake the thread when a new message is to be sent
     */
    EventSem messageTriggerSem;

    /**
     * The id of the thread sending the messages
     */
    TID messageTriggerTID;

    /** 
     * The priority of the thread sending the messages
     */
    int32 messageTriggerPriority;

    /**
     * The cpu mask for the message trigger thread
     */
    int32 cpuMask;

    /**
     * Set to true while the component is alive
     */
    bool alive;
    
    /**
     * Callback function of the thread
     */ 
    void MessageTrigger();

    /**
     * Analyses the input data and send the messages if any of the masks
     * is valid
     */ 
    void MessageTriggered();

    /**
     * The last value of time when Trigger was called
     */
    int64 usecTime;

    /**
     * True if the mask and message sending should be handled in
     * a different thread
     */
    bool useThread;

public:

    MessageTriggeringTimeService(){
        numberOfBytesPerChannel = 0;
        messageTriggerTID       = 0;
        messageTriggerPriority  = 0;
        nMaskObjs               = 0;
        usecTime                = 0;
        cpuMask                 = 0;
        maskList                = NULL;
        dataBuffer              = NULL;
        alive                   = False;
        useThread               = True;

        messageTriggerSem.Create();
    }

    virtual ~MessageTriggeringTimeService(){
        Stop();
        if(maskList != NULL){
            delete []maskList;
        }
        if(dataBuffer != NULL){
            (void *&)dataBuffer;
        }

        messageTriggerSem.Close();
    }

    /**
     * Starts the message sending thread
     */
    bool Start();

    /**
     * Stops the message sending thread
     */
    bool Stop();

    /**
     * @sa TimeServiceActivity
     */
    virtual bool Trigger(int64 usecTime);

    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
};

#endif

