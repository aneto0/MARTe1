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

#define SLEEP_PRIVATE
#include "Sleep.h"

#if (defined(_MSC_VER) || defined(_CY32) || defined(_RTAI) || defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))

/// Get intSleepFrequency (in Hz !!)
int GetSleepFrequency(){
    return 100;
}

///
int GetSleepGranularity(){
    return 10000;
}

/** Set sleepGranularity
    value is in usec and it should be
    in the range [200,1000000] */
bool SetSleepGranularity(int value){
    return (value == 10000);
}

#elif defined(_OS2)

/// Get intSleepFrequency (in Hz !!)
int GetSleepFrequency(){
    return 33;
}

///
int GetSleepGranularity(){
    return 32000;
}

/** Set sleepGranularity
    value is in usec and it should be
    in the range [200,1000000] */
bool SetSleepGranularity(int value){
    return (value == 32000);
}


#elif defined(_VXWORKS)
#include <vxWorks.h>
#include <time.h>

extern "C"
{
#include <rts.h>   
}

static time_t clockTime;

/// SleepGranularity Class
class SleepGranularity{
    /// Sleep granularity (in usec)
    /// Default value 1000 usec (1 msec)
    int granularity;

    /// Frequency in Hz as int
    int frequency;
public:
    /// Constructor
    SleepGranularity(){

        granularity = 1000;
        frequency = 1000;

	// Synchronize the system clock with the Server clock
	jvSetClockFromHost();
        /// Initial granularity is 1000 usec
        bool ret = SetGranularity(1000);
        if(!ret){
             printf("Error while setting sleep granularity\n");
        }
    }

    /// Get sleepGranularity
    inline int GetGranularity(){
        return granularity;
    }

    /// Get sleepFrequency (in kHz !!)
    inline int GetFrequency(){
        return frequency;
    }

    /** Set sleepGranularity
        value is in usec and it should be
        in the range [200,1000000] */
    bool SetGranularity(int value){
        if(sysClkRateSet(1000000/value)==OK){
	    time(&clockTime);
            granularity = value;
            frequency = 1000000/granularity;
            return True;
        }
        return False;
    }
} SleepGranularity_;

/// Get intSleepFrequency (in Hz !!)
int GetSleepFrequency(){
    return SleepGranularity_.GetFrequency();
}

///
int GetSleepGranularity(){
    return SleepGranularity_.GetGranularity();
}

/** Set sleepGranularity
    value is in usec and it should be
    in the range [200,1000000] */
bool SetSleepGranularity(int value){
    return SleepGranularity_.SetGranularity(value);
}

#endif

int GetDateSeconds(){
#if defined(_VXWORKS)
	const int defaultClkRate = 60;
	time_t actualTime;
	time(&actualTime);
	actualTime = (actualTime - clockTime)*defaultClkRate;
	actualTime /= SleepGranularity_.GetFrequency();
	return (int)(actualTime + clockTime);
#else
    return time((time_t *)NULL);
#endif
}


void ResynchronizeClock(){
#if defined(_VXWORKS)
    jvSetClockFromHost();
    SetSleepGranularity(GetSleepGranularity());
    return;
#else
    return;
#endif
}
