/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
   permissions and limitations under the Licence. 
 *
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 * @file
 * Access processor's information
 */
#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "GeneralDefinitions.h"

extern "C" {

    uint32 ProcessorFamily();

    const char *ProcessorVendorId();

    uint32 ProcessorsAvailable();

    uint32 ProcessorModel();
}

/** Defines some methods to get information about the processor. */
class Processor {
public:

    /** The processor type. */
    static inline const char *VendorId(){
        return  ProcessorVendorId();
    }

    /** The processor family */
    static inline uint32 Family(){
        return  ProcessorFamily();
    }

    /** The processor family */
    static inline uint32 Model(){
        return  ProcessorModel();
    }

    /** The number of cpus avaible */
    static inline uint32 Available(){
        return ProcessorsAvailable();
    }
};

#endif

