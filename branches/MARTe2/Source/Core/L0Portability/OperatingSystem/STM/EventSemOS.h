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
 * @brief Linux implementation of the event semaphore
 */
#ifndef EVENT_SEM_OS_H
#define EVENT_SEM_OS_H

#include "../../TimeoutType.h"
#include "../../GeneralDefinitions.h"
#include "cmsis_os.h" 

/** @brief Private event semaphore class used for Solaris and Linux when using pThread.
 *  
 *  The event semaphore is impemented using pthread_cond functions, but this class
 *  use also a pthread_mutex to assure consistency of critical operations on the event semaphore
 *  shared by threads. */
class PrivateEventSemStruct {
	
	osSemaphoreId eventVariable;

public:
    /** @brief Constructor. */
    PrivateEventSemStruct() {
    }
    /** @brief Destructor. */
    ~PrivateEventSemStruct() {
    }

    /** @brief Initialize the semaphore with the right attributes.
      * @return false if something wrong with pthread_mutex and pthread_cond initializations. */
    bool Init() {

	const osSemaphoreDef_t semaphoreDefinition = {0};
	eventVariable=osSemaphoreCreate(&semaphoreDefinition, 1);

	return eventVariable!=NULL;	    
    }


    /** @brief Destroy the semaphore.
      * @return false if something wrong in pthread_mutex and pthread_cond destructions. */
    bool Close() {
	if(osSemaphoreRelease(eventVariable)!=0){
		return False;
	}
	
	return osSemaphoreDelete(eventVariable)==0;
	
    }


    /** @brief Wait until a post condition or until the timeout expire.
      * @param msecTimeout is the desired timeout.
      * @return false if lock or wait functions fail or if the timeout causes the wait fail. */
    bool Wait(TimeoutType msecTimeout = TTInfiniteWait) {

	if(msecTimeout==TTInfiniteWait){
		//returns the numbers of avaiable tokens. Since we have only one it should return 1.
		return osSemaphoreWait(eventVariable, osWaitForever)==1;
	}
	else{
		return osSemaphoreWait(eventVariable, msecTimeout.msecTimeout)==1;
	}

   }


    /** @brief Post condition. Free all threads stopped in a wait condition.
      * @return true if the eventVariable is set to zero. */
    bool Post() {
	return osSemaphoreRelease(eventVariable)==0;
    }

    /** @brief Reset the semaphore for a new possible wait condition.
      * @return false if the mutex lock fails. */
    bool Reset() {
	return Post();
    }
};

/** 
 * @see EventSem::Create
 * @brief Create a new PrivateEventSemStruct. 
 * @param semH is a pointer to the new PrivateEventSemStruct in return.
 * @return false if the new or the Init functions fail, true otherwise.
 */
static bool EventSemCreate(HANDLE &semH) {
    if (semH != (HANDLE) NULL) {
        delete (PrivateEventSemStruct *) semH;
    }
    semH = (HANDLE) new PrivateEventSemStruct;
    if (semH == (HANDLE) NULL) {
        return False;
    }
    // Initialize the Semaphore
    bool ret = ((PrivateEventSemStruct *) semH)->Init();
    if (!ret) {
        delete (PrivateEventSemStruct *) semH;
        semH = (HANDLE) NULL;
        return False;
    }
    return True;
}

/**
 * @see EventSem::Close.
 * @brief Destroy the event semaphore.
 * @param semH is the pointer to the semaphore.
 * @return true if the Close function has success, false otherwise.
 */
static bool EventSemClose(HANDLE &semH) {
    if (semH == (HANDLE) NULL) {
        return True;
    }
    bool ret = ((PrivateEventSemStruct *) semH)->Close();
    delete (PrivateEventSemStruct *) semH;
    semH = (HANDLE) NULL;
    return ret;
}

/** 
 * @see EventSem::Wait
 * @brief Wait condition.
 * @param semH is a pointer to the event semaphore.
 * @param msecTimeout is the desired timeout.
 * @return the result of PrivateEventSemStruct::Wait.
 */
static inline bool EventSemWait(HANDLE &semH, TimeoutType msecTimeout) {
    if (semH == (HANDLE) NULL) {
        return False;
    }
    return ((PrivateEventSemStruct *) semH)->Wait(msecTimeout);
}

/**
 * @see EventSem::Post   
 * @brief Post condition.
 * @param semH is a pointer to the event semaphore.
 * @return the result of PrivateEventSemStruct::Post.
 */
static inline bool EventSemPost(HANDLE &semH) {
    if (semH == (HANDLE) NULL) {
        return False;
    }
    return (((PrivateEventSemStruct *) semH)->Post());
}

/** 
 * @see EventSem::Reset 
 * @brief Reset the semaphore for a new wait condition.
 * @param semH is a pointer to the event semaphore.
 * @return the result of PrivateEventSemStruct::Reset.
 */
static inline bool EventSemReset(HANDLE &semH) {
    if (semH == (HANDLE) NULL) {
        return False;
    }
    return ((PrivateEventSemStruct *) semH)->Reset();
}

/** 
 * @see EventSem::ResetWait
 * @brief Reset and then perform a wait condition of the event semaphore.
 * @param semH is a pointer to the event semaphore.
 * @param msecTimeout is the desired timeout.
 * @return the result of PrivateEventSemStruct::Wait.
 */
static inline bool EventSemResetWait(HANDLE &semH, TimeoutType msecTimeout) {
    EventSemReset(semH);
    return EventSemWait(semH, msecTimeout);
}

#endif
