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
#ifndef PROCESSOR_H
#define PROCESSOR_H

/** Defines the Processor Family for the Intel */
#define FAMILY_INTEL_X86     0x10010000
/** Defines the Processor Family for the 68K */
#define FAMILY_MOTOROLA_68K  0x20010000
/** Defines the Processor Family for the PPC */
#define FAMILY_MOTOROLA_PPC  0x20020000
/** Defines the Processor Family for the Sparc */
#define FAMILY_SPARC         0x40010000

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,ProcessorOS.h)
#include INCLUDE_FILE_ARCHITECTURE(ARCHITECTURE,ProcessorA.h)

/** Defines some methods to get information about the processor. */
class Processor {
public:

    /** The processor type. */
    static void Name(char *name){
        return  ProcessorName(name);
    }

    /** The processor family INTEL/MOTOROLA/\.\.. */
    static uint32 Family(){
        return  ProcessorFamily();
    }

   
    /** The number of cpus avaible */
    static int32 Available(){
        return ProcessorsAvailable();
    }
};

#endif

