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

#if !defined(_TIME_AND_TRIGGERING_SERVICE_INTERFACE)
#define _TIME_AND_TRIGGERING_SERVICE_INTERFACE

#include "System.h"
#include "GCRTemplate.h"
#include "GCReferenceContainer.h"

#include "TimeServiceActivity.h"
#include "HRT.h"

class GenericAcqModule;

/// Structure containing the Time information
struct TimeInfo{

    /// Last Time obtained from the Time Module (in usec)
    uint32 lastPeriodUsecTime;

    /// Last Sample from the Cpu Internal clock as ticks
    int64  lastProcessorTickTime;

    /// Override copy since it doesn't seem to work with interrupts
    inline void operator=(const TimeInfo &x){
        lastPeriodUsecTime    = x.lastPeriodUsecTime;
        lastProcessorTickTime = x.lastProcessorTickTime;
    }

    TimeInfo(){
        lastPeriodUsecTime    = 0;
        lastProcessorTickTime = 0;
    }
};

class TimeTriggeringServiceInterface;

extern "C"{
    bool TTSIObjectLoadSetup(TimeTriggeringServiceInterface &ttis, ConfigurationDataBase &info,StreamInterface *err);
    bool TTSIObjectDescription(TimeTriggeringServiceInterface &ttis, StreamInterface &s,bool full=False,StreamInterface *err=NULL);
    bool TTSITrigger(TimeTriggeringServiceInterface &ttis);
    bool TTSIStart(TimeTriggeringServiceInterface &ttis);
    bool TTSIStop(TimeTriggeringServiceInterface &ttis);
}

OBJECT_DLL(TimeTriggeringServiceInterface)

/** External Time and Triggering Service Class.
    It's a container of TimeServiceActivity objects */
class TimeTriggeringServiceInterface : public GCReferenceContainer{

    OBJECT_DLL_STUFF(TimeTriggeringServiceInterface)

    friend bool TTSIObjectLoadSetup(TimeTriggeringServiceInterface &ttis, ConfigurationDataBase &info,StreamInterface *err);
    friend bool TTSIObjectDescription(TimeTriggeringServiceInterface &ttis, StreamInterface &s,bool full,StreamInterface *err);
    friend bool TTSITrigger(TimeTriggeringServiceInterface &ttis);
    friend bool TTSIStart(TimeTriggeringServiceInterface &ttis);
    friend bool TTSIStop(TimeTriggeringServiceInterface &ttis);

private:

    ////////////////
    // REFERENCES //
    ////////////////

    /** Contains the list of Time Service Activities */
    GCRTemplate<TimeServiceActivity>     *timeActivities;

protected:
    /** Reference to the TimeModule in the DriverPool linkedlist */
    GCRTemplate<GenericAcqModule>        timeModule;


protected:
    ///////////////////
    // CONFIGURATION //
    ///////////////////

    /** Flag to monitor Online Pulsing Activities. To be controlled
	by the owning object. */
    bool                                 onlinePulsing;

    /** Time service period for Online Operations in usec */
    int32                                tsOnlineUsecPeriod;

    /** Time service phase for Online Operations in usec */
    int32                                tsOnlineUsecPhase;

    /** Time service period for Offline Operations in usec */
    int32                                tsOfflineUsecPeriod;

    /** Time service phase for Offline Operations in usec */
    int32                                tsOfflineUsecPhase;

    /** Time service timeout */
    int32                                tsTimeOutMsecTime;

private:

    ////////////
    // STATUS //
    ////////////

    /** Flag monitoring the status of the activities. */
    bool                                 serviceRunning;

    /** Double buffer containing the time information.
        Always there is one write-only TimeInfo structure
        while the other is read-only.
        Is the the Trigger() method, updating the time, the one who
        switch  the index of the write-only buffer */
    TimeInfo                             timeInfo[2];

    /** Index of the write only buffer for time information.
        The read only buffer is equal to 1-writeBuffer */
    int                                  writeBuffer;


    /** Get timeInfo (return the read-only buffer) */
    inline TimeInfo GetTimeInfo() const { return timeInfo[1-writeBuffer]; }

public:

    /** Constructor */
    TimeTriggeringServiceInterface(){
        timeActivities        =  NULL;
        tsOnlineUsecPeriod    =     0;
        tsOnlineUsecPhase     =     0;
        tsOfflineUsecPeriod   =     0;
        tsOfflineUsecPhase    =     0;
	    tsTimeOutMsecTime     =    -1;
        writeBuffer           =     0;
        onlinePulsing         = False;
        serviceRunning        = False;
    }

    /** Destructor */
    virtual ~TimeTriggeringServiceInterface(){
        AssertErrorCondition(Information,"TimeTriggeringServiceInterface being destroyed");
        // Stop the service if it is running
        if(serviceRunning == True) Stop();
        if(timeActivities != NULL) delete[] timeActivities;
        CleanUp();
    }

    /** Set the Online Flag */
    void             SetOnlineActivities(bool online = False){ onlinePulsing = online; }

    /** Starts TimeTriggeringServiceInterface */
    bool             Start(){return TTSIStart(*this);}

    /** Stops  TimeTriggeringServiceInterface */
    bool             Stop(){return TTSIStop(*this);};

    /////////////////////////
    // Timing Informations //
    /////////////////////////

    /** Gets internal cycle time in ticks */
    inline int64     GetInternalCycleTickTime() const { return HRT::HRTCounter() - GetTimeInfo().lastProcessorTickTime;   }

    /** Gets the last sample from the Cpu Internal clock as ticks */
    inline uint64    GetLastProcessorTickTime() const { return GetTimeInfo().lastProcessorTickTime; }

    /** Gets time in usec but as multiple of cycle time */
    inline uint32    GetPeriodUsecTime()        const { return GetTimeInfo().lastPeriodUsecTime; }

    /** Returns the period in usec */
    inline int32     GetUsecPeriod()            const {
        if( onlinePulsing ) return tsOnlineUsecPeriod;
        return                     tsOfflineUsecPeriod; 
    }

    /** Returns the period in cpu ticks*/
    inline int64     GetTickPeriod()            const {
        int64 tickPeriod = GetUsecPeriod()*HRT::HRTFrequency()/1000000;
        return tickPeriod;
    }

    /** This function is to be called by the synchronising GenericAcqModule
        on data arrival. For data arrival that is signaled by interrupt, this
        method is to be called within the ISR. For data arrival that is based
        on polling a register, this method is to be called within the GetData
        method of the synchronising GenericAcqModule.
        Functionality:
        If ((ActualTime % Period) == Phase) this method updates the timeInfo 
        and calls the the SignalNewCycle() method. 
        The Trigger methods of the registered TimeServiceActivities 
        @param timeActivities are called.
     */
    virtual bool     Trigger(){return TTSITrigger(*this);};

    /** Save Time And Triggering Service configuration parameters */
    virtual bool     ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL){return TTSIObjectDescription(*this, s, full, err);}

    /** Load Time And Triggering Service configuration parameters. */
    virtual bool     ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){return TTSIObjectLoadSetup(*this, info, err);}

    //////////////////////////////////
    // Interfaces to be implemented //
    //////////////////////////////////

    /** Synchronizes the system to the cycle time.
        For Triggering methods posted by interrupts the
        Synchronise() function waits on a semaphore to be posted
        by the ISR(), namely the SignalNewCycle() function.
        For Triggering methods based on data arrival without 
        interrupts, the Synchronise() function returns immediately 
        and the synchronisation is performed by the GetData method of
        the synchronising module. 
     */
    virtual bool     Synchronise() = 0;

private:

    /** This function is used within the Trigger() method.
        It performs a set of activities marking the start of the new real-time
        cycle. For interrupt driven Synchronising methods, the SignalNewCycle()
        function posts the semaphore the Synchronise function is waiting on.
        For Triggering methods based on data arrival, the SignalNewCycle returns
        without performing any activity. 
     */
    virtual bool     SignalNewCycle() = 0;
};

#endif
