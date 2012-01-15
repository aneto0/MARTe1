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
 * @brief Provides the Signal interface definition
 *
 * Describes the minimum interface requirements to have a signal
 */
#ifndef _SIGNAL_INTERFACE
#define _SIGNAL_INTERFACE
#include "GCNamedObject.h"
#include "FString.h"
#include "BasicTypes.h"
#include "CDBExtended.h"

class SignalInterface{
public:
    virtual bool CopyData(
                BasicTypeDescriptor         type            = BTDInt32,
                uint32                      numberOfSamples = 0,
                const void *                buffer          = NULL,
		MemoryAllocationFlags       allocFlags      = MEMORYStandardMemory
            ) = 0;
    
    virtual bool ReferData(
                BasicTypeDescriptor         type            = BTDInt32,
                uint32                      numberOfSamples = 0,
                void *                      buffer          = NULL
            ) = 0;
    
    /** how many samples */
    virtual uint32 NumberOfSamples() const = 0;
    
    /** the type of the data */
    virtual BasicTypeDescriptor     Type() const = 0;
    
    /** the data */
    virtual const void *            Buffer() const = 0;    
};
#endif

