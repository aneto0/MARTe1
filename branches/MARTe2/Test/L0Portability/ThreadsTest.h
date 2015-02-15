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
 * Tests the Threads class and associated functions
 */


#ifndef THREADS_TEST_H
#define THREADS_TEST_H

#include "Threads.h"
#include "EventSem.h"

class ThreadsTest{
public:
    ThreadsTest(){
        callbackTestSuccessful = False;
        incrementCounter       = 0;
        eventSem.Create();
    }
    /**
     * Variable incremented by the call back function ThreadsTestIncrementCounter
     */
    uint32 incrementCounter;

    /**
     * Generic event sem used for synchronisations in the test
     */
    EventSem eventSem;

    /**
     * To be used to communicate between callbacks and the 
     * function spawing the thread
     */
    bool callbackTestSuccessful;

    /**
     * Tests the thread creation
     * @param nOfThreads number of threads to test
     * @return True if nOfThreads are created
     */
    bool BeginThread(uint32 nOfThreads);

    /**
     * Tests the change of priority levels and classes. This a very basic test
     * and does not check if (and how) the priority request is actually propagated to 
     * the operating system
     * @return True if the priority level and class are changed consistently with
     * the request
     */
    bool Priorities();
};

#endif

