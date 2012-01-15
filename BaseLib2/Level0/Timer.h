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
 * Multi-platform generic timer implementation
 */
#ifndef _TIMER_H
#define _TIMER_H

#if !defined(_VX5100) && !defined(_VX5500)&& ! defined(_V6X5100) && !defined(_V6X5500) && !defined(_WIN32)
#include <sys/time.h>
#include <signal.h>
#elif defined(_WIN32)
//#include<windows.h>
//#include<mmsystem.h>
//#include <stdio.h>
#pragma comment(lib, "winmm.lib")
#endif
#include "Threads.h"
#include "Sleep.h"
#include "System.h"

#if defined(_VX5100) || defined(_VX5500) || defined(_V6X5100) || defined(_V6X5500)
#define AUX_CLK_INT_VECNUM 32
#endif

#define TIMER_NOT_CONFIGURED 0
#define TIMER_CONFIGURED     1
#define TIMER_RUNNING        2

class Timer;
#if !defined(_VX5100) && !defined(_VX5500) && !defined(_V6X5100) && !defined(_V6X5500)
static Timer *timerThisPtr;
#endif

extern "C" {
#if !defined(_WIN32)
  void  timerServiceRoutine(int sig);
#else
  void CALLBACK timerServiceRoutine(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#endif
  bool  TimerConfigTimer(Timer &t, int32 usec, int32 cpuMask);
  bool  TimerConfigAndStartTimer(Timer &t, int32 usec, int32 cpuMask);
  int64 TimerGetTimerUsecPeriod(Timer &t);
  bool  TimerResetTimer(Timer &t);
  void  TimerInit(Timer &t);
  bool  TimerStartTimer(Timer &t);
  bool  TimerStopTimer(Timer &t);
  void  TimerShowStats(Timer &t);
  void  TimerCServiceRoutine(Timer &t);
}

class Timer { 
  friend bool  TimerConfigTimer(Timer &t, int32 usec, int32 cpuMask);
  friend bool  TimerConfigAndStartTimer(Timer &t, int32 usec, int32 cpuMask);
  friend int64 TimerGetTimerUsecPeriod(Timer &t);
  friend bool  TimerResetTimer(Timer &t);
  friend void  TimerInit(Timer &t);
  friend bool  TimerStartTimer(Timer &t);
  friend bool  TimerStopTimer(Timer &t);
  friend void  TimerShowStats(Timer &t);
  friend void  TimerCServiceRoutine(Timer &t);

public:

#if !defined(_WIN32)
  friend void timerServiceRoutine(int sig);
#else
  friend void CALLBACK timerServiceRoutine(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#endif

private:

  /** Variable containing timer status */
  int32      timerStatus;

#if !defined(_VX5100) && !defined(_VX5500) && !defined(_V6X5100) && !defined(_V6X5500) && !defined(_RTAI) && !defined(_WIN32)
  /** Timer configurations */
  struct itimerval  timeVals; // 
  struct sigaction  sact;     // Struct used to register actions on timer expiration
#endif

#if defined(_RTAI)
  RT_TASK *task;
#endif

#ifdef _WIN32
  MMRESULT        FTimerID;
#endif
  
  /** Statistics */
  int64      timerCounter;
  int64      currentHRTTick;
  int64      oldHRTTick;
  int64      timerMeanTicks;
  int64      timerVarTicks2;
  int64      timerMaxTicks;
  int64      timerMinTicks;
  int64      configPeriodUsec;

public:

  /** Constructor - creates, configures and starts the timer with period usec */
  Timer(int32 usec){
    Init();
    if(!ConfigAndStartTimer(usec)) {
      CStaticAssertErrorCondition(InitialisationError, "Timer::Timer(int32 usec) -> timer setup did not complete successfully");
    }
  };

  /** Constructor - creates the timer */
  Timer(){
    Init();
  };

  /** Initialise timer parameters */
  void                Init(){
    TimerInit(*this);
  }

  /** Initialise statistics parameters */
  void                InitStats(){
    timerCounter     = 0;
    currentHRTTick   = 0;
    oldHRTTick       = 0;
    timerMeanTicks   = 0;
    timerVarTicks2   = 0;
    timerMaxTicks    = 0;
#if defined(_VX5100) || defined(_VX5500) || defined(_V6X5100) || defined(_V6X5500)
    timerMinTicks    = 100*GetTimerUsecPeriod()*HRT::HRTFrequency()/1000000;
#elif defined(_RTAI) || defined(_WIN32)
    timerMinTicks    = 10000000;
#else
    timerMinTicks    = 100*timeVals.it_interval.tv_usec*HRT::HRTFrequency()/1000000;
#endif
  }

  /** Destructor */
  virtual ~Timer(){
    if(IsTimerRunning()) {
      if(!StopTimer()) {
        CStaticAssertErrorCondition(InitialisationError, "Timer::~Timer() -> StopTimer() failed");
      }
    }
#if !defined(_VX5100) && !defined(_VX5500)&& ! defined(_V6X5100) && !defined(_V6X5500)
    timerThisPtr = NULL;
#endif
  }

  /** Configure the timer - this method can be called anytime as long as the object exists */
  bool                ConfigTimer(int32 usec, int32 cpuMask = 0xff){
    return TimerConfigTimer(*this, usec, cpuMask);
  }

  /** Configure and start the timer - this method can be called anytime as long as the object exists and even if a timer is already running */
  bool                ConfigAndStartTimer(int32 usec, int32 cpuMask = 0xff){
    return TimerConfigAndStartTimer(*this, usec, cpuMask);
  }

  /** Starts a timer that has already been configured */
  bool                StartTimer(){
    return TimerStartTimer(*this);
  }

  /** Get current timer period in microseconds */
  int64               GetTimerUsecPeriod(){
    return TimerGetTimerUsecPeriod(*this);
  }

  /** Get value for internal timer counter */
  int64               GetTimerCounter() {return timerCounter;};

  /** Show timer statistics in microseconds */
  void                ShowStats(){
    TimerShowStats(*this);
  }

  /** Stop timer */
  bool                StopTimer(){
    return TimerStopTimer(*this);
  }

  /** Reset timer internal counter and statistics */
  bool                ResetTimer(){
    return TimerResetTimer(*this);
  }

  /** Checks if timer is running */
  bool                IsTimerRunning() {
    if(timerStatus == TIMER_RUNNING) {
      return True;
    }
    return False;
  };

  /** Checks if timer is configured */
  bool                IsTimerConfigured() {
    if(timerStatus == TIMER_CONFIGURED) {
      return True;
    }
    return False;
  };

  /** Collects statistical info and calls the user's timer service routine */
  void                TimerServiceRoutine(){
    TimerCServiceRoutine(*this);
  }

  /** User's timer service routine - is called once every period usec */
  virtual void        UserTimerServiceRoutine() {};

};

#endif

