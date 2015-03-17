#if !defined (TYPE_DESCRIPTOR)
#define TYPE_DESCRIPTOR


#include "BasicTypeEnumerated.h"


/** 
   Used to describe the type pointed to by a pointer
   Depending on the first bit isStructuredData it may contain a code identifying a structure
   or the remaining bit can be used to identify a specific basic type
   basic types are ints 8-64 bit floats doubles char *
   basic types also include exotic definitions like 23 bit integers
   see BasicTypeEnumerated for more information
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
            BasicTypeEnumerated type:4;
          
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
        return (sameType && sameType);
    }
} TypeDescriptor;

/// describes int8
const TypeDescriptor TDSignedInteger8Bit          = {  false, false, {{ BTDSignedInteger , 1}} };

/// describes uint8
const TypeDescriptor TDUnsignedInteger8Bit        = { false, false, {{ BTDUnsignedInteger , 1}} };

/// describes int16
const TypeDescriptor TDSignedInteger16Bit         = { false, false, {{ BTDSignedInteger , 2}} };

/// describes uint16
const TypeDescriptor TDUnsignedInteger16Bit       = { false, false, {{ BTDUnsignedInteger , 2}} };

/// describes int32
const TypeDescriptor TDSignedInteger32Bit         = { false, false, {{ BTDSignedInteger , 4}} };

/// describes uint32
const TypeDescriptor TDUnsignedInteger32Bit       = { false, false, {{ BTDUnsignedInteger , 4}} };

/// describes int64
const TypeDescriptor TDSignedInteger64Bit         = { false, false, {{ BTDSignedInteger , 8}} };

/// describes uint64
const TypeDescriptor TDUnsignedInteger64Bit       = { false, false, {{ BTDUnsignedInteger , 8}} };

/// describes const int8
const TypeDescriptor TDConstSignedInteger8Bit     = { false, true , {{ BTDSignedInteger , 1}} };

#endif

