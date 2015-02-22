/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/
/**
 * @file
 * Multi-platform generic timer implementation
 */
#ifndef TIMER_H
#define TIMER_H

#include "Threads.h"
#include "Sleep.h"

class Timer;

extern "C" {
bool TimerConfigTimer(Timer &t, int32 usec, int32 cpuMask);
bool TimerConfigAndStartTimer(Timer &t, int32 usec, int32 cpuMask);
int64 TimerGetTimerUsecPeriod(Timer &t);
bool TimerResetTimer(Timer &t);
void TimerInit(Timer &t);
bool TimerStartTimer(Timer &t);
bool TimerStopTimer(Timer &t);
void TimerCServiceRoutine(Timer &t);
}

class Timer {
private:
    /** Variable containing timer status */
    int32 timerStatus;

    /** Friend functions*/
    friend bool TimerConfigTimer(Timer &t, int32 usec, int32 cpuMask);
    friend bool TimerConfigAndStartTimer(Timer &t, int32 usec, int32 cpuMask);
    friend int64 TimerGetTimerUsecPeriod(Timer &t);
    friend bool TimerResetTimer(Timer &t);
    friend void TimerInit(Timer &t);
    friend bool TimerStartTimer(Timer &t);
    friend bool TimerStopTimer(Timer &t);
    friend void TimerCServiceRoutine(Timer &t);

public:
    /** Period set for the timer */
    int64 timerUsecPeriod;

    /** Statistics */
    int64 timerCounter;
    int64 currentHRTTick;
    int64 oldHRTTick;

    static int32 NOT_CONFIGURED;
    static int32 CONFIGURED;
    static int32 RUNNING;

public:

    /** Constructor - creates, configures and starts the timer with period usec */
    Timer(int32 usec) {
        Init();
        if (!ConfigAndStartTimer(usec)) {
            //CStaticAssertErrorCondition(InitialisationError, "Timer::Timer(int32 usec) -> timer setup did not complete successfully");
        }
    }

    /** Constructor - creates the timer */
    Timer() {
        Init();
    }

    /** Initialise timer parameters */
    void Init() {
        TimerInit(*this);
    }

    /** Initialise statistics parameters */
    void InitStats() {
        timerCounter = 0;
        currentHRTTick = 0;
        oldHRTTick = 0;
    }

    /** Destructor */
    virtual ~Timer() {
        if (timerStatus == RUNNING) {
            if (!StopTimer()) {
                //CStaticAssertErrorCondition(InitialisationError, "Timer::~Timer() -> StopTimer() failed");
            }
        }
    }

    /** Configure the timer - this method can be called anytime as long as the object exists */
    bool ConfigTimer(int32 usec, int32 cpuMask = 0xff) {
        return TimerConfigTimer(*this, usec, cpuMask);
    }

    /** Configure and start the timer - this method can be called anytime as long as the object exists and even if a timer is already running */
    bool ConfigAndStartTimer(int32 usec, int32 cpuMask = 0xff) {
        return TimerConfigAndStartTimer(*this, usec, cpuMask);
    }

    /** Starts a timer that has already been configured */
    bool StartTimer() {
        return TimerStartTimer(*this);
    }

    /** Get current timer period in microseconds */
    int64 GetTimerUsecPeriod() {
        return TimerGetTimerUsecPeriod(*this);
    }

    /** Get value for internal timer counter */
    int64 GetTimerCounter() {
        return timerCounter;
    }

    /** Stop timer */
    bool StopTimer() {
        return TimerStopTimer(*this);
    }

    /** Reset timer internal counter and statistics */
    bool ResetTimer() {
        return TimerResetTimer(*this);
    }

    /**
     * @return the Timer status
     */
    int32 TimerStatus() {
        return timerStatus;
    }

    /** Collects statistical info and calls the user's timer service routine */
    void TimerServiceRoutine() {
        TimerCServiceRoutine(*this);
    }

    /** User's timer service routine - is called once every period usec */
    virtual void UserTimerServiceRoutine() {
    }
};

#endif

