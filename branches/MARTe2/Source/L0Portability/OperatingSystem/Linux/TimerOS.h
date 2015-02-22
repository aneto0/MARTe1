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
 * Linux timer implementation based on signals.
 */
#include <sys/time.h>
#include <signal.h>

static Timer *timerThisPtr = NULL;
void TimerOSTimerServiceRoutine(int sig) {
    if (sig == SIGALRM) {
        timerThisPtr->TimerServiceRoutine();
    }
}

void TimerOSInit(Timer &t) {
    timerThisPtr = &t;
}

bool TimerOSConfigTimer(Timer &t, int32 usec, int32 cpuMask) {
    struct sigaction sact;
    /* Setup Signal Handler */
    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = TimerOSTimerServiceRoutine;
    if (sigaction(SIGALRM, &sact, NULL) == -1) {
        //CStaticAssertErrorCondition(InitialisationError, "Timer::ConfigTimer(): Unable register signal handler");
        return False;
    }

    return True;
}

bool TimerOSStartTimer(Timer &t) {
    struct itimerval timeVals;
    /* Configure Timer Values */
    timeVals.it_interval.tv_sec = t.timerUsecPeriod / 1000000;
    timeVals.it_interval.tv_usec = t.timerUsecPeriod % 1000000;
    timeVals.it_value.tv_sec = t.timerUsecPeriod / 1000000;
    timeVals.it_value.tv_usec = t.timerUsecPeriod % 1000000;

    // Start Timer
    if (setitimer(ITIMER_REAL, &timeVals, NULL) == -1) {
        //CStaticAssertErrorCondition(InitialisationError, "Timer::StartTimer(): Unable to start timer");
        return False;
    }

    return True;
}

bool TimerOSStopTimer(Timer &t) {
    if (!t.ConfigAndStartTimer(0)) {
        //CStaticAssertErrorCondition(InitialisationError, "Timer::StopTimer() -> ConfigAndStartTimer() failed");
        return False;
    }

    return True;
}
