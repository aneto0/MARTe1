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
 * @file shows how to use GCReferenceContainers and GCNamedObjects
 */

#include "GCReference.h"
#include "GCRTemplate.h"
#include "GCNamedObject.h"
#include "GCReferenceContainer.h"
#include "GarbageCollectable.h"

/**
 * A simple class implementing  GarbageCollectable, so that it can be 
 * tracked by a GCReference, and Object, so that it can be created by name
 */
OBJECT_DLL(SimpleClass)
class SimpleClass : public GarbageCollectable, public Object{
OBJECT_DLL_STUFF(SimpleClass)
public:
    SimpleClass(){
    }
    virtual ~SimpleClass(){}
};
//Usually a version id is set as the second argument
OBJECTLOADREGISTER(SimpleClass, "$Id: GCReferenceContainerExample.cpp,v 1.1 2011/07/14 09:41:53 aneto Exp $")

/**
 * A simple class inheriting from GCNamedObject so that it can be
 * automatically GarbageCollectable, Object and to store the actual
 * name given to the object
 */
OBJECT_DLL(SimpleNamedObject)
class SimpleNamedObject : public GCNamedObject{
OBJECT_DLL_STUFF(SimpleNamedObject)
public:
    SimpleNamedObject(){
    }
    virtual ~SimpleNamedObject(){}
};
//Usually a version id is set as the second argument
OBJECTLOADREGISTER(SimpleNamedObject, "$Id: GCReferenceContainerExample.cpp,v 1.1 2011/07/14 09:41:53 aneto Exp $")

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 
    
    //Try to create a SimpleNamedObject by name
    GCRTemplate<SimpleNamedObject> simpleNamedObj("SimpleNamedObject");
    //It should be valid
    if(simpleNamedObj.IsValid()){
        CStaticAssertErrorCondition(Information, "The simpleNamedObj is valid as expected.");
    }
    else{
        //This will never happen
        CStaticAssertErrorCondition(FatalError, "The simpleNamedObj should be valid!!!");
    }
    //Because it is a named object, we can associate a name to it
    simpleNamedObj->SetObjectName("myObject");

    //One way of verifying if an object is of given type is to use GCRTemplate
    GCRTemplate<GCNamedObject> namedObj = simpleNamedObj;
    //If this is valid (and in this case it is) then the reference is valid...
    if(namedObj.IsValid()){
        CStaticAssertErrorCondition(Information, "As expected, namedObj is of type GCNamedObject and its name is: %s", namedObj->Name());
    }
    else{
        //This will never happen
        CStaticAssertErrorCondition(FatalError, "The namedObj should be valid!!!");
    }

    //Create another object of type SimpleClass
    GCRTemplate<SimpleClass> simpleClassObj(GCFT_Create);
    if(!simpleNamedObj.IsValid()){
        CStaticAssertErrorCondition(FatalError, "The simpleClassObj should be valid!!!");
        return -1;
    }

    //A SimpleClass if not of type GCNamedObject
    GCRTemplate<SimpleClass> simpleClassObj2 = simpleNamedObj;
    //So it should not be valid...
    if(!simpleClassObj2.IsValid()){
        CStaticAssertErrorCondition(Information, "As expected, simpleNamedObj is not of type SimpleClass, so simpleClassObj2 is not valid.");
    }
    else{
        CStaticAssertErrorCondition(FatalError, "The simpleClassObj2 should not be valid!!!");
    }
    //Make it a valid reference
    simpleClassObj2 = simpleClassObj;
    if(!simpleClassObj2.IsValid()){
        CStaticAssertErrorCondition(FatalError, "The simpleClassObj2 should now be valid!!!");
        return -1;
    }
    //Create a second named object
    GCRTemplate<SimpleNamedObject> simpleNamedObj2("SimpleNamedObject");
    if(!simpleNamedObj2.IsValid()){
        CStaticAssertErrorCondition(Information, "The simpleNamedObj2 is not valid!");
        return -1;
    }
    simpleNamedObj2->SetObjectName("myObject2");

    //Create a GCReferenceContainer to store our references
    GCReferenceContainer gcRefContainer;
    if(!gcRefContainer.Insert(namedObj)){
        CStaticAssertErrorCondition(FatalError, "Failed inserting namedObj to GCReferenceContainer");
        return -1;
    }
    if(!gcRefContainer.Insert(simpleClassObj)){
        CStaticAssertErrorCondition(FatalError, "Failed inserting simpleClassRef1 to GCReferenceContainer");
        return -1;
    }
    if(!gcRefContainer.Insert(simpleClassObj2)){
        CStaticAssertErrorCondition(FatalError, "Failed inserting simpleClassRef2 to GCReferenceContainer");
        return -1;
    }
    if(!gcRefContainer.Insert(simpleNamedObj2)){
        CStaticAssertErrorCondition(FatalError, "Failed inserting simpleNamedObj2 to GCReferenceContainer");
        return -1;
    }
    //Query the size of the container
    CStaticAssertErrorCondition(Information, "The size of the gcRefContainer is: %d", gcRefContainer.Size());
    //Look for objects based on name
    GCRTemplate<SimpleNamedObject> myObject3 = gcRefContainer.Find("myObject2");
    if(myObject3.IsValid()){
        CStaticAssertErrorCondition(Information, "As expected found %s in the container", myObject3->Name());
    }
    else{
        CStaticAssertErrorCondition(FatalError, "Failed to found myObject2 in the container");
    }
    //Look for objects based on index
    //By using the IsValid the type of the object can be tested
    int32 i=0;
    for(i=0; i<gcRefContainer.Size(); i++){
        GCRTemplate<SimpleClass> obj = gcRefContainer.Find(i);
        if(obj.IsValid()){
            CStaticAssertErrorCondition(Information, "Found an object of type SimpleClass in position %d", i);
        }
    }
    //Remove all the objects of type GCNamedObject
    for(i=0; i<gcRefContainer.Size(); i++){
        GCRTemplate<GCNamedObject> obj = gcRefContainer.Find(i);
        if(obj.IsValid()){
            gcRefContainer.Remove(i);
            //The size has changed...
            i=0;
        }
    }
    CStaticAssertErrorCondition(Information, "After removing the GCNamedObjects, the size of the gcRefContainer is: %d", gcRefContainer.Size());
    return 0;
}

