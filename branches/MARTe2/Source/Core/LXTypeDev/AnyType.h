#if !defined (TYPED_POINTER)
#define TYPED_POINTER

#include "GeneralDefinitions.h"
#include "TypeDescriptor.h"

/**
   a pointer and a type descriptor
   used to process data on the basis of its type
*/
class AnyType{
public:
	
    /// data may be RW or RO
    void * 				dataPointer;

	/// what type is the data 
	TypeDescriptor		dataDescriptor;

	/// used only for bit range types
    unsigned int        bitAddress:3;	
	
public:
	/// constructor from an integer 8 bit
    AnyType(const int8 &i){
		dataPointer = (void *) &i; 
		dataDescriptor = TDConstSignedInteger8Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(int8 &i){
		dataPointer = &i;
		dataDescriptor = TDSignedInteger8Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint8 &i){
		dataPointer = &i;
		dataDescriptor = TDUnsignedInteger8Bit;
	}   	
	/// constructor from an integer 8 bit
    AnyType(int16 &i){
		dataPointer = &i;
		dataDescriptor = TDSignedInteger16Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint16 &i){
		dataPointer = &i;
		dataDescriptor = TDUnsignedInteger16Bit;
	}   	
};
#endif

