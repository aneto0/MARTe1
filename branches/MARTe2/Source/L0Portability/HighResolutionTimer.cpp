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

#include "HighResolutionTimer.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,HighResolutionTimerCalibratorOS.h)

static HighResolutionTimerCalibratorOS highResolutionTimerCalibratorOS;

/** the frequency of the HRT Clock. */
int64   HighResolutionTimerFrequency(){
    return highResolutionTimerCalibratorOS.HRTFrequency;
}

/** the HRT Clock period in seconds */
double  HighResolutionTimerPeriod(){
    return highResolutionTimerCalibratorOS.HRTPeriod;
}

/** how many ticks in a msec for the HRT */
uint32  HighResolutionTimerMSecTics(){
    return highResolutionTimerCalibratorOS.HRTmSecTics;
}

