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

#include "LinuxTimerDrv.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "Threads.h"
#include "EventSem.h"
#include "Sleep.h"


void __thread_decl TimerThread(void* data){
    if (data == NULL) {
        CStaticAssertErrorCondition(InitialisationError,"LinuxTimerDrv::TimerThread: data is void!");
        return;
    }
	
    LinuxTimerDrv* Timer = (LinuxTimerDrv*)data;
	
    // Set high priority for this thread
    Threads::SetRealTimeClass();
    Threads::SetPriorityLevel(32);
	
    // Signal correct initialization to ObjectLoadSetup
    Timer->StartStopSem.Post();

    double hrtPeriod = HRT::HRTPeriod();
	
    // Main cycle
    while (Timer->keepAlive) {
        if(Timer->busy == Busy) {
	    SleepBusy((Timer->timerPeriodUsec)*1E-6 - Timer->timeCorrection);
        } else if(Timer->busy == SemiBusy) {
	    SleepSemiBusy((Timer->timerPeriodUsec)*1E-6 - Timer->timeCorrection, Timer->nonBusySleepPeriodUsec*1E-6);
        } else {
	    SleepNoMore((Timer->timerPeriodUsec)*1E-6 - Timer->timeCorrection);
	}
        // Update localTime
	int64 timeMeasurement = HRT::HRTCounter();
    	Timer->InterruptServiceRoutine();
	Timer->timeCorrection = (HRT::HRTCounter() - timeMeasurement) * hrtPeriod;
    }
    
    // Signal correct end of execution to LinuxTimerDrv deconstructor
    Timer->StartStopSem.Post();
    return;
}


// Constructor
LinuxTimerDrv::LinuxTimerDrv(){
    // Init sem
    StartStopSem.Create();
    pollSynchSem.Create();
    localTime      = 0;
    timeCorrection = 0;
    busy           = Default;
    usePolling     = False;
}


// Deconstructor
LinuxTimerDrv::~LinuxTimerDrv(){
    StartStopSem.Reset();
    keepAlive=False;
    // If the thread didn't die by itself, kill it
    if (!StartStopSem.Wait(200)) Threads::Kill(tid);
}


bool LinuxTimerDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"LinuxTimerDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }
    CDBExtended cdb(info);

    cdb.ReadInt32(timerPeriodUsec,"TimerPeriodUsec", 1000);
    AssertErrorCondition(Information,"LinuxTimerDrv::ObjectLoadSetup: %s Timer Period set to %d microseconds.",Name(),timerPeriodUsec);

    FString sleepNatureStr = "";
    cdb.ReadFString(sleepNatureStr, "SleepNature", "Default");
    if(sleepNatureStr == "Default") {
        busy = Default;
    } else if(sleepNatureStr == "Busy") {
        busy = Busy;
    } else if(sleepNatureStr == "SemiBusy") {
        busy = SemiBusy;
	cdb.ReadInt32(nonBusySleepPeriodUsec, "NonBusySleepPeriodUsec", timerPeriodUsec*0.75);
	if(nonBusySleepPeriodUsec > timerPeriodUsec) {
	    AssertErrorCondition(Warning, "LinuxTimerDrv::ObjectLoadSetup: %s NonBusySleepPeriodUsec > TimerPeriodUsec", Name());
	    return False;
	}
    } else {
        AssertErrorCondition(Warning, "LinuxTimerDrv::ObjectLoadSetup: %s SleepNature parameter with unknown value, assuming %s", Name(), sleepNatureStr.Buffer());
        busy = Default;
    }

    FString usePollingStr;
    cdb.ReadFString(usePollingStr, "UsePolling", "no");
    usePolling = ((usePollingStr == "yes") || (usePollingStr == "True") || (usePollingStr == "Yes") || (usePollingStr == "true"));

    // Returns just the time and the packet number
//    numberOfInputChannels  = 2;
    numberOfOutputChannels = 0;

    keepAlive=True;
    int32 runOnCPU = 0;
    cdb.ReadInt32(runOnCPU,"RunOnCPU",0xff);
    AssertErrorCondition(Information,"LinuxTimerDrv::ObjectLoadSetup: %s RunOnCPU set to %d.",Name(),runOnCPU);
    tid = Threads::BeginThread(TimerThread,this,THREADS_DEFAULT_STACKSIZE,"TimerThread",XH_NotHandled, runOnCPU);

    // Wait for the thread to start
    StartStopSem.Wait();

    return True;
}

int32 LinuxTimerDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber){
    if(buffer == NULL) return -1;
    // Packet Number
    buffer[0] = (int32)(localTime);
    // Packet Usec Time
    buffer[1] = (int32)(localTime * timerPeriodUsec);
    return True;
}

bool LinuxTimerDrv::ObjectDescription(StreamInterface &s,bool full, StreamInterface *err){
    return True;
}

void LinuxTimerDrv::InterruptServiceRoutine(){
    if(!usePolling){
	localTime++;
        for(int i = 0; i < nOfTriggeringServices; i++){
            triggerService[i].Trigger();
        }
    }
    else{
        pollSynchSem.FastUnLock();
    }
}

bool LinuxTimerDrv::Poll(){
    while(!pollSynchSem.FastTryLock());
    localTime++;
    
    for(int i = 0; i < nOfTriggeringServices; i++){
        triggerService[i].Trigger();
    }
    return True;
}

OBJECTLOADREGISTER(LinuxTimerDrv,"$Id: LinuxTimerDrv.cpp,v 1.24 2011/07/21 15:04:59 aneto Exp $")
