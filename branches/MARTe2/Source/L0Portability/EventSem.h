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
 * A semaphore used to synchronise several tasks.
 *
 * After being Reset the semaphore is ready to Wait.
 * Once waiting, until a Post arrives all the tasks will wait on 
 * the semaphore. After the post all tasks are allowed to proceed.
 * A Reset is then required to use the semaphore again.
 */
#ifndef EVENT_SEM_H
#define EVENT_SEM_H

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,EventSemOS.h)
#include "SemCore.h"


/** Definition of an event shemaphore. */
class EventSem: public SemCore{
public:
    /** */
    EventSem(){
    }

    /** */
    EventSem(HANDLE h){
        Init(h);
    }

    /** copies semaphore and special infos as well. */
    void operator=(EventSem &s){
        *this = s;
    }

    /** */
    ~EventSem(){
        Close();
    }

    /** Creates the semafore */
    bool Create(){
        return EventSemCreate(semH);
   }

    /** closes the semafore */
    bool Close(void){
        return EventSemClose(semH);
    }

    /** wait for an event */
    bool Wait(TimeoutType msecTimeout = TTInfiniteWait){
        return EventSemWait(semH, msecTimeout);
    }

    /** resets the semafore and then waits*/
    bool ResetWait(TimeoutType msecTimeout = TTInfiniteWait){
        return EventSemResetWait(semH, msecTimeout);
    }

    /** Send an event to semafore */
    bool Post(){
        return EventSemPost(semH);
    }

    /** reset the semafore to its unposted state */
    bool Reset(){
        return EventSemReset(semH);
    }
};

#endif

