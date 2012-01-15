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
 * @file BasicTypes example
 */
#include "FString.h"
#include "BasicTypes.h"
#include "ErrorManagement.h"

//Print information about a BasicTypeDescriptor
void PrintBTDInfo(BasicTypeDescriptor &btd){
    FString name;
    BTConvertToString(btd, name);
    CStaticAssertErrorCondition(Information, "Number of bytes in %s is %d (%d)", name.Buffer(), btd.ByteSize(), btd.BitSize());
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Print information regarding a 32 bit integer
    BasicTypeDescriptor anInt32 = BTDInt32;
    PrintBTDInfo(anInt32);

    //BasicTypeDescriptors can also be created from a string (for instance in a cfg file!)
    BasicTypeDescriptor btdFloat;
    BTConvertFromString(btdFloat, "float"); 
    PrintBTDInfo(btdFloat);

    //Convert between two types
    int32  in = 123456789;
    double out;
    
    //Very useful if you don't know the input type and want to convert to something using a generic call
    BTConvert(1, BTDDouble, &out, BTDInt32, &in);
    CStaticAssertErrorCondition(Information, "Converted %d to %lf", in, out);

    return 0;
}

