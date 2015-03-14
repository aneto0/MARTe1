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

/**
 * Implementation of the cpuid function for x86 and cl
 */
static inline void ProcessorCPUID(uint32 code, uint32 &A, uint32 &B, uint32 &C,
                                  uint32 &D) {
    uint32 a, b, c, d;
__asm {
    mov eax , code
    _emit 0x0F
    _emit 0xA2
    mov a,eax
    mov b,ebx
    mov c,ecx
    mov d,edx
}
A = a;
B = b;
C = c;
D = d;
}

/** 
 * @see Processor::Family()
 */
uint32 ProcessorFamily() {
uint32 eax = 0;
uint32 ebx = 0;
uint32 ecx = 0;
uint32 edx = 0;
ProcessorCPUID(1, eax, ebx, ecx, edx);
uint32 family = (eax >> 8) & 0xf;
if (family == 0xf) {
    family += (eax >> 20) & 0xf;
}
return family;
}

/** 
 * @see Processor::Model()
 */
uint32 ProcessorModel() {
uint32 eax = 0;
uint32 ebx = 0;
uint32 ecx = 0;
uint32 edx = 0;
ProcessorCPUID(1, eax, ebx, ecx, edx);
return (eax >> 4) & 0xf;
}

/**
 * Stores the processor vendor
 */
static char processorVendorId[13];

/** 
 * @see Processor::VendorId()
 */
const char *ProcessorVendorId() {

uint32 eax = 0;
ProcessorCPUID(0, eax, (uint32 &) processorVendorId[0],
               (uint32 &) processorVendorId[8],
               (uint32 &) processorVendorId[4]);
processorVendorId[12] = 0;
return &processorVendorId[0];
}
#endif

