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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RTAIConsoleAPI.h"

int main(int argc, char **argv){
    
    int callParameters[10];
    callParameters[0] = strtoll(argv[1], (char **)NULL, 16);
    callParameters[1] = 0;
    CallRemoteFunction(callParameters);
    
    /*
    long long longlong1 = 7000212000LL;
    long long longlong2 = 12000212000LL;
    float float1 = 6.0;
    float float2 = -0.12121;
    int floatptr;
    
    char *toWrite = "This is a string to test the console API\n";            
    if(argc < 4){
        printf("Please specify the address of rt_printk console_print_long_long console_print_float\n");
        return -1;
    }
    
    //TO PASS POINTERS
    callParameters[0] = strtoll(argv[1], (char **)NULL, 16);
    callParameters[1] = 1;
    callParameters[2] = CopyToKernel((int)toWrite, strlen(toWrite));
    CallRemoteFunction(callParameters);
    
    //TO PASS LONGS
    callParameters[0] = strtoll(argv[2], (char **)NULL, 16);
    callParameters[1] = 6;
    callParameters[2] = -100;
    callParameters[3] = (int)(longlong1 & 0xFFFFFFFFLL);
    callParameters[4] = (int)(longlong1 >> 32);    
    callParameters[5] = 100;
    callParameters[6] = (int)(longlong2 & 0xFFFFFFFFLL);
    callParameters[7] = (int)(longlong2 >> 32);        
    CallRemoteFunction(callParameters);
    
    //TO PASS FLOATS
    callParameters[0] = strtoll(argv[3], (char **)NULL, 16);
    callParameters[1] = 5;
    callParameters[2] = 55;
    callParameters[3] = -55;
    callParameters[4] = 0;
    floatptr = *((int *)&float1);
    printf("float1 = %x\n", floatptr);
    callParameters[5] = floatptr;    
    floatptr = *((int *)&float2);
    printf("float2 = %x\n", floatptr);
    callParameters[6] = floatptr;
    CallRemoteFunction(callParameters);*/
}
