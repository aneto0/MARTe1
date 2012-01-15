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

#include "RealTimeThread.h"
#include "GlobalObjectDataBase.h"
#include "CDBExtended.h"
#include "Threads.h"
#include "GAM.h"

#include "DDBOutputInterface.h"
#include "MessageDispatcher.h"

/// UNINITIALISED STATE - RTApplicationThread has not been initialized
static const int32 RTAPP_UNINITIALISED = 0xFFFFFFFF;
/// READY STATE - RTApplicationThread object has been initialised
static const int32 RTAPP_READY         = 0x00000000;
/// SAFETY STATE
static const int32 RTAPP_SAFETY        = 0x00000002;


/**************************************/
/*  Internal Statuses                 */
/**************************************/

static const int32 SM_IDLE            =             101;
static const int32 SM_WAITING_PRE     =             102;
static const int32 SM_PREPULSE        =             103;
static const int32 SM_PULSING         =             104;
static const int32 SM_POSTPULSE       =             105;
static const int32 SM_INITIALISING    =             106;

/** Used to AddInterface to all GAM type objects in GODB */
class GAMLister : public IteratorT<GCReference> {

    private:

        GCRTemplate<DDB> glddb;

        bool returnValue;

    public:

        bool ReturnValue() {
            return returnValue;
        }

    public:

        /**  */
        GAMLister(GCRTemplate<DDB> &ddbRef) {
            if(ddbRef.IsValid()) {
                glddb = ddbRef;
            }
            returnValue = True;
        }

        virtual ~GAMLister() {
        }

        /** actual function */
        virtual void Do(GCReference data) {
            GCRTemplate<GAM> gcrtgam;
            gcrtgam = data;
            if(gcrtgam.IsValid()) {
                if(!glddb->AddInterface(gcrtgam)) {
                    glddb->AssertErrorCondition(InitialisationError,"GAMLister::Do: Failed adding interface for GAM %s to DDB", gcrtgam->GamName());
                    returnValue &= False;
                    return;
                }
            }
        }

        /** actual function */
        virtual void Do2(GCReference data, SFTestType mode) {
            if (mode == SFTTNull) {
                Do(data);
            }
        }
};

/** Used to CreateLink to all GAM type objects in GODB */
class GAMLister2 : public IteratorT<GCReference> {

    private:

        GCRTemplate<DDB> glddb;

        bool returnValue;

    public:

        bool ReturnValue() {
            return returnValue;
        }

    public:

        /**  */
        GAMLister2(GCRTemplate<DDB> &ddbRef) {
            if(ddbRef.IsValid()) {
                glddb = ddbRef;
            }
            returnValue = True;
        }

        virtual ~GAMLister2() {
        }

        /** actual function */
        virtual void Do(GCReference data) {
            GCRTemplate<GAM> gcrtgam;
            gcrtgam = data;
            if(gcrtgam.IsValid()) {
                if(!glddb->CreateLink(gcrtgam)){
                    glddb->AssertErrorCondition(InitialisationError,"GAMLister2::Do: CreateLink Failed for GAM %s", gcrtgam->GamName());
                    returnValue &= False;
                    return;
                }
            }
        }

        /** actual function */
        virtual void Do2(GCReference data, SFTestType mode) {
            if (mode == SFTTNull) {
                Do(data);
            }
        }
};

bool ExecutionModule::ObjectLoadSetup( FString gamName, GAM_FunctionNumbers gamCode, StreamInterface *err, RealTimeThread &rt){
    GCReference gcR = rt.Find(gamName.Buffer());
    if(!gcR.IsValid()){
        CStaticAssertErrorCondition(FatalError, "ExecutionModule::ObjectLoadSetup: Failed searching for module %s in %s",gamName.Buffer(), rt.Name());
        return False;
    }

    GCRTemplate<GAM>  gcGam = gcR;
    if(!gcGam.IsValid()){
        CStaticAssertErrorCondition(FatalError, "ExecutionModule::ObjectLoadSetup: module %s is not of GAM type", gamName.Buffer());
        return False;
    }

    gam = gcGam;

    code = gamCode;

    return True;

}

RealTimeThread::RealTimeThread(){

    rtStatus                    = RTAPP_UNINITIALISED;
    smStatus                    = SM_IDLE;
    runOnCPU                    = 0;

    priority                    = 29;
    isThreadRunning             = False;
    stopThread                  = False;
    performanceInterface        = NULL;

    onlineModules               = NULL;
    offlineModules              = NULL;
    safetyModules               = NULL;
    initialisingModules         = NULL;

    nOfOnlineGams               = 0;
    nOfOfflineGams              = 0;
    nOfSafetyGams               = 0;
    nOfInitialisingGams         = 0;

    safetyMsecSleep             = 1;
    numberOfInitialisingCycles  = 0;
    maxnOfInitialisingCycles    = 0;

    prepulseCycleCount          = 0;
    postpulseCycleCount         = 0;
    pulsingCycleCount           = 0;
    offlineCycleCount           = 0;

    realTimeThreadCleanSem.Create();
}

void RTAppThread(RealTimeThread &rTThread){
    rTThread.RTThread();
}

void RealTimeThread::RTThread(){
    Threads::SetRealTimeClass();
    Threads::SetPriorityLevel(priority);
    isThreadRunning = True;

    AssertErrorCondition(Information,"RealTimeThread::RTThread: RTThread Started");
    while(!stopThread){
        smStatus.Refresh();

        if(smStatus == SM_INITIALISING){
            if(nOfInitialisingGams > 0) {
                for(int i = 0; i < nOfInitialisingGams; i++)initialisingModules[i].Execute();
                if(numberOfInitialisingCycles++ > maxnOfInitialisingCycles){
                    AssertErrorCondition(Information,"RealTimeThread::%s : Exiting Initialisation phase.",Name());
                    smStatus = SM_IDLE;
                    numberOfInitialisingCycles = 0;
                }
            }
            else {
                AssertErrorCondition(Warning,"RealTimeThread::%s : No Initialising GAM has been specified. Skipping Initialisation phase",Name());
                smStatus = SM_IDLE;
            }

        }
        else if(smStatus == SM_PREPULSE){
            pulsingCycleCount = 0;
            prepulseCycleCount++;
            rtStatus = RTAPP_READY;
            for(int i = 0; i < nOfOnlineGams; i++){
                if(!onlineModules[i].Reference()->Execute(GAMPrepulse)){
                    AssertErrorCondition(FatalError,"RealTimeThread::PulseStart: GAM %s reported error during Pre Pulse",onlineModules[i].Reference()->GamName());
                    rtStatus = RTAPP_SAFETY;
                    break;
                }
            }
            //Check for errors
            if(rtStatus == RTAPP_SAFETY){
                //Let the error be handled by the safety gams
                smStatus = SM_IDLE;                 
            }
            else{
                //Ready to go pulsing
                trigger->SetOnlineActivities(True);
                smStatus = SM_PULSING;
            }
        }
        else if(smStatus == SM_POSTPULSE){
            offlineCycleCount = 0;
            postpulseCycleCount++;

            for(int i = 0; i < nOfOnlineGams; i++){
                if(!onlineModules[i].Reference()->Execute(GAMPostpulse)){
                    AssertErrorCondition(FatalError,"RealTimeThread::PostPulse: GAM %s reported error during Post Pulse",onlineModules[i].Reference()->GamName());
                    rtStatus = RTAPP_SAFETY;
                }
            }
            //Even if we have errors this code should run and we should go to offline (safety gams are executed nevertheless)
            trigger->SetOnlineActivities(False);
            smStatus = SM_IDLE; 
        }
        else if(smStatus == SM_PULSING){
            if(rtStatus == RTAPP_READY){
                pulsingCycleCount++;
                for(int i = 0; i < nOfOnlineGams; i++){
                    performanceMonitor.StartGAMMeasureCounter();
                    if(!onlineModules[i].Execute()){
                        AssertErrorCondition(FatalError,"RealTimeThread::%s : GAM %s failed during online   pulsing",Name(),onlineModules[i].Reference()->GamName());
                        rtStatus = RTAPP_SAFETY;
                        break;
                    }
                    performanceMonitor.StorePerformance(i);
                }
                performanceInterface->Write();
            }
            else{
                //Safety
                if(nOfSafetyGams > 0){
                    for(int i = 0; i < nOfSafetyGams; i++)safetyModules[i].Execute();
                }
                else{
                    SleepMsec(safetyMsecSleep);
                }
            }
        }
        else{
            // Set the change state request
            rtStatus.Refresh();
            {
                if(rtStatus == RTAPP_UNINITIALISED){
                    SleepMsec(1);
                }
                else if(rtStatus == RTAPP_SAFETY){
                    //Safety
                    if(nOfSafetyGams > 0){
                        for(int i = 0; i < nOfSafetyGams; i++)safetyModules[i].Execute();
                    }
                    else{
                        SleepMsec(safetyMsecSleep);
                    }
                }
                else{
                    //OFFLINE PROCESSING
                    offlineCycleCount++;
                    for(int i = 0; i < nOfOfflineGams; i++){
                        performanceMonitor.StartGAMMeasureCounter();
                        if(!offlineModules[i].Execute()){
                            rtStatus = RTAPP_SAFETY;
                            GMDSendMessageDeliveryRequest(sendFatalErrorMessage, TTInfiniteWait ,False);
                            AssertErrorCondition(FatalError,"RealTimeThread::%s : GAM %s failed during offline pulsing",Name(),offlineModules[i].Reference()->GamName());
                        }
                        performanceMonitor.StorePerformance(i);
                    }
                    performanceInterface->Write();
                }
            }
        }
    }

    isThreadRunning = False;
    AssertErrorCondition(Information,"RealTimeThread::%s : RTThread Stopped", Name());
    return;
}

bool RealTimeThread::ProcessHttpMessage(HttpStream &hStream){
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    const char *css = "table.bltable {"
        "margin: 1em 1em 1em 2em;"
        "background: whitesmoke;"
        "border-collapse: collapse;"
        "}"
        "table.bltable th, table.bltable td {"
        "border: 1px silver solid;"
        "padding: 0.2em;"
        "}"
        "table.bltable th {"
        "background: gainsboro;"
        "text-align: left;"
        "}"
        "table.bltable caption {"
        "margin-left: inherit;"
        "margin-right: inherit;"
        "}";
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style>\n" );

    hStream.Printf("<html><head><title>%s</title></head><body>",Name());
    FString rrtStatus;
    if(rtStatus == RTAPP_UNINITIALISED) rrtStatus = "UNINITIALISED";
    if(rtStatus == RTAPP_READY)         rrtStatus = "READY";
    if(rtStatus == RTAPP_SAFETY)        rrtStatus = "SAFETY";
    hStream.Printf("<P>Thread Status = %s</P>",rrtStatus.Buffer());

    FString sssmStatus;
    if(smStatus == SM_IDLE)         sssmStatus = "IDLE";
    if(smStatus == SM_WAITING_PRE)  sssmStatus = "WAITING FOR PRE";
    if(smStatus == SM_PREPULSE)     sssmStatus = "PRE PULSE";
    if(smStatus == SM_PULSING)      sssmStatus = "PULSING";
    if(smStatus == SM_POSTPULSE)    sssmStatus = "POST PULSE";
    if(smStatus == SM_INITIALISING) sssmStatus = "INITIALIZING";
    hStream.Printf("<P>State Machine Status = %s</P>",sssmStatus.Buffer());

    hStream.Printf("<H2>Cycle Counters</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TH>State</TH><TH>Counter</TH></TR>\n");
    hStream.Printf("<TR><TD>Offline</TH><TH>%d</TH></TR>\n", offlineCycleCount);
    hStream.Printf("<TR><TD>Prepulse</TH><TH>%d</TH></TR>\n", prepulseCycleCount);
    hStream.Printf("<TR><TD>Pulsing</TH><TH>%d</TH></TR>\n", pulsingCycleCount);
    hStream.Printf("<TR><TD>Postpulse</TH><TH>%d</TH></TR>\n", postpulseCycleCount);
    hStream.Printf("</TABLE>\n");


    hStream.Printf("<H2>Offline GAMs</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TH>N</TH><TH>Name</TH><TH>Last Time Executed</TH><TH>Last Time Duration</TH></TR>\n");
    float lastExecutionTime    = 0;
    float lastAmountOfExecTime = 0;
    int64 currentTimeCounter   = HRT::HRTCounter();

    realTimeThreadCleanSem.Lock();
    int32 i;
    for(i = 0; i < nOfOfflineGams; i++){
        lastExecutionTime    = offlineModules[i].lastExecutionTimeCounts == 0 ? 0 : (currentTimeCounter - offlineModules[i].lastExecutionTimeCounts) * HRT::HRTPeriod();
        lastAmountOfExecTime = offlineModules[i].lastAmountOfExecTimeCounts * HRT::HRTPeriod();
        hStream.Printf("<TR><TD>%d</TD><TD>%s</TD><TD>%e</TD><TD>%e</TD></TR>\n", i, offlineModules[i].Reference()->Name(), lastExecutionTime, lastAmountOfExecTime);
    }
    hStream.Printf("</TABLE>\n");

    hStream.Printf("<H2>Online GAMs</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TH>N</TH><TH>Name</TH><TH>Last Time Executed</TH><TH>Last Time Duration</TH></TR>\n");
    for(i = 0; i < nOfOnlineGams; i++){
        lastExecutionTime    = onlineModules[i].lastExecutionTimeCounts == 0 ? 0 : (currentTimeCounter - onlineModules[i].lastExecutionTimeCounts) * HRT::HRTPeriod();
        lastAmountOfExecTime = onlineModules[i].lastAmountOfExecTimeCounts * HRT::HRTPeriod();
        hStream.Printf("<TR><TD>%d</TD><TD>%s</TD><TD>%e</TD><TD>%e</TD></TR>\n", i, onlineModules[i].Reference()->Name(), lastExecutionTime, lastAmountOfExecTime);
    }
    hStream.Printf("</TABLE>\n");

    hStream.Printf("<H2>Safety GAMs</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TH>N</TH><TH>Name</TH><TH>Last Time Executed</TH><TH>Last Time Duration</TH></TR>\n");
    for(i = 0; i < nOfSafetyGams; i++){
        lastExecutionTime    = safetyModules[i].lastExecutionTimeCounts == 0 ? 0 : (currentTimeCounter - safetyModules[i].lastExecutionTimeCounts) * HRT::HRTPeriod();
        lastAmountOfExecTime = safetyModules[i].lastAmountOfExecTimeCounts * HRT::HRTPeriod();
        hStream.Printf("<TR><TD>%d</TD><TD>%s</TD><TD>%e</TD><TD>%e</TD></TR>\n", i, safetyModules[i].Reference()->Name(), lastExecutionTime, lastAmountOfExecTime);
    }
    hStream.Printf("</TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<H2>Initialising GAMs</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TH>N</TH><TH>Name</TH><TH>Last Time Executed</TH><TH>Last Time Duration</TH></TR>\n");
    for(i = 0; i < nOfInitialisingGams; i++){
        lastExecutionTime    = initialisingModules[i].lastExecutionTimeCounts ? 0 : (currentTimeCounter - initialisingModules[i].lastExecutionTimeCounts) * HRT::HRTPeriod();
        lastAmountOfExecTime = initialisingModules[i].lastAmountOfExecTimeCounts * HRT::HRTPeriod();
        hStream.Printf("<TR><TD>%d</TD><TD>%s</TD><TD>%e</TD><TD>%e</TD></TR>\n", i, initialisingModules[i].Reference()->Name(), lastExecutionTime, lastAmountOfExecTime);
    }
    hStream.Printf("</TABLE>\n");
    hStream.Printf("</body></html>");
    realTimeThreadCleanSem.UnLock();
    hStream.WriteReplyHeader(True);

    return True;
}

bool RealTimeThread::Check(){

    if(rtStatus == RTAPP_UNINITIALISED){
        AssertErrorCondition(FatalError,"RealTimeThread::Check: The Thread has not been initialized yet.");
        return False;
    }

    if(!rtStatus.Request(RTAPP_UNINITIALISED)){
        AssertErrorCondition(FatalError,"RealTimeThread::Check: Failed requesting state change of the RealTimeThread to status UNINITIALISED");
        return False;
    }

    for(int i = 0; i < nOfOnlineGams; i++){
        if(!onlineModules[i].Reference()->Execute(GAMCheck)){
            AssertErrorCondition(FatalError,"RealTimeThread::Check: GAM %s reported error during Check",onlineModules[i].Reference()->Name());
            // Leave without changing state.
            return False;
        }
    }

    if(!rtStatus.Request(RTAPP_READY)){
        AssertErrorCondition(FatalError,"RealTimeThread::Check: Failed requesting state change of the RealTimeThread to status READY");
        return False;
    }

    return True;
}

bool RealTimeThread::PulseStart(){

    AssertErrorCondition(Information,"RealTimeThread::PulseStart");

    if(rtStatus != RTAPP_READY){
        AssertErrorCondition(FatalError,"RealTimeThread::PulseStart: The Thread has not been initialized yet.");
        return False;
    }

    if (!smStatus.Request(SM_PREPULSE, SM_PULSING)) {
        AssertErrorCondition(FatalError,"RealTimeThread::Check: Failed requesting state change of the StateMachineStatus to status SM_PREPULSE waiting for SM_PULSING");
        return False;
    }

    return (rtStatus != RTAPP_SAFETY);
}

bool RealTimeThread::PostPulse(){

    AssertErrorCondition(Information,"RealTimeThread::PostPulse");

    if(rtStatus == RTAPP_UNINITIALISED){
        AssertErrorCondition(FatalError,"RealTimeThread::PostPulse: The Thread has not been initialized yet.");
        return False;
    }

    if(rtStatus == RTAPP_SAFETY){
        AssertErrorCondition(FatalError,"RealTimeThread::PostPulse: The Thread is in SAFETY mode");
        GMDSendMessageDeliveryRequest(sendFatalErrorMessage, TTInfiniteWait ,False);
    }

    if(!smStatus.Request(SM_POSTPULSE, SM_IDLE)){
        AssertErrorCondition(FatalError,"RealTimeThread::Check: Failed requesting state change of the StateMachineStatus to status SM_POSTPULSE waiting for SM_IDLE");
        return False;
    }

    return (rtStatus != RTAPP_SAFETY);
}

bool RealTimeThread::CleanRealTimeThread(){
    realTimeThreadCleanSem.Lock();
    //Free Local Structures
    if(onlineModules        != NULL){
        // Remove local copies of GAMs
        for(int i = 0; i < nOfOnlineGams; i++) {
            AssertErrorCondition(Information, "Removing GAM %s from online", onlineModules[i].Reference()->Name());
            Remove(onlineModules[i].Reference()->Name());
        }
        delete[] onlineModules;
    }

    if(offlineModules        != NULL){
        // Remove local copies of GAMs
        for(int i = 0; i < nOfOfflineGams; i++) {
            AssertErrorCondition(Information, "Removing GAM %s from offline", offlineModules[i].Reference()->Name());
            Remove(offlineModules[i].Reference()->Name());
        }
        delete[] offlineModules;
    }

    if(safetyModules        != NULL){
        // Remove local copies of GAMs
        for(int i = 0; i < nOfSafetyGams; i++) {
            AssertErrorCondition(Information, "Removing GAM %s from safety", safetyModules[i].Reference()->Name());
            Remove(safetyModules[i].Reference()->Name());
        }
        delete[] safetyModules;
    }

    if(initialisingModules  != NULL){
        // Remove local copies of GAMs
        for(int i = 0; i < nOfInitialisingGams ; i++) {
            AssertErrorCondition(Information, "Removing GAM %s from initialising", initialisingModules[i].Reference()->Name());
            Remove(initialisingModules[i].Reference()->Name());
        }
        delete[] initialisingModules;
    }
    //Reset counters
    nOfOnlineGams       = 0;
    nOfOfflineGams      = 0;
    nOfSafetyGams       = 0;
    nOfInitialisingGams = 0;

    //Reset The DDB
    if(ddb.IsValid()){
        ddb->Reset();
        ddb.RemoveReference();
    }

    onlineModules               = NULL;
    offlineModules              = NULL;
    safetyModules               = NULL;
    initialisingModules         = NULL;
    realTimeThreadCleanSem.UnLock();

    return True;
}

bool RealTimeThread::HandleLevel1Message(ConfigurationDataBase &info){

    if((smStatus == SM_PULSING) || (smStatus == SM_WAITING_PRE)){
        AssertErrorCondition(FatalError,"RealTimeThread::HandleLevel1Message: Cannot handle message while online.");
        return False;
    }

    //Lock Semaphore
    if(!rtStatus.Request(RTAPP_UNINITIALISED)){
        AssertErrorCondition(FatalError,"RealTimeThread::HandleLevel1Message: Failed requesting state change of the RealTimeThread to status UNINITIALISED");
        return False;
    }

    AssertErrorCondition(Information,"RealTimeThread::HandleLevel1Message: Processing Level1 Message.");
    
    //Destroy all contained Objects
    CleanRealTimeThread();
    CleanUp();

    //Rebuild GAMs
    CDBExtended cdb(info);
    if(!GCReferenceContainer::ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: GCReferenceContainer::ObjectLoadSetup Failed", Name());
        return False;
    }

    //Get Reference  DDB
    GCReference ddbReference = Find("DDB");
    if(!ddbReference.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed finding DDB object.", Name());
        return False;
    }

    ddb = ddbReference;
    if(!ddb.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: DDB object is not a valid DDB.", Name());
        return False;
    }

    //Get the List of GAMs to be run during online operation
    {
        FString onlineGAMsNames;
        if (!cdb.ReadBString(onlineGAMsNames, "Online", "")){
            AssertErrorCondition(FatalError, "RealTimeThread::HandleLevel1Message: %s: Online section was not set", Name());
            CleanRealTimeThread();
            return False;
        }
        FString token;
        while (onlineGAMsNames.GetToken(token,", \n\t")){
            nOfOnlineGams++;
            token.SetSize(0);
        }
        onlineGAMsNames.Seek(0);

        if(nOfOnlineGams <= 0 ){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: No online GAM has been specified ", Name());
            CleanRealTimeThread();
            return False;
        }

        onlineModules = new ExecutionModule[nOfOnlineGams];
        if(onlineModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed allocating space for %d online Modules ", Name(), nOfOnlineGams);
            CleanRealTimeThread();
            return False;
        }

        onlineGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfOnlineGams; nOfOnGam++){
            if(!onlineGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for online module %d ", Name(), nOfOnGam);
            }
            if(!onlineModules[nOfOnGam].ObjectLoadSetup(token, GAMOnline, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for online module %d ", Name(), nOfOnGam);
                CleanRealTimeThread();
                return False;
            }
            token.SetSize(0);
        }
    }

    //Get the List of GAMs to be run during offline operation
    {
        FString offlineGAMsNames;
        if (!cdb.ReadBString(offlineGAMsNames, "Offline", "")){
            AssertErrorCondition(FatalError, "RealTimeThread::HandleLevel1Message: %s: Offline section was not set", Name());
            CleanRealTimeThread();
            return False;
        }
        FString token;
        while (offlineGAMsNames.GetToken(token,", \n\t")){
            nOfOfflineGams++;
            token.SetSize(0);
        }
        offlineGAMsNames.Seek(0);

        if(nOfOfflineGams <= 0 ){
            AssertErrorCondition(FatalError,"RealTimeThread::HandleLevel1Message: %s: No offline GAM has been specified ", Name());
            CleanRealTimeThread();
            return False;
        }

        offlineModules = new ExecutionModule[nOfOfflineGams];
        if(offlineModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed allocating space for %d offline Modules ", Name(), nOfOfflineGams);
            CleanRealTimeThread();
            return False;
        }

        offlineGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfOfflineGams; nOfOnGam++){
            if(!offlineGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for offline module %d ", Name(), nOfOnGam);
            }
            if(!offlineModules[nOfOnGam].ObjectLoadSetup(token, GAMOffline, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for offline module %d ", Name(), nOfOnGam);
                CleanRealTimeThread();
                return False;
            }
            token.SetSize(0);
        }
    }

    //Get the List of GAMs to be run during safety operation
    if(!cdb->Exists("Safety")){
        if(!cdb.ReadInt32(safetyMsecSleep,"SafetyMsecSleep")){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: SafetyMsecSleep has not been specified ", Name());
            CleanRealTimeThread();
            return False;
        }
    }else{
        FString safetyGAMsNames;
        if (!cdb.ReadBString(safetyGAMsNames, "Safety", "")){
            AssertErrorCondition(Warning, "RealTimeThread::HandleLevel1Message: %s: Safety section was not set", Name());
        }
        FString token;
        while (safetyGAMsNames.GetToken(token,", \n\t")){
            nOfSafetyGams++;
            token.SetSize(0);
        }
        safetyGAMsNames.Seek(0);

        if(nOfSafetyGams <= 0 ){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: No Safety GAM has been specified ", Name());
        }

        safetyModules = new ExecutionModule[nOfSafetyGams];
        if(safetyModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed allocating space for %d safety Modules ", Name(), nOfSafetyGams);
            CleanRealTimeThread();
            return False;
        }

        safetyGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfSafetyGams; nOfOnGam++){
            if(!safetyGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for safety module %d ", Name(), nOfOnGam);
            }
            if(!safetyModules[nOfOnGam].ObjectLoadSetup(token, GAMSafety, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for safety module %d ", Name(), nOfOnGam);
                CleanRealTimeThread();
                return False;
            }
            token.SetSize(0);
        }
    }

    //Get the List of GAMs to be run during initialising operation
    {
        FString initalisingGAMsNames;
        if (!cdb.ReadBString(initalisingGAMsNames, "Initialising", "")){
            AssertErrorCondition(Information, "RealTimeThread::HandleLevel1Message: %s: Initialising section was not set", Name());
        }
        FString token;
        while (initalisingGAMsNames.GetToken(token,", \n\t")){
            nOfInitialisingGams++;
            token.SetSize(0);
        }
        initalisingGAMsNames.Seek(0);

        if(nOfInitialisingGams <= 0 ){
            AssertErrorCondition(Information,"RealTimeThread::HandleLevel1Message: %s: No initialising GAM has been specified ", Name());
        }

        initialisingModules = new ExecutionModule[nOfInitialisingGams];
        if(initialisingModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed allocating space for %d initialising Modules ", Name(), nOfInitialisingGams);
            CleanRealTimeThread();
            return False;
        }

        initalisingGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfInitialisingGams; nOfOnGam++){
            if(!initalisingGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for initialising module %d ", Name(), nOfOnGam);
            }
            if(!initialisingModules[nOfOnGam].ObjectLoadSetup(token, GAMStartUp, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: Failed loading execution information for initialising module %d ", Name(), nOfOnGam);
                CleanRealTimeThread();
                return False;
            }
            token.SetSize(0);
        }
    }
    // Check how many cycle to perform in Initialisation
    if(nOfInitialisingGams > 0){
        if(!cdb.ReadInt32(maxnOfInitialisingCycles,"MaximumNumberOfInitialisingCycles")){
            AssertErrorCondition(InitialisationError,"RealTimeThread::HandleLevel1Message: %s: MaximumNumberOfInitialisingCycles has not been specified", Name());
            CleanRealTimeThread();
            return False;
        }
    }

    /** Iterate and Do() on all GAM type objects in GODB */
    GAMLister gamLister(ddb);  
    Iterate(&gamLister, GCFT_Recurse);
    if(!gamLister.ReturnValue()) {
        CleanRealTimeThread();
        return False;
    }

    //Create Performance Monitors for the GAMs
    if(!CreatePerformanceMonitors4Gams(info)) {
        AssertErrorCondition(FatalError, "Unable to Create Performance Monitors for the GAMs");
        return False;
    }

    // Initialise performance monitor activities
    performanceMonitor.Initialise(nOfOnlineGams, performanceInterface->Buffer(), trigger);

    if(!rtStatus.Request(RTAPP_READY)){
        AssertErrorCondition(FatalError,"RealTimeThread::HandleLevel1Message:: Failed requesting state change of the RealTimeThread to status READY");
        SleepMsec(10);
        return False;
    }

    AssertErrorCondition(Information,"RealTimeThread::HandleLevel1Message: %s: Successfully Handled Level1 Message ",Name());

    SleepMsec(10);

    return True;
}

bool RealTimeThread::Start(){
    // Check if the class has been initialzed
    if(rtStatus != RTAPP_READY){
        AssertErrorCondition(Warning,"RealTimeThread::Start: RTThread %s has not been initialized yet.", Name());
        return False;
    }

    // Check if the thread has been started already
    if(isThreadRunning){
        AssertErrorCondition(Warning,"RealTimeThread::Start: RTThread %s has already started", Name());
        return True;
    }

    // Start the Triggering Service
    if(!trigger->Start()){
        AssertErrorCondition(Warning,"RealTimeThread::Start: Failed Starting Triggering Service %s", trigger->Name());
        return False;
    }

    stopThread = False;

    threadID = Threads::BeginThread((void (__thread_decl *)(void *))RTAppThread,this,THREADS_DEFAULT_STACKSIZE, Name(),XH_NotHandled, runOnCPU);
    // Waits for thread
    while(!isThreadRunning)   SleepMsec(10);

    return True;
}

bool RealTimeThread::Stop() {
    // Check if the thread has already stopped
    if(!isThreadRunning){
        AssertErrorCondition(Warning,"RealTimeThread::Stop: Thread %s is not running", Name());
        return False;
    }

    // Request termination
    stopThread = True;

    // Wait for the thread to stop
    for(int i=0;((i<100)&&(isThreadRunning));i++) SleepMsec(10);

    // If the thread is still running kill it!
    if(isThreadRunning) Threads::Kill(threadID);

    // Start the Triggering Service
    if(!trigger->Stop()){
        AssertErrorCondition(Warning,"RealTimeThread::Start: Failed Stopping Triggering Service %s", trigger->Name());
        return False;
    }

    return True;
}

bool RealTimeThread::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    if((smStatus == SM_PULSING) || (smStatus == SM_WAITING_PRE)){
        AssertErrorCondition(FatalError,"RealTimeThread::ObjectLoadSetup: %s Cannot modify parameters during Pulse in progress", Name());
        return False;
    }

    //Set Name to the Object
    if(isThreadRunning){
        AssertErrorCondition(Warning,"RealTimeThread::ObjectLoadSetup: %s: Stopping thread to allow reinitialization", Name());
        Stop();
        rtStatus        = RTAPP_UNINITIALISED;
    }

    //Clear the List of Objects before adding a new set.
    CleanUp();
    CleanRealTimeThread();
    //Get Information for GAM Construction
    CDBExtended cdb(info);

    if(!GCReferenceContainer::ObjectLoadSetup(cdb,err)){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: GCReferenceContainer::ObjectLoadSetup Failed", Name());
        return False;
    }

    //Check Object Contained
    // Only GAMs and DDB are allowed in the RealTimeThread
    for(int o = 0; o < Size(); o++){
        GCReference obj   = Find(o);
        bool ret = False;
        GCRTemplate<DDB>  oDDB(obj);
        GCRTemplate<GAM>  oGAM(obj);
        ret = (oDDB.IsValid() || oGAM.IsValid());
        if(!ret){
            GCRTemplate<GCNamedObject> oNamed(obj);
            if(oNamed.IsValid())  AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Only GAMs and DDB are allowed in the RealTimeThread. Object %s is none.", Name(), oNamed->Name());
            else                  AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Only GAMs and DDB are allowed in the RealTimeThread", Name());
            return False;
        }
    }

    //Get thread parameters
    if(!cdb.ReadInt32(priority,"ThreadPriority")){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed reading entry ThreadPriority", Name());
        return False;
    }

    if(!cdb.ReadInt32(runOnCPU,"RunOnCPU")){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed reading entry RunOnCPU", Name());
        return False;
    }

    int32  timeOutRequest = 0;
    if(!cdb.ReadInt32(timeOutRequest,"RTStatusChangeMsecTimeout")){
        timeOutRequest    = 20;
        AssertErrorCondition(Warning,"RealTimeThread::ObjectLoadSetup: %s: Failed reading entry RTStatusChangeMsecTimeout. Assuming 20 msec.",Name());
    }

    rtStatus.SetMsecTimeOut(timeOutRequest);

    if(!cdb.ReadInt32(timeOutRequest,"SMStatusChangeMsecTimeout")){
        timeOutRequest    = 20;
        AssertErrorCondition(Warning,"RealTimeThread::ObjectLoadSetup: %s: Failed reading entry SMStatusChangeMsecTimeout. Assuming 20 msec.",Name());
    }

    smStatus.SetMsecTimeOut(timeOutRequest);

    //Get Reference  DDB
    GCReference ddbReference = Find("DDB");
    if(!ddbReference.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed finding DDB object.", Name());
        return False;
    }

    if(ddb.IsValid()){
        ddb->Reset();
        ddb.RemoveReference();
    }
    ddb = ddbReference;
    if(!ddb.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: DDB object is not a valid DDB.", Name());
        return False;
    }

    //Get References to ExternalTimeTriggeringSerivce
    FString triggeringServiceName;
    if(!cdb.ReadFString(triggeringServiceName,"TriggeringServiceName")){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: TriggeringServiceName has not been specified.", Name());
        return False;
    }

    GCReference triggerReference = GODBFindByName(triggeringServiceName.Buffer(), NULL, True);
    if(!triggerReference.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed retrieving reference for%s in Global Container", Name(),triggeringServiceName.Buffer());
        return False;
    }

    trigger = triggerReference;
    if(!trigger.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: The object %s is not of TimeTriggeringServiceInterface Type", Name(),triggeringServiceName.Buffer());
        return False;
    }

    GCReference message = GODBFindByName("SafetyErrorMessage", NULL, True);
    if(!message.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: Failed finding SafetyErrorMessage");
        return False;
    }

    sendFatalErrorMessage = message;
    if(!sendFatalErrorMessage.IsValid()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: SafetyErrorMessage is not a Valid MessageDeliveryRequest");
        return False;
    }

    //Count number of GAMs and initialize gams vector
    if(Size() == 0){
        AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: No valid Object has been specified", Name());
        return False;
    }

    //Get the List of GAMs to be run during online operation
    {
        FString onlineGAMsNames;
        if (!cdb.ReadBString(onlineGAMsNames, "Online", "")){
            AssertErrorCondition(FatalError, "RealTimeThread::ObjectLoadSetup: %s: Online section was not set", Name());
            Stop();
            return False;
        }
        FString token;
        while (onlineGAMsNames.GetToken(token,", \n\t")){
            nOfOnlineGams++;
            token.SetSize(0);
        }
        onlineGAMsNames.Seek(0);

        if(nOfOnlineGams <= 0 ){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: No online GAM has been specified ", Name());
            Stop();
            return False;
        }

        onlineModules = new ExecutionModule[nOfOnlineGams];
        if(onlineModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed allocating space for %d online Modules ", Name(), nOfOnlineGams);
            Stop();
            return False;
        }

        onlineGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfOnlineGams; nOfOnGam++){
            if(!onlineGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for online module %d ", Name(), nOfOnGam);
            }
            if(!onlineModules[nOfOnGam].ObjectLoadSetup(token, GAMOnline, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for online module %d ", Name(), nOfOnGam);
                Stop();
                return False;
            }
            token.SetSize(0);
        }
    }

    //Get the List of GAMs to be run during offline operation
    {
        FString offlineGAMsNames;
        if (!cdb.ReadBString(offlineGAMsNames, "Offline", "")){
            AssertErrorCondition(FatalError, "RealTimeThread::ObjectLoadSetup: %s: Offline section was not set", Name());
            Stop();
            return False;
        }
        FString token;
        while (offlineGAMsNames.GetToken(token,", \n\t")){
            nOfOfflineGams++;
            token.SetSize(0);
        }
        offlineGAMsNames.Seek(0);

        if(nOfOfflineGams <= 0 ){
            AssertErrorCondition(FatalError,"RealTimeThread::ObjectLoadSetup: %s: No offline GAM has been specified ", Name());
            Stop();
            return False;
        }

        offlineModules = new ExecutionModule[nOfOfflineGams];
        if(offlineModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed allocating space for %d offline Modules ", Name(), nOfOfflineGams);
            Stop();
            return False;
        }

        offlineGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfOfflineGams; nOfOnGam++){
            if(!offlineGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for offline module %d ", Name(), nOfOnGam);
            }
            if(!offlineModules[nOfOnGam].ObjectLoadSetup(token, GAMOffline, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for offline module %d ", Name(), nOfOnGam);
                Stop();
                return False;
            }
            token.SetSize(0);
        }
    }

    //Get the List of GAMs to be run during safety operation
    if(!cdb->Exists("Safety")){
        if(!cdb.ReadInt32(safetyMsecSleep,"SafetyMsecSleep")){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: SafetyMsecSleep has not been specified ", Name());
            return False;
        }
    }else{
        FString safetyGAMsNames;
        if (!cdb.ReadBString(safetyGAMsNames, "Safety", "")){
            AssertErrorCondition(Warning, "RealTimeThread::ObjectLoadSetup: %s: Safety section was not set", Name());
        }
        FString token;
        while (safetyGAMsNames.GetToken(token,", \n\t")){
            nOfSafetyGams++;
            token.SetSize(0);
        }
        safetyGAMsNames.Seek(0);

        if(nOfSafetyGams <= 0 ){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: No Safety GAM has been specified ", Name());
        }

        safetyModules = new ExecutionModule[nOfSafetyGams];
        if(safetyModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed allocating space for %d safety Modules ", Name(), nOfSafetyGams);
            Stop();
            return False;
        }

        safetyGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfSafetyGams; nOfOnGam++){
            if(!safetyGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for safety module %d ", Name(), nOfOnGam);
            }
            if(!safetyModules[nOfOnGam].ObjectLoadSetup(token, GAMSafety, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for safety module %d ", Name(), nOfOnGam);
                Stop();
                return False;
            }
            token.SetSize(0);
        }
    }

    //Get the List of GAMs to be run during initialising operation
    {
        FString initalisingGAMsNames;
        if (!cdb.ReadBString(initalisingGAMsNames, "Initialising", "")){
            AssertErrorCondition(Information, "RealTimeThread::ObjectLoadSetup: %s: Initialising section was not set", Name());
        }
        FString token;
        while (initalisingGAMsNames.GetToken(token,", \n\t")){
            nOfInitialisingGams++;
            token.SetSize(0);
        }
        initalisingGAMsNames.Seek(0);

        if(nOfInitialisingGams <= 0 ){
            AssertErrorCondition(Information,"RealTimeThread::ObjectLoadSetup: %s: No initialising GAM has been specified ", Name());
        }

        initialisingModules = new ExecutionModule[nOfInitialisingGams];
        if(initialisingModules == NULL){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed allocating space for %d initialising Modules ", Name(), nOfInitialisingGams);
            Stop();
            return False;
        }

        initalisingGAMsNames.Seek(0);
        token.SetSize(0);
        for(int nOfOnGam = 0; nOfOnGam < nOfInitialisingGams; nOfOnGam++){
            if(!initalisingGAMsNames.GetToken(token,", \n\t")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for initialising module %d ", Name(), nOfOnGam);
            }
            if(!initialisingModules[nOfOnGam].ObjectLoadSetup(token, GAMStartUp, NULL, *this)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: Failed loading execution information for initialising module %d ", Name(), nOfOnGam);
                Stop();
                return False;
            }
            token.SetSize(0);
        }
    }
    // Check how many cycle to perform in Initialisation
    if(nOfInitialisingGams > 0){
        if(!cdb.ReadInt32(maxnOfInitialisingCycles,"MaximumNumberOfInitialisingCycles")){
            AssertErrorCondition(InitialisationError,"RealTimeThread::ObjectLoadSetup: %s: MaximumNumberOfInitialisingCycles has not been specified", Name());
            Stop();
            return False;
        }
    }

    /** Iterate and Do() on all GAM type objects in GODB */
    GAMLister gamLister(ddb);  
    Iterate(&gamLister, GCFT_Recurse);
    if(!gamLister.ReturnValue()) {
        CleanRealTimeThread();
        return False;
    }

    //Create Performance Monitors for the GAMs
    if(!CreatePerformanceMonitors4Gams(info)) {
        AssertErrorCondition(FatalError, "RealTimeThread::ObjectLoadSetup: %s:Unable to Create Performance Monitors for the GAMs", Name());
        return False;
    }

    rtStatus = RTAPP_READY;
    smStatus = SM_IDLE;

    // Initialise performance monitor activities
    performanceMonitor.Initialise(nOfOnlineGams, performanceInterface->Buffer(), trigger);

    AssertErrorCondition(Information,"RealTimeThread::ObjectLoadSetup: %s: Successfully Completed",Name());

    return True;
}

bool RealTimeThread::ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err){

    if(rtStatus == RTAPP_UNINITIALISED){
        AssertErrorCondition(Warning,"RealTimeThread::ObjectSaveSetup: Thread not initialized");
        return False;
    }

    CDBExtended cdb(info);

    cdb->AddChildAndMove("DDB");
    if(!ddb.IsValid()){
        AssertErrorCondition(Warning,"RealTimeThread::ObjectSaveSetup: DDB is not a valid pointer");
        return False;
    }

    ddb->ObjectSaveSetup(cdb,err);
    cdb->MoveToFather();
    cdb->AddChildAndMove("GAMs");

    for(int nOfObjects = 0; nOfObjects < Size(); nOfObjects++){
        GCReference gc = Find(nOfObjects);
        GCRTemplate<GAM> gamReference(gc);
        if(gamReference.IsValid()){
            cdb->AddChildAndMove(gamReference->GamName());
            gamReference->ObjectSaveSetup(cdb,err);
            cdb->MoveToFather();

        }
    }

    cdb->MoveToFather();

    return True;
}

bool RealTimeThread::ProcessMessage2(GCRTemplate<MessageEnvelope> envelope){

    GCRTemplate<Message> message = envelope->GetMessage();
    if (!message.IsValid()){
        AssertErrorCondition(CommunicationError,"RealTimeThread::ProcessMessage: %s: Received invalid Message", Name());
        return False;
    }

    FString messageContent = message->Content();
    FString messageSender  = envelope->Sender();
    //Messages from the State Machine
    AssertErrorCondition(Information,"RealTimeThread::ProcessMessage: %s: Processing Message [%s] [id=%d], from %s.",Name(),messageContent.Buffer(), message->Id(), messageSender.Buffer());


    bool replyExpected = envelope->ManualReplyExpected();
    //Configuration related Messages //
    {
        if(strcasecmp(messageContent.Buffer(),"LEVEL1") == 0){
            if((smStatus ==SM_WAITING_PRE ) || (smStatus == SM_PULSING) || (smStatus == SM_POSTPULSE) ){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ProcessMessage: %s: Cannot Handle Configuration Message While in Pulse In Progress", Name());
                if(replyExpected) SendMessageReply(envelope, False);
                return True;
            }

            GCReference cdb = message->Find(0);
            if(!cdb.IsValid()){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ProcessMessage: %s: Message Content is not valid", Name());
                if(replyExpected) SendMessageReply(envelope, False);
                return True;
            }

            ConfigurationDataBase configuration(cdb);
            if(!configuration.IsValid()){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ProcessMessage: %s: Message Content is not a valid ConfigurationDataBase", Name());
                if(replyExpected) SendMessageReply(envelope, False);
                return True;
            }

            if(!HandleLevel1Message(configuration)){
                AssertErrorCondition(InitialisationError,"RealTimeThread::ProcessMessage: %s: Failed Processing Configuration Message", Name());
                if(replyExpected) SendMessageReply(envelope, False);
                return True;
            }

            if(replyExpected) SendMessageReply(envelope, True);
            return True;
        }
    }
    ////////////////////////////////
    // Countdown related Messages //
    ////////////////////////////////
    bool error = False;
    {
        if(strcasecmp(messageContent.Buffer(),"ERROR") == 0){
            rtStatus = RTAPP_SAFETY;
            AssertErrorCondition(Information,"RealTimeThread::ProcessMessage: %s: Going into Safety mode.",Name(),messageContent.Buffer());
        }else if(strcasecmp(messageContent.Buffer(),"PREPULSECHECK") == 0){
            if(!Check()){
                rtStatus = RTAPP_SAFETY;
                AssertErrorCondition(FatalError,"RealTimeThread::ProcessMessage: %s: %s Failed.",Name(),messageContent.Buffer());
                error = True;
            }else{
                smStatus = SM_WAITING_PRE;
            }
        }else if(strcasecmp(messageContent.Buffer(),"PULSESTART") == 0){
            if(!PulseStart()){
                rtStatus = RTAPP_SAFETY;
                smStatus = SM_IDLE;
                AssertErrorCondition(FatalError,"RealTimeThread::ProcessMessage: %s: %s Failed.",Name(),messageContent.Buffer());
                error = True;
            }
        }else if(strcasecmp(messageContent.Buffer(),"PULSESTOP") == 0){
            if(!PostPulse()){
                rtStatus = RTAPP_SAFETY;
                smStatus = SM_IDLE;
                AssertErrorCondition(FatalError,"RealTimeThread::ProcessMessage: %s: %s Failed.",Name(),messageContent.Buffer());
                error = True;
            }
        }else if(strcasecmp(messageContent.Buffer(),"COLLECTIONCOMPLETED") == 0){
            smStatus = SM_IDLE;
        }else if(strcasecmp(messageContent.Buffer(),"INITIALISE") == 0){
            if((smStatus == SM_WAITING_PRE) || (smStatus == SM_PULSING) || (smStatus == SM_POSTPULSE)){
                AssertErrorCondition(Warning,"RealTimeThread::ProcessMessage: %s: Cannot handle %s message during pulse in progress.",Name(),messageContent.Buffer());
                error = True;
            }else{
                smStatus = SM_INITIALISING;
            }
        }else if(strcasecmp(messageContent.Buffer(),"ABORT") == 0){
            if((smStatus == SM_PULSING) || (smStatus == SM_POSTPULSE)){
                AssertErrorCondition(Warning,"RealTimeThread::ProcessMessage: %s: Cannot handle %s message during pulse in progress.",Name(),messageContent.Buffer());
            }else{
                smStatus = SM_IDLE;
            }
        }else if(strcasecmp(messageContent.Buffer(),"RESET") == 0){
            if((smStatus == SM_PULSING) || (smStatus == SM_POSTPULSE)){
                AssertErrorCondition(Warning,"RealTimeThread::ProcessMessage: %s: Cannot handle %s message during pulse in progress.",Name(),messageContent.Buffer());
            }else{
                smStatus = SM_IDLE;
                rtStatus = RTAPP_READY;
            }
        }else{
            AssertErrorCondition(CommunicationError,"RealTimeThread::ProcessMessage: %s: %s invalid message content.",Name(),messageContent.Buffer());
            error = True;
        }
    }

    if(replyExpected) SendMessageReply(envelope, !error);
    return !error;
}

bool RealTimeThread::SendMessageReply(GCRTemplate<MessageEnvelope> envelope, bool ok){

    GCRTemplate<Message> gcrtm(GCFT_Create);
    if(!gcrtm.IsValid()){
        AssertErrorCondition(FatalError, "RealTimeThread::PrepareMessageReply: %s: Failed creating response message for Level1", Name());
        return False;
    }

    GCRTemplate<MessageEnvelope> mec(GCFT_Create);
    if (!mec.IsValid()){
        AssertErrorCondition(FatalError,"RealTimeThread::PrepareMessageReply: %s: Failed creating copy for Level1", Name());
        return False;
    }

    if(ok == False)  gcrtm->Init(0, "ERROR");
    else             gcrtm->Init(0, "OK");

    mec->PrepareReply(envelope,gcrtm);
    MessageHandler::SendMessage(mec);

    return True;
}

bool TriggerMenu(StreamInterface &in,StreamInterface &out,void *userData){

    RealTimeThread *rtt = (RealTimeThread *)userData;
    if(userData == NULL) return False;
    if(rtt->trigger.IsValid()) {
        rtt->trigger->ObjectDescription(out);
        return True;
    }

    return False;
}

bool BrowseRTDDBMenu(StreamInterface &in,StreamInterface &out,void *userData){

    RealTimeThread *rtt = (RealTimeThread *)userData;
    if(userData == NULL) return False;
    if(rtt->ddb.IsValid()){
        return DDBBrowseMenu(in,out,rtt->ddb.operator->());
    }

    return False;
}
bool DumpRTDDBMenu(StreamInterface &in,StreamInterface &out,void *userData){

    RealTimeThread *rtt = (RealTimeThread *)userData;
    if(userData == NULL) return False;
    if(rtt->ddb.IsValid()){
        return DDBDumpMenu(in,out,rtt->ddb.operator->());
    }

    return False;
}

bool RealTimeThread::CreatePerformanceMonitors4Gams(ConfigurationDataBase &info) {

    //Create Performance Monitors for the GAMs
    if(performanceInterface != NULL) delete performanceInterface;
    performanceInterface = new DDBOutputInterface(Name(),"PerformanceMonitor",DDB_WriteMode);
    if(performanceInterface == NULL){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Creating DDBOutputInterface for the Performance Monitoring",Name());
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    if(!performanceInterface->AddSignal("performaceUsecTime","int32")){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Adding signal performaceUsecTime To %s Interface",Name(),performanceInterface->InterfaceName());
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    if(!performanceInterface->AddSignal("CycleUsecTime","float")){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Adding signal CycleUsecTime To %s Interface",Name(),performanceInterface->InterfaceName());
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    int nOfObjects;
    // Add Monitoring Relative signals
    for(nOfObjects = 0; nOfObjects < nOfOnlineGams; nOfObjects++){
        FString signalName;
        signalName.Printf("%sRelativeUsecTime",onlineModules[nOfObjects].Reference()->Name());
        if(!performanceInterface->AddSignal(signalName.Buffer(),"float")){
            AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Adding signal %s To %s Interface",Name(),signalName.Buffer(),performanceInterface->InterfaceName());
            if(isThreadRunning) {
                CleanRealTimeThread();
            }
            return False;
        }
    }

    // Add Monitoring Relative signals for Offline modules
    for(nOfObjects = 0; nOfObjects < nOfOfflineGams; nOfObjects++){
        bool addOfflineGamOk = 1;
        // Check if we are trying to add offline GAMs which are also online GAMs
        for(int checkCounter = 0 ; checkCounter < nOfOnlineGams ; checkCounter++){
            if(offlineModules[nOfObjects].Reference()->Name() == onlineModules[checkCounter].Reference()->Name()) {
                addOfflineGamOk = 0;
            }
        }
        if(addOfflineGamOk == 1) {
            FString signalName;
            signalName.Printf("%sRelativeUsecTime",offlineModules[nOfObjects].Reference()->Name());
            if(!performanceInterface->AddSignal(signalName.Buffer(),"float")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Adding signal %s To %s Interface",Name(),signalName.Buffer(),performanceInterface->InterfaceName());
                if(isThreadRunning) {
                    CleanRealTimeThread();
                }
                return False;
            }
        }
    }

    // Add Monitoring Absolute signals
    for(nOfObjects = 0; nOfObjects < nOfOnlineGams; nOfObjects++){
        FString signalName;
        signalName.Printf("%sAbsoluteUsecTime",onlineModules[nOfObjects].Reference()->Name());
        if(!performanceInterface->AddSignal(signalName.Buffer(),"float")){
            AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Adding signal %s To %s Interface",Name(),signalName.Buffer(),performanceInterface->InterfaceName());
            if(isThreadRunning) {
                CleanRealTimeThread();
            }
            return False;
        }
    }

    // Add Monitoring Absolute signals for Offline modules
    for(nOfObjects = 0; nOfObjects < nOfOfflineGams; nOfObjects++){
        bool addOfflineGamOk = 1;
        // Check if we are trying to add offline GAMs which are also online GAMs
        for(int checkCounter = 0 ; checkCounter < nOfOnlineGams ; checkCounter++){
            if(offlineModules[nOfObjects].Reference()->Name() == onlineModules[checkCounter].Reference()->Name()) {
                addOfflineGamOk = 0;
            }
        }
        if(addOfflineGamOk == 1) {
            FString signalName;
            signalName.Printf("%sAbsoluteUsecTime",offlineModules[nOfObjects].Reference()->Name());
            if(!performanceInterface->AddSignal(signalName.Buffer(),"float")){
                AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Adding signal %s To %s Interface",Name(),signalName.Buffer(),performanceInterface->InterfaceName());
                if(isThreadRunning) {
                    CleanRealTimeThread();
                }
                return False;
            }
        }
    }

    if(!performanceInterface->Finalise()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed Finalising Interface %s ",Name(),performanceInterface->InterfaceName());
        if(isThreadRunning) {
            Stop();
        }
        return False;
    }

    if(!ddb->AddInterface(*performanceInterface)){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed adding interface for Performance Monitoring %s to DDB",Name(),performanceInterface->InterfaceName());
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    if(isThreadRunning){
        CDBExtended cdb(info);
        int32  timeOutRequest = 0;
        if(!cdb.ReadInt32(timeOutRequest,"RTStatusChangeMsecTimeout")){
            timeOutRequest    = 20;
            AssertErrorCondition(Warning,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed reading entry RTStatusChangeMsecTimeout. Assuming 20 msec.",Name());
        }

        rtStatus.SetMsecTimeOut(timeOutRequest);

        if(!cdb.ReadInt32(timeOutRequest,"SMStatusChangeMsecTimeout")){
            timeOutRequest    = 20;
            AssertErrorCondition(Warning,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: Failed reading entry SMStatusChangeMsecTimeout. Assuming 20 msec.",Name());
        }

        smStatus.SetMsecTimeOut(timeOutRequest);
    }

    /////////////////////////////
    // Check DDB Compatibility //
    /////////////////////////////

    if(!ddb->CheckAndAllocate()){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: DDB CheckAndAllocate Failed", Name());
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    /** Iterate and Do() on all GAM type objects in GODB */
    GAMLister2 gamLister2(ddb);
    Iterate(&gamLister2, GCFT_Recurse);
    if(!gamLister2.ReturnValue()) {
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    if(!ddb->CreateLink(*performanceInterface)){
        AssertErrorCondition(InitialisationError,"RealTimeThread::CreatePerformanceMonitors4Gams: %s: CreateLink Failed for Performance Monitoring %s",Name(),performanceInterface->InterfaceName());
        if(isThreadRunning) {
            CleanRealTimeThread();
        }
        return False;
    }

    return True;
}

OBJECTLOADREGISTER(RealTimeThread,"$Id: RealTimeThread.cpp,v 1.40 2011/09/10 07:43:36 aneto Exp $")
