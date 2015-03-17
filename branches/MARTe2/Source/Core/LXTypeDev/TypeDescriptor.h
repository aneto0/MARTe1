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
 * $Id: $
 *
 **/
/**
 * @file
 */
#ifndef TYPE_DESCRIPTOR
#define TYPE_DESCRIPTOR

#include "BasicType.h"

/** 
   Used to describe the type pointed to by a pointer
   Depending on the first bit isStructuredData it may contain a code identifying a structure
   or the remaining bit can be used to identify a specific basic type
   basic types are ints 8-64 bit floats doubles char *
   basic types also include exotic definitions like 23 bit integers
   see BasicType for more information
*/
typedef struct TypeDescriptor {

    /** 
	    if True then the data is a structure or class and its definition 
        has to be found in the ObjectRegistryDatabase
    */
    bool isStructuredData:1;

	/** 
	    the data is constant - cannot be written to
	*/
    bool isConstant:1;

    union {

        /** used to describe basic types */
        struct  {

            /** 
		the actual type of data
            */
            BasicType::Enum type:4;
          
            /** 
		the size in bytes or bits (bitset types)
            */
            unsigned int size:10;

        };
		
        /** identifies univoquely a record in the ObjectRegistryDatabase */
        unsigned int structuredDataIdCode:14;
		
    };

    bool operator==(const TypeDescriptor &typeDescriptor) const {
        if(isStructuredData){
            return structuredDataIdCode == typeDescriptor.structuredDataIdCode;
        }
        bool sameType = (type == typeDescriptor.type);
        bool sameSize = (size == typeDescriptor.size);
        return (sameType && sameSize);
    }
} TypeDescriptor;

/// describes int8
const TypeDescriptor SignedInteger8Bit          = {  False, False, {{ BasicType::SignedInteger , 1}} };

/// describes uint8
const TypeDescriptor UnsignedInteger8Bit        = { False, False, {{ BasicType::UnsignedInteger , 1}} };

/// describes int16
const TypeDescriptor SignedInteger16Bit         = { False, False, {{ BasicType::SignedInteger , 2}} };

/// describes uint16
const TypeDescriptor UnsignedInteger16Bit       = { False, False, {{ BasicType::UnsignedInteger , 2}} };

/// describes int32
const TypeDescriptor SignedInteger32Bit         = { False, False, {{ BasicType::SignedInteger , 4}} };

/// describes uint32
const TypeDescriptor UnsignedInteger32Bit       = { False, False, {{ BasicType::UnsignedInteger , 4}} };

/// describes int64
const TypeDescriptor SignedInteger64Bit         = { False, False, {{ BasicType::SignedInteger , 8}} };

/// describes uint64
const TypeDescriptor UnsignedInteger64Bit       = { False, False, {{ BasicType::UnsignedInteger , 8}} };

/// describes const int8
const TypeDescriptor ConstSignedInteger8Bit     = { False, True , {{ BasicType::SignedInteger , 1}} };

#endif

