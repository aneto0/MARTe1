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

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_OPERATING_SYSTEM(OPERATING_SYSTEM,ProcessorOS.h)
#include INCLUDE_FILE_ARCHITECTURE(ARCHITECTURE,ProcessorP.h)

/** Defines some methods to get information about the processor. */
class Processor {
public:

    /** The high resolution timer clock in hertz (=CPU in some platforms). */
    static uint64 ClockFrequency(){
        return  ProcessorClockFrequency();
    }

    /** The clock period in seconds. */
    static double ClockPeriod(){
        return  ProcessorClockPeriod();
    }

    /** The processor type. */
    static const char *Name(){
        return  ProcessorName();
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

