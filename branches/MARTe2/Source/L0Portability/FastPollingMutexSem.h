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
 * $Id: FastPollingMutexSem.h 3 2012-01-15 16:26:07Z aneto $
 *
 **/

/**
 * @file
 * A semaphore based on spin locks.
 */
#ifndef FAST_POLLING_MUTEX_SEM
#define FAST_POLLING_MUTEX_SEM

#include "GeneralDefinitions.h"
#include "Atomic.h"
#include "HighResolutionTimer.h"
#include "TimeoutType.h"
#include "Sleep.h"

/** using just a single instruction: A SPINLOCK. Uses Atomic::TestAndSet. The Timeout is calculated using HRT */
class FastPollingMutexSem {
protected:
    volatile int32 flag;
public:
    /** The constructor */
    FastPollingMutexSem() {
        flag = 0;
    }

    /** Initializes the semaphore and readies it. */
    bool Create(bool locked = False) {
        if (locked == True)
            flag = 1;
        else
            flag = 0;
        return True;
    }

    /** Undo semphore initialization. */
    bool Close() {
        return True;
    }

    /** returns the status of the semaphore */
    inline bool Locked() {
        return flag == 1;
    }

    /** Locks Mux returns False if locking failed. Waits polling at a max rate of a msec
     Timeout is the minimum Timeout. The actual time can be much bigger    */
    inline bool FastLock(TimeoutType msecTimeout = TTInfiniteWait) {
        int64 ticksStop = msecTimeout.HighResolutionTimerTicks()();
        ticksStop += HighResolutionTimer::Counter()();
        while (!Atomic::TestAndSet((int32 *) &flag)) {
            if (msecTimeout != TTInfiniteWait) {
                int64 ticks = HighResolutionTimer::Counter();
                if (ticks > ticksStop)
                    return False;
            }
            // yield CPU
            SleepMSec(1);
        }
        return True;
    }

    /** Tries without waiting. */
    inline bool FastTryLock() {
        return (Atomic::TestAndSet((int32 *) &flag));
    }

    /** Unlocks mux. */
    inline bool FastUnLock(void) {
        flag = 0;
        return True;
    }
};

#endif

