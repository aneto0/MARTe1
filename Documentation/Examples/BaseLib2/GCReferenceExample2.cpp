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
 * @file shows how to use create objects by name
 */

#include "GCReference.h"
#include "GCRTemplate.h"
#include "GarbageCollectable.h"

/**
 * A simple class implementing  GarbageCollectable, so that it can be 
 * tracked by a GCReference, and Object, so that it can be created by name
 */
//These macros create a series of hidden functions which allow to automatically create the object
OBJECT_DLL(SimpleClass)
class SimpleClass : public GarbageCollectable, public Object{
OBJECT_DLL_STUFF(SimpleClass)
public:
    /**
     * A variable which uniquely identifies the object
     */
    int32 uniqueID;
    SimpleClass(){
        uniqueID = 0;
        AssertErrorCondition(Information, "Creating a SimpleClass");
        CStaticAssertErrorCondition(Information, "Creating a SimpleClass");
    }
    virtual ~SimpleClass(){}
};
//Usually a version id is set as the second argument
OBJECTLOADREGISTER(SimpleClass, "$Id: GCReferenceExample2.cpp,v 1.3 2011/08/05 13:41:59 aneto Exp $")

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 
    
    //Try to create the object by name
    GCRTemplate<SimpleClass> simpleClassRef1("SimpleClass");
    //It should be valid
    if(simpleClassRef1.IsValid()){
        CStaticAssertErrorCondition(Information, "The simpleClassRef1 is valid as expected.");
    }
    else{
        //This will never happen
        CStaticAssertErrorCondition(FatalError, "The simpleClassRef1 should be valid!!!");
    }

    //The object is now ready to be used
    simpleClassRef1->uniqueID = 987654321;
    //Create another object
    GCRTemplate<SimpleClass> simpleClassRef2(GCFT_Create);
    if(simpleClassRef2.IsValid()){ 
        simpleClassRef2->uniqueID = 123456789;
    }
    //This object should be different from the other
    if(simpleClassRef1 != simpleClassRef2){
        CStaticAssertErrorCondition(Information, "As expected, simpleClassRef1 and simpleClassRef2 are references for different objects");
    }
    else{
        CStaticAssertErrorCondition(FatalError, "As expected, simpleClassRef1 and simpleClassRef2 should be references for different objects!!!");
    }
    CStaticAssertErrorCondition(Information, "simpleClassRef1->uniqueID = %d and simpleClassRef2->uniqueID = %d", simpleClassRef1->uniqueID, simpleClassRef2->uniqueID);
    return 0;
}

