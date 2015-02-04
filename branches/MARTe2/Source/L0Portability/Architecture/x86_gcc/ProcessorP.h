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
 * $Id: Processor.cpp 43 2012-02-08 17:07:58Z astephen $
 *
**/

#ifndef PROCESSOR_P_H
#define PROCESSOR_P_H

// executes the CPUID function
inline void ProcessorCPUID(uint32 type,uint32 &A,uint32 &B,uint32 &C,uint32 &D){
    int a[8];
    asm(
        "movl %%eax,16(%0)\n"
        "movl %%ebx,20(%0)\n"
        "movl %%ecx,24(%0)\n"
        "movl %%edx,28(%0)\n"
        "movl %1,%%eax\n"
        "cpuid\n"
        "movl %%eax, (%0)\n"
        "movl %%ebx, 4(%0)\n"
        "movl %%ecx, 8(%0)\n"
        "movl %%edx, 12(%0)\n"
        "movl 16(%0), %%eax\n"
        "movl 20(%0), %%ebx\n"
        "movl 24(%0), %%ecx\n"
        "movl 28(%0), %%edx\n"
        :
        : "r" (&a[0]) ,"r" (type)
        : "eax","ebx","ecx","edx"
        );
    A=a[0];
    B=a[1];
    C=a[2];
    D=a[3];
}
#endif

