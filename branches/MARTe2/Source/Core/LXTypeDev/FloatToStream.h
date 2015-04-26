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

#if !defined FLOAT_TO_STREAM
#define FLOAT_TO_STREAM

#include "GeneralDefinitions.h"
#include "IntegerToStream.h"
#include "FormatDescriptor.h"
#include <math.h>




#define CHECK_AND_REDUCE(number,step,exponent)\
if (number >= 1E ## step){ \
	exponent+=step; \
	number *= 1E- ## step; \
} 
#define CHECK_AND_INCREASE(number,step,exponent)\
if (number <= 1E- ## step){ \
	exponent-=(step+1); \
	number *= 10E ## step; \
} 

// exponent is increased or decreased,not set
// support numbers up to quad precision
template <typename T> 
static inline void NormalizeFloatNumberPrivate(T &positiveNumber, int16 &exponent){
	// used internally 
	if (positiveNumber <= 0.0) return ;

	// check and normalize progressively following a logaritmic pattern
	if (positiveNumber >= 1.0){
		if (sizeof(T)>8){
            CHECK_AND_REDUCE(positiveNumber,2048,exponent)
            CHECK_AND_REDUCE(positiveNumber,1024,exponent)
            CHECK_AND_REDUCE(positiveNumber,512,exponent)
	    }
		if (sizeof(T)>4){
            CHECK_AND_REDUCE(positiveNumber,256,exponent)
            CHECK_AND_REDUCE(positiveNumber,128,exponent)
            CHECK_AND_REDUCE(positiveNumber,64,exponent)
		}
        CHECK_AND_REDUCE(positiveNumber,32,exponent)
        CHECK_AND_REDUCE(positiveNumber,16,exponent)
        CHECK_AND_REDUCE(positiveNumber,8,exponent)
        CHECK_AND_REDUCE(positiveNumber,4,exponent)
        CHECK_AND_REDUCE(positiveNumber,2,exponent)
        CHECK_AND_REDUCE(positiveNumber,1,exponent)
	} else {
		if (sizeof(T)>8){
			CHECK_AND_INCREASE(positiveNumber,2048,exponent)
		    CHECK_AND_INCREASE(positiveNumber,1024,exponent)
            CHECK_AND_INCREASE(positiveNumber,512,exponent)
	    }
		if (sizeof(T)>4){
            CHECK_AND_INCREASE(positiveNumber,256,exponent)
            CHECK_AND_INCREASE(positiveNumber,128,exponent)
            CHECK_AND_INCREASE(positiveNumber,64,exponent)
		}
        CHECK_AND_INCREASE(positiveNumber,32,exponent)
        CHECK_AND_INCREASE(positiveNumber,16,exponent)
        CHECK_AND_INCREASE(positiveNumber,8,exponent)
        CHECK_AND_INCREASE(positiveNumber,4,exponent)
        CHECK_AND_INCREASE(positiveNumber,2,exponent)
        CHECK_AND_INCREASE(positiveNumber,1,exponent)	
        CHECK_AND_INCREASE(positiveNumber,0,exponent)	
	}
}

// rapid calculation of 10 to n both positive and negative
// suports up to quad precision
template <typename T> 
static inline void FastPowerOf10Private(T &output,int16 exponent){
	T radix = 10.0;
	if (exponent < 0) {
		radix = 0.1;
		exponent = -exponent;
	}
	
	// double logaritmic approach
	// decompose exponent in sum of powers of 2	
	output = 1.0;
	// limit to range of quad precision (128 bits)
	uint16 mask = 0x2000;
	// loop through trailing zeroes
	while ((mask > 0) && (!(exponent & mask))) mask >>=1;
	// loop from first one
	while (mask > 0){
		// at each step calculates the square of the power so far
		output *= output;
		// if the bit is set then multiply or divide by 10 
		if (exponent & mask) output *= radix;
		mask >>=1;
	}
}

/// rapid determination of size of exponet part
static inline 
uint8 NumberOfDigitsOfExponent(int16 exponent){
	// no exponent!
	if (exponent == 0) return 0;
	
	// workout the size of exponent
	// the size of exponent is 2+expNDigits
	// sign of exponent is always produced
	int16 exponentNumberOfDigits = 3;// E+n

	// remove sign
	if (exponent < 0) exponent = - exponent;
	
	// work out size
	if (exponent < 100){ // below 100 is either 3 or 4
		if (exponent >= 10){
			exponentNumberOfDigits ++;
		} 
	} else { // above or equal 100 is at least 5
		exponentNumberOfDigits +=2;
		// just add one for each size step above
		if (exponent >= 1000){
			exponentNumberOfDigits ++;
		} 
		// just add one for each size step above
		if (exponent >= 10000){
			exponentNumberOfDigits ++;
		} 
	}	
	return exponentNumberOfDigits;
}

/** decompose an exponent into a multiple of 3 and a remainder part
 *  returns the multiple of 3
 *  exponent is modified to be the remiander
 *  original exponent is the sum of the 2 
 */
static inline int16 ExponentToEngineeringPrivate(int16 &exponent){
    int16 engineeringExponent = 0;
    // if negative we need to bias by 2 so that exp=-1 => eng_exp = -3 and not 0
    if (exponent < 0) engineeringExponent = (exponent-2) / 3;
	// if positive it is simply exp/3  
    else              engineeringExponent = exponent / 3;

    // multiply by 3 so that it is the actual exponent
    engineeringExponent *= 3;
    // calculate remainder 
    exponent = exponent - engineeringExponent;
    
    return engineeringExponent;
}

// calculate size of fixed numeric representation considering the zeros and the . needed beyond the significative digits 
// excludes from the size the eventual sign 
// precision is int16 to allow safe subtraction
// precision is updated to fit within maximumSize
// negative or zero precision means cannot fit
static inline uint8 NumberOfDigitsFixedNotation(int16 exponent,bool hasSign,int16 &precision, uint16 maximumSize){    
	uint8 fixedNotationSize = 0;	
	if (exponent >= 0){
		// fixed notation for large numbers needs a number of digits = 1+ exponent in this case no . is used 
		fixedNotationSize = exponent+1;
		// consider the sign
	    if (hasSign) fixedNotationSize++;
		
		// are we within limits even when disregarding precision?		
	    if (fixedNotationSize > maximumSize){
    		// less than 1 is meaningless
	    	fixedNotationSize = 1;
	    	// -1 means no space! 
	    	precision = -1;
	    } else {
			// if we need to go below zero consider also the . this  is why +1
		    if (fixedNotationSize < (precision-1)) fixedNotationSize = precision + 1;
			// consider the sign
		    if (hasSign) fixedNotationSize++;

			// are we within limits now that we have introduced precision?		
		    if (fixedNotationSize > maximumSize){
		    	// we adjust precision
		    	precision += (maximumSize-fixedNotationSize);
		    	// size depends on precision
		    	// if precision exceeds exponent than we consider the space for the . as well
		    	if (precision > (exponent+1)) fixedNotationSize = maximumSize;
		    	else 						  fixedNotationSize = maximumSize-1;
		    }
		    
	    }
	} else { // negative exponent 
		exponent = -exponent;
		// a precision 1 exp = -1 needs 3 (0.x) so add 1 
		fixedNotationSize = exponent+precision+1;
		// consider the sign
	    if (hasSign) fixedNotationSize++;
		
		// are we within limits?		
	    if (fixedNotationSize > maximumSize){
	    	// try reducing precision
	    	precision += (maximumSize - fixedNotationSize);
	    	if (precision >= 1){
	    		// fits to the limits
		    	fixedNotationSize = maximumSize;
	    	} else {
	    		// less than 1 is meaningless
		    	fixedNotationSize = 1;
		    	// -1 means no space! 
		    	precision = -1;
	    	}
	    }
	}
    return fixedNotationSize;
}

// calculate size of smart numeric representation considering the exponent and the desired precision
// excludes from the size the eventual sign 
// precision is int16 to allow safe subtraction
// precision is updated to fit within maximumSize
// negative or zero precision means cannot fit
static inline uint8 NumberOfDigitsExponentialNotation(int16 exponent,bool hasSign,int16 &precision, uint16 maximumSize){    
	//	exponential notation number size
	uint8 exponentNotationSize = 0;
	// include exponent size 
	exponentNotationSize += NumberOfDigitsOfExponent(exponent);    
	// include mantissa size 
    uint8 mantissaSize = NumberOfDigitsFixedNotation(0,hasSign,precision,maximumSize-exponentNotationSize);
    // does not fit
    if (precision < 0) return 1;
    return exponentNotationSize + mantissaSize;
}

// calculate size of engineering representation considering the exponent and the desired precision
// excludes from the size the eventual sign 
// precision is int16 to allow safe subtraction
// precision is updated to fit within maximumSize
// negative or zero precision means cannot fit
static inline uint8 NumberOfDigitsEngineeringNotation(int16 exponent,bool hasSign,int16 &precision, uint16 maximumSize){
		
	// decompose exponent in two parts 
	int16 exponentRemainder = exponent;
	int16 engineeringExponent = ExponentToEngineeringPrivate(exponentRemainder);

	uint8 engineeringNotationSize = 0;
	// include exponent size 
	engineeringNotationSize += NumberOfDigitsOfExponent(engineeringExponent);    
	// include mantissa size 
    uint8 mantissaSize = NumberOfDigitsFixedNotation(exponentRemainder,hasSign,precision,maximumSize-engineeringNotationSize);
    // does not fit
    if (precision < 0) return 1;
    return engineeringNotationSize+mantissaSize;
}

// calculate size of smart numeric representation considering the exponent and the desired precision
// excludes from the size the eventual sign 
// precision is int16 to allow safe subtraction
// precision is updated to fit within maximumSize
// negative or zero precision means cannot fit
static inline uint8 NumberOfDigitsSmartNotation(int16 exponent,bool hasSign, int16 &precision, uint16 maximumSize){
	
	// decompose exponent in two parts 
	int16 exponentRemainder = exponent;
	int16 engineeringExponent = ExponentToEngineeringPrivate(exponentRemainder);

	uint8 smartNotationSize = 0;
    // check if in range for smart replacement of exponent
    if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
    	// if so the exponent is simply a letter
    	smartNotationSize++;
    } else {
    	// or the whole E-xxx
    	smartNotationSize += NumberOfDigitsOfExponent(engineeringExponent);
    }
	// include mantissa size 
    uint8 mantissaSize = NumberOfDigitsFixedNotation(exponentRemainder,hasSign,precision,maximumSize-smartNotationSize );
    // does not fit
    if (precision < 0) return 1;
    
    return smartNotationSize + mantissaSize;
}	

/**
 * converts a couple of positiveNumber/exponent (or any other equivalent) to a string using fixed format
 * positiveNumber is not 0 nor Nan nor Inf and is positive
 * sizeLeft is the buffer size left
 * pBuffer is a writable area of memory of at least sizeLeft
 * precision determines the number of significative digits and is always not 0
 */
template <typename T, class streamer> 
bool FloatToFixedPrivate(
		streamer &		stream, 
		T 				positiveNumber,
		int16           exponent,
		uint8 			precision){

	// what is the exponent associated to the least significative figure?
	int16 leastSignificativeExponent = exponent - precision + 1;  
	
	// round up
	if (leastSignificativeExponent >= 0 ) positiveNumber += 0.5;
	else {
		// to round up add a correctionvalue just below last visible digit
		T correction;
		FastPowerOf10Private(correction,leastSignificativeExponent);
		correction *= 0.5;
		positiveNumber += correction;
	}
	
	// numbers below 1.0
	if (exponent < 0){

		stream.PutC('0');
		stream.PutC('.');
		
		// loop and add zeros
		int i;
		for (i = 0; i < -(exponent+1); i++){
			stream.PutC('0');
		}
		// exponent has only the job of marking where to put the '.'
		// here is lowered by 1 to avoid adding a second '.' in the following code
		exponent--;
	} 
	
	// loop to fulfil precision 
	// also must reach the end of the integer part thus exponent is checked
	while ((exponent > 0) || (precision > 0) ){
		// before outputting the fractional part add a '.'
		if (exponent == -1) stream.PutC('.');

		// get a digit and shift the number
		uint8 digit = (uint8)(positiveNumber );
		positiveNumber -= digit;
		positiveNumber *= 10.0;

		stream.PutC('0'+ digit);

		// update precision and exponent
		if (precision > 0) precision--;
		exponent--;
	}
	return true;
}

/**
Encodes the exponent in the classical form E+/-nn
 */
template <class streamer> 
static inline void ExponentToStreamPrivate(
		streamer &		stream, 
		int16           exponent
){
    // output exponent if exists
    if (exponent != 0){
		stream.PutC('E');
		// print the exponent sign (both)
		// get the absolute value
		if (exponent > 0){
			stream.PutC('+');
		} else {
			exponent = -exponent;
			stream.PutC('-');
		}
		// fast convert to int
		Number2StreamDecimalNotationPrivate(stream, exponent);
    }
}


/// to manage the behaviour of the function
enum FloatDisplayModes{
	FixedFloat               =0,
	ExponentialFloat         =1,
	EngineeringFloat         =2,
	SmartFloat               =3,
	
	ZeroFloat                =11,
	NanFloat                 =22,
	InfPFloat                =33,
	InfNFloat                =44,
	InsufficientSpaceForFloat=77, // not enough space
	NoFormat                 =99
}  chosenMode =  NoFormat;

/**
 * performs standard operations for all float representations
 * 
 */
template <typename T> 
FloatDisplayModes CheckNumber(
		T 				number,
		uint16   		maximumSize,
		uint8 &			neededSize){
	
	if (isnan(number)) {
		if (maximumSize < 3) {
			neededSize = 1;
			return InsufficientSpaceForFloat;
		}
		neededSize = 3;
		return NanFloat;
	}

	if (isinf(number)) {
		if (maximumSize < 4) {
			neededSize = 1;
			return InsufficientSpaceForFloat;
		}
		neededSize = 4;
		if (number < 0)   return InfNFloat;
		return InfNFloat;
	}

	if (number == 0) {
		neededSize = 1;
		return ZeroFloat;
	}

	neededSize = 0;
	return NoFormat;
}



/**
 * converts a float/double (or any other equivalent) to a string using whatever format achieves best compact representation
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T, class streamer> 
bool FloatToStream(
		streamer &		    stream,                        // must have a GetC(c) function where c is of a type that can be obtained from chars  
		T 				    number,
		FormatDescriptor	format)
{

	
	uint16 maximumSize = format.size;
	// 1000 should not constitute a limit
	if (maximumSize == 0) maximumSize = 1000;
	
	uint8 precision = format.precision;
	// on precision 0 the max useful precision is chosen
	if (precision == 0){
		if (sizeof(T)  > 8)precision = 34; 
		if (sizeof(T) == 8)precision = 15;
		if (sizeof(T) <  8)precision = 7;
	}
	
	
	// this is the second main objective of the first part
	// to find out the size 
	uint8 numberSize; 

	// this will be used everywhere!
	T positiveNumber = number;

	// hold the exponent after normalisation
	int16 exponent = 0;
	
	// whether the - needs to be output
	bool hasSign = false;
	
	// adjust sign 
	if (number < 0){
		hasSign = true;
		positiveNumber = - number;
	}
	
	// check for special cases where notation is not relevant
	// if found them then mode and size are assigned
	chosenMode = CheckNumber(number,maximumSize,numberSize);
	
	// no chosen mode yet try all formats
	if (chosenMode == NoFormat){
				
		// normalize number
		NormalizeFloatNumberPrivate(positiveNumber,exponent);
    	
        // precision 0 means no significant bits		
		int16 chosenPrecision = 0;
		// just the space for '?'
	    numberSize = 1;	   
	    // assume the worst 
	    chosenMode = InsufficientSpaceForFloat;

	    
		int16 testPrecision;
	    uint16 testFormatSize;
	    // if selected fixed notation or compact then test effectiveness of fixed 
	    if ((format.floatNotation == Notation::FixedPointNotation) || (format.floatNotation == Notation::CompactNotation)){
	    	// obtain size and associated realizable precision
	    	testPrecision = precision;
	    	testFormatSize = NumberOfDigitsFixedNotation(exponent,hasSign,testPrecision,maximumSize);
	    	// if the number fits then precision is >=1
	    	if (testPrecision > chosenPrecision){	    		
	    		chosenMode = FixedFloat;
	    		chosenPrecision = testPrecision;
	    		numberSize = testFormatSize; 
	    	}
	    }
	    
	    if ((format.floatNotation == Notation::EngineeringNotation) || (format.floatNotation == Notation::CompactNotation)){ 		
	    	testPrecision = precision;
	    	testFormatSize = NumberOfDigitsEngineeringNotation(exponent,hasSign,testPrecision,maximumSize);
	    	if (testPrecision > chosenPrecision){
	    		chosenMode = EngineeringFloat;
	    		chosenPrecision = testPrecision;
	    		numberSize = testFormatSize; 
	    	}
	    }

	    if ((format.floatNotation == Notation::ExponentNotation) || (format.floatNotation == Notation::CompactNotation)){ 		
	    	testPrecision = precision;
	    	testFormatSize = NumberOfDigitsExponentialNotation(exponent,hasSign,testPrecision,maximumSize);
	    	if (testPrecision > chosenPrecision){
	    		chosenMode = ExponentialFloat;
	    		chosenPrecision = testPrecision; 
	    		numberSize = testFormatSize; 
	    	}
	    }
		
	    if  (format.floatNotation == Notation::CompactNotation){ 		
	    	testPrecision = precision;
	    	testFormatSize = NumberOfDigitsSmartNotation(exponent,hasSign,testPrecision,maximumSize);
	    	if (testPrecision > chosenPrecision){
	    		chosenMode = SmartFloat;
	    		chosenPrecision = testPrecision; 
	    		numberSize = testFormatSize; 
	    	}
	    }    
	}

	// in case of left alignment
	if (format.padded && !format.leftAligned){
		for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
	}
	
    switch (chosenMode){
        case FixedFloat:{
        	// output sign
        	if (hasSign)stream.PutC('-');

        	// does all the work of conversion but for the sign and special cases
        	FloatToFixedPrivate(stream,positiveNumber,exponent,precision);
            
        }break;
        case ExponentialFloat:{
        	// output sign
        	if (hasSign)stream.PutC('-');
    
            // does all the work of conversion but for the sign and special cases
        	FloatToFixedPrivate(stream,positiveNumber, 0,precision);

            // writes exponent
        	ExponentToStreamPrivate(stream, exponent);
            
        }break;
        case EngineeringFloat:{
        	// output sign
        	if (hasSign)stream.PutC('-');

//printf(" %i ",exponent)  ;          

        	// partitions the exponent between engineering part and residual 
            int16 engineeringExponent = ExponentToEngineeringPrivate(exponent); 

//printf("%i %i \n",engineeringExponent,exponent)   ;         
    
            // does all the work of conversion but for the sign and special cases
            FloatToFixedPrivate(stream,positiveNumber,exponent,precision);

            // writes exponent
            ExponentToStreamPrivate(stream, engineeringExponent);
            
        }break;
        case SmartFloat:{
        	// output sign
        	if (hasSign)stream.PutC('-');

        	// partitions the exponent between engineering part and residual 
            int16 engineeringExponent = ExponentToEngineeringPrivate(exponent); 
    
            // does all the work of conversion but for the sign and special cases
            FloatToFixedPrivate(stream,positiveNumber, exponent,precision);
            // check if exponent in correct range
            if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
                static const char *symbols = "pnum kMGT";
                stream.PutC(symbols[engineeringExponent/3]);
            } else {
                // writes exponent
             	ExponentToStreamPrivate(stream, engineeringExponent);
            }            

        }break;
        case NoFormat:
        case InsufficientSpaceForFloat:{
        	stream.PutC('?');
        }break;
        case NanFloat:{
        	stream.PutC('N');
        	stream.PutC('a');
        	stream.PutC('N');
        }break;
        case InfPFloat:{
        	stream.PutC('+');
        	stream.PutC('I');
        	stream.PutC('n');
        	stream.PutC('f');
        }break;
        case InfNFloat:{
        	stream.PutC('-');
        	stream.PutC('I');
        	stream.PutC('n');
        	stream.PutC('f');
        }break;
        case ZeroFloat:{
        	stream.PutC('0');
        }break;
    }

	// in case of right alignment
	if (format.padded && format.leftAligned){
		for (int i = numberSize;i < maximumSize;i++) stream.PutC(' ');
	}
    
    
    return true;
}

#endif	
