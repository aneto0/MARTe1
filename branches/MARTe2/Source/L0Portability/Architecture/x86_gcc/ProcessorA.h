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
static inline void ProcessorCPUID(uint32 info, uint32 &eax,uint32 &ebx,uint32 &ecx,uint32 &edx){
    __asm__(
        "cpuid;"                                            /* assembly code */
        :"=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) /* outputs */
        :"a" (info)                                         /* input: info into eax */
                                                            /* clobbers: none */
    );
}

/** The processor family INTEL MOTOROLA ...
    @return The processor family
*/
static inline uint32 ProcessorFamily(){
    uint32 eax = 0;
    uint32 ebx = 0;
    uint32 ecx = 0;
    uint32 edx = 0;
    ProcessorCPUID(1, eax, ebx, ecx, edx);
    uint32 family = (eax >> 8) & 0xf;
    if(family == 0xf){
        family += (eax >> 20) & 0xf;
    }
    return family;
}

/** 
    @return The processor name
*/
static inline void ProcessorName(char *name){
    uint32 eax = 0;
    ProcessorCPUID(0, eax, (uint32 &)name[0], (uint32 &)name[8], (uint32 &)name[4]);
}
#endif

