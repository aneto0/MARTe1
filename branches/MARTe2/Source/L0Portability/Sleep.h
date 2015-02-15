/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
   permissions and limitations under the Licence. 
 *
 * $Id: $
 *
**/
/**
 * @file
 * Implentation of task sleeping 
 */
#ifndef SLEEP_H
#define SLEEP_H

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,SleepOS.h)
extern "C" {
    /** Retrieve the time as seconds from the start of time */
    int GetDateSeconds();
};


/** Sleeps for the time requested or more */
static inline void SleepAtLeast(double sec){
    SleepOSAtLeast(sec);
}

/** Sleeps no more than the time requested */
static inline void SleepNoMore(double sec){
    SleepOSNoMore(sec);
}

/** sec/Granularity is converted to the
nearest integer to be used as number of ticks
to sleep. */
static inline void SleepSec(double sec){
    SleepOSSecDouble(sec);
}

/** sec/Granularity is converted to the
nearest integer to be used as number of ticks
to sleep. */
static inline void SleepSec(float sec){
    SleepOSSecFloat(sec);
}

/** msec/Granularity is converted to the
nearest integer to be used as number of ticks
to sleep. */
static inline void SleepMSec(int32 msec){
    SleepOSMSec(msec); 
}

static inline void SleepBusy(double sec) {
    int64 startCounter = HighResolutionTimer::Counter();
    int64 sleepUntil   = startCounter + (int64)(sec * HighResolutionTimer::Frequency());
    while(HighResolutionTimer::Counter() < sleepUntil);
}

/** Sleeps no more than the time requested */
static inline void SleepSemiBusy(double totalSleepSec, double nonBusySleepSec) {
    SleepOSSemiBusy(totalSleepSec, nonBusySleepSec);
}
#endif

