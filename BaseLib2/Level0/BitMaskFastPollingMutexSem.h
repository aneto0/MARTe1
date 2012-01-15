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
 * a set of 32 bit memory polling semaphores that can be set and reset using masks 
 */
#ifndef BITMASK_FAST_POLLING_MUTEXSEM
#define BITMASK_FAST_POLLING_MUTEXSEM

#include "FastPollingMutexSem.h"
#include "Atomic.h"

class BitMaskFastPollingMutexSem: protected FastPollingMutexSem {
    /** the sempahore status */
    uint32 mask;

public:
    /** Initializes the semaphore and readies it */
    bool            Create(uint32 mask=0)
    {
        FastPollingMutexSem::Create();
        this->mask = mask;
        return True;
    }

    /** Undo semphore initialization */
    bool            Close(){ return True; }

    /** returns the status of the semaphore */
    inline uint32   Locked() { return mask; }

    /** Locks Mux returns False if locking failed. waits polling at a max rate of a msec
        msecTimeout is the minimum value. The actual time can be much bigger    */
    inline bool     FastLock(uint32 mask,TimeoutType msecTimeout = TTInfiniteWait){
        while (msecTimeout != TTNoWait){
            while (!Atomic::TestAndSet((int32 *)&flag)){
                SleepMsec(1);
            }
            if ((this->mask & mask)==0){
                this->mask |= mask;
                flag = 0;
                return True;
            }
            flag = 0;
            SleepMsec(1);
            if (msecTimeout != TTInfiniteWait) msecTimeout -= 1;
        }
        return False;
    }

    /** Unlocks mux: returns False if it wasn't locked */
    inline bool FastUnLock(uint32 mask){
        while(!Atomic::TestAndSet((int32 *)&flag))SleepMsec(1);
        this->mask &= ~mask;
        flag = 0;
        return True;
    }

};

#endif
