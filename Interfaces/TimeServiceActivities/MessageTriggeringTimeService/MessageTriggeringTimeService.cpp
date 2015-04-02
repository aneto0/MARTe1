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

#include "MessageTriggeringTimeService.h"
#include "TimeTriggeringServiceInterface.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"
#include "MessageDispatcher.h"

void MessageTriggerFn(MessageTriggeringTimeService &mtts){
    mtts.MessageTrigger();
}

void MessageTriggeringTimeService::MessageTriggered(){
    int32 i    = 0;
    int32 j    = 0;
    int64 data = 0;

    //Get data from driver
    driver->GetData((int32)usecTime, (int32 *)dataBuffer);
    for(i=0; i<nMaskObjs; i++){
        if(numberOfBytesPerChannel == 4)
            data = *((int32 *)(dataBuffer + maskList[i]->channelNumber * numberOfBytesPerChannel));
        else if(numberOfBytesPerChannel == 8)
            data = *((int64 *)(dataBuffer + maskList[i]->channelNumber * numberOfBytesPerChannel));
        else if(numberOfBytesPerChannel == 2)
            data = *((int16 *)(dataBuffer + maskList[i]->channelNumber * numberOfBytesPerChannel));
        else if(numberOfBytesPerChannel == 1)
            data = *(dataBuffer + maskList[i]->channelNumber * numberOfBytesPerChannel);
        //If data is valid
        if(maskList[i]->AssertMask(data)){
            //Send all associated messages
            for(j=0; j<maskList[i]->Size(); j++){
                GMDSendMessageDeliveryRequest(maskList[i]->Find(j), TTInfiniteWait ,False);
            }
        }
    }
}

void MessageTriggeringTimeService::MessageTrigger(){
    Threads::SetRealTimeClass();
    Threads::SetPriorityLevel(messageTriggerPriority);
    //To synch with Start()
    alive      = True;

    while(alive){
        //New cycle
        messageTriggerSem.ResetWait();
        MessageTriggered();    
    }
    //To synch with Stop()
    alive = True;
}

bool MessageTriggeringTimeService::Start(){
    if(!driver.IsValid()) {
        AssertErrorCondition(FatalError, "%s::Start(): driver is not valid.", Name());
        return False;
    }

    //Get the number of channels from the driver. Must be done here to assure the driver was already configured
    int32 nInputChannels = driver->NumberOfInputs();
    //alocate the memory to read from the driver
    dataBuffer = (char *)malloc(numberOfBytesPerChannel * nInputChannels);
    if(dataBuffer == NULL){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: Failed to allocate %d bytes for buffer to read from driver", Name(), numberOfBytesPerChannel * nInputChannels);
        return False;
    }

    if(useThread){
        alive             = False;
        messageTriggerTID = Threads::BeginThread((void (__thread_decl *)(void *))&MessageTriggerFn,this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
        while(!alive){
            SleepMsec(10);
        }
    }
    return True;
}

bool MessageTriggeringTimeService::Stop(){
    if(messageTriggerTID == 0){
        return True;
    }
    alive = False;
    if(useThread){
        messageTriggerSem.Post();
        int32 exitCounter = 0;
        while(!alive){
            exitCounter++;
            SleepMsec(10);
            if(exitCounter > 100){
                AssertErrorCondition(FatalError, "%s::Stop(): failed to stop the message sending thread. Waited for 1 second.", Name());
                break;
            }
        }
        if(exitCounter > 100){
            Threads::Kill(messageTriggerTID);
        }
        messageTriggerTID = 0;
    }

    if(dataBuffer != NULL){
        free((void *&)dataBuffer);
    }
    return True;
}

bool MessageTriggeringTimeService::Trigger(int64 time){
    usecTime = time;
    if(useThread){
        messageTriggerSem.fastPost();
    }
    else{
        MessageTriggered();
    }
    return True;
}

bool MessageTriggeringTimeService::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    if(!TimeServiceActivity::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"MessageTriggeringTimeService::ObjectLoadSetup: %s: TimeServiceActivity::ObjectLoadSetup Failed",Name());
        return False;
    }
    CDBExtended cdb(info);
    //Try to get a reference for the driver
    FString driverLoc;
    if(!cdb.ReadFString(driverLoc, "Driver")){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: Please specify the location of the driver in the cfg", Name());
        return False;
    }
    //Get the data word size for each channel
    if(!cdb.ReadInt32(numberOfBytesPerChannel, "NumberOfBytesPerChannel")){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: NumberOfBytesPerChannel was not specified", Name());
        return False;
    }

    //CPU mask for the thread
    if(!cdb.ReadInt32(cpuMask, "CPUMask")){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: CPUMask was not specified", Name());
        return False;
    }

    //If to start an independent thread
    FString useThreadStr;
    cdb.ReadFString(useThreadStr, "UseThread");
    if((useThreadStr == "False") || (useThreadStr == "false") || (useThreadStr == "FALSE") || (useThreadStr == "no") || (useThreadStr == "No") || (useThreadStr == "NO")){
        useThread = False;
    }
    else{
        useThread = True;
    }

    driver = GetGlobalObjectDataBase()->Find(driverLoc.Buffer());
    if(!driver.IsValid()){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: Invalid or not found driver reference:%s", Name(), driverLoc.Buffer());
        return False;
    }

    nMaskObjs = Size();
    if(nMaskObjs < 1){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: At least one MessageTriggeringMask must be specified", Name());
        return False;
    }
    maskList = new GCRTemplate<MessageTriggeringMask>[nMaskObjs];
    if(maskList == NULL){
        AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: Failed creating the mask list", Name());
        return False;
    }
    int32 i=0;
    for(i=0; i<nMaskObjs; i++){
        GCRTemplate<MessageTriggeringMask> maskObj = Find(i);
        if(!maskObj.IsValid()){
            AssertErrorCondition(InitialisationError, "MessageTriggeringTimeService::ObjectLoadSetup: %s: Found ilegal object at position:%d. Type must be MessageTriggeringMask", Name(), i);
            return False;
        }
        maskList[i] = maskObj;
    }

    cdb.ReadInt32(messageTriggerPriority, "MessageTriggerPriority", 31);
    return True;
}

OBJECTLOADREGISTER(MessageTriggeringTimeService, "$Id$")

