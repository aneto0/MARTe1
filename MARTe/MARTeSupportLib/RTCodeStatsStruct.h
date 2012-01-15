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

#ifndef _RT_CODE_PERMORMANCE
#define _RT_CODE_PERMORMANCE

#include "System.h"
#include "GCRTemplate.h"
#include "TimeTriggeringServiceInterface.h"
#include "HRT.h"

/** Real-Time Code Performance Monitoring Class */

class RTCodeStatsStruct {
private:

    /** Number of entries to monitor */
    int32           numberOfEntries;

private:

    /** Previous start of cycle time */
    int64           previousCycleStartTickTime;    

private:

    /** Pointer to the area in the DDB Interface that contains the time base */
    uint32          *timeBase;

    /** Pointer to the area in the DDB Interface that contains the cycle time */
    float           *cycleTime;

    /**  Pointer to the area in the DDB Interface that contains the relative Time Points */
    float           *relativeUsecTimePoint;

    /**  Pointer to the area in the DDB Interface that contains the absolute Time Points */
    float           *absoluteUsecTimePoint;

    /** The execution start time for this GAM*/
    int64            GAMexecutionStartTimeCounter;
private:

    /** Reference to the external Time and triggering service of the RT Thread */
    GCRTemplate<TimeTriggeringServiceInterface>   trigger;

private:
    /** Initialise parameters for construction */
    void Init(){
        numberOfEntries              = 0;
        previousCycleStartTickTime   = 0;
        GAMexecutionStartTimeCounter = 0;
        timeBase                     = NULL;
        cycleTime                    = NULL;
        relativeUsecTimePoint        = NULL;
        absoluteUsecTimePoint        = NULL;
    }

public:

    /** Constructor */
    RTCodeStatsStruct(){
        Init();
    }

    /** Destructor */
    ~RTCodeStatsStruct(){};

    /** Initialise the RTCodeStatsStruct parameters.
        This function is to be called at PRE PULSE, because at Initialise the DDB Interfaces buffers are not
        available.
        The code assumes that the first entry in the DDB Interface data buffer is the time in microseconds relative
        to the monitoring measurement, the cycle Time is the following entry, followed by the relative Time Points
        and the absolute Time Points
        @param entries The number of entries to monitor
        @param ddbBuffer Pointer to the DDB Interface buffer where to write the results.
        @return True if everything was ok. False otherwise.
    */

    bool Initialise(int32 entries, const int32 *ddbBuffer, GCRTemplate<TimeTriggeringServiceInterface>   trigger){

        Init();
        if(ddbBuffer == NULL) return False;

        numberOfEntries       = entries;
        // Time Base
        timeBase              = (uint32 *)(ddbBuffer);
        // First entry
        cycleTime             = (float  *)(timeBase + 1);
        // Relative Points
        relativeUsecTimePoint = (float   *)cycleTime + 1;
        // Absolute Points
        absoluteUsecTimePoint = relativeUsecTimePoint + numberOfEntries;

        // Init content
        *cycleTime = 0;
        for(int i = 0; i < numberOfEntries; i++){
            relativeUsecTimePoint[i] = 0.0;
            absoluteUsecTimePoint[i] = 0.0;
        }

        this->trigger = trigger;
        if(!(this->trigger.IsValid()))return False;

        return True;
    }

    /** Compute performance for entry k.
        @param entry The module number to monitor
        @param return True if everything was ok. False if entry is out of range.
    */
    bool StorePerformance(int entry){
	if(cycleTime == NULL) return False;
        if((entry < 0 ) || (entry >= numberOfEntries)) return False;
        float hrtPeriod = HRT::HRTPeriod();
        int64 deltaT = (HRT::HRTCounter() - GAMexecutionStartTimeCounter);
        relativeUsecTimePoint[entry] = deltaT * hrtPeriod;
        absoluteUsecTimePoint[entry] = (trigger->GetInternalCycleTickTime())*hrtPeriod;

        int64 newCycleStartTickTime = trigger->GetLastProcessorTickTime();
        if(previousCycleStartTickTime < newCycleStartTickTime){
            *timeBase                  = trigger->GetPeriodUsecTime();
            *cycleTime                 = (newCycleStartTickTime - previousCycleStartTickTime)*hrtPeriod;
            previousCycleStartTickTime =  newCycleStartTickTime;
        }

        return True;
    }

    /** Signals a new performance measurament cycle*/
    void StartGAMMeasureCounter(){
        GAMexecutionStartTimeCounter = HRT::HRTCounter();
    }

};

#endif
