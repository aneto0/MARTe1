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
 * @file Semaphores, atomic and thread
 */

#include "Threads.h"
#include "Sleep.h"
#include "MutexSem.h"
#include "EventSem.h"
#include "Atomic.h"

//An exit condition for all the threads
static int32 exitAfterCalls = 50;

//Shared variable to be incremented by the threads
static int32 sharedVariable = 0;

//Event semaphore so that all the threads start at the same time
static EventSem eventSem;

//Mutex semaphore protecting the sharedVariable
static MutexSem mutexSem;

//The number of threads
static int32 numberOfThreads = 10;

//The number of threads to be started
static int32 numberOfThreadsLeftToStart = numberOfThreads;

//Simulate complicated analysis
void ComplexAnalysis(float sec){
    SleepSec(sec);
}

//Thread function call back
void IncrementDecrementFunction(void *threadID){
    intptr thisThreadID = (intptr)threadID;
    CStaticAssertErrorCondition(Information, "Thread with id = %d waiting for event sem", thisThreadID);
    numberOfThreadsLeftToStart--;
    if(!eventSem.Wait()){
        CStaticAssertErrorCondition(FatalError, "Thread with id = %d failed to wait in event sem (timeout?)", thisThreadID);
    }
    CStaticAssertErrorCondition(Information, "Thread with id = %d started", thisThreadID);
    while(exitAfterCalls > 0){
        //The mutex protects this region of code
        if(!mutexSem.Lock()){
            CStaticAssertErrorCondition(FatalError, "Thread with id = %d failed to wait in mutex sem (timeout?)", thisThreadID);
        }
        sharedVariable++;
        ComplexAnalysis((thisThreadID + 1) * 1e-3);
        sharedVariable--;
        exitAfterCalls--;
        if(!mutexSem.UnLock()){
            CStaticAssertErrorCondition(FatalError, "Thread with id = %d failed to unlock mutex sem", thisThreadID);
        }
    }
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    int32 i               = 0;
   
    //Configure the semaphores
    eventSem.Create();
    eventSem.Reset();
    mutexSem.Create(False);
 
    CStaticAssertErrorCondition(Information, "Number of threads = %d", numberOfThreads);
    for(i=0; i<numberOfThreads; i++){
        Threads::BeginThread(&IncrementDecrementFunction, (int32 *)i, THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, 0x1);
    }
    //Wait for all threads to be ready and waiting on the event semaphore (this isn't actually needed, but guarantees that the
    //CStaticAssertErrorCondition come in the right order)
    while(!Atomic::TestAndSet(&numberOfThreadsLeftToStart));

    CStaticAssertErrorCondition(Information, "Starting all threads!");
    //Allow all threads to start
    eventSem.Post();
    //Wait for the thread to run for exitAfterCalls times
    while(exitAfterCalls > 0){        
        SleepSec(1e-3);
    }
    //Wait if some calculation is still to be terminated
    mutexSem.Lock();
    CStaticAssertErrorCondition(Information, "Value of sharedVariable = %d (should be zero)", sharedVariable);
    return 0;
}

