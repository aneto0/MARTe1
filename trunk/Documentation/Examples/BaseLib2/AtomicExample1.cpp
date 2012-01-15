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

#include "Atomic.h"
#include "Threads.h"
#include "ErrorManagement.h"

//The lock variable
static int32 locked  = 0;

//Unlock after sleeping
void UnlockWithTestAndSet(void *args){
    SleepSec(2.0);
    locked = 0;
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    int32 a = 3;
    int32 b = 4;
    
    CStaticAssertErrorCondition(Information, "Starting with a=%d b=%d", a, b);
    //Exchange
    b = Atomic::Exchange(&a, b);
    CStaticAssertErrorCondition(Information, "After exchanging a=%d b=%d", a, b);
    //Atomic increment
    Atomic::Increment(&a);
    CStaticAssertErrorCondition(Information, "After incrementing a=%d", a);
    
    locked = 1; 
    //Try to enter a locked region
    if(!Atomic::TestAndSet(&locked)){
        CStaticAssertErrorCondition(Information, "As expected TestAndSet failed");
    }
    locked = 0;
    if(!Atomic::TestAndSet(&locked)){
        CStaticAssertErrorCondition(FatalError, "TestAndSet and failed");
    }

    //Lock again
    CStaticAssertErrorCondition(Information, "locked should now be 1 locked = %d", locked);
    //Create thread to perform the unlock 
    CStaticAssertErrorCondition(Information, "Going to wait for thread to unlock");
    Threads::BeginThread(UnlockWithTestAndSet);
    //Spin lock
    while(!Atomic::TestAndSet(&locked));

    CStaticAssertErrorCondition(Information, "Unlocked by thread");

    return 0;
}

