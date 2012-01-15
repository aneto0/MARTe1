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

#include "TimeTriggeringServiceInterface.h"
#include "GlobalObjectDataBase.h"
#include "CDBExtended.h"
#include "Message.h"
#include "GenericAcqModule.h"

bool TTSIObjectLoadSetup(TimeTriggeringServiceInterface &ttis, ConfigurationDataBase &info,StreamInterface *err){

    ttis.CleanUp();
    CDBExtended cdb(info);

    // Load time service period
    if(!cdb.ReadInt32(ttis.tsOnlineUsecPeriod,"TsOnlineUsecPeriod")){
        ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: TsOnlineUsecPeriod not declared");
        return False;
    }

    // Load time service phase
    if(!cdb.ReadInt32(ttis.tsOnlineUsecPhase,"TsOnlineUsecPhase")){
        ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: TsOnlineUsecPhase not declared");
        return False;
    }

    // Load time service period
    if(!cdb.ReadInt32(ttis.tsOfflineUsecPeriod,"TsOfflineUsecPeriod")){
        ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: TsOfflineUsecPeriod not declared");
        return False;
    }

    // Load time service phase
    if(!cdb.ReadInt32(ttis.tsOfflineUsecPhase,"TsOfflineUsecPhase")){
        ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: TsOfflineUsecPhase not declared");
        return False;
    }

    // Load time service timout value
    if(!cdb.ReadInt32(ttis.tsTimeOutMsecTime, "TsTimeOutMsecTime", -1)){
      ttis.AssertErrorCondition(Warning, "ExternalTimeTriggeringService::ObjectLoadSetup: TsTimeOutMsecTime not found");
    }
    
    //////////////////////
    // Init Time Module //
    //////////////////////

    if(!cdb->Exists("TimeModule")){
        ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: TimeModule has not been specified");
        return False;
    }

    {
        cdb->Move("TimeModule");

        FString boardName;
        if(!cdb.ReadFString(boardName,"BoardName")){
            ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: Failed reading TimeModule BoardName");
            return False;
        }

        GCReference module = GetGlobalObjectDataBase()->Find(boardName.Buffer(),GCFT_Recurse);
        if(!module.IsValid()){
            ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: Failed retrieving reference for %s in GlobalContainer", boardName.Buffer());
            return False;
        }

        ttis.timeModule = module;
        if(!ttis.timeModule.IsValid()){
            ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: The object %s is not of GenericAcqModule Type", boardName.Buffer());
            return False;
        }

        cdb->MoveToFather();
    }

    /////////////////////////////////
    // Add Time Service Activities //
    /////////////////////////////////

    if(ttis.timeActivities != NULL) delete[] ttis.timeActivities;

    if(!ttis.GCReferenceContainer::ObjectLoadSetup(cdb,err)){
        ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: GCReferenceContainer::ObjectLoadSetup() Failed");
        return False;
    }

    int32 nOfRegisteredActivities = ttis.Size();
    if(nOfRegisteredActivities != 0){
        ttis.timeActivities = new GCRTemplate<TimeServiceActivity>[nOfRegisteredActivities];
        if(ttis.timeActivities == NULL){
            ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: Failed allocating memory for %d TimeServiceActivities",ttis.Size());
            return False;
        }

        for(int activity = 0; activity < nOfRegisteredActivities; activity++){
            GCReference gc = ttis.Find(activity);
            GCRTemplate<TimeServiceActivity> activityReference(gc);
            if(activityReference.IsValid()){
                ttis.timeActivities[activity] = activityReference;
            }else{
                ttis.AssertErrorCondition(InitialisationError,"ExternalTimeTriggeringService::ObjectLoadSetup: GCReference[%d] is not of type TimeServiceActivity",activity);
                return False;
            }
        }
    }else{
        ttis.AssertErrorCondition(Information,"ExternalTimeTriggeringService::ObjectLoadSetup: No TimeServiceActivity has been specified");
    }

    ttis.AssertErrorCondition(Information,"ExternalTimeTriggeringService Initialized Correctly");
    return True;
}

bool TTSITrigger(TimeTriggeringServiceInterface &ttis){

    // Get time in usec from timeModule
    int64 usecTime = ttis.timeModule->GetUsecTime();
    // Checks phase in the period &
    // updates timeInfo if the phase is good
    if(ttis.onlinePulsing){
#ifndef _RTAI
        int64 module = usecTime % ttis.tsOnlineUsecPeriod;
#else
        int64 temp   = usecTime;
        int64 module = do_div(temp, ttis.tsOnlineUsecPeriod);
#endif
        if(module == ttis.tsOnlineUsecPhase){
            // The period time is stored in usec
            ttis.timeInfo[ttis.writeBuffer].lastPeriodUsecTime    = usecTime;
            // Save Processor internal counter value
            // It will be used to compute the Internal Cycle Time
            ttis.timeInfo[ttis.writeBuffer].lastProcessorTickTime = HRT::HRTCounter();
            // Switch write-only buffer index
            ttis.writeBuffer = 1-ttis.writeBuffer;
            // Signal the Start of a new real time cycle
            ttis.SignalNewCycle();
        }
    }else{
#ifndef _RTAI
        int64 module = usecTime % ttis.tsOfflineUsecPeriod;
#else
        int64 temp   = usecTime;
        int64 module = do_div(temp, ttis.tsOfflineUsecPeriod);        
#endif
        if(module == ttis.tsOfflineUsecPhase){
            // The period time is stored in usec
            ttis.timeInfo[ttis.writeBuffer].lastPeriodUsecTime    = usecTime;
            // Save Processor internal counter value
            // It will be used to compute the Internal Cycle Time
            ttis.timeInfo[ttis.writeBuffer].lastProcessorTickTime = HRT::HRTCounter();
            // Switch write-only buffer index
            ttis.writeBuffer = 1-ttis.writeBuffer;
            // Signal the Start of a new real time cycle
            ttis.SignalNewCycle();
        }
    }

    for(int activity = 0; activity < ttis.Size(); activity++){
        // Activity period
        int32 aUsecPeriod = ttis.timeActivities[activity]->GetActivityUsecPeriod();
        // Activity phase
        int32 aUsecPhase  = ttis.timeActivities[activity]->GetActivityUsecPhase();
        // Check activity phase
#ifndef _RTAI
        int64 module = usecTime % aUsecPeriod;
#else
        int64 temp   = usecTime;
        int64 module = do_div(temp, aUsecPeriod);
#endif
        if(module == aUsecPhase){
            if(!ttis.timeActivities[activity]->Trigger(usecTime)) return False;
        }
    }

    return True;
}

bool TTSIStart(TimeTriggeringServiceInterface &ttis){
    if(!ttis.serviceRunning){
        for(int i = 0; i < ttis.Size(); i++){
            if(!ttis.timeActivities[i]->Start()){
                ttis.AssertErrorCondition(CommunicationError,"%s:Start(): Failed starting service activity for object %s\n",ttis.Name(),ttis.timeActivities[i]->Name());
                // Stop all started activities
                for(int j = i-1; j > 0; j--) ttis.timeActivities[j]->Stop();
                return False;
            }
        }
        // Enable Triggering
        ttis.timeModule->EnableTimeService(&ttis);
        ttis.serviceRunning = True;
    }

    return True;
}

bool TTSIStop(TimeTriggeringServiceInterface &ttis){
    if(ttis.serviceRunning){
        ttis.serviceRunning = False;
        // Stop the Triggering
        ttis.timeModule->DisableTimeService();
        // Stop associated activities
        for(int i = 0; i < ttis.Size(); i++){
            if(!ttis.timeActivities[i]->Stop()){
                ttis.AssertErrorCondition(CommunicationError,"%s:Stop(): Failed starting service activity for object %s\n",ttis.Name(),ttis.timeActivities[i]->Name());
            }
        }
    }
    return True;
}

bool TTSIObjectDescription(TimeTriggeringServiceInterface &ttis, StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("%s %s\n",ttis.ClassName(),ttis.Version());

    s.Printf("Time Service Online Usec period --> %d\n",ttis.tsOnlineUsecPeriod);
    s.Printf("Time Service Online Usec phase  --> %d\n",ttis.tsOnlineUsecPhase);

    s.Printf("Time Service Offline Usec period --> %d\n",ttis.tsOfflineUsecPeriod);
    s.Printf("Time Service Offline Usec phase  --> %d\n",ttis.tsOfflineUsecPhase);

    s.Printf("Time Module Parameters\n");
    ttis.timeModule->ObjectDescription(s,full,err);

    for(int activity = 0; activity < ttis.Size(); activity++){
        s.Printf("\nTime Service Activity #%d\n",activity);
        ttis.timeActivities[activity]->ObjectDescription(s,full,err);
    }

    return True;
}


OBJECTREGISTER(TimeTriggeringServiceInterface,"$Id")
