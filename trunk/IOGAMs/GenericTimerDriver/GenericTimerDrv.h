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

#if !defined(_GENERICTIMER_DRV)
#define _GENERICTIMER_DRV

#include "GenericAcqModule.h"
#include "Timer.h"

OBJECT_DLL(GenericTimerDrv)

/** Generic Timer Module Class */
class GenericTimerDrv : public GenericAcqModule , public Timer {

    OBJECT_DLL_STUFF(GenericTimerDrv)

public:
    
    /** Object Load Setup */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
    
    /** Object Description */
    bool ObjectDescription(StreamInterface &s,bool full = False,StreamInterface *err = NULL);
    
    /** Get Data */
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);

    /** Write Data - not supported by this driver */
    bool WriteData(uint32 usecTime, const int32 *buffer){return False;}

    /** Interrupt Service Routine */
    void UserTimerServiceRoutine();

    /** Set board used as input */
    virtual bool SetInputBoardInUse(bool on = True) {
      if(inputBoardInUse && on){
	AssertErrorCondition(InitialisationError, "GenericTimerDrv::SetInputBoardInUse: Board %s is already in use", Name());
	return False;
      }
      
      inputBoardInUse  = on;
      return True;
    }
    
    virtual bool SetOutputBoardInUse(bool on = True){
      AssertErrorCondition(InitialisationError, "GenericTimerDrv::SetOutputBoardInUse: Board %s cannot be used as output", Name());
      return False;
    }

    /** Get Time as Useconds */
    int64 GetUsecTime() {
      return (GetTimerCounter()*GetTimerUsecPeriod());
    }
    
    bool PulseStart() {
      if(!ResetTimer()) {
	AssertErrorCondition(FatalError, "GenericTimerDrv::PulseStart() unable to ResetTimer()");
	return False;
      }
      
      return True;
    }
};

#endif
