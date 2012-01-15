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
 * @file CDB reading example using the CDBExtended
 */
#include "GarbageCollectable.h"
#include "Object.h"
#include "ConfigurationDataBase.h"
#include "CDB.h"
#include "CDBExtended.h"

OBJECT_DLL(SimpleClass)
class SimpleClass : public GarbageCollectable, public Object{
OBJECT_DLL_STUFF(SimpleClass)
private:
    /**
     * Several data types to be populated by the ObjectLoadSetup
     */
    int32    myInt;
    float    myFloat;
    FString  myString;
    float   *myFloatArray;
    FString *myStringArray;
    float   *myFloatMatrix;

public:
    SimpleClass(){
        myInt              = 0;
        myFloat            = 0;
        myString           = "";
        myFloatArray       = NULL;
        myStringArray      = NULL;
        myFloatMatrix      = NULL;
    }

    virtual ~SimpleClass(){
        if(myFloatArray != NULL){
            free((void *&)myFloatArray);            
        }
        if(myFloatMatrix != NULL){
            free((void *&)myFloatMatrix);            
        }
        if(myStringArray != NULL){
            delete []myStringArray;
        }
    }

    /**
     * Configure an object using a configuration database
     */
    bool ObjectLoadSetup(ConfigurationDataBase &cdb,StreamInterface *err){
        CDBExtended cdbe(cdb);
        //Move to the place in the cdb
        if(!cdbe->Move("MySimpleClass")){
            AssertErrorCondition(FatalError, "Could not move to MySimpleClass");
            return False;
        }
        //Read the int value. Notice that scalar values can be read by passing NULL and 0 
        //for the size and dim of the array
        if(!cdbe.ReadInt32(myInt, "MyInt", 1234)){
            AssertErrorCondition(Warning, "MyInt was not defined. Using default of %d", myInt);
        }
        if(!cdbe.ReadFloat(myFloat, "MyFloat", 1.234)){
            AssertErrorCondition(Warning, "MyFloat was not defined. Using default of %f", myFloat);
        }
        if(!cdbe.ReadFString(myString, "MyString", "Some default")){
            AssertErrorCondition(Warning, "MyString was not defined. Using default of %s", myString.Buffer());
        }

        //In order to read a proper array first get the array dimensions (1=vector, 2=matrix, ...) and the size
        //of each dimension
        //Notice that the initialisation value of arrayDimension is also the maximum number of dimensions searched
        //by the ReadArray function
        int32 arrayDimension = 2;
        //Just put 2 to recycle later for matrix. 1 would be enough for vector
        int32 arraySize[2];
        if(cdbe->GetArrayDims(arraySize, arrayDimension, "MyFloatArray")){
            if(arrayDimension == 1){
                AssertErrorCondition(Information, "MyFloatArray dimension is 1 as expected");
                AssertErrorCondition(Information, "MyFloatArray has %d elements", arraySize[0]);
                //Try to allocate memory
                myFloatArray = (float *)malloc(arraySize[0] * sizeof(float));
                if(myFloatArray == NULL){
                    AssertErrorCondition(FatalError, "Failed to allocate %d bytes for myFloatArray", (arraySize[0] * sizeof(float)));
                    return False;
                }
                //Do the actual reading of values
                if(!cdbe.ReadFloatArray(myFloatArray, arraySize, arrayDimension, "MyFloatArray")){
                    AssertErrorCondition(FatalError, "Failed reading data to MyFloatArray");
                    return False;
                }
                //Print the actual values
                int32 i=0;
                for(i=0; i<arraySize[0]; i++){
                    AssertErrorCondition(Information, "MyFloatArray[%d]=%f", i, myFloatArray[i]);
                }
            } 
            else{
                AssertErrorCondition(Warning, "MyFloatArray dimension is not 1! The value is: %d", arrayDimension);
            }
        }
        else{
            AssertErrorCondition(Warning, "Failed reading MyFloatArray");
        }
        //Reading a matrix is very similar
        arrayDimension = 2;
        arraySize[0] = 0;
        arraySize[1] = 0;
        if(cdbe->GetArrayDims(arraySize, arrayDimension, "MyFloatMatrix")){
            if(arrayDimension == 2){
                AssertErrorCondition(Information, "MyFloatMatrix dimension is 2 as expected");
                int32 myFloatMatrixNRows = arraySize[0];
                int32 myFloatMatrixNCols = arraySize[1];
                AssertErrorCondition(Information, "MyFloatMatrix has %d rows and %d columns", myFloatMatrixNRows, myFloatMatrixNCols);
                //Try to allocate memory
                myFloatMatrix = (float *)malloc(myFloatMatrixNRows * myFloatMatrixNCols * sizeof(float));
                if(myFloatMatrix == NULL){
                    AssertErrorCondition(FatalError, "Failed to allocate %d bytes for MyFloatMatrix", (myFloatMatrixNRows * sizeof(float *)));
                    return False;
                }
                //Do the actual reading of values
                if(!cdbe.ReadFloatArray(myFloatMatrix, arraySize, arrayDimension, "MyFloatMatrix")){
                    AssertErrorCondition(FatalError, "Failed reading data to MyFloatMatrix");
                    return False;
                }
                //Print the actual values
                int32        r = 0;
                int32        c = 0;
                for(r=0; r<myFloatMatrixNRows; r++){
                    for(c=0; c<myFloatMatrixNCols; c++){
                        AssertErrorCondition(Information, "MyFloatMatrix[%d][%d]=%f", r, c, myFloatMatrix[r * myFloatMatrixNCols + c]);
                    }
                }
            } 
            else{
                AssertErrorCondition(Warning, "MyFloatMatrix dimension is not 2! The value is: %d", arrayDimension);
            }
        }
        else{
            AssertErrorCondition(Warning, "Failed reading MyFloatMatrix");
        }
        //Reading an array of strings is basically the same (but using a proper array created with a new)
        if(cdbe->GetArrayDims(arraySize, arrayDimension, "MyStringArray")){
            if(arrayDimension == 1){
                AssertErrorCondition(Information, "MyStringArray dimension is 1 as expected");
                AssertErrorCondition(Information, "MyStringArray has %d elements", arraySize[0]);
                //Try to allocate memory
                myStringArray = new FString[arraySize[0]];
                if(myStringArray == NULL){
                    AssertErrorCondition(FatalError, "Failed to allocate %d elements for MyStringArray", arraySize[0]);
                    return False;
                }
                //Do the actual reading of values
                if(!cdbe.ReadFStringArray(myStringArray, arraySize, arrayDimension, "MyStringArray")){
                    AssertErrorCondition(FatalError, "Failed reading data to MyStringArray");
                    return False;
                }
                //Print the actual values
                int32 i = 0;
                for(i=0; i<arraySize[0]; i++){
                    AssertErrorCondition(Information, "MyStringArray[%d]=%s", i, myStringArray[i].Buffer());
                }
            } 
            else{
                AssertErrorCondition(Warning, "MyStringArray dimension is not 1! The value is: %d", arrayDimension);
            }
        }
        else{
            AssertErrorCondition(Warning, "Failed reading MyStringArray");
        }
        
        cdbe->MoveToFather();

        AssertErrorCondition(Information, "MyInt    value is: %d", myInt);
        AssertErrorCondition(Information, "MyFloat  value is: %f", myFloat);
        AssertErrorCondition(Information, "MyString value is: %s", myString.Buffer());

        return True;
    }

};
OBJECTLOADREGISTER(SimpleClass, "$Id$")

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Configuration stored in an FString (usually this is a file or a tcp stream)
    FString cdbTxt = 
        "MySimpleClass = {\n"
        "    MyInt         = 10\n"
        "    MyFloat       = 5.0\n"
        "    MyString      = \"A string\"\n"
        "    MyFloatArray  = {-1.234 1.789 0.1233 1e2}\n"
        "    MyStringArray = {\"AAA\" \"BBB\" \"CCC\" ABC}\n"
        "    MyFloatMatrix = {\n"
        "                     0 = {0.123 -1e3}\n"
        "                     1 = {12345 .233}\n"
        "                     2 = {-1    1.32}\n"
        "                    }\n"
        "}\n";
    //Create the configuration database and load from a string
    ConfigurationDataBase cdb;
    if(!cdb->ReadFromStream(cdbTxt)){
        CStaticAssertErrorCondition(FatalError, "Failed reading from stream!");
        return -1;
    }

    GCRTemplate<SimpleClass> myObj(GCFT_Create);
    if(!myObj.IsValid()){
        CStaticAssertErrorCondition(FatalError, "myObj is not valid");
        return -1;
    }
    if(!myObj->ObjectLoadSetup(cdb, NULL)){
        CStaticAssertErrorCondition(FatalError, "Failed to configure myObj");
        return -1;
    }
    return 0;
}

