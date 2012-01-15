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
 * @brief A generic signal definition.
 */
#ifndef _SIGNAL_DEFINITION
#define _SIGNAL_DEFINITION

#include "GCNamedObject.h"
#include "FString.h"
#include "BasicTypes.h"
#include "CDBExtended.h"
#include "SignalInterface.h"

OBJECT_DLL(Signal)

/** A signal : a ordered set of numbers(objects) */
class Signal: public SignalInterface, public GCNamedObject{

    OBJECT_DLL_STUFF(Signal);

private:

    /** Signal Type Descriptor */
    BasicTypeDescriptor         type;

    /** Signal Number Of samples */
    int32                       numberOfSamples;

    /** size of buffer as bytes
        or < 0 if not allocated but referred */
    int32                       bufferByteSize;

    /** Buffer containing signal data */
    void *                      buffer;

private:

    /** re-allocate memory
        returns False on error  */

    bool                    Allocate(
            BasicTypeDescriptor         type,
            uint32                      numberOfSamples,
	    MemoryAllocationFlags       allocFlags = MEMORYStandardMemory);

    /** use an existing allocated buffer  */
    bool                    Refer(
            BasicTypeDescriptor         type,
            uint32                      numberOfSamples,
            void *                      buffer);

public:

    /** constructor */
                            Signal();

    /** destructor */
    virtual                 ~Signal();

    /** initialise the object by copying from a buffer */
    bool                    CopyData(
  	        BasicTypeDescriptor         type            = BTDInt32,
		uint32                      numberOfSamples = 0,
                const void *                buffer          = NULL,
		MemoryAllocationFlags       allocFlags      = MEMORYStandardMemory
	    );

    /** initialise the object by referring to an existing buffer */
    bool                    ReferData(
                BasicTypeDescriptor         type            = BTDInt32,
                uint32                      numberOfSamples = 0,
                void *                      buffer          = NULL
            );

    /** copy constructor */
                            Signal(
                const Signal &              signal);

    /** Initialise the object from a cdb */
    virtual     bool        ObjectLoadSetup(
                ConfigurationDataBase &     info,
                StreamInterface *           err);

    /** Save the object to a cdb */
    virtual     bool        ObjectSaveSetup(
                ConfigurationDataBase &     info,
                StreamInterface *           err);

    /** how many samples */
    virtual uint32                  NumberOfSamples() const
    {
        return numberOfSamples;
    }

    /** the type of the data */
    virtual BasicTypeDescriptor     Type() const
    {
        return type;
    }

    /** the data */
    virtual const void *            Buffer() const
    {
        return buffer;
    }

};

#endif
