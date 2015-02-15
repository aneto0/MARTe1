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
 *  Mutex semafore Linux implementation
 */

#ifndef MUTEX_SEM_OS_H
#define MUTEX_SEM_OS_H

#include "../../TimeoutType.h"
#include <pthread.h>
#include <math.h>
#include <sys/timeb.h>

/** Auxiliary class needed to implement the MutexSem class under
    Solaris and Linux. */
class PrivateMutexSemStruct{
    /**  Mutex Handle */
    pthread_mutex_t       mutexHandle;
    /** Mutex Attributes */
    pthread_mutexattr_t   mutexAttributes;
public:
    /** */
    PrivateMutexSemStruct(){}
    /** */
    ~PrivateMutexSemStruct(){}

    /** */
    bool Init(){
        if(pthread_mutexattr_init(&mutexAttributes) != 0)                              return False;
        if(pthread_mutexattr_setprotocol(&mutexAttributes, PTHREAD_PRIO_INHERIT) != 0) return False;
        if(pthread_mutexattr_settype(&mutexAttributes,PTHREAD_MUTEX_RECURSIVE)!=0)     return False;
        if(pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0)                       return False;
        return True;
        }

    /** */
    bool Close(){
        if(!pthread_mutexattr_destroy(&mutexAttributes))                          return False;
        if(!pthread_mutex_destroy(&mutexHandle))                                  return False;
        return True;
    }

    /** */
    bool Lock(TimeoutType msecTimeout = TTInfiniteWait){
        if(msecTimeout == TTInfiniteWait){
            if(pthread_mutex_lock(&mutexHandle) != 0)                 return False;
        }else{
            struct timespec timesValues;
            timeb tb;
            ftime( &tb );
            double sec        = ((msecTimeout.msecTimeout + tb.millitm)*1e-3 + tb.time);
            double roundValue = floor(sec);
            timesValues.tv_sec  = (int)roundValue;
            timesValues.tv_nsec = (int)((sec-roundValue)*1E9);
            int err = 0;
            if((err = pthread_mutex_timedlock(&mutexHandle, &timesValues)) != 0){
                return False;
            }
        } 
        return True;
    }

    /** */
    bool UnLock(){
        return (pthread_mutex_unlock(&mutexHandle)==0);
    }

    /** */
    bool TryLock(){
        return (pthread_mutex_trylock(&mutexHandle)==0);
    }


};

/** open the semafore with a given initial state */
static bool MutexSemOSCreate(HANDLE &semH, bool locked){
    if(semH != (HANDLE)NULL){
        delete (PrivateMutexSemStruct *)semH;
    }
    // Create the Structure
    semH = (HANDLE) new PrivateMutexSemStruct();
    if(semH == (HANDLE)NULL){
        return False;
    }
    // Initialize the Semaphore
    bool ret = ((PrivateMutexSemStruct *)semH)->Init();
    if(!ret){
        delete (PrivateMutexSemStruct *)semH;
        semH = (HANDLE)NULL;
        return False;
    }
    if(locked == True){
        ((PrivateMutexSemStruct *)semH)->Lock(TTInfiniteWait);
    }

    return True;
}

/** close the semafore handle */
static inline bool MutexSemOSClose(HANDLE &semH){
    if (semH==(HANDLE)NULL){
        return True;
    }
    semH=(HANDLE)NULL;
    return True;
}

/** grab the semafore */
static inline bool MutexSemOSLock(HANDLE &semH, TimeoutType msecTimeout){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return ((PrivateMutexSemStruct *)semH)->Lock(msecTimeout);
}

/** returns the ownership */
static inline bool MutexSemOSUnLock(HANDLE &semH){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return True;
}

/** locks without wasting time */
static inline bool MutexSemOSFastLock(HANDLE &semH, TimeoutType msecTimeout){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return ((PrivateMutexSemStruct *)semH)->Lock(msecTimeout);
}


/** unlock semafore fast */
static inline bool MutexSemOSFastUnLock(HANDLE &semH){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return ((PrivateMutexSemStruct *)semH)->UnLock();
}

/** just try to lock it returning immediately */
static inline bool MutexSemOSFastTryLock(HANDLE &semH){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return ((PrivateMutexSemStruct *)semH)->TryLock();
}

#endif

