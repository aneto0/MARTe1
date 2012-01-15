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


#include "System.h"
#include "Processor.h"

/** Reads the High Resolution Timer as 32 bit.Fast inline assembler. */
static inline uint32 HRTRead32() {
#if defined(_MSC_VER)
    __asm  {
        _emit 0x0F
        _emit 0x31
    }
#elif (defined(_CY32) || defined(__EMX__) || defined(_RSXNT) || defined(_LINUX) || defined(_RTAI) || defined(_MACOSX))

    uint64 perf;
    uint32 *pperf = (uint32 *)&perf;
    asm(
    "\n"
    "        rdtsc        \n"
    : "=a"(pperf[0]) , "=d"(pperf[1])
    :
    : "eax","edx"
    );
//    return loPerf; non riconosce loPerf!! luigi
    return (uint32)perf;
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)
    uint32 time ;
    asm(
        "mftb %0\n"
        : "=r" (time)
    );

    return time;
#else
#endif
}


/** Reads the High Resolution Timer as 64 bit int. Fast inline */
static inline int64 HRTRead64 () {
#if defined(_MSC_VER)
    __asm  {
        _emit 0x0F
        _emit 0x31
    }
#elif (defined(_CY32) || defined(__EMX__) || defined(_RSXNT))
    volatile int64 perf;
    uint32 *pperf = (uint32 *)&perf;
    asm(
"\n"
"        rdtsc        \n"
       : "=a"(pperf[0]) , "=d"(pperf[1])
       :
       : "eax","edx"
    );
    return perf;
#elif (defined(_LINUX) || defined(_MACOSX))
    volatile int64 perf;
    uint32 *pperf = (uint32 *)&perf;
    asm volatile(
"\n"
"        rdtsc        \n"
       : "=a"(pperf[0]) , "=d"(pperf[1])
    );
    return perf;
#elif (defined(_RTAI) )
    int64 perf;
    uint32 *pperf = (uint32 *)&perf;
    asm(
        "rdtsc"
       : "=a"(pperf[0]) , "=d"(pperf[1])
       :
//       : "eax","edx"
    );
    return perf;
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

    int a ,b ,c ,d;
    asm(
        "mftbu %0\n"
        "mftb %1\n"
        "mftbu %2\n"
        "mftb %3\n"
        : "=r" (a), "=r" (b), "=r" (c), "=r" (d)
    );

    int64 time;
    int *pout = (int *) &time;
    if (a != c){
        pout[0] = c;
        pout[1] = d;
    } else {
        pout[0] = a;
        pout[1] = b;
    }
    return time;
#else
    return 0;
#endif
}


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
        return HRTRead64();
    }

    /** an high resolution time counter. Only valid on pentiums CPUs and above */
    static inline uint32 HRTCounter32(){
        return HRTRead32();
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

