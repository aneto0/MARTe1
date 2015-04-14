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

/** 
   Used to describe the type pointed to by a pointer
   Depending on the first bit isStructuredData it may contain a code identifying a structure
   or the remaining bit can be used to identify a specific basic type
   basic types are ints 8-64 bit floats doubles char *
   basic types also include exotic definitions like 23 bit integers
   see BasicType for more information
*/
struct TypeDescriptor {
    /// The basic types that can be used 
    enum BasicType{
        /** An integer   pointer = intxx * 
            size is in bytes */
        SignedInteger         = 0,

        /** An integer   pointer = uintxx * 
            size is in bytes */
        UnsignedInteger       = 1,

        /** standard float   pointer = float32 or float64 * 
            size is in bytes */
        Float                 = 2,

        /** Pointers  
            size is of that of the native pointer
            size field is not used
         */
        Pointer               = 3,

        /** pointer to a zero terminated string   pointer = const char *
            size field is not used
        */
        CCString              = 8,

        /** pointer to a pointer to a zero terminated string that has been allocated using malloc 
            pointer = char ** 
            BTConvert will free and re allocate memory 
            size field is not used
        */
        PCString              = 9,

        /** pointer to an array of characters of size specified in size field 
            pointer = char[size] 
            string will be 0 terminated and (size-1) truncated
            size field is in bytes
        */
        CArray                = 10,

        /** BString class, 
            pointer = BasicString *
            it is a pointer to a single BString
            size field is not used
        */
        BasicString           = 11,

        /** StreamAble class, 
            size field is meaningless 
     	 */
        StreamAble            = 12

    };	
	
    /** 
	    if True then the data is a structure or class and its definition 
        has to be found in the ObjectRegistryDatabase
    */
    bool isStructuredData:1;

	/** 
	    the data is constant - cannot be written to
	*/
    bool isConstant:1;

    union {  // 14 bit unnamed union

        /** used to describe basic types */
        struct  {

            /** 
		the actual type of data
            */
            BasicType 		type:4;
          
            /** 
		the size in bytes or bits (bitset types)
            */
            unsigned int 	size:10;

        };
		
        /** identifies univoquely a record in the ObjectRegistryDatabase */
        unsigned int structuredDataIdCode:14;
		
    };

    /** Compare types */
    bool operator==(const TypeDescriptor &typeDescriptor) const {
        if(isStructuredData){
            return structuredDataIdCode == typeDescriptor.structuredDataIdCode;
        }
        bool sameType = (type == typeDescriptor.type);
        bool sameSize = (size == typeDescriptor.size);
        return (sameType && sameSize);
    }
} TypeDescriptor;

/// describes int0
const TypeDescriptor VoidType                   = {  False, False, {{ SignedInteger , 0}} };

/// describes int8
const TypeDescriptor SignedInteger8Bit          = {  False, False, {{ SignedInteger , 1}} };

/// describes uint8
const TypeDescriptor UnsignedInteger8Bit        = { False, False, {{ UnsignedInteger , 1}} };

/// describes int16
const TypeDescriptor SignedInteger16Bit         = { False, False, {{ SignedInteger , 2}} };

/// describes uint16
const TypeDescriptor UnsignedInteger16Bit       = { False, False, {{ UnsignedInteger , 2}} };

/// describes int32
const TypeDescriptor SignedInteger32Bit         = { False, False, {{ SignedInteger , 4}} };

/// describes uint32
const TypeDescriptor UnsignedInteger32Bit       = { False, False, {{ UnsignedInteger , 4}} };

/// describes int64
const TypeDescriptor SignedInteger64Bit         = { False, False, {{ SignedInteger , 8}} };

/// describes uint64
const TypeDescriptor UnsignedInteger64Bit       = { False, False, {{ UnsignedInteger , 8}} };

/// describes const int8
const TypeDescriptor ConstSignedInteger8Bit     = { False, True , {{ SignedInteger , 1}} };

#endif

