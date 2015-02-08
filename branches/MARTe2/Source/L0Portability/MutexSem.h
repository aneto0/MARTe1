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
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 *  @file 
 *  Mutex semafore
 */
#ifndef MUTEX_SEM
#define MUTEX_SEM

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,MutexSemOS.h)
#include "SemCore.h"

/** a mutual exclusion semaphore */
class MutexSem : public SemCore {

public:
    /** constructor */
    MutexSem(HANDLE h){
        Init(h);
    }

    /** default constructor */
    MutexSem(){
    }

    /** destructor */
    ~MutexSem(){
        Close();
    }

    /** open the semafore with a given initial state */
    bool Create(bool locked=False){
        return MutexSemCreate(semH, locked);
    }

    /** close the semafore handle */
    bool Close(){
        return MutexSemClose(semH);
    }

    /** grab the semafore */
    bool Lock(TimeoutType msecTimeout = TTInfiniteWait){
        return MutexSemLock(semH, msecTimeout);
    }

    /** returns the ownership */
    bool UnLock(){
        return MutexSemUnLock(semH);
    }

    /** locks without wasting time */
    inline bool FastLock(TimeoutType msecTimeout = TTInfiniteWait){
        return MutexSemFastLock(semH, msecTimeout);
    }

    /** unlock semafore fast */
    inline bool FastUnLock(){
        return MutexSemFastUnLock(semH);
    }

    /** just try to lock it returning immediately */
    inline bool FastTryLock(){
        return MutexSemFastTryLock(semH);
    }
};
#endif

