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
#ifndef ANY_TYPE_H
#define ANY_TYPE_H

#include "GeneralDefinitions.h"
#include "TypeDescriptor.h"

#include <stdio.h>

/**
   a pointer and a type descriptor
   used to process data on the basis of its type
*/
class AnyType{
public:
	
    /// data may be RW or RO
	void * 				dataPointer;

	/// what type is the data 
    TypeDescriptor      dataDescriptor;
 
	/** 
	 * used only for bit range types
	 * range is 0 - 255 
	 */
    uint8               bitAddress;	
	
public:
	/// constructor from an integer 8 bit
    AnyType(void){
		dataPointer 		= NULL; 
		bitAddress  		= 0;
		dataDescriptor 		= VoidType;
	}   	
    
    ///
    AnyType(const AnyType &x){
		dataPointer 		= x.dataPointer; 
		bitAddress  		= x.bitAddress;
		dataDescriptor 		= x.dataDescriptor;
	}   	
    
    /// check for void type
    bool IsVoid() const{
    	return (dataDescriptor == VoidType);
    }

    /// constructor from a float
    AnyType(float &i){
		dataPointer = (void *) &i; 
		bitAddress  = 0;
		dataDescriptor = Float32Bit;
	}   
    
    /// constructor from a float
    AnyType(const float &i){
		dataPointer = (void *) &i; 
		bitAddress  = 0;
		dataDescriptor = Float32Bit;
		dataDescriptor.isConstant = true;
	}   	

    /// constructor from a float
    AnyType(double &i){
		dataPointer = (void *) &i; 
		bitAddress  = 0;
		dataDescriptor = Float64Bit;
	}   	

    /// constructor from a float
    AnyType(const double &i){
		dataPointer = (void *) &i; 
		bitAddress  = 0;
		dataDescriptor = Float64Bit;
		dataDescriptor.isConstant = true;
	}   	

    /// constructor from an integer 8 bit
    AnyType(const int8 &i){
		dataPointer = (void *) &i; 
		bitAddress  = 0;
		dataDescriptor = SignedInteger8Bit;
		dataDescriptor.isConstant = true;
	}   	

	/// constructor from an integer 8 bit
    AnyType(int8 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = SignedInteger8Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint8 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = UnsignedInteger8Bit;
	}   	
	/// constructor from an integer 8 bit
    AnyType(int16 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = SignedInteger16Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint16 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = UnsignedInteger16Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint32 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = UnsignedInteger32Bit;
	}   
    
	/// constructor from an integer 8 bit
    AnyType(int32 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = SignedInteger32Bit;
	}   
    
	/// 
    AnyType(const int32 &i){
		dataPointer = (void *)&i;
		bitAddress  = 0;
		dataDescriptor = SignedInteger32Bit;
		dataDescriptor.isConstant = true;
	}   
    
	/// constructor from an integer 64 bit
    AnyType(uint64 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = UnsignedInteger64Bit;
	}   
    
	/// constructor from an integer 64 bit
    AnyType(int64 &i){
		dataPointer = &i;
		bitAddress  = 0;
		dataDescriptor = SignedInteger64Bit;
	}   
    
	/// 
    AnyType(const int64 &i){
		dataPointer = (void *)&i;
		bitAddress  = 0;
		dataDescriptor = SignedInteger64Bit;
		dataDescriptor.isConstant = true;
	}
	/// 
    AnyType(const void * p){
		dataPointer = (void *)p;
		bitAddress  = 0;
		dataDescriptor.type = TypeDescriptor::Pointer;
		dataDescriptor.isConstant = true;
		dataDescriptor.size = 8 * sizeof(void *);	
	} 
   
	/// 
    AnyType(void * p){
		dataPointer = (void *)p;
		bitAddress  = 0;
		dataDescriptor.type = TypeDescriptor::Pointer;
		dataDescriptor.isConstant = false;
		dataDescriptor.size = 8 * sizeof(void *);
	}
    
	/// 
    AnyType(const char *p){
		dataPointer = (void *)p; // we will either print the variable or the string
		bitAddress  = 0;
		dataDescriptor.type = TypeDescriptor::CCString;
		dataDescriptor.isConstant = true;
	} 
};

// void data marker
static const AnyType voidAnyType;

#endif

