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
 * $Id: Processor.h 43 2012-02-08 17:07:58Z astephen $
 *
**/

/**
 * @file
 * Access processor's information
 */
#ifndef _PROCESSOR_OS_H
#define _PROCESSOR_OS_H

/** Defines the Processor Family for the Intel */
#define FAMILY_INTEL_X86     0x10010000

    /** 
        @return The processor Clock Rate
    */
    uint64 ProcessorClockRate(){
        return Processor_HRTFrequency;
    }

    /** The clock period in seconds.
        @return The processor clock cycle.
    */
    double  ProcessorClockCycle(){
        return Processor_HRTPeriod;
    }

    /** The processor type.
        @return The processor type.
    */
    const char *ProcessorName(){
        return "x86";
    }

    /** The processor family INTEL MOTOROLA ...
        @return The processor family
    */
    uint32 ProcessorFamily(){
        return FAMILY_INTEL_X86;
    }

    /** The number of cpus avaible */
    int32 ProcessorsAvailable(){
        return sysconf(_SC_NPROCESSORS_ONLN);
    }
}

#endif

