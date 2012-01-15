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

#include "SimulationTimerDrv.h"
#include "CDBExtended.h"

bool SimulationTimerDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    // Common initializations
    if(!GenericAcqModule::ObjectLoadSetup(cdb,err)){
        AssertErrorCondition(InitialisationError,"SimulationTimerDrv::ObjectLoadSetup: %s Generic initialization failed",Name());
        return False;
    }

    if(!cdb.ReadInt32(timerUsecPeriod, "TimerUsecPeriod")){
        AssertErrorCondition(InitialisationError,"SimulationTimerDrv::ObjectLoadSetup: TimerUsecPeriod not found in configuration");
        return False;
    }
    if(timerUsecPeriod < 0) {
        AssertErrorCondition(InitialisationError,"SimulationTimerDrv::ObjectLoadSetup: TimerUsecPeriod < 0");
        return False;
    }

    if(!cdb.ReadInt32(sleepAfterSecs, "SleepAfterSecs", 500)){
        AssertErrorCondition(Warning,"SimulationTimerDrv::ObjectLoadSetup: SleepAfterSecs not found in configuration");
    }
    //Start in a sleepy state
    counter = sleepAfterSecs * 1e6 / timerUsecPeriod;

    return True;
}


bool SimulationTimerDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err) {
    return True;
}


int32 SimulationTimerDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {

    if( buffer == NULL ) return -1;
    buffer[0] = (int32)(counter * timerUsecPeriod); // Time in Usec
    buffer[1] = (int32)counter;                     // Counter

    return 1;
}

OBJECTLOADREGISTER(SimulationTimerDrv,"$Id$")

