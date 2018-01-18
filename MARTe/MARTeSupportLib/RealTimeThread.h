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


#ifndef _REAL_TIME_THREAD_H_
#define _REAL_TIME_THREAD_H_

/**
  @file
  */

#include "Object.h"
#include "DDB.h"
#include "MutexSem.h"

#include "RTCodeStatsStruct.h"

#include "DDBOutputInterface.h"

#include "StartStopMessageHandlerInterface.h"
#include "GCReferenceContainer.h"
#include "MenuInterface.h"

#include "GAM.h"
#include "TimeTriggeringServiceInterface.h"
#include "GenericAcqModule.h"
#include "HttpInterface.h"

class RealTimeThread;

class ExecutionModule{
private:
    /** Reference to the GAM */
    GCRTemplate<GAM>         gam;

    /** Execution code */
    GAM_FunctionNumbers      code;

public:

    /** The last amount of execution time in count units*/
    int64 lastAmountOfExecTimeCounts;

    /** The last time it was executed*/
    int64 lastExecutionTimeCounts;

    /** Constructor */
    ExecutionModule(GCRTemplate<GAM> gam, GAM_FunctionNumbers code){
        this->gam                  = gam;
        this->code                 = code;
        lastAmountOfExecTimeCounts = 0;
        lastExecutionTimeCounts    = 0;
    }

    /** Default Constructor */
    ExecutionModule(){}

    /** Execute gam with code */
    bool Execute(){
        lastExecutionTimeCounts = HRT::HRTCounter();
        bool ok = gam->Execute(code);
        lastAmountOfExecTimeCounts = HRT::HRTCounter() - lastExecutionTimeCounts;
        return ok;
    }

    /** */
    GCRTemplate<GAM> Reference(){return gam;}

    /** Load info for execution module */
    bool ObjectLoadSetup(FString gamName, GAM_FunctionNumbers gamCode, StreamInterface *err, RealTimeThread &rt);
};


OBJECT_DLL(RealTimeThread)
extern "C"
{
    bool TriggerMenu(StreamInterface &in,StreamInterface &out,void *userData); 
    bool BrowseRTDDBMenu(StreamInterface &in,StreamInterface &out,void *userData);
    bool DumpRTDDBMenu(StreamInterface &in,StreamInterface &out,void *userData);
}

/** The class representing a real time thread of execution. */
class RealTimeThread : public GCReferenceContainer,public StartStopMessageHandlerInterface, public HttpInterface{

    OBJECT_DLL_STUFF(RealTimeThread)
    //MENU INTERFACES
    friend bool TriggerMenu(StreamInterface &in,StreamInterface &out,void *userData);
    friend bool BrowseRTDDBMenu(StreamInterface &in,StreamInterface &out,void *userData);
    friend bool DumpRTDDBMenu(StreamInterface &in,StreamInterface &out,void *userData);

private:
    /** Process Http Message */
    virtual bool ProcessHttpMessage(HttpStream &hStream);

private:    
    //REAL TIME THREAD MESSAGE HANDLER INTERFACE

    virtual bool                                   ProcessMessage2(GCRTemplate<MessageEnvelope> envelope);

    /** The Time and Triggering Service. */
    GCRTemplate <MessageDeliveryRequest>           sendFatalErrorMessage;

private:
    /** CPU identifier where to run the RT Thread */
    int32                                          runOnCPU;

    /** Mux protects access to the private ExecutionModule(s), otherwise
     * racing conditions will occur between the CleanRealTimeThread and ProcessHttpMessage
     */
    MutexSem                                       realTimeThreadCleanSem;

    /** Implements the thread functionality
      @param rTThread The reference to the RealTimeThread object.
      */
    friend void                                    RTAppThread(RealTimeThread &rTThread);

    /** Implements the Real Time Thread Activities.*/
    void                                           RTThread();

private:
    //RealTimeThread GAMs and DDB lists
    /** Vector of execution modules for real time activities*/
    ExecutionModule                               *onlineModules;

    /** Number of gams in the online activities vector */
    int32                                         nOfOnlineGams;

    /** Vector of execution modules for offline  activities*/
    ExecutionModule                               *offlineModules;

    /** Number of gams in the offline activities vector */
    int32                                         nOfOfflineGams;

    /** Vector of execution modules for safety activities*/
    ExecutionModule                               *safetyModules;

    /** Number of gams in the safety activities vector */
    int32                                         nOfSafetyGams;

    /** Millisecond Sleep if not module is specified. Only for safety */
    int32                                         safetyMsecSleep;

    /** Vector of execution modules that need initialisation phase*/
    ExecutionModule                               *initialisingModules;

    /** Number of gams in the initialisation activities vector */
    int32                                         nOfInitialisingGams;

    /** Number of consecutive initialising cycles*/
    int32                                         numberOfInitialisingCycles;

    /** Max number of initialising cycles */
    int32                                         maxnOfInitialisingCycles;

    /** Number of cycles spent in state SM_PREPULSE */
    int32 prepulseCycleCount;

    /** Number of cycles spent in state SM_POSTPULSE */
    int32 postpulseCycleCount;

    /** Number of cycles spent in state SM_PULSING : reset at PREPULSE */
    int32 pulsingCycleCount;

    /** Number of cycles spent in state OFFLINE : reset at POSTPULSE */
    int32 offlineCycleCount;

    /** The DDB used by the real time thread.*/
    GCRTemplate<DDB>                               ddb;

private:

    /** Monitors the performance of the RTThread and GAMs */
    RTCodeStatsStruct                               performanceMonitor;

    /** Saves the performance of the Real Time Thread and GAMs into the DDB */
    DDBOutputInterface                             *performanceInterface;

    bool                                            CreatePerformanceMonitors4Gams(ConfigurationDataBase &info);

private:

    /** The Time and Triggering Service. */
    GCRTemplate<TimeTriggeringServiceInterface>     trigger;

private:

    class Status{
private:
        /** Status */
        int32       status;

        /** Requested status */
        int32       requestedStatus;

        /** Timeout for the request to be completed */
        int32       mSecTimeOut;
public:
        Status(){
            status          = 0;
            requestedStatus = 0;
            mSecTimeOut     = 0;
        }

        ~Status(){};

        /** Assignement constructor */
        Status& operator=(int32 newStatus){
            requestedStatus = newStatus;
            status          = newStatus;
            return *this;
        }

        /** Operator == */
        bool  operator == (int32 status){
            return (this->status == status);
        }

        /** Operator != */
        bool  operator != (int32 status){
            return (this->status != status);
        }

        /** Request a status change.
          Requests a change in the status and waits for
          the accomplishment.
          */
        bool       Request(int32 newStatus){
            requestedStatus = newStatus;
            int sleepCount  = 0;
            while((status != newStatus) && (sleepCount++ <= mSecTimeOut)){
                SleepMsec(1);
            }

            return (status == newStatus);
        }

        /** Request a status change without waiting : allows caller to check more flexibly. */

        bool       RequestWithoutWait(int32 newStatus){
            requestedStatus = newStatus;
            return True;
        }

        /** Request a status change.
          Requests a change in the status and waits for status to advance to a different state.
          */
        bool       Request(int32 newStatus, int32 waitStatus){
            requestedStatus = newStatus;
            int sleepCount  = 0;
            while((status != waitStatus) && (sleepCount++ <= mSecTimeOut)) {
                SleepMsec(1);
            }

            return (status == waitStatus);
        }

        /** Copies the requested status on the status.          
        */
        void       Refresh(){
            status = requestedStatus;
        }

        /** Sets the timeout value */
        void       SetMsecTimeOut(int32 timeOut){
            mSecTimeOut = timeOut;
        }

        /** Returns the timeout value : useful for RequestWithoutWait */
        int32 GetMsecTimeOut(void) {
            return mSecTimeOut;
        }
    };

private:
    /** State Machine Status, changed by message processing.
        Possible values of the status are:
        SM_IDLE         : performs offline activities
        SM_WAITING_PRE  : performs check activities
        SM_PREPULSE     : performs the PulseStart activities before switching to SM_PULSING
        SM_PULSING      : performs online activities
        SM_POSTPULSE    : performs the PostPulse activities before switching to SM_IDLE
    */
    Status                                          smStatus;
    /** Status of the Real Time Thread.
        Possible values of status are:
        RTAPP_UNINITIALISED: During initialization
        RTAPP_READY    : During standard operation
        RTAPP_SAFETY   : When a major fault has been identified
    */
    Status                                          rtStatus;

    /** Switch status request. It is used to avoid the presence
      of the mutex semaphore int the real time thred. The function
      that requires the change asks for the transition to the RTAPP_UNINITIALISED
      and waits that @param rtStatus assumes the requested value. 
      */
    int32                                           rtStatusRequest;

    /** The Thread ID.*/
    TID                                             threadID;

    /** The Thread Priority.*/
    int32                                           priority;

    /** Specify if the thread is running or is dead.*/
    bool                                            isThreadRunning;

    /** Request the thread termination.*/
    bool                                            stopThread;

private:
    /** Starts the thread activities.
      @return True if the thread started correctly. False if the thread is already alive or failed to start.
      */
    bool Start();

    /** Stops the thread activities.
      @return True if the thread stopped correctly.
      */
    bool Stop();

    /** Runs the GAMs prepulse activities in response to pulse in progress message.
      @return True everything is ok.
      */
    bool PulseStart();

    /** Runs the GAMs postpulse activities in response to pulse in termination message.
      @return True everything is ok.
      */
    bool PostPulse();

    /** Runs the GAMs check activities in response to a waiting for pre message.
      @return True everything is ok.
      */
    bool Check();

    /** Stops the Real Time Thread on a semaphore and reinitializes all GAM according
      to the information stored in the CDB */
    bool HandleLevel1Message(ConfigurationDataBase &cdb);


    /** Prepare Message Reply to the Senders when processing message */
    bool SendMessageReply(GCRTemplate<MessageEnvelope> envelope, bool ok);

    /** Clean the RealTimeThread GAMs and DDB*/
    bool CleanRealTimeThread();
public:

    RealTimeThread();

    bool CheckStatus(int32 rqstatus);

    virtual ~RealTimeThread(){
        Stop();
        if(onlineModules        != NULL) delete[] onlineModules;
        if(offlineModules       != NULL) delete[] offlineModules;  
        if(safetyModules        != NULL) delete[] safetyModules;
        if(initialisingModules  != NULL) delete[] initialisingModules;        
        if(performanceInterface != NULL) delete   performanceInterface;

        realTimeThreadCleanSem.Close();
    }

    /** Initializes the thread parameters and the GAMs from the configuration file.*/
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** Saves the thread parameters and the GAMs parameters in the configuration file.*/
    bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err);

};

#endif

