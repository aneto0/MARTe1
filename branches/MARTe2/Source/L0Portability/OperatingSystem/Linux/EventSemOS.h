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
 * @file 
 * Linux implementation of the EventSem
 */
#ifndef EVENT_SEM_OS_H
#define EVENT_SEM_OS_H

#include "../../TimeoutType.h"
#include <pthread.h>
#include <math.h>
#include <sys/timeb.h>

/** Private structure used for solaris adn linux when using pThread. */
class PrivateEventSemStruct{
    // Mutex Handle
    pthread_mutex_t       mutexHandle;

    // Mutex Attributes
    pthread_mutexattr_t   mutexAttributes;

    // Conditional Variable
    pthread_cond_t        eventVariable;

    // boolean semaphore
    bool                  stop;

public:
    //
    PrivateEventSemStruct(){
        stop = True;
    }
    //
    ~PrivateEventSemStruct(){}

    bool Init(){
        stop = True;
        if (pthread_mutexattr_init(&mutexAttributes) != 0){
            return False;
        }
        if (pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0){
            return False;
        }
        if (pthread_cond_init(&eventVariable,NULL)!=0){
            return False;
        }
        return True;
    }

    bool Close(){
        Post();
        if(!pthread_mutexattr_destroy(&mutexAttributes)){
            return False;
        }
        if(pthread_mutex_destroy(&mutexHandle)  != 0){
            return False;
        }
        if(pthread_cond_destroy(&eventVariable) != 0){
            return False;
        }
        return True;
    }

    bool Wait(TimeoutType msecTimeout = TTInfiniteWait){
        if(msecTimeout == TTInfiniteWait){
            if(pthread_mutex_lock(&mutexHandle) != 0){
                return False;
            }
	        if(stop == True){
                if(pthread_cond_wait(&eventVariable,&mutexHandle) != 0){
                     pthread_mutex_unlock(&mutexHandle);
                     return False;
                }
	        }
            if(pthread_mutex_unlock(&mutexHandle) != 0){
                return False;
            }
        }
        else{
            struct timespec timesValues;
            timeb tb;
            ftime(&tb);
            
            double sec = ((msecTimeout.msecTimeout + tb.millitm)*1e-3 + tb.time);
            
            double roundValue = floor(sec);
            timesValues.tv_sec  = (int)roundValue;
            timesValues.tv_nsec = (int)((sec-roundValue)*1E9);
            if(pthread_mutex_timedlock(&mutexHandle, &timesValues) != 0){
                return False;
            }
	        if(stop == True){

                if(pthread_cond_timedwait(&eventVariable,&mutexHandle,&timesValues) != 0){
                    pthread_mutex_unlock(&mutexHandle);
                    return False;
                }
	        }
            if(pthread_mutex_unlock(&mutexHandle) != 0){ 
                return False;
            }
        } 
        return True;
    }

    bool Post(){
        if(pthread_mutex_lock(&mutexHandle) != 0){
            return False;
        }
        stop = False;
        if(pthread_mutex_unlock(&mutexHandle) != 0){
              return False;
        }
        return (pthread_cond_broadcast(&eventVariable) == 0);
    }

    bool Reset(){
        if(pthread_mutex_lock(&mutexHandle) != 0){
            return False;
        }
        stop = True;
        if(pthread_mutex_unlock(&mutexHandle) != 0){
            return False;
        }
        return stop;
    }
};

/** 
 * @see EventSem::Create
 */
static bool EventSemCreate(HANDLE &semH){
    if(semH != (HANDLE)NULL){
        delete (PrivateEventSemStruct *)semH;
    }
    semH = (HANDLE) new PrivateEventSemStruct;
    if(semH == (HANDLE)NULL) {
        return False;
    }
    // Initialize the Semaphore
    bool ret = ((PrivateEventSemStruct *)semH)->Init();
    if(!ret){
        delete (PrivateEventSemStruct *)semH;
        semH = (HANDLE)NULL;
        return False;
    }
    return True;
}

/**
 * @see EventSem::Close
 */
static bool EventSemClose(HANDLE &semH){
    if(semH == (HANDLE)NULL){
        return True;
    }
    bool ret = ((PrivateEventSemStruct *)semH)->Close();
    delete (PrivateEventSemStruct *)semH;
    semH = (HANDLE)NULL;
    return ret;
}

/** 
 * @see EventSem::Wait
 */
static inline bool EventSemWait(HANDLE &semH, TimeoutType msecTimeout = TTInfiniteWait){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return ((PrivateEventSemStruct *)semH)->Wait();
}

/**
 * @see EventSem::Post
 */
static inline bool EventSemPost(HANDLE &semH){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return (((PrivateEventSemStruct *)semH)->Post());
}

/** 
 * @see EventSem::Reset
 */ 
static inline bool EventSemReset(HANDLE &semH){
    if(semH == (HANDLE)NULL){
        return False;
    }
    return ((PrivateEventSemStruct *)semH)->Reset();
}

/** 
 * @see EventSem::ResetWait
 */
static inline bool EventSemResetWait(HANDLE &semH, TimeoutType msecTimeout = TTInfiniteWait){
    EventSemReset(semH);
    return EventSemWait(semH, msecTimeout);
}

#endif

