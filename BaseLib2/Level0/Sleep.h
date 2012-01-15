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
 * Implentation of task sleeping 
 */
#ifndef _SLEEP_H
#define _SLEEP_H

#include "System.h"
#include "FastMath.h"
#include "HRT.h"

#ifndef LINUX_SLEEP_NO_MORE_MIN_USEC_TIME
#define LINUX_SLEEP_NO_MORE_MIN_USEC_TIME 5000
#endif

extern "C" {

    /** Get intSleepFrequency (in Hz !!). */
    int GetSleepFrequency();

    /** Set sleepGranularity.
        @param value value is in usec and it should be
        in the range [200,1000000].     */
    bool SetSleepGranularity(int value);

    /** Get the sleep granularity.
        @return  The values is expressed in micro seconds.      */
    int GetSleepGranularity();

    /** Retrieve the time as seconds from the start of time */
    int GetDateSeconds();

    /** Synchronise the local clock with a network server (VXWORKS only )*/
    void ResynchronizeClock();

};


/** Sleeps for the time requested or more */
static inline void SleepAtLeast(double sec){
    int ticks = (int)(GetSleepFrequency()*sec+0.9999);
    if(ticks < 0) return;
#if defined(_CINT)
#elif defined(_MSC_VER) || defined(_CY32)
     Sleep(ticks);
#elif defined(_OS2)
     DosSleep(ticks);
#elif defined(_VXWORKS)
     taskDelay(ticks);
#elif defined(_RTAI)
     // Sleep requested seconds PLUS the minimum jitter,
     // so we are certain to sleep at least sec.
     // Note that RTAI_min_jitter is an absolute value!
     rt_sleep(nano2count((RTIME)((sec*1E9) + RTAI_min_jitter)));    
#elif defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
     int64                 hrtCounter;
     long                  nsecRemainder;
     struct timespec       timesValues;
     struct timespec       remTimesValues;
     double roundValue   = floor(sec);
     timesValues.tv_sec  = (time_t)roundValue;
     timesValues.tv_nsec = (long)((sec-roundValue)*1E9);
     hrtCounter = HRT::HRTCounter();
     while(1) {
         while(nanosleep(&timesValues, &remTimesValues) == -1) {
             if(errno != EINTR) {
	         return;
	     }
	     memcpy(&timesValues, &remTimesValues, sizeof(struct timespec));
	 }
	 nsecRemainder = (long)(((HRT::HRTCounter()-hrtCounter)*HRT::HRTPeriod() - sec)*1E9);
	 if(nsecRemainder >= 0) {
	     break;
	 } else {
	     timesValues.tv_sec  = 0;
	     timesValues.tv_nsec = nsecRemainder;
	 }
     }
     
#else
#endif
}

/** Sleeps no more than the time requested */
static inline void SleepNoMore(double sec){
    int ticks = (int)(GetSleepFrequency()*sec);
    if(ticks < 0) return;
#if defined(_CINT)
#elif defined(_MSC_VER) || defined(_CY32)
     Sleep(ticks);
#elif defined(_OS2)
     DosSleep(ticks);
#elif defined(_VXWORKS)
     taskDelay(ticks);
#elif defined(_RTAI)
     int64 temp = (int64)(sec*HRT::HRTFrequency());
     RTIME end = HRT::HRTCounter() + temp;
     sec *= 1E9;
     if(sec > RTAI_max_jitter) {
         rt_sleep(nano2count((RTIME)(sec-RTAI_max_jitter)));
     }
     
     while(end > HRT::HRTCounter());

#elif defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
     int64 secCounts = (int64)(sec * HRT::HRTFrequency());
     sec -= LINUX_SLEEP_NO_MORE_MIN_USEC_TIME * 1e-6;
     int64 start     = HRT::HRTCounter();
     if(sec > 0){
         struct timespec       timesValues;
         struct timespec       remTimesValues;
         double roundValue   = floor(sec);
         timesValues.tv_sec  = (time_t)roundValue;
         timesValues.tv_nsec = (long)((sec-roundValue)*1E9);
	 while(nanosleep(&timesValues, &remTimesValues) == -1) {
             if(errno != EINTR) {
	         return;
	     }
	     memcpy(&timesValues, &remTimesValues, sizeof(struct timespec));
	 }
     }
     int64 sleepUntil = secCounts + start;
     while(HRT::HRTCounter() < sleepUntil);
#else
#endif
}

/** sec/Granularity is converted to the
nearest integer to be used as number of ticks
to sleep. */
static inline void SleepSec(double sec){
     if(sec<0) return;
#if defined(_CINT)
#elif defined(_MSC_VER) || defined(_CY32)
     Sleep((unsigned long)(sec*1000.0+0.5));
#elif defined(_OS2)
     DosSleep((unsigned long)(sec*1000.0+0.5));
#elif defined(_VXWORKS)
     taskDelay(FastFloat2Int(sec*GetSleepFrequency()));
#elif defined(_RTAI)
     rt_sleep(nano2count((RTIME)(sec*1E9)));     
#elif defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
     struct timespec       timesValues;
     struct timespec       remTimesValues;
     double roundValue   = floor(sec);
     timesValues.tv_sec  = (time_t)roundValue;
     timesValues.tv_nsec = (long)((sec-roundValue)*1E9);
     while(nanosleep(&timesValues, &remTimesValues) == -1) {
         if(errno != EINTR) {
	     return;
	 }
         memcpy(&timesValues, &remTimesValues, sizeof(struct timespec));
     }
#else
#endif
}

/** sec/Granularity is converted to the
nearest integer to be used as number of ticks
to sleep. */
static inline void SleepSec(float sec){
     if(sec<0) return;
#if defined(_CINT)
#elif defined(_MSC_VER) || defined(_CY32)
     Sleep((unsigned long)(sec*1000.0+0.5));
#elif defined(_OS2)
     DosSleep((unsigned long)(sec*1000.0+0.5));
#elif defined(_VXWORKS)
     taskDelay(FastFloat2Int(sec*GetSleepFrequency()));
#elif defined(_RTAI)	
    rt_sleep(nano2count((RTIME)(((double)sec)*1E9)));    
#elif defined (_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
     struct timespec       timesValues;
     struct timespec       remTimesValues;
     double roundValue   = floor(sec);
     timesValues.tv_sec  = (time_t)roundValue;
     timesValues.tv_nsec = (long)((sec-roundValue)*1E9);
     while(nanosleep(&timesValues, &remTimesValues) == -1) {
         if(errno != EINTR) {
	     return;
	 }
         memcpy(&timesValues, &remTimesValues, sizeof(struct timespec));
     }
#else
#endif
}

/** msec/Granularity is converted to the
nearest integer to be used as number of ticks
to sleep. */
static inline void SleepMsec(int32 msec){
     if(msec<0) return;
#if defined(_CINT)
#elif defined(_MSC_VER) || defined(_CY32)
     Sleep(msec);
#elif defined(_OS2)
     DosSleep(msec);
#elif defined(_VXWORKS)
     taskDelay(FastFloat2Int((0.001 * msec * GetSleepFrequency())+0.5));
#elif defined(_RTAI)
    rt_sleep(nano2count((RTIME)(((long long)msec)*1E6)));    
#elif defined (_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
     int sec     = 0;
     int nanosec = 0;
     if(msec >=1000){
     sec     = (int)(msec/1000);
     nanosec = (int)((msec - sec*1000)*1E6);
     }else{
     sec = 0;
     nanosec = (int)(msec*1E6);
     }
     struct timespec       timesValues;
     struct timespec       remTimesValues;
     timesValues.tv_sec  = (time_t)sec;
     timesValues.tv_nsec = (long)nanosec;
     while(nanosleep(&timesValues, &remTimesValues) == -1) {
         if(errno != EINTR) {
	     return;
	 }
         memcpy(&timesValues, &remTimesValues, sizeof(struct timespec));
     }
#else
#endif
}

static inline void SleepBusy(double sec) {
    int64 startCounter = HRT::HRTCounter();
    int64 sleepUntil   = startCounter + (int64)(sec * HRT::HRTFrequency());
    while(HRT::HRTCounter() < sleepUntil);
}

#if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
/** Sleeps no more than the time requested */
static inline void SleepSemiBusy(double totalSleepSec, double nonBusySleepSec) {
    int64 startCounter      = HRT::HRTCounter();
    int64 sleepUntilCounter = startCounter + (int64)(totalSleepSec*HRT::HRTFrequency());
    if((nonBusySleepSec < totalSleepSec) && (nonBusySleepSec > 0.0)) {
        struct timespec   timesValues;
	struct timespec   remTimesValues;
	double roundValue   = floor(nonBusySleepSec);
	timesValues.tv_sec  = (time_t)roundValue;
	timesValues.tv_nsec = (long)((nonBusySleepSec-roundValue)*1E9);
	while(nanosleep(&timesValues, &remTimesValues) == -1) {
	    if(errno != EINTR) {
	        return;
	    }
	    memcpy(&timesValues, &remTimesValues, sizeof(struct timespec));
	}
    }
    while(HRT::HRTCounter() < sleepUntilCounter);
}
#endif

#if !defined (SLEEP_PRIVATE)
/** Mask Sleep to trigger an error */
#define Sleep #error do not use sleep
#endif

#endif

