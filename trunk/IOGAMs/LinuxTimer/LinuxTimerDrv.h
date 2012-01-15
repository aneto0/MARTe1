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


#if !defined (_LINUXTIMER_DRV)
#define _LINUXTIMER_DRV

#include "System.h"
#include "GenericAcqModule.h"
#include "FString.h"
#include "File.h"

enum LinuxTimerSleepNature {
  Default  = 0,
  Busy     = 1,
  SemiBusy = 2
};

OBJECT_DLL(LinuxTimerDrv)

/// Timer Module Class
class LinuxTimerDrv:public GenericAcqModule{
    OBJECT_DLL_STUFF(LinuxTimerDrv)

private:
    int64           localTime;
  
    public:
    // Sem which signals correct start/end of the chardev-reader thread
    EventSem StartStopSem;
    FastPollingMutexSem pollSynchSem;
    // When set to false the reader thread should die
    bool keepAlive;
    // reader thread ID
    TID tid;
    // Timer frequency in microsecs
    int32 timerPeriodUsec;
    // Flag to control whether or not to use SleepBusy
    LinuxTimerSleepNature busy;
    // Used only in the SemiBusy case
    int32 nonBusySleepPeriodUsec;
    //
    double timeCorrection;

    /** Constructor */
    LinuxTimerDrv();

    /** Deconstructor */
    virtual ~LinuxTimerDrv();

    // Object Load Setup
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    // Object Description
    virtual bool ObjectDescription(StreamInterface &s,bool full = False, StreamInterface *err=NULL);

    // Get Data
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);

    /** Update the output of the module using n = numberOfOutputChannels data word of the source buffer starting from absoluteOutputPosition */
    bool WriteData(uint32 usecTime, const int32 *buffer){return True;};

    /** Interrupt Service Routine */
    void InterruptServiceRoutine();

    /** Configured to be used by a DataPollingDrivenTTS */
    bool usePolling;

    /** Get Time as Useconds */
    int64 GetUsecTime(){
        return localTime*timerPeriodUsec;
    }

    /** Set board used as input */
    virtual bool SetInputBoardInUse( bool on = True){

        if( inputBoardInUse && on){
            AssertErrorCondition(InitialisationError, "LinuxTimerDrv::SetInputBoardInUse: Board %s is already in use", Name());
            return False;
        }

        inputBoardInUse  = on;
        return True;
    }

    virtual bool SetOutputBoardInUse(bool on = True){

        if(outputBoardInUse && on){
            AssertErrorCondition(InitialisationError, "LinuxTimerDrv::SetOutputBoardInUse: Board %s is already in use", Name());
            return False;
        }

        outputBoardInUse = on;
        return True;
    }

    virtual bool Poll();
public:
    bool PulseStart(){
        localTime = 0;
        return True;
    }

};


#endif
