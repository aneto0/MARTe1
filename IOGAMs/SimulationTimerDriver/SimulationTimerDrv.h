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

#if !defined(_SIMULATION_TIMER_DRV)
#define _SIMULATION_TIMER_DRV

#include "GenericAcqModule.h"

OBJECT_DLL(SimulationTimerDrv)

/** @file
 * A timing driver to be used on simulations. It runs in polling mode as fast as the 
 * the system can execute.
 * */
class SimulationTimerDrv : public GenericAcqModule {

    OBJECT_DLL_STUFF(SimulationTimerDrv)

private:
    /** An internal counter of the number of times the polling method was called*/
    int32 counter;
    
    /** The pseudo-period as specified in the configuration file*/
    int32 timerUsecPeriod;

    /** When the time goes over this period in seconds it will sleep...*/
    int32 sleepAfterSecs;
public:

    SimulationTimerDrv(){
        counter         = 0;
        timerUsecPeriod = 0;
        sleepAfterSecs  = 0;
    }

    virtual ~SimulationTimerDrv(){}
    
    /** Object Load Setup */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
    
    /** Object Description */
    bool ObjectDescription(StreamInterface &s,bool full = False,StreamInterface *err = NULL);
    
    /** Get Data */
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);

    /** Write Data - not supported by this driver */
    bool WriteData(uint32 usecTime, const int32 *buffer){return False;}

    /** Set board used as input */
    virtual bool SetInputBoardInUse(bool on = True) {
        if(inputBoardInUse && on){
        	AssertErrorCondition(InitialisationError, "SimulationTimerDrv::SetInputBoardInUse: Board %s is already in use", Name());
	        return False;
          }
      
          inputBoardInUse  = on;
          return True;
    }
    
    virtual bool SetOutputBoardInUse(bool on = True){
        AssertErrorCondition(InitialisationError, "SimulationTimerDrv::SetOutputBoardInUse: Board %s cannot be used as output", Name());
        return False;
    }

    virtual bool Poll(){
        if(GetUsecTime() > (sleepAfterSecs * 1e6)){
            SleepSec(timerUsecPeriod * 0.5e-6);
        }
        counter++;
        for(int i = 0; i < nOfTriggeringServices; i++){
            triggerService[i].Trigger();
        }
        return True;
    }

    virtual int64 GetUsecTime(){
        return counter * timerUsecPeriod;
    }
    
    bool PulseStart() {
        counter = 0;
        return True;
    }
};

#endif

