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
 * @file shows how to use the StaticListHolder
 */
#include "ErrorManagement.h"
#include "StaticListHolder.h"
#include "StaticListTemplate.h"

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //32 bit floating point elements to insert in the list
    float f1 = 1.23;
    float f2 = 3210;
    float f3 = -1.23;

    //The constructor receives the elements size, as a multiple of intptr size. All elements must have the same size.
    StaticListHolder slh(1);
    
    //Protect the concurrent access to the list (this is case is single-threaded, so this is useless)
    slh.SetAccessTimeout();

    //intprt is the size of unsigned long pointer (usually 32 bits in a 32 bit machine and 64 bits in a 64 bit machine)
    slh.ListAdd((const intptr *) &f1);
    slh.ListAdd((const intptr *) &f2);
    slh.ListAdd((const intptr *) &f3);

    CStaticAssertErrorCondition(Information, "My list holder size is: %d", slh.ListSize());
    //Remove the second element and copy the value to retrivedValue
    float retrivedValue;
    if(!slh.ListExtract((intptr *) &retrivedValue, 1)){
        CStaticAssertErrorCondition(FatalError, "Failed to retrieve the second element on the list!");
        return -1;
    }
    //Print the value (which I already now it was a float)
    CStaticAssertErrorCondition(Information, "The value of the second element in the list is %f", retrivedValue);
    CStaticAssertErrorCondition(Information, "My list holder size is: %d", slh.ListSize());
    //Look at value, without removing it
    if(!slh.ListPeek((intptr *) &retrivedValue, 1)){
        CStaticAssertErrorCondition(FatalError, "Failed to peek the second element on the list!");
        return -1;
    }
    CStaticAssertErrorCondition(Information, "The value of the second element in the list is %f", retrivedValue);
    CStaticAssertErrorCondition(Information, "My list holder size is: %d", slh.ListSize());
    //Search for an element
    int32 elementPos = slh.ListFind((const intptr *)&f3);
    CStaticAssertErrorCondition(Information, "Value %f is in position: %d", f3, elementPos);
    //Search for an element that no longer exists (f2 because we removed it)
    elementPos = slh.ListFind((const intptr *)&f2);
    CStaticAssertErrorCondition(Information, "Value %f is in position: %d", f2, elementPos);
    //Add the second element back where it was (notice that now we are not adding to the end of the queue)
    if(!slh.ListAdd((const intptr *) &f2, 1)){
        CStaticAssertErrorCondition(FatalError, "Failed to add the second element back to the list!");
        return -1;
    }
    //Search again for f2. Now it should find it. 
    elementPos = slh.ListFind((const intptr *)&f2);
    CStaticAssertErrorCondition(Information, "Value %f is in position: %d", f2, elementPos);
    CStaticAssertErrorCondition(Information, "My list holder size is: %d", slh.ListSize());
    //Remove an element from the list
    if(!slh.ListDelete((const intptr *)&f1)){
        CStaticAssertErrorCondition(Information, "Failed to remove value %f", f1);
        return -1;
    }
    CStaticAssertErrorCondition(Information, "My list holder size is: %d", slh.ListSize());

    return 0;
}

