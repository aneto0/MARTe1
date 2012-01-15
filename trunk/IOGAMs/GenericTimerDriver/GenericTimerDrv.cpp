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

#include "GenericTimerDrv.h"
#include "CDBExtended.h"
#if defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)
#include "sysLib.h"
#endif

///
bool GenericTimerDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
  
  CDBExtended cdb(info);
  
  // Common initializations
  if(!GenericAcqModule::ObjectLoadSetup(cdb,err)){
    AssertErrorCondition(InitialisationError,"GenericTimerDrv::ObjectLoadSetup: %s Generic initialization failed",Name());
    return False;
  }

  int32 timerUsecPeriod;
  if(!cdb.ReadInt32(timerUsecPeriod, "TimerUsecPeriod", -1)){
    AssertErrorCondition(InitialisationError,"GenericTimerDrv::ObjectLoadSetup: TimerUsecPeriod not found in configuration");
    return False;
  }
  if(timerUsecPeriod < 0) {
    AssertErrorCondition(InitialisationError,"GenericTimerDrv::ObjectLoadSetup: TimerUsecPeriod < 0");
    return False;
  }
 
  int32 cpuMask;
  cdb.ReadInt32(cpuMask, "CPUMask", 0xff);
  if(!ConfigAndStartTimer(timerUsecPeriod, cpuMask)) {
    AssertErrorCondition(InitialisationError,"GenericTimerDrv::ObjectLoadSetup: %s error in Timer::ConfigAndStartTimer()",Name());
    return False;
  }


  if((int32)(GetTimerUsecPeriod()) != timerUsecPeriod) {
    AssertErrorCondition(InitialisationError,"GenericTimerDrv::ObjectLoadSetup: %s error in Timer::ConfigAndStartTimer(), caught on the readback",Name());
    return False;
  }

  if(!IsTimerRunning()) {
    AssertErrorCondition(InitialisationError,"GenericTimerDrv::ObjectLoadSetup: %s -> timer not running",Name());
    return False;
  }

  return True;
}


bool GenericTimerDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err) {
    return True;
}


int32 GenericTimerDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {

    if( buffer == NULL ) return -1;

    int32 counter = (int32)GetTimerCounter();

    buffer[0] = (int32)(counter*((int32)GetTimerUsecPeriod())); // Time in Usec
    buffer[1] = (int32)counter;                                 // Counter

    return 1;
}

///
void GenericTimerDrv::UserTimerServiceRoutine() {

   for(int i = 0; i < nOfTriggeringServices; i++)
       triggerService[i].Trigger();
  
}

OBJECTLOADREGISTER(GenericTimerDrv,"$Id$")
