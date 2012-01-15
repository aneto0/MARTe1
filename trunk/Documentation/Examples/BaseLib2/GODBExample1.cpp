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
 * @file Object creation with a GlobalObjectDataBase
 */
#include "GCNamedObject.h"
#include "ConfigurationDataBase.h"
#include "CDB.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"

//Inherit from GCNamedObject in order to automatically retrieve the name
OBJECT_DLL(SimpleClass)
class SimpleClass : public GCNamedObject{
OBJECT_DLL_STUFF(SimpleClass)
private:
    int32    myInt;

public:
    SimpleClass(){
        myInt              = 0;
    }

    virtual ~SimpleClass(){
    }

    int GetMyIntValue(){
        return myInt;
    }

    /**
     * Configure an object using a configuration database
     */
    bool ObjectLoadSetup(ConfigurationDataBase &cdb,StreamInterface *err){
        //Automatically read the object name
        GCNamedObject::ObjectLoadSetup(cdb, err);
        CDBExtended cdbe(cdb);
        //Read the int value. Notice that scalar values can be read by passing NULL and 0 
        //for the size and dim of the array
        if(!cdbe.ReadInt32(myInt, "MyInt", 1234)){
            AssertErrorCondition(Warning, "MyInt was not defined. Using default of %d", myInt);
        }
        AssertErrorCondition(Information, "MyInt    value is: %d", myInt);

        return True;
    }

};
OBJECTLOADREGISTER(SimpleClass, "$Id$")

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Configuration stored in an FString (usually this is a file or a tcp stream)
    //Notice the + and the Class = 
    FString cdbTxt = 
        "+MySimpleClass1 = {\n"
        "    Class         = SimpleClass\n"
        "    MyInt         = 10\n"
        "}\n"
        "+MyGCRefC = {\n"
        "    Class = GCReferenceContainer \n"
        "    +MySimpleClass2 = {\n"
        "        Class         = SimpleClass\n"
        "    }\n"
        "    +MySimpleClass3 = {\n"
        "        Class         = SimpleClass\n"
        "        MyInt         = -10\n"
        "    }\n"
        "}\n";
    //Create the configuration database and load from a string
    ConfigurationDataBase cdb;
    if(!cdb->ReadFromStream(cdbTxt)){
        CStaticAssertErrorCondition(FatalError, "Failed reading from stream!");
        return -1;
    }

    //Let the GlobalObjectDataBase automatically create the objects
    if(!GetGlobalObjectDataBase()->ObjectLoadSetup(cdb, NULL)){
        CStaticAssertErrorCondition(FatalError, "Failed to load cdb");
        return -1;
    }
    
    //We can now look for the MyGCRef in the GlobalObjectDataBase
    GCRTemplate<GCReferenceContainer> ref = GetGlobalObjectDataBase()->Find("MyGCRefC");
    if(ref.IsValid()){
        CStaticAssertErrorCondition(Information, "Found MyGCRefC as expected");
        CStaticAssertErrorCondition(Information, "Number of children inside MyGCRefC is: %d", ref->Size()); 
    }
    else{
        CStaticAssertErrorCondition(FatalError, "Could not find MyGCRefC");
    }
    //List all the objects
    GCRCLister lister;
    GetGlobalObjectDataBase()->Iterate(&lister,GCFT_Recurse);

    return 0;
}

