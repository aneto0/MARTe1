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
 * $Id: HRT.cpp 43 2012-02-08 17:07:58Z astephen $
 *
**/

#include "HRT.h"
#include "Processor.h"
#include "ErrorManagement.h"


#if defined(INTEL_PLATFORM) || defined(_SOLARIS)
    ///
    extern uint32 Processor_mSecTics;
    ///
    extern uint64 Processor_HRTFrequency;
    ///
    extern double Processor_HRTPeriod;
#endif

int64 HRTClockRate(){
#if (defined(_MSC_VER) || defined(_CY32) || defined(__EMX__) || defined(_RSXNT) || defined(_LINUX) || defined (_RTAI) || defined(_MACOSX)) || defined(_SOLARIS)
    return Processor_HRTFrequency;
#elif defined(_VX5500) || defined(_V6X5500)
    return 33000000;
#elif defined(_VX5100) || defined(_V6X5100)
    return 25000000;
#elif defined(_VX68K)
    return 16000000;
#else
#endif
}

/// the clock period in seconds
double  HRTClockCycle(){
#if defined(_MSC_VER) || defined(_CY32) || defined(__EMX__) || defined(_RSXNT) || defined(_LINUX) || defined (_RTAI) || defined(_MACOSX) || defined(_SOLARIS)
    return Processor_HRTPeriod;
#elif defined(_VX5500) || defined(_V6X5500)
    return 30e-9;
#elif defined(_VX5100) || defined(_V6X5100)
    return 40e-9;
#elif defined(_VX68K)
    return 62.5e-9;
#else
#endif
}

/// the clock tics in a msec
uint32 HRTMSecTics(){
#if (defined(_MSC_VER) || defined(_CY32) || defined(__EMX__) || defined(_RSXNT) || defined(_LINUX) || defined (_RTAI) || defined(_MACOSX)) || defined(_SOLARIS)
    return Processor_mSecTics;
#elif defined(_VX5500) || defined(_V6X5500)
    return 33000;
#elif defined(_VX5100) || defined(_V6X5100)
    return 25000;
#elif defined(_VX68K)
    return 16000;
#else
#endif
}


uint32 HRTSystemMsecTime(){
    int64 count      =  HRTRead64();
    double msecTime  =  HRTClockCycle() * 1000.0;
    msecTime = msecTime * count;
    return (uint32) msecTime;
}


