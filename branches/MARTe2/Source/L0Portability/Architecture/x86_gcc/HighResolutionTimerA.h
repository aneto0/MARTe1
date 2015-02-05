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
#ifndef _HIGH_RESOLUTION_TIMER_P_H_
#define _HIGH_RESOLUTION_TIMER_P_H_


/** Reads the High Resolution Timer as 32 bit.Fast inline assembler. */
static inline uint32 HighResolutionTimerRead32() {
    uint64 perf;
    uint32 *pperf = (uint32 *)&perf;
    asm(
    "\n"
    "        rdtsc        \n"
    : "=a"(pperf[0]) , "=d"(pperf[1])
    :
    : "eax","edx"
    );
    return (uint32)perf;
}


/** Reads the High Resolution Timer as 64 bit int. Fast inline */
static inline int64 HighResolutionTimerRead64 () {
    volatile int64 perf;
    uint32 *pperf = (uint32 *)&perf;
    asm volatile(
"\n"
"        rdtsc        \n"
       : "=a"(pperf[0]) , "=d"(pperf[1])
    );
    return perf;
}

#endif

