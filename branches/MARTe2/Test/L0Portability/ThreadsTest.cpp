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

#include "GeneralDefinitions.h"
#include "Sleep.h"
#include "ThreadsTest.h"

void ThreadsTestIncrementCounter(ThreadsTest &tt) {
    tt.incrementCounter++;
}

void PrioritiesCallback(ThreadsTest &tt) {
    bool ok = True;
    TID threadId = Threads::Id();
    int32 priorityClass = Threads::GetPriorityClass(threadId);
    Threads::SetPriorityLevel(threadId, Threads::PRIORITY_LOWEST);
    //Verify that the class was not changed
    ok = ok && (priorityClass == Threads::GetPriorityClass(threadId));
    //verify that the priority is as expected
    ok =
            ok
                    && (Threads::PRIORITY_LOWEST
                            == Threads::GetPriorityLevel(threadId));

    tt.callbackTestSuccessful = ok;
    tt.eventSem.Post();

}

bool ThreadsTest::BeginThread(uint32 nOfThreads) {
    uint32 i = 0;
    for (i = 0; i < nOfThreads; i++) {
        //Each thread will increment incrementCounter and its value should arrive
        //to nOfThreads
        Threads::BeginThread((ThreadFunctionType) ThreadsTestIncrementCounter,
                             this);
    }
    //Give some time for all the threads to have started...
    while (incrementCounter != nOfThreads) {
        SleepMSec(20);
        i--;
        //Waited 20 ms x nOfThreads and the value of the incrementCounter is still
        //not nOfThreads which likely indicates a problem with the creation of the thread
        if (i == 0) {
            return False;
        }
    }
    return True;
}

bool ThreadsTest::Priorities() {
    eventSem.Reset();
    Threads::BeginThread((ThreadFunctionType) PrioritiesCallback, this);

    if (!eventSem.Wait()) {
        return False;
    }

    return callbackTestSuccessful;
}

