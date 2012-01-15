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

#include "Timer.h"

void TimerCServiceRoutine(Timer &t) {

#if !defined(_RTAI)
  /** Collect statistical data */
  t.currentHRTTick = HRT::HRTCounter();
  if(t.timerCounter > 0) {
    int64 diff     = t.currentHRTTick - t.oldHRTTick;    
    t.timerMeanTicks = ((t.timerCounter-1)*t.timerMeanTicks + diff)/t.timerCounter;
    int64 newDiff  = diff - t.timerMeanTicks;
    t.timerVarTicks2 = ((t.timerCounter-1)*t.timerVarTicks2 + newDiff*newDiff)/t.timerCounter;
    if(diff > t.timerMaxTicks) {
      t.timerMaxTicks = diff;
    }
    if(diff < t.timerMinTicks) {
      t.timerMinTicks = diff;
    }
  }
  t.oldHRTTick = t.currentHRTTick;
#endif  
  t.timerCounter++;
  /** Call user defined signal handler routine */
  t.UserTimerServiceRoutine();
//  printf("t.timerCounter = %lld\n", t.timerCounter);
}

#if defined(_WIN32)
void CALLBACK timerServiceRoutine(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2){
	Timer *timer = (Timer*)dwUser;
	if( timer == NULL ) return;
	timer->TimerServiceRoutine();
}
#else
void timerServiceRoutine(int sig) {
#if defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100) || defined(_V6X5500)
  Timer *timerPtr = (Timer *)sig;
  timerPtr->TimerServiceRoutine();
#elif defined(_RTAI)
  while(timerThisPtr->IsTimerRunning()){
    timerThisPtr->TimerServiceRoutine();
    rt_task_wait_period();
  }
#else
  if(sig != SIGALRM) {
    CStaticAssertErrorCondition(Warning, "Timer class -> timerServiceRoutine(): received signal different from SIGALRM");
  } else {
    timerThisPtr->TimerServiceRoutine();
  }
#endif
}
#endif

void TimerInit(Timer &t) {
  t.configPeriodUsec = 0;
#if !defined(_VX5100) && !defined(_VX5500) && !defined(_V6X5100) && !defined(_V6X5500)
  timerThisPtr = &t;
#endif
#if defined(_RTAI)
  t.task = (RT_TASK *)malloc(RT_TASK_STRUCT_SIZE);
  rt_task_init(t.task, (void (*)(long int))timerServiceRoutine, 0, 32768, 0, 1, NULL);
#endif
  t.timerStatus  = TIMER_NOT_CONFIGURED;
}

bool TimerConfigTimer(Timer &t, int32 usec, int32 cpuMask) {
#if defined(_VX5100) || defined(_VX5500) || defined(_V6X5100) || defined(_V6X5500)
  if(sysAuxClkConnect((FUNCPTR)(timerServiceRoutine), (int)&t) == ERROR) {
    CStaticAssertErrorCondition(InitialisationError,"Timer::ConfigTimer(): Unable register signal handler");
  }
  if(sysAuxClkRateSet((int32)(1000000/usec)) == ERROR) {
    CStaticAssertErrorCondition(InitialisationError,"Timer::ConfigTimer(): Unable to set timer frequency");
  }
#elif defined(_RTAI)
  t.configPeriodUsec = usec;
  rt_set_runnable_on_cpus(t.task, cpuMask);
#elif defined(_WIN32)
	t.configPeriodUsec = usec;
#else
  /* Setup Signal Handler */
  sigemptyset(&t.sact.sa_mask);
  t.sact.sa_flags = 0;
  t.sact.sa_handler = timerServiceRoutine;
  if(sigaction(SIGALRM, &t.sact, NULL) == -1) {
    CStaticAssertErrorCondition(InitialisationError,"Timer::ConfigTimer(): Unable register signal handler");
    return False;
  }

  /* Configure Timer Values */
  t.timeVals.it_interval.tv_sec  = 0;
  t.timeVals.it_interval.tv_usec = usec;
  t.timeVals.it_value.tv_sec     = 0;
  t.timeVals.it_value.tv_usec    = usec;
#endif

  /** Set timer status */
  t.timerStatus = TIMER_CONFIGURED;
  
  return True;
}

bool TimerStartTimer(Timer &t) {
  // Check if timer has already been configured or is already running
  if(!t.IsTimerConfigured() && !t.IsTimerRunning()) {
    CStaticAssertErrorCondition(InitialisationError,"Timer::StartTimer(): Timer not yet configured");
    return False;
  }

  /** Reset statistic variables */
  t.InitStats();

#if defined(_VX5100) || defined(_VX5500) || defined(_V6X5100) || defined(_V6X5500)
  if(t.IsTimerRunning()) {
    CStaticAssertErrorCondition(InitialisationError,"Timer::StartTimer(): Timer is already running, stop it, configure it and then start it");
    return False;
  } else {
    sysAuxClkEnable();
  }
#elif defined (_RTAI)
  int32 periodTicks = nano2count(t.configPeriodUsec * 1000LL);
  rt_task_make_periodic(t.task, periodTicks, periodTicks);
#elif defined (_WIN32)
  timeBeginPeriod(t.configPeriodUsec/1000);
  t.FTimerID = timeSetEvent(t.configPeriodUsec/1000, t.configPeriodUsec/1000, timerServiceRoutine, (unsigned int)&t, TIME_PERIODIC);
  if( t.FTimerID == NULL ){
    CStaticAssertErrorCondition(InitialisationError,"TimerDrv: StartTimer(): Unable to start timer");
    return False;
  }
#else
  // Start Timer
  if(setitimer(ITIMER_REAL, &t.timeVals, NULL) == -1) {
    CStaticAssertErrorCondition(InitialisationError,"Timer::StartTimer(): Unable to start timer");
    return False;
  }
#endif

  /** Set timer status */
  t.timerStatus = TIMER_RUNNING;

  return True;
}

bool TimerConfigAndStartTimer(Timer &t, int32 usec, int32 cpuMask) {

  if(!t.ConfigTimer(usec, cpuMask)) {
    CStaticAssertErrorCondition(InitialisationError, "Timer::ConfigAndStartTimer(int32 usec) -> ConfigTimer() failed");
    return False;
  }
  if(!t.StartTimer()) {
    CStaticAssertErrorCondition(InitialisationError, "Timer::ConfigAndStartTimer(int32 usec) -> StartTimer() failed");
    return False;
  }
  return True;
}

bool TimerStopTimer(Timer &t) {
#if defined(_VX5100) || defined(_VX5500) || defined(_V6X5100) || defined(_V6X5500)
  sysAuxClkDisable();
  jetIntDisconnect(INUM_TO_IVEC(AUX_CLK_INT_VECNUM),(VOIDFUNCPTR)timerServiceRoutine,(int)&t);
#elif defined(_RTAI)
  rt_task_suspend(t.task);
#elif defined(_WIN32)
  if( t.FTimerID != NULL ){
    timeKillEvent(t.FTimerID);
  }
  timeEndPeriod(t.configPeriodUsec/1000);
#else
  if(!t.ConfigAndStartTimer(0)) {
    CStaticAssertErrorCondition(InitialisationError, "Timer::StopTimer() -> ConfigAndStartTimer() failed");
  }
#endif
  
  /** Set timer status */
  t.timerStatus = TIMER_NOT_CONFIGURED;
  
  return True;
}

bool TimerResetTimer(Timer &t) {
  if(!t.IsTimerRunning()) {
    CStaticAssertErrorCondition(FatalError, "Timer::ResetTimer() -> timer not yet running");
    return False;
  }
  
  t.InitStats();
  
  return True;
}

int64 TimerGetTimerUsecPeriod(Timer &t) {
#if defined(_VX5100) || defined(_VX5500) || defined(_V6X5100) || defined(_V6X5500)
  return((int64)(1000000/sysAuxClkRateGet()));
#elif defined(_RTAI) || defined(_WIN32)
  return t.configPeriodUsec;
#else
  if(!t.IsTimerRunning()) {
    CStaticAssertErrorCondition(InitialisationError, "Timer::GetTimerUsecPeriod() -> Timer not running");
  }

  itimerval value;
  if(getitimer(ITIMER_REAL, &value) != 0) {
    CStaticAssertErrorCondition(FatalError, "Timer::GetTimerUsecPeriod() -> unable to get timer period");
    return -1;
  } else {
    return ((int64)(value.it_interval.tv_usec));
  }
#endif
}

void TimerShowStats(Timer &t) {
#if defined(_RTAI)
  printf("Not supported in RTAI\n");
#else
  printf("Timer Counter                  = %lld\n", t.GetTimerCounter());
  printf("Timer mean cycle time          = %f (usecs)\n", t.timerMeanTicks*HRT::HRTPeriod()*1e6);
  printf("Timer std deviation cycle time = %f (usecs)\n", sqrt(t.timerVarTicks2)*HRT::HRTPeriod()*1e6);
  printf("Timer max cycle time           = %f (usecs)\n", t.timerMaxTicks*HRT::HRTPeriod()*1e6);
  printf("Timer min cycle time           = %f (usecs)\n", t.timerMinTicks*HRT::HRTPeriod()*1e6);
#endif
}

