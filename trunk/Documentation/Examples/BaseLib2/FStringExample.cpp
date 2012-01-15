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
 * @file Basic FString functionality
 */
#include "ErrorManagement.h"
#include "FString.h"

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Create an initialised FString
    FString str1 = "/a/path/to/somewhere/";
    //Create an empty FString
    FString str2;
    //Copy the contents of str1 to str2
    str2 = str1;
    //Compare the values (can also use the !=)
    if(str2 == str1){
        CStaticAssertErrorCondition(Information, "As expected the contents of str1 are the same as of str2");
    }
    else{
        //This should never happen
        CStaticAssertErrorCondition(FatalError, "The contents of str1 are not the same as of str2, as it should!");
    }
    //It is also possible to printf in a string
    FString str3;
    str3.Printf("The value is %f", 0.12345);
    //The string buffer (char *) is accessible using the Buffer() method
    CStaticAssertErrorCondition(Information, "%s", str3.Buffer());
    //To tokenize a string use the GetToken
    FString token;
    while(str1.GetToken(token, "/")){
        CStaticAssertErrorCondition(Information, "Token = %s", token.Buffer());
        //Reset a string
        token.SetSize(0);
    }
    //It is also possible to concatenate to strings
    str1 += "yet/another/string";
    CStaticAssertErrorCondition(Information, "After concatenating the value of str1 is %s", str1.Buffer());
    //The size of a string 
    CStaticAssertErrorCondition(Information, "The size of str1 is %d", str1.Size());

    return 0;
}

