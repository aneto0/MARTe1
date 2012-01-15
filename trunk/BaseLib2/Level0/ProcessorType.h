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
 * Defines the processors where a particular task should run
 * e.g., if a task is to run on cpu 2 and 3 the mask is: 0x06 (00000110)
 */ 
#ifndef PROCESSOR_TYPE_H
#define PROCESSOR_TYPE_H

#include "GenDefs.h"

extern "C" 
{
    uint32 ProcessorTypeGetDefaultCPUs();
    void   ProcessorTypeSetDefaultCPUs(uint32 defaultMask);
}

class ProcessorType{
public:
    /** The processor mask */
    uint32 processorMask;

    /** The default CPU mask. Initialised to zero.*/
    static uint32 defaultCPUs;

public:
    friend uint32 ProcessorTypeGetDefaultCPUs();
    friend void   ProcessorTypeSetDefaultCPUs(uint32 mask);

#if !defined (_CINT)
    /** Constructor from integer
     *  The default is to run the tasks all CPUs but the first
     */
    ProcessorType(const uint32 cpuMask = 0xFE){
        processorMask = cpuMask;
    }

    ProcessorType(const ProcessorType &pt){
        processorMask = pt.processorMask;
    }

#endif

    void SetMask(const uint32 mask){
        processorMask = mask;
    }

    void AddCPU(const uint32 cpuNumber){
        processorMask |= (1 << (cpuNumber - 1));
    }

    void operator=(const uint32 cpuMask){
        processorMask = cpuMask;
    }

    void operator=(const ProcessorType &pt){
        processorMask = pt.processorMask;
    }

    void operator|=(const uint32 cpuMask){
        processorMask |= cpuMask;
    }

    void operator|=(const ProcessorType &pt){
        processorMask |= pt.processorMask;
    }

    bool operator==(const ProcessorType &pt){
        return processorMask == pt.processorMask;
    }

    bool operator==(const uint32 mask){
        return processorMask == mask;
    }

    bool operator!=(const ProcessorType &pt){
        return processorMask != pt.processorMask;
    }

    bool operator!=(const uint32 mask){
        return processorMask != mask;
    }

    static uint32 GetDefaultCPUs(){
        return ProcessorTypeGetDefaultCPUs();
    }

    static void SetDefaultCPUs(const uint32 mask){
        ProcessorTypeSetDefaultCPUs(mask);
    }
};

/** Declares that the number of CPUs is undefined or there is no interest
  * in specifying*/
#ifndef _CINT
const ProcessorType PTUndefinedCPUs(0);
#else
#define PTUndefinedCPUs 0
#endif

#endif

