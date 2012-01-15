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
 * @file shows how to create threads
 */

#include "Threads.h"
#include "Sleep.h"

//An exit condition for all the threads
static int32 exitAfterCalls = 500;

//Shared variable to be incremented by the threads
static int32 sharedVariable = 0;

//Simulate complicated analysis
void ComplexAnalysis(float sec){
    SleepSec(sec);
}

//Thread function call back
void IncrementDecrementFunction(void *threadID){
    int32 thisThreadID = (int32)threadID;
    CStaticAssertErrorCondition(Information, "Thread with id = %d started", thisThreadID);
    while(exitAfterCalls > 0){
        sharedVariable++;
        ComplexAnalysis((thisThreadID + 1) * 1e-3);
        sharedVariable--;
        exitAfterCalls--;
    }
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    int32 numberOfThreads = 10;
    int32 i               = 0;
    
    CStaticAssertErrorCondition(Information, "Number of threads = %d", numberOfThreads);
    for(i=0; i<numberOfThreads; i++){
        Threads::BeginThread(&IncrementDecrementFunction, (int32 *)i, THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, 0x1);
    }
    //Wait for the thread to run for exitAfterCalls times
    while(exitAfterCalls > 0){
        SleepSec(1e-3);
    }
    CStaticAssertErrorCondition(Information, "Value of sharedVariable = %d", sharedVariable);
    return 0;
}

