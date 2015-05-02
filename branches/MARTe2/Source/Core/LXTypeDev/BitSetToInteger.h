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
 * $Id: CStream.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 * @file 
 * @brief Functions to print integer numbers on generic streams.
*/
#if !defined BITSET_TO_INTEGER
#define BITSET_TO_INTEGER

#include "GeneralDefinitions.h"
#include "FormatDescriptor.h"
#include <math.h>
#include "DoubleInteger.h"


template <typename T>
void BSToBS(
    T *&                destination,
    uint8  &            destinationBitShift,
    uint8               destinationBitSize,
    bool                destinationIsSigned,
    T *&                source,
    uint8  &            sourceBitShift,
    uint8               sourceBitSize,
    bool                sourceIsSigned)
{
	
	uint8 dataSize = sizeof(T)*8;
	
	T sourceMask           = 0 ;
	sourceMask 			   =~sourceMask;
	sourceMask 			   >>= (dataSize - sourceBitSize);
	
	T destinationMask      = 0;
	destinationMask 	   =~destinationMask;
	destinationMask 	   >>= (dataSize - destinationBitSize);
	
	T sourceSignMask       = 1;
	sourceSignMask         <<= (sourceBitSize - 1);
	
	T destinationSignMask  = 1;
	destinationSignMask    <<= (destinationBitSize - 1);
	
	T sourceCopy;
	
	sourceCopy = *source;
	sourceCopy >>= sourceBitShift;	
	sourceCopy &= sourceMask;

	T signBit = sourceCopy & sourceSignMask;
	
	// is negative if it has sign and last bit is 1
	bool sourceIsNegative = sourceIsSigned && (signBit != 0);

	// this means both that has sign and that the last bit is 1
	if (sourceIsNegative ){
		// if destination is not signed saturates to 0
		if (!destinationIsSigned){
			sourceCopy = 0;
		} else {
			// if I need to squeeze 
			if (sourceBitSize > destinationBitSize){
				// create a mask of bits covering the bits where source exceeds destination 
				T mask = sourceMask - destinationMask;
				// if any of these bits is not 1 then we have a larger negative number
				if ((sourceCopy & mask) != mask){
					// 0x8000.. is the maximum negative
					sourceCopy = destinationSignMask;
				}					
			} 
		}			
	} else {
		// 0xFFF.. is the max value for unsigned
		T maxPositiveValue = destinationMask; 
		if (destinationIsSigned){
			// 0x7FF.. is the max value for signed
			maxPositiveValue = (destinationMask >> 1);
		} 
		// saturate to max
		if (sourceCopy > maxPositiveValue){
			sourceCopy = maxPositiveValue;					
		}
	}
	
	sourceCopy <<= destinationBitShift;
	
	destinationMask <<= destinationBitShift;
	
	destinationMask = ~destinationMask;
	destinationMask &= *destination;
	sourceCopy |= destinationMask;
	
	*destination = sourceCopy;  

}

/**
 * Converts an integer of bitSize sourceBitSize 
 * located at address source and bitAddress sourceBitShift
 * into an integer of bitSize destinationBitSize 
 * located at address destination and bitAddress destinationBitShift
 * 
 * T must be of byte size power of 2 (8,16,32,64,128...) not 24 48 etc...
 */
template <typename T>
static inline bool BitSetToBitSet(
        T *&                destination,
        uint8  &            destinationBitShift,
        uint8               destinationBitSize,
        bool                destinationIsSigned,
        T *&                source,
        uint8  &            sourceBitShift,
        uint8               sourceBitSize,
        bool                sourceIsSigned)
{
	
	
	int16 granularity      = sizeof(T) * 8;
	int16 granularityMask  = granularity - 1;
	int16 granularityShift = 3;
	int16 temp = sizeof(T);
	while (temp > 1){
		granularityShift++;
		temp >>=1;
	}
		
	if (sourceBitShift >= granularity){
		source += (sourceBitShift >> granularityShift);
		sourceBitShift &= granularityMask;
	} 
	if (destinationBitShift >= granularity){
		destination += (destinationBitShift >> granularityShift);
		destinationBitShift &= granularityMask;
	}

	
	int sourceBitEnd      =  sourceBitShift     +destinationBitSize;
	int destinationBitEnd =  destinationBitShift+sourceBitSize;
	
	if ((sourceBitEnd < 8) && (destinationBitEnd < 8) && (granularity == 8)){

		uint8 *destination8 = (uint8 *)destination;
		uint8 *source8      = (uint8 *)source;

		BSToBS(destination8,destinationBitShift,destinationBitSize,destinationIsSigned,source8,sourceBitShift,sourceBitSize,sourceIsSigned);	
		
	} else if ((sourceBitEnd < 16) && (granularity <= 16)){
		uint16 *destination16 = (uint16 *)destination;
		uint16 *source16      = (uint16 *)source;

		BSToBS(destination16,destinationBitShift,destinationBitSize,destinationIsSigned,source16,sourceBitShift,sourceBitSize,sourceIsSigned);
			
	} else if ((sourceBitEnd < 32) && (granularity <= 32)){
		uint32 *destination32 = (uint32 *)destination;
		uint32 *source32      = (uint32 *)source;

		BSToBS(destination32,destinationBitShift,destinationBitSize,destinationIsSigned,source32,sourceBitShift,sourceBitSize,sourceIsSigned);
	
	} else if ((sourceBitEnd < 64) && (granularity <= 64)){
		uint64 *destination64 = (uint64 *)destination;
		uint64 *source64      = (uint64 *)source;

		BSToBS(destination64,destinationBitShift,destinationBitSize,destinationIsSigned,source64,sourceBitShift,sourceBitSize,sourceIsSigned);
	}
    else if ((sourceBitEnd < 128) && (granularity <= 128)){
    	DoubleInteger<uint64> *destination128  = (DoubleInteger<uint64> *)destination;
    	DoubleInteger<uint64> *source128       = (DoubleInteger<uint64> *)source;
    	BSToBSPrivate(destination128,destinationBitShift,destinationBitSize,destinationIsSigned,source128,sourceBitShift,sourceBitSize,sourceIsSigned);
	}
	
	sourceBitShift      += sourceBitSize;
	destinationBitShift += destinationBitSize;

	return true;
}


/**
 * T,T2 must be of byte size power of 2 (8,16,32,64,128...) not 24 48 etc...
 */
template <typename T,typename T2>
static inline bool BitSetToInteger(
        T2 &                dest,
        T *&                source,
        uint8  &            sourceBitShift,
        uint8               sourceBitSize,
        bool                sourceIsSigned)
{

    T *destination = (T*) &dest;
    uint8 destinationBitShift = 0;
    uint8 destinationBitSize = sizeof(T2) *8;
    bool destinationIsSigned = (((T2)-1) < 0); 

    return BitSetToBitSet(destination,destinationBitShift,destinationBitSize,destinationIsSigned,source,sourceBitShift,sourceBitSize,sourceIsSigned);
}

/**
 * T,T2 must be of byte size power of 2 (8,16,32,64,128...) not 24 48 etc...
 */
template <typename T,typename T2>
static inline bool IntegerToBitSet(
        T *&                destination,
        uint8  &            destinationBitShift,
        uint8               destinationBitSize,
        bool                destinationIsSigned,
        T2 &               src)
{

    T *source = (T*) &src;
    uint8 sourceBitShift = 0;
    uint8 sourceBitSize = sizeof(T2) *8;
    bool sourceIsSigned = (((T2)-1) < 0); 

    return BitSetToBitSet(destination,destinationBitShift,destinationBitSize,destinationIsSigned,source,sourceBitShift,sourceBitSize,sourceIsSigned);
}


#endif