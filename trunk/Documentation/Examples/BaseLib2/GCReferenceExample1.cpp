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
 * @file shows how to use GCReference and GCRTemplate to manage and garbage
 * collect objects
 */

#include "GCReference.h"
#include "GCRTemplate.h"
#include "GarbageCollectable.h"

/**
 * A simple class implementing  GarbageCollectable so that it can be 
 * tracked by a GCReference 
 */
class SimpleClass : public GarbageCollectable{
public:
    /**
     * A variable which uniquely identifies the object
     */
    int32 uniqueID;
    SimpleClass(){
        uniqueID = 0;
    }
    virtual ~SimpleClass(){}
};

/**
 * Simple function to increment the number of references
 */
void SimpleFunction(GCRTemplate<SimpleClass> &simpleClassRef){
    GCRTemplate<SimpleClass> simpleClassRefLocal = simpleClassRef;
    //Number of references should have been incremented by 1
    CStaticAssertErrorCondition(Information, "Inside SimpleFunction. Number of references pointing to this SimpleClass object is:%d", simpleClassRefLocal.NumberOfReferences());
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //This is not enough to use the object... it does not actually
    //create an instance of SimpleClass. It is just declaring that it
    //is a reference variable to SimpleClass objects... be careful...
    GCRTemplate<SimpleClass> simpleClassRef1;
    //It should not be valid
    if(!simpleClassRef1.IsValid()){
        CStaticAssertErrorCondition(Information, "The simpleClassRef1 is not valid as expected.");
    }
    else{
        //This will never happen
        CStaticAssertErrorCondition(Information, "The simpleClassRef1 should not be valid!!!");
    }
    //Objects are created using GCFT_Create
    GCRTemplate<SimpleClass> simpleClassRef2(GCFT_Create);
    //This reference should be valid
    if(simpleClassRef2.IsValid()){
        CStaticAssertErrorCondition(Information, "The simpleClassRef2 is valid as expected.");
    }
    else{
        //This will never happen
        CStaticAssertErrorCondition(Information, "The simpleClassRef2 should be valid!!!");
    }
    //Object variables and functions are access using the -> operator
    simpleClassRef2->uniqueID = 123456789;
    CStaticAssertErrorCondition(Information, "Object uniqueID is:%d", simpleClassRef2->uniqueID);
    //One reference should be pointing to our object
    CStaticAssertErrorCondition(Information, "Number of references pointing to this SimpleClass object is:%d", simpleClassRef2.NumberOfReferences());
    //Now put another reference pointing to the same object
    simpleClassRef1 = simpleClassRef2;
    CStaticAssertErrorCondition(Information, "Number of references now pointing to this SimpleClass object is:%d", simpleClassRef2.NumberOfReferences());
    //And the reference should now be valid because it is pointing to a valid pointer
    if(simpleClassRef1.IsValid()){
        CStaticAssertErrorCondition(Information, "The simpleClassRef1 is now valid as expected.");
    }
    else{
        //This will never happen
        CStaticAssertErrorCondition(Information, "The simpleClassRef1 should be valid!!!");
    }
    CStaticAssertErrorCondition(Information, "Number of references pointing to this SimpleClass object before calling the function is:%d", simpleClassRef2.NumberOfReferences());
    //The number of references should be incremented by 1. It would be 2 if passed by value and not reference...
    SimpleFunction(simpleClassRef2);
    //Number of references should return to its original value
    CStaticAssertErrorCondition(Information, "Number of references pointing to this SimpleClass object after calling the function is:%d", simpleClassRef2.NumberOfReferences());
    return 0;
}

