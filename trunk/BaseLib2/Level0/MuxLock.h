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
 * Simplifies the usage of a MutexSem. 
 * It ensures that at every exit point of a function the mux is Unlocked.
 */
#ifndef MUXLOCK
#define MUXLOCK

#include "MutexSem.h"
#include "System.h"
#include "ErrorManagement.h"

/** use in conjunction with a MutexSem.
    Guarantees mux unlocking upon exiting the scope */
class MuxLock{

    /** */
    MutexSem *mux;

public:
    /** */
    MuxLock(){
        mux = NULL;
    }

    /** unlocks! */
    ~MuxLock(){
        UnLock();
    }

    /** use this to lock your sem !*/
    bool Lock(MutexSem &sem,TimeoutType msecTimeout = TTInfiniteWait){
        UnLock();
        if (sem.Lock(msecTimeout)){
            this->mux = &sem;
            return True;
        }
        return False;
    }

    /** once used the reference is lost */
    void UnLock(){
        if (mux) mux->UnLock();
        mux = NULL;
    }

};

#endif

