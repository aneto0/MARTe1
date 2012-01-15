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
 * @file
 * Access processor's information
 */
#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#include "System.h"

/** Defines the Processor Family for the Intel */
#define FAMILY_INTEL_X86     0x10010000
/** Defines the Processor Family for the 68K */
#define FAMILY_MOTOROLA_68K  0x20010000
/** Defines the Processor Family for the PPC */
#define FAMILY_MOTOROLA_PPC  0x20020000

/** @name Intel related defines
    @{
*/
// INTELX86 proc type
#define CPUID_PROCTYPE_OEM       0
#define CPUID_PROCTYPE_OVERDRIVE 1
#define CPUID_PROCTYPE_DUAL      2
#define CPUID_PROCTYPE_RESERVED  3

// INTELX86 characteristics
/** This will return the pointer to astring containg either "genuineintel" or something else. */
#define CPU_IDENT       0x00000001
#define CPU_FPU_ON_CHIP 0x00000002
#define CPU_ENH_V8086   0x00000003
#define CPU_DEBUG_EXT   0x00000004
#define CPU_PAGESZ_EXT  0x00000005
#define CPU_TSC_AVAIL   0x00000006
#define CPU_MSR_AVAIL   0x00000007
#define CPU_PAE_AVAIL   0x00000008
#define CPU_MCE_AVAIL   0x00000009
#define CPU_CX8_AVAIL   0x0000000A
#define CPU_APIC_AVAIL  0x0000000B
#define CPU_MTRR_AVAIL  0x0000000C
#define CPU_PGE_AVAIL   0x0000000D
#define CPU_MCAA_AVAIL  0x0000000E
#define CPU_CMOV_AVAIL  0x0000000F
#define CPU_MMX_AVAIL   0x00000010
/** OEM | OVERDRIVE | DUAL | RESV. */
#define CPU_TYPE        0x00000011
#define CPU_FAMILY_     0x00000012
#define CPU_MODEL       0x00000013
#define CPU_STEPPING    0x00000014
/** The whole feature info bitset. */
#define CPU_FEATINFO    0x00000015
/** The whole version info bitset. */
#define CPU_VERSINFO    0x00000016
/** If it is a genuine intel. */
#define CPU_INTEL       0x00000017
#define CPU_MAXREAD     0x00000018

/** @} */

extern "C" {

    /** Retunrs the Processor Clock Rate
        @return The processor Clock Rate
    */
    uint64 ProcessorClockRate();

    /** The clock period in seconds.
        @return The processor clock cycle.
    */
    double  ProcessorClockCycle();

    /** The processor type.
        @return The processor type.
    */
    const char  *ProcessorName();

    /** The processor family INTEL MOTOROLA ...
        @return The processor family
    */
    uint32 ProcessorFamily();

    /** 1 = True - 0 = False - otherwise the value. */
    intptr ProcessorCharacteristic(uint32 capId);

    /** Use it on constructors used on static object: the order of initialization is not guaranteed. */
    void   ProcessorReScanCPU();
    
    /** The number of cpus avaible */
    int32 ProcessorsAvailable();

}

/** Defines some methods to get information about the processor. */
class Processor {
public:

    /** The high resolution timer clock in hertz (=CPU in some platforms). */
    static uint64 ClockFrequency(){
        return  ProcessorClockRate();
    }

    /** The clock period in seconds. */
    static double ClockPeriod(){
        return  ProcessorClockCycle();
    }

    /** The processor type. */
    static const char *Name(){
        return  ProcessorName();
    }

    /** The processor family INTEL/MOTOROLA/\.\.. */
    static uint32 Family(){
        return  ProcessorFamily();
    }

    /** 1 = True - 0 = False - otherwise the value. */
    static intptr Characteristic(uint32 capId){
        return  ProcessorCharacteristic(capId);
    }

    /** Use it on constructors used on static object: the order of initialization is not guaranteed. */
    static void ReScanCPU(){
        ProcessorReScanCPU();
    }
    
    /** The number of cpus avaible */
    static int32 Available()
    {
        return ProcessorsAvailable();
    }
};

#endif

