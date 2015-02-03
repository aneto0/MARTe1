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
 * $Id: HRT.h 43 2012-02-08 17:07:58Z astephen $
 *
**/

/**
 * @file
 * Access to the high resolution counters. These routines enable
 * the calculation of high resolution timings.
 *
 * The time ellapsed can be calculated using:
 * int64 t1 = HRT::HRTCounter();
 * SOME CODE
 * double totalTime = (HRT::HRTCounter() - t1) * HRT::HRTPeriod();
 */
#ifndef __HRT_H___
#define __HRT_H___

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_ARCHITECTURE(ARCHITECTURE,AtomicP.h)

extern "C" {
    /** the frequency of the HRT Clock. */
    int64   HRTClockRate();

    /** the HRT Clock period in seconds */
    double  HRTClockCycle();

    /** how many ticks in a msec for the HRT */
    uint32  HRTMSecTics();

    /** how many seconds from start of system as calculated using the HRT */
    uint32  HRTSystemMsecTime();

}


/** Access to a calibrated high frequency counter. Uses the CPU internal register */
class HRT{

public:

    /** an high resolution time counter. Only valid on pentiums CPUs and above */
    static inline int64 HRTCounter(){
        return HighResolutionTimerRead64();
    }

    /** an high resolution time counter. Only valid on pentiums CPUs and above */
    static inline uint32 HRTCounter32(){
        return HighResolutionTimerRead32();
    }

    /** to interpret the value returned by HRTCounter. Also the CPU clock!! */
    static inline int64 HRTFrequency(){
        return HRTClockRate();
    }

    /** The length of a clock period in seconds */
    static inline double HRTPeriod(){
        return HRTClockCycle();
    }

    /** converts HRT ticks to time */
    static inline double TicksToTime(int64 tStop,int64 tStart = 0){
        int64 dT = tStop-tStart;
        return dT * HRTPeriod();
    }

    /** use with care: the object must be Created
    This is a roughly 1 msec counter counting up */
    uint32 SystemMsecTime(){
        return HRTSystemMsecTime();
    }

};

#endif

