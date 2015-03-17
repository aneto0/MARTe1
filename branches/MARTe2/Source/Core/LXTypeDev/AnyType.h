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
#ifndef TYPED_POINTER_H
#define TYPED_POINTER_H

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
		dataDescriptor = ConstSignedInteger8Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(int8 &i){
		dataPointer = &i;
		dataDescriptor = SignedInteger8Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint8 &i){
		dataPointer = &i;
		dataDescriptor = UnsignedInteger8Bit;
	}   	
	/// constructor from an integer 8 bit
    AnyType(int16 &i){
		dataPointer = &i;
		dataDescriptor = SignedInteger16Bit;
	}   	

	/// constructor from an integer 8 bit
    AnyType(uint16 &i){
		dataPointer = &i;
		dataDescriptor = UnsignedInteger16Bit;
	}   	
};
#endif

