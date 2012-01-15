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
 * Defines the timeout
 */
#ifndef TIMEOUT_TYPE_H
#define TIMEOUT_TYPE_H

#include "GenDefs.h"
#include "HRT.h"

/** max value for the delay that is treated  */
const uint32 TTMaxDelay = 0xFFFF0000;

/** type to indicate specific wait in the timeout parameters */
class TimeoutType{
public:
    /** how many msecs to wait */
    uint32 msecTimeout;
public:
    /** constructor from integer*/
    TimeoutType(uint32 msecs = (uint32)0xFFFFFFFF){
        msecTimeout = msecs;
    }

    /** constructor from float */
    void SetTimeOutSec(double secs){
        msecTimeout = (uint32)(secs * 1000.0);
    }

    /** constructor from HRT ticks */
    void SetTimeOutHRTTicks(int64 ticks){
        if (ticks < 0) ticks = 0;
        double msDT = 1000.0 *(ticks * HRT::HRTPeriod());
        msecTimeout = (uint32)msDT;
    }

    /** constructor from HRT ticks */
    int64 HRTTicks() const{
        double dT = msecTimeout;
        dT = dT * 1e-3;
	double freq = HRT::HRTFrequency();
        dT = dT * freq;
        int64 ticks = (int64)dT;
        return ticks;
    }

    /** bounded reduction */
    void operator-=(int n){
        if (msecTimeout > n) msecTimeout-=n;
        else msecTimeout = 0;
    }

    /** comparison */
    bool operator==(const TimeoutType tt){
        return msecTimeout == tt.msecTimeout;
    }

    /** comparison */
    bool operator!=(const TimeoutType tt){
        return msecTimeout != tt.msecTimeout;
    }

    /** copy */
    void operator=(const TimeoutType tt){
        msecTimeout = tt.msecTimeout;
    }

    /** A finite timeout is not infinite not unprotected */
    bool IsFinite(){
        return (msecTimeout < (uint32)0xFFFFFFFE);
    }

};

#if !defined (_CINT)

/** Do not wait (or wait indefinitely if blocking is set */
const TimeoutType TTNoWait((uint32)0x00000000);

/** Infinite wait Timeout */
const TimeoutType TTInfiniteWait((uint32)0xFFFFFFFF);

/** Used in semafore protected codes to specify to bypass the check! */
const TimeoutType TTUnProtected ((uint32)0xFFFFFFFD);

/** Used in semafore protected codes to specify to bypass the check! */
const TimeoutType TTDefault ((uint32)0xFFFFFFFE);

#else

#define TTNoWait 0
#define TTInfiniteWait  -1
#define TTUnProtected -3
#define TTDefault -2

#endif

#endif

