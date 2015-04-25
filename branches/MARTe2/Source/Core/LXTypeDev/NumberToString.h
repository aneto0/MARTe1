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

#if !defined NUMBER_TO_STRING
#define NUMBER_TO_STRING

#include "GeneralDefinitions.h"
#include <math.h>


// returns the exponent
// positiveNumber is the abs (number)
// operates by comparing with 10**N with converging by bisection to the correct value
// returns the number of digits after the first or the exponent
template <typename T> uint8 GetOrderOfMagnitude(T positiveNumber){       
    T tenToExponent = 1;
    T temp ;
    uint8 exp = 0;
    // check whether exponent greater than 10  
    if (sizeof(T)>=8){ // max 19
        temp = tenToExponent * 10000000000; // 10 zeros 
        if (positiveNumber >= temp )  {
            tenToExponent = temp;
            exp += 10;
        }
    }
    
    // check whether exponent greater than 5  
    if (sizeof(T)>=4){ // max 9 
        temp = tenToExponent * 100000; // 5 zeros
        if (positiveNumber >= temp ) {
            tenToExponent = temp;
            exp += 5;
        }
    }
    
    // check whether exponent greater than 2  
    if (sizeof(T)>=2){ // max 4 zeros
        temp = tenToExponent * 100; // 2 zeros
        if (positiveNumber >= temp ) {
            tenToExponent = temp;
            exp += 2;
        }
    }
    
    // check whether exponent greater than 1  
    temp = tenToExponent * 10; // 1 
    if (positiveNumber >= temp ){
            tenToExponent = temp;
            exp ++;
    }
    
    // check whether exponent greater than 1  
    temp = tenToExponent * 10;  // 1
    // avoid overflowing in case of signed number
    if (temp > tenToExponent){
    	if (positiveNumber >= temp ){
            tenToExponent = temp;
            exp ++;
        }
    }
    return exp;
}

// returns the number of digits necessary to represent this number -1 
// positiveNumber is the abs (number)
template <typename T> uint8 GetOrderOfMagnitudeHex(T positiveNumber){
	uint8 exp = 0;

	// check if larger than 2**32
	if (sizeof(T)==8)
		if  (positiveNumber >= 0x100000000){
			exp += 8;
			positiveNumber >>= 32;
		}

	// check if larger than 2**16
	if (sizeof(T)>=4)
		if  (positiveNumber >= 0x10000){
			exp += 4;
			positiveNumber >>= 16;
		}

	// check if larger than 2**8
	if (sizeof(T)>=2)
		if  (positiveNumber >= 0x100){
			exp += 2;
			positiveNumber >>= 8;
		}

	// check if larger than 2**4
	if  (positiveNumber >= 0x10){
		exp += 1;
		positiveNumber >>= 4;
	}

    return exp;
}

// returns the number of digits necessary to represent this number -1 
// positiveNumber is the abs (number)
template <typename T> uint8 GetOrderOfMagnitudeOct(T positiveNumber){
	uint8 exp = 0;
	if (sizeof(T)==8)
		if  (positiveNumber >= 0x1000000000000){
			exp += 16;
			positiveNumber >>= 48;
		} 

	if (sizeof(T)>=4)
		if  (positiveNumber >= 0x1000000){
			exp += 8;
			positiveNumber >>= 24;
		}

	// check if larger than 2**12
	if (sizeof(T)>=2)
		if  (positiveNumber >= 0x1000){
			exp += 4;
			positiveNumber >>= 12;
		}

	// check if larger than 2**6
	if  (positiveNumber >= 0x40){
		exp += 2;
		positiveNumber >>= 6;
	}

	// check if larger than 2**2
	if  (positiveNumber >= 0x8){
		exp += 1;
		positiveNumber >>= 3;
	}

    return exp;
}


// returns the number of digits necessary to represent this number -1 
// positiveNumber is the abs (number)
template <typename T> uint8 GetOrderOfMagnitudeBin(T positiveNumber){
	uint8 exp = 0;

	// check if larger than 2**32
	// if so shift 
	if (sizeof(T)==8)
		if  (positiveNumber >= 0x100000000){
			exp += 32;
			positiveNumber >>= 32;
		}

	// check if larger than 2**16
	if (sizeof(T)>=4)
		if  (positiveNumber >= 0x10000){
			exp += 16;
			positiveNumber >>= 16;
		}

	// check if larger than 2**8
	if (sizeof(T)>=2)
		if  (positiveNumber >= 0x100){
			exp += 8;
			positiveNumber >>= 8;
		}
	
	// check if larger than 2**4
	if  (positiveNumber >= 0x10){
		exp += 4;
		positiveNumber >>= 4;
	}

	// check if larger than 2**2
	if  (positiveNumber >= 0x4){
		exp += 2;
		positiveNumber >>= 2;
	}

	// check if larger than 2**1
	if  (positiveNumber >= 0x2){
		exp += 1;
		positiveNumber >>= 1;
	}

    return exp;
}



/** implements a 2 step conversion - step1 32/64 to 16bit step2 10bit to decimal
 *  this way the number of 32/64 bit operations are reduced
 *  numberFillLength shall be set to 4 
 *  streamer must have a PutC(char) method. It will be used to output the digits
 */   
template <typename T, class streamer> void NToDecimalStreamPrivate(streamer &s, T positiveNumber,int8 numberFillLength=0){

	if (numberFillLength < 0) numberFillLength=0;
	// calculates the number of obligatory digits for first section by removing the nearest multiple of 4
	if (numberFillLength > 0) numberFillLength &= 0x3;
	
	// treat 64 bit numbers dividing them into 5 blocks of max 4 digits
	if (sizeof(T)==8){  
		const uint64 tests[4] = {10000000000000000,1000000000000,100000000,10000};
		// calculates the number of obligatory digits for first section 
		if (numberFillLength > 0)numberFillLength %=4;
		int i;
		for (i=0;i<4;i++){
			// enter if a big number or if zero padding required
			if ((positiveNumber > tests[i])|| (numberFillLength>0))  {
				// call this template with 16 bit number
				// otherwise infinite recursion!
				uint16 x = positiveNumber / tests[i];
				positiveNumber %= tests[i];
				// process the upper part as uint16
				// recurse into this function
				NToDecimalStreamPrivate(s,x,numberFillLength);
				// next blocks will all be padded to 4 digits 
				numberFillLength = 4;
			} 
		}
		// call this template with 16 bit number
		// otherwise infinite recursion!
		uint16 x = positiveNumber;
		// recurse into this function
		NToDecimalStreamPrivate(s,x,numberFillLength);
		return;
	}  

	// treat 32 bit numbers dividing them into 3 blocks of max 4 digits
	if (sizeof(T)==4){  
		const uint32 tests[2] = {100000000,10000};
		int i;
		for (i=0;i<2;i++){
			if ((positiveNumber > tests[i])|| (numberFillLength>0))  {
				uint16 x = positiveNumber / tests[i];
				positiveNumber %= tests[i];
				// process the upper part as uint32
				NToDecimalStreamPrivate(s,x,numberFillLength);
				numberFillLength = 4;
			} // after this 11 max
		}
		uint16 x = positiveNumber;
		NToDecimalStreamPrivate(s,x,numberFillLength);
		return;
	}

	// 16 bit code 
	if (sizeof(T)<=2){ 
		// sufficient for  a 16 - 8 bit number NO terminator needed
		char buffer[5]; 

		uint8 index = sizeof(buffer)-1;
		// if not zero extract digits backwards
		while (positiveNumber > 0){
			uint8 digit    = positiveNumber % 10;
			positiveNumber = positiveNumber / 10;
			buffer[index--] = '0' + digit;
		}
		
		// first fill in all necessary zeros 
		uint8 i= 0;		
		if (numberFillLength > 0){
			for (i=(5-numberFillLength);i<=index;i++) s.PutC('0');
		}
		// then complete by outputting all digits 
		for (i=index+1;i<=4;i++) s.PutC(buffer[i]);
	}
}

/**
 * Converts any integer type, signed and unsigned to string 
 * uses any class with method PutC
 * if it does not fit returns "?" 
 */
template <typename T, class streamer> 
bool NumberToDecimalStream(
			streamer &		stream,                        // must have a GetC(c) function where c is of a type that can be obtained from chars  
			T 				number,
			uint8 			maximumSize			= 0,       // 0 means that the number is printed in its entirety
			bool 			padded				= false,   // if maximumSize!=0 & align towards the right or the left
			bool 			leftAligned			= false,   // if padded and maximumSize!=0 align towards the left
			bool 			addPositiveSign		= false)   // prepend with + not just with - for negative numbers
{

	// put here the unsigned version of the number
	T positiveNumber;
	// put here the total space needed for the number
	// 1 always for the one figure to print
	uint8 numberSize = 1;

		// if negative invert it and account for the '-' in the size
	if (number < 0) {
		positiveNumber = -number;
		numberSize++;
	} else {
		// if positive copy it and account for the '+' in the size if addPositiveSign set
		positiveNumber = number;
		if (addPositiveSign) numberSize++;
	}

	// add the number of figures beyond the first 
	numberSize += GetOrderOfMagnitude(positiveNumber);

	// if no limits set the numberSize as the limit
	if (maximumSize==0) maximumSize=numberSize;

    // is there enough space for the number?
	if (maximumSize < numberSize){
		// if no than we shall print '?' so the size is 1 now
		numberSize = 1; 

		// fill up to from 1 maximumSize with ' '
		if (padded && !leftAligned){
			for (int i=1;i < maximumSize;i++) stream.PutC(' ');
		}
		
		// put the ?
		stream.PutC('?');
		
	} else { // enough space
		
		// fill up from numberSize to maximumSize with ' '
		if (padded && !leftAligned){
			for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
		}
		// add sign 
		if (number < 0)      stream.PutC('-');
		else 
        if (addPositiveSign) stream.PutC('+');

		// put number
		NToDecimalStreamPrivate(stream, positiveNumber);
	}
	
	// fill up from numberSize to maximumSize with ' '
	if (padded && leftAligned){
		for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
	}
    return true;	
}


/**
 * Converts any integer type, signed and unsigned to string in hexadecimal notation 
 * writes to a buffer up to the length of the buffer
 * if it does not fit returns "?" 
 * size contains the size of the buffer (including space for terminator 0)
 * size returns the size of the string excluding the trailing 0
 * if there is enough space a pointer is returned to the start of the number in buffer.
 * buffer is filled from the end backwards
 */
template <typename T, class streamer> 
bool NumberToHexadecimalStream(
			streamer &		stream, 
			T 				number,
			uint8 			maximumSize			=0,
			bool 			padded				=false,
			bool 			leftAligned			=false, 
			bool 			putTrailingZeros	=false,
			bool 			addHeader			=false)
{       
	// put here size of number
	uint8 headerSize       = 0;

	// adding two chars 0x header
	if (addHeader) headerSize =2;

	// if we add the trailing zeroes
	// sizeof(number) * 8 = totalBits
	// divided by 4 = number of digits	
	uint8 numberOfDigitsPadded= sizeof (T) * 2;
	
	// actual meaningful digist
	uint8 totalNumberOfDigits   = GetOrderOfMagnitudeHex(number) + 1;
	
	// add header for total size if padded
	uint8 fullNumberSize  = headerSize + totalNumberOfDigits;

	// nort padded size : add header for total size if padded
	uint8 numberSize = headerSize + numberOfDigits;

	// if not limits then use the number size as limt
	if (maximumSize==0) maximumSize = fullNumberSize;

	// cannot fit the number
	if (maximumSize < numberSize){
		numberSize = 1; // just the '?'
		
		// pad on the left
		if (padded && !leftAligned){
			for (int i=1;i < maximumSize;i++) stream.PutC(' ');
		}
		// put the ?
		stream.PutC('?');
		
	} else {

		//In case of trailing zeros the digits are the maximum possible or equal to maximum size (-2 if there is header)
		if (putTrailingZeros){

			// check if adding all zeros number will not fit
			if (fullNumberSize > maximumSize){
				// how much is exceeding?
				uint8 excess   = fullNumberSize - maximumSize;
				// number is made to fit the available space
				numberSize     = maximumSize;
				// we cannot print all the zeros, remove excess
				numberOfDigits = totalNumberOfDigits - excess; 
			} else {	
				// we will use the full number size
				numberSize     = fullNumberSize;
				// we will print all digits
				numberOfDigits = totalNumberOfDigits; 
			}
		}

		// in case of left alignment
		if (padded && !leftAligned){
			for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
		}

		// add header
		if (addHeader) {
			stream.PutC('0');
			stream.PutC('x');
		}
		
		// work out how much to shift number to extract most significative hex
		// we use the above claculate number size 
		int bits=(numberDigits-1)*4;	
	
		// loop backwards stepping each nibble
		for (int i = bits; i>=0; i-=4){
			//to get the digit, shift the number and by masking select only the 4 LSB bits  
			uint8 digit = (number >> i) & 0xF;			
			
			// skips trailing zeros until we encounter the first non zero, or if putTrailingZeros was already set
			if ((digit != 0) || (putTrailingZeros)){
				putTrailingZeros = true;
				if (digit < 10)   stream.PutC('0'+digit);
				else              stream.PutC('A'+digit-10);
			} 
		}
	}
	
	// in case of right alignment
	if (padded && leftAligned){
		for (int i = numberSize;i < maximumSize;i++) stream.PutC(' ');
	}
    return true;	
	
}

template <typename T, class streamer> 
bool NumberToOctalStream(       
	streamer &		stream, 
	T 				number,
	uint8 			maximumSize			=0,
	bool 			padded				=false,
	bool 			leftAligned			=false, 
	bool 			putTrailingZeros	=false,
	bool 			addHeader			=false){

	// put here size of number
	uint8 headerSize       = 0;

	// adding two chars 0x header
	if (addHeader) headerSize =2;

	// if we add the trailing zeroes
	// sizeof(number) * 8 = totalBits
	// divided by 3 and rounded up = number of digits	
	uint8 numberOfDigitsPadded= (sizeof(T) * 8 + 2)/3;
	
	// actual meaningful digist
	uint8 totalNumberOfDigits   = GetOrderOfMagnitudeOct(number) + 1;
	
	// add header for total size if padded
	uint8 fullNumberSize  = headerSize + totalNumberOfDigits;

	// nort padded size : add header for total size if padded
	uint8 numberSize = headerSize + numberOfDigits;

	// if not limits then use the number size as limt
	if (maximumSize==0) maximumSize = fullNumberSize;

	// cannot fit the number
	if (maximumSize < numberSize){
		numberSize = 1; // just the '?'
		
		// pad on the left
		if (padded && !leftAligned){
			for (int i=0;i < maximumSize-1;i++) stream.PutC(' ');
		}
		
		stream.PutC('?');
		
	} else {
	
		//In case of trailing zeros the digits are the maximum possible or equal to maximum size (-2 if there is header)
		if (putTrailingZeros){

			// check if adding all zeros number will not fit
			if (fullNumberSize > maximumSize){
				// how much is exceeding?
				uint8 excess   = fullNumberSize - maximumSize;
				// number is made to fit the available space
				numberSize     = maximumSize;
				// we cannot print all the zeros, remove excess
				numberOfDigits = totalNumberOfDigits - excess; 
			} else {	
				// we will use the full number size
				numberSize     = fullNumberSize;
				// we will print all digits
				numberOfDigits = totalNumberOfDigits; 
			}
		}

		
		// in case of left alignment
		if (padded && !leftAligned){
			for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
		}

		// add header
		if (addHeader) {
			stream.PutC('0');
			stream.PutC('o');
		}
		
		// work out how much to shift number to extract most significative hex
		// we use the above claculate number size 
		int bits=(numberDigits-1)*3;	
	
		// loop backwards stepping each nibble
		for (int i = bits; i >= 0; i-= 3){
			//to get the digit, shift the number and by masking select only the 4 LSB bits  
			uint8 digit = (number >> i) & 0x7;			
			
			// skips trailing zeros until we encounter the first non zero, or if putTrailingZeros was already set
			if ((digit != 0) || (putTrailingZeros)){
				putTrailingZeros = true;
				stream.PutC('0'+digit);
			} 
		}	
	}
	
	// in case of right alignment
	if (padded && leftAligned){
		for (int i=0;i < maximumSize-numberSize;i++) stream.PutC(' ');
	}
    return true;	
}

template <typename T, class streamer> 
bool NumberToBinaryStream(
		streamer &		stream, 
		T 				number,
		uint8 			maximumSize			=0,
		bool 			padded				=false,
		bool 			leftAligned			=false, 
		bool 			putTrailingZeros	=false,
		bool 			addHeader			=false){

	// put here size of number
	uint8 headerSize       = 0;

	// adding two chars 0x header
	if (addHeader) headerSize =2;

	// if we add the trailing zeroes
	// sizeof(number) * 8 = totalBits
	// divided by 4 = number of digits	
	uint8 numberOfDigitsPadded= sizeof (T) * 8;
	
	// actual meaningful digist
	uint8 totalNumberOfDigits   = GetOrderOfMagnitudeBin(number) + 1;
	
	// add header for total size if padded
	uint8 fullNumberSize  = headerSize + totalNumberOfDigits;

	// nort padded size : add header for total size if padded
	uint8 numberSize = headerSize + numberOfDigits;

	// if not limits then use the number size as limt
	if (maximumSize==0) maximumSize = fullNumberSize;

	// cannot fit the number
	if (maximumSize < numberSize){
		numberSize = 1; // just the '?'
		
		// pad on the left
		if (padded && !leftAligned){
			for (int i=0;i < maximumSize-1;i++) stream.PutC(' ');
		}
		
		stream.PutC('?');
		
	} else {

		//In case of trailing zeros the digits are the maximum possible or equal to maximum size (-2 if there is header)
		if (putTrailingZeros){

			// check if adding all zeros number will not fit
			if (fullNumberSize > maximumSize){
				// how much is exceeding?
				uint8 excess   = fullNumberSize - maximumSize;
				// number is made to fit the available space
				numberSize     = maximumSize;
				// we cannot print all the zeros, remove excess
				numberOfDigits = totalNumberOfDigits - excess; 
			} else {	
				// we will use the full number size
				numberSize     = fullNumberSize;
				// we will print all digits
				numberOfDigits = totalNumberOfDigits; 
			}
		}
		
		// in case of left alignment
		if (padded && !leftAligned){
			for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
		}

		// add header
		if (addHeader) {
			stream.PutC('0');
			stream.PutC('b');
		}

		// work out how much to shift number to extract most significative hex
		// we use the above claculate number size 
		int bits=numberDigits-1;	

		// loop backwards stepping each nibble
		for (int i = bits; i >= 0; i--){
			//to get the digit, shift the number and by masking select only the 4 LSB bits  
			uint8 digit = (number >> i) & 0x1;			
			
			// skips trailing zeros until we encounter the first non zero, or if putTrailingZeros was already set
			if ((digit != 0) || (putTrailingZeros)){
				putTrailingZeros = true;
				stream.PutC('0'+digit);
			} 
		}	
	}
	

// in case of right alignment
	if (padded && leftAligned){
		for (int i=0;i < maximumSize-numberSize;i++) stream.PutC(' ');
	}
    return true;	
	
}


/// This function allows determining rapidly the minimum number of digits 
/// necessary to describe a number
/// it takes the exponent in base2 and multiplies it by log10(2)
/// this is the minimum log of the number
/// the maximum is this value + log10(2)
/// all of this unless the float is subnormal...
/*static inline template <typename T> unsigned short FastLog10(T x){
unsigned short exponent;
if (sizeof(x) == 4){
    unsigned long  &px = (unsigned long &)x;
    exponent = ((px & 0x7F800000) >> 23)-127;   
}
if (sizeof(x) == 8){
    unsigned long long &px = (unsigned long long &)x;
    exponent = ((px & 0x7FFC000000000000) >> 52)-1023;
}
    return (0.30102996 * exponent );
}*/

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
template <typename T> 
static inline void NormalizeNumber(T &positiveNumber, int16 &exponent){
	// used internally 
	if (positiveNumber <= 0.0) return ;

	if (positiveNumber >= 1.0){
        CHECK_AND_REDUCE(positiveNumber,256,exponent)
        CHECK_AND_REDUCE(positiveNumber,128,exponent)
        CHECK_AND_REDUCE(positiveNumber,64,exponent)
        CHECK_AND_REDUCE(positiveNumber,32,exponent)
        CHECK_AND_REDUCE(positiveNumber,16,exponent)
        CHECK_AND_REDUCE(positiveNumber,8,exponent)
        CHECK_AND_REDUCE(positiveNumber,4,exponent)
        CHECK_AND_REDUCE(positiveNumber,2,exponent)
        CHECK_AND_REDUCE(positiveNumber,1,exponent)
	} else {
        CHECK_AND_INCREASE(positiveNumber,256,exponent)
        CHECK_AND_INCREASE(positiveNumber,128,exponent)
        CHECK_AND_INCREASE(positiveNumber,64,exponent)
        CHECK_AND_INCREASE(positiveNumber,32,exponent)
        CHECK_AND_INCREASE(positiveNumber,16,exponent)
        CHECK_AND_INCREASE(positiveNumber,8,exponent)
        CHECK_AND_INCREASE(positiveNumber,4,exponent)
        CHECK_AND_INCREASE(positiveNumber,2,exponent)
        CHECK_AND_INCREASE(positiveNumber,1,exponent)	
        CHECK_AND_INCREASE(positiveNumber,0,exponent)	
	}
}

template <typename T> 
static inline void fastPow10(T &output, int16 exponent){
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

/**
 * converts a couple of normalizedNumber/exponent (or any other equivalent) to a string using fixed format
 * normalizedNumber is not 0 nor Nan nor Inf and is positive
 * sizeLeft is the buffer size left
 * pBuffer is a writable area of memory of at least sizeLeft
 * precision determines the number of significative digits and is always not 0
 */
template <typename T, class streamer> 
bool FPToFixedStream(
		streamer &		stream, 
		T 				normalizedNumber,
		int16           exponent,
		uint8 			precision){

	// what is the exponent associated to the least significative figure?
	int16 leastSignificativeExponent = exponent - precision + 1;  
	
	// round up
	if (leastSignificativeExponent >= 0 ) normalizedNumber += 0.5;
	else {
		// to round up add a correctionvalue just below last visible digit
		T correction;
		fastPow10(correction,leastSignificativeExponent) * 0.5;
		normalizedNumber += correction;
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
		uint8 digit = (uint8)(normalizedNumber );
		normalizedNumber -= digit;
		normalizedNumber *= 10.0;

		stream.PutC('0'+ digit);

		// update precision and exponent
		if (precision > 0) precision--;
		exponent--;
	}
}

/**
Encodes the exponent in the classical form E+nn
 */
template <class streamer> 
static inline void ExpToDecimalStream(
		streamer &		stream, 
		int16           exponent
){
    // output exponent if exists
    if (exponent != 0){
		stream.PutC('E'+ digit);
		NumberToDecimalStream(stream,exponent,0,false,false,true);
    }
}

template <typename T, class streamer> 
bool BasicFloatConversion(
		streamer &		stream, 
		T 				number,
		uint8 			precision){

	if (isnan(number)) {
		stringSize = 3;
		pBuffer = "NaN";	
        return false;
	}

	if (isinf(number)) {
		stringSize = 3;
		pBuffer = "Inf";		
        return false;
	}

	if (number == 0) {
		stringSize = 1;
		pBuffer = "0";		
        return false;
	}

	// 0 not allowed
	if (precision == 0) precision = 1;

	// not space even for "0"!
	if (sizeLeft < 1) {
		stringSize = 1;
		pBuffer = "?"; 
        return false;
	}
	// stringSize is also an index within the buffer for the next free space
	stringSize = 0;

	// flip sign if necessary;
	if (number < 0){
		number = -number;
		if (sizeLeft--) *pBuffer++ = '-';
	}
    
    return true;
}

static inline uint8 FixedFormatSize(int16 exponent,uint16 precision){    

	// fixed notation 
	uint8 fixedNotationSize = precision;
    if (exponent > (precision-1)) fixedNotationSize = exponent;
    if (exponent < (precision-1)) fixedNotationSize++;
    return fixedNotationSize;
}

/**
 * converts a float/double (or any other equivalent) to a string using fixed format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan to a const char)
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToFixed(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}

static inline uint16 NumberOfDigitsOfExponent(int16 exponent){
	// workout the size of exponent
	// the size of exponent is 2+expNDigits 
	int16 exponentNumberOfDigits = 3;// E+nn
	uint16 absExponent = exponent;
	if(absExponent<0){
		absExponent*=-1;
	}
	while (absExponent > 10){
		exponentNumberOfDigits++;
		absExponent /= 10;
	}
    return exponentNumberOfDigits;
}

static inline uint8 ExponentialFormatSize(int16 exponent,uint16 precision){    
	//	exponential notation number size
	uint8 exponentNotationSize = precision+NumberOfDigitsOfExponent(exponent);
    // add space for .
    if (1 < precision) exponentNotationSize++;
    return exponentNotationSize;
}

/**
 * converts a float/double (or any other equivalent) to a string using exponential format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan to a const char)
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToExponential(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, 0,precision);
    
    // writes exponent
    ExpToDecimal(pBuffer,sizeLeft, exponent);

	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}

static inline int16 ExponentToEngineering(int16 &exponent){
    int16 engineeringExponent = exponent / 3;
    if (engineeringExponent < 0) engineeringExponent = (exponent-2)/3;
    engineeringExponent *= 3;
    exponent -= engineeringExponent;
    return engineeringExponent;
}

static inline uint8 EngineeringFormatSize(int16 exponent,uint16 precision){    
	uint8 engineeringNotationSize = precision;
	int16 exponentCopy=exponent;
    engineeringNotationSize += NumberOfDigitsOfExponent(ExponentToEngineering(exponentCopy));
        
    if (exponent > (precision-1)) engineeringNotationSize = exponentCopy;
    if (exponent < (precision-1)) engineeringNotationSize++;
    return engineeringNotationSize;
}

/**
 * converts a float/double (or any other equivalent) to a string using engineering format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToEngineering(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // partitions the exponent between engineering part and residual 
    int16 engineeringExponent = ExponentToEngineering(exponent);
    
    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, exponent,precision);
    
    // output exponent if exists
    if (exponent != 0){
		if (sizeLeft--) *pBuffer++ = 'E';
        char buffer2[7];
        uint16 stringSize;
        const char *expNumber = NumberToDecimal(stringSize,buffer2,sizeof(buffer2),exponent,true);
        while (*expNumber != 0){
            *pBuffer++ = *expNumber++;
        }
    }

    	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}

static inline uint8 SmartFormatSize(int16 exponent,uint16 precision){    
	uint8 smartNotationSize = precision;
    int16 exponentCopy = exponent;
	uint16 engineeringNotationSize=precision;
    int16 engineeringExponent = ExponentToEngineering(exponentCopy);
        
    if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
        engineeringNotationSize++;
    } else {
        engineeringNotationSize += NumberOfDigitsOfExponent(engineeringExponent);
    }
        
    if (exponent > (precision-1)) smartNotationSize = exponentCopy;
    if (exponent < (precision-1)) smartNotationSize++;
    
    return smartNotationSize;
}	
/**
 * converts a float/double (or any other equivalent) to a string using engineering format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToSmart(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // partitions the exponent between engineering part and residual 
    int16 engineeringExponent = ExponentToEngineering(exponent);
    
    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

    if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
        static const char *symbols = "pnum kMGT";
		if (sizeLeft--) *pBuffer++ = symbols[engineeringExponent/3];        
    } else {
        // writes exponent
        ExpToDecimal(pBuffer,sizeLeft, engineeringExponent);
    }
      
   	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}


/**
 * converts a float/double (or any other equivalent) to a string using whatever format achieves best compact representation
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToCompact(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);
    
    uint8 fs[4];
    fs[0] = FixedFormatSize(exponent,precision);
    fs[1] = ExponentialFormatSize(exponent,precision);
    fs[2] = EngineeringFormatSize(exponent,precision);
    fs[3] = SmartFormatSize(exponent,precision);
    
    int chosen = 0;
    int size = fs[0];
    for (int i = 1; i < 4;i++){
        if (((fs[i] <=  sizeLeft) && (fs[i] > fs[chosen])) || ((fs[i] < fs[chosen]) && (fs[chosen] > sizeLeft))) chosen = i;
    }
    if (fs[chosen] > sizeLeft){
        precision -= (fs[chosen] + sizeLeft);
    }
    
    if ( precision < 1) return "?";
    
    switch (chosen){
        case 0:{
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, exponent,precision);
            
        }break;
        case 1:{
    
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, 0,precision);

            // writes exponent
            ExpToDecimal(pBuffer,sizeLeft, exponent);
            
        }break;
        case 2:{
            // partitions the exponent between engineering part and residual 
            int16 engineeringExponent = ExponentToEngineering(exponent); 
    
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

            // writes exponent
            ExpToDecimal(pBuffer,sizeLeft, engineeringExponent);
            
        }break;
        case 3:{
            // partitions the exponent between engineering part and residual 
            int16 engineeringExponent = ExponentToEngineering(exponent); 
    
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

            if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
                static const char *symbols = "pnum kMGT";
                if (sizeLeft--) *pBuffer++ = symbols[engineeringExponent/3];        
                } else {
                   // writes exponent
                ExpToDecimal(pBuffer,sizeLeft, engineeringExponent);
            }            
        }break;
    }

    	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}







///////////////////////////////////////////////////
#if 0

template <typename T> void NtoDecimalPrivate(char *buffer,int &nextFree,T number){       
	while (number > 0){
		unsigned short  digit = number % 10;
		number                = number / 10;
		buffer[nextFree--] = '0' + digit;
	}
}

/**
 * Converts any integer type, signed and unsigned to string 
 * writes to a buffer up to the length of the buffer
 * if it does not fit returns "?" 
 * bufferSize contains the size of the buffer (including space for terminator 0)
 * stringSize returns the size of the string excluding the trailing 0
 * if there is enough space a pointer is returned to the start of the number in buffer.
 * buffer is filled from the end backwards
 */
template <typename T> char *NumberToDecimal(uint16 &stringSize,char *buffer,uint16 bufferSize,T number,bool addPositiveSign=false){
	// 5/4,7/6,12/11,22/21 depending on the byte size of number - include 0 and eventual sign
	int neededSize = (sizeof(number)*5 +3)/2;	
	if (number<0)neededSize++; 
	
	// there must be space for the number in the worst case
    if (bufferSize < neededSize)   {
    	stringSize = 1;
    	return (char*)"?";
    }

    stringSize = 0;

	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = bufferSize-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;
	
	// always output a 0 at least
	// space is guaranteed by the first check against 2
	if (number == 0){
	    buffer[nextFree--] = '0';
	} else
		
	// deal with negative by removing the sign and outputting it separately
    if (number < 0) {
        number = -number;

        // convert each digit
        NtoDecimalPrivate(buffer,nextFree,number);

        // put the -
        buffer[nextFree--] = '-';
    } else {

        // convert each digit
        NtoDecimalPrivate(buffer,nextFree,number);
        
        // put the +
        if (addPositiveSign) buffer[nextFree--] = '+';
    }
    
	// nextFree is the next location to be used in the buffer going backwards
	// firstUsed is the first location used in the buffer 
	int firstUsed = nextFree+1; 
	
	// size is the space available for the number in the buffer including terminator 0
	// size = 10 and first used = 0 ==> 9 characters
	stringSize = bufferSize - firstUsed;
	stringSize -= 1;
    //
	return buffer + firstUsed;
}

/**
 * Converts any integer type, signed and unsigned to string in hexadecimal notation 
 * writes to a buffer up to the length of the buffer
 * if it does not fit returns "?" 
 * size contains the size of the buffer (including space for terminator 0)
 * size returns the size of the string excluding the trailing 0
 * if there is enough space a pointer is returned to the start of the number in buffer.
 * buffer is filled from the end backwards
 */
template <typename T> char *NumberToHexadecimal(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, bool putTrailingZeros=false,bool addHeader=false){       
	// sizeof(number) * 8 = totalBits
	// divided by 4 = number of digits
	int totalNumberSize = sizeof(number) * 2;
	
	// consider terminator 0 and header
	int totalBufferSize = totalNumberSize+1;
	if (addHeader) totalBufferSize +=2;
	
	// Even if trailing zeroes are skipped
	// we need this buffer size to process number
	// from now on we do need to check for buffer size anymore!
	if (bufferSize < totalBufferSize) {
		stringSize = 1;
    	return (char*)"?";
    }
	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = totalBufferSize-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;

    for (int i = 0;i < totalNumberSize; i++ ){
        unsigned short digit = number & 0xF;
        if (digit < 10)   buffer[nextFree--] = '0' + digit;
        else              buffer[nextFree--] = 'A' + digit - 10;
        
        // maybe better /=16??? check with signed numbers!
        number >>=4;
        
        // this code is alternative to the one commented below 
        if ((number == 0) && !putTrailingZeros) break; 
    }

	if (addHeader) {
		// the space is guaranteed by the check at the beginning!
		buffer[nextFree--] = 'x';
		buffer[nextFree--] = '0';
	}

    // now use a pointer to the first digit
	int firstUsed = nextFree+1; 
	
	stringSize = bufferSize - firstUsed;
	stringSize -= 1;
    //
	return buffer + firstUsed;
	
}

/**
 * Converts any integer type, signed and unsigned to string in octal notation 
 * writes to a buffer up to the length of the buffer
 * if it does not fit returns "?" 
 * size contains the size of the buffer (including space for terminator 0)
 * size returns the size of the string excluding the trailing 0
 * if there is enough space a pointer is returned to the start of the number in buffer.
 * buffer is filled from the end backwards
 */
template <typename T> char *NumberToOctal(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, bool putTrailingZeros,bool addHeader=false){       
	// sizeof(number) * 8 = totalBits
	// divided by 3 and rounded up = number of digits
	int totalNumberSize = (sizeof(number) * 8 + 2 ) / 3;

	// consider terminator 0 and header
	int totalBufferSize = totalNumberSize+1;
	if (addHeader) totalBufferSize +=2;
	
	// Even if trailing zeroes are skipped
	// we need this buffer size to process number
	// from now on we do need to check for buffer size anymore!
	if (bufferSize < totalBufferSize) {
		stringSize = 1;
    	return (char*)"?";
    }
	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = totalBufferSize-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;

    for (int i = 0;i < totalNumberSize;  i++ ){
        unsigned short digit = number & 0x7;
        buffer[nextFree--] = '0' + digit;

        // maybe better /=8??? check with signed numbers!
        number >>=3;
        
        // this code is alternative to the one commented below 
        if ((number == 0) && !putTrailingZeros) break; 
    
    }

	if (addHeader) {
		// the space is guaranteed by the check at the beginning!
		buffer[nextFree--] = 'o';
		buffer[nextFree--] = '0';
	}

    // now use a pointer to the first digit
	int firstUsed = nextFree+1; 

	stringSize = bufferSize - firstUsed;
	stringSize -= 1;

	return buffer + firstUsed;
}

/**
 * Converts any integer type, signed and unsigned to string in binary notation 
 * writes to a buffer up to the length of the buffer
 * if it does not fit returns "?" 
 * size contains the size of the buffer (including space for terminator 0)
 * size returns the size of the string excluding the trailing 0
 * if there is enough space a pointer is returned to the start of the number in buffer.
 * buffer is filled from the end backwards
 */
template <typename T> char *NumberToBinary(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, bool putTrailingZeros,bool addHeader=false){       

	// sizeof(number) * 8 = totalBits
	int totalNumberSize = sizeof(number) * 8;
	
	// consider terminator 0 and header
	int totalBufferSize = totalNumberSize+1;
	if (addHeader) totalBufferSize +=2;
	
	// Even if trailing zeroes are skipped
	// we need this buffer size to process number
	// from now on we do need to check for buffer size anymore!
	if (bufferSize < totalBufferSize) {
		stringSize = 1;
    	return "?";
    }
	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = bufferSize-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;

    for (int i = 0;i < totalNumberSize;  i++ ){
        unsigned short digit = number & 0x1;
        buffer[nextFree--] = '0' + digit;
        // maybe better /=2??? check with signed numbers!
        number >>=1;
        
        // skip excessing zeros 
        if ((number == 0) && !putTrailingZeros) break; 
        
    }

	if (addHeader){
		// the space is guaranteed by the check at the beginning!
		buffer[nextFree--] = 'b';
		buffer[nextFree--] = '0';
	}

    // now use a pointer to the first digit
	int firstUsed = nextFree+1; 
	
	stringSize = bufferSize - firstUsed;
	stringSize -= 1;
	
    return buffer + firstUsed;
}

/// This function allows determining rapidly the minimum number of digits 
/// necessary to describe a number
/// it takes the exponent in base2 and multiplies it by log10(2)
/// this is the minimum log of the number
/// the maximum is this value + log10(2)
/// all of this unless the float is subnormal...
/*static inline template <typename T> unsigned short FastLog10(T x){
unsigned short exponent;
if (sizeof(x) == 4){
    unsigned long  &px = (unsigned long &)x;
    exponent = ((px & 0x7F800000) >> 23)-127;   
}
if (sizeof(x) == 8){
    unsigned long long &px = (unsigned long long &)x;
    exponent = ((px & 0x7FFC000000000000) >> 52)-1023;
}
    return (0.30102996 * exponent );
}*/

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
template <typename T> static inline void NormalizeNumber(T &positiveNumber, int16 &exponent){
	// used internally 
	if (positiveNumber <= 0.0) return ;

	if (positiveNumber >= 1.0){
        CHECK_AND_REDUCE(positiveNumber,256,exponent)
        CHECK_AND_REDUCE(positiveNumber,128,exponent)
        CHECK_AND_REDUCE(positiveNumber,64,exponent)
        CHECK_AND_REDUCE(positiveNumber,32,exponent)
        CHECK_AND_REDUCE(positiveNumber,16,exponent)
        CHECK_AND_REDUCE(positiveNumber,8,exponent)
        CHECK_AND_REDUCE(positiveNumber,4,exponent)
        CHECK_AND_REDUCE(positiveNumber,2,exponent)
        CHECK_AND_REDUCE(positiveNumber,1,exponent)
	} else {
        CHECK_AND_INCREASE(positiveNumber,256,exponent)
        CHECK_AND_INCREASE(positiveNumber,128,exponent)
        CHECK_AND_INCREASE(positiveNumber,64,exponent)
        CHECK_AND_INCREASE(positiveNumber,32,exponent)
        CHECK_AND_INCREASE(positiveNumber,16,exponent)
        CHECK_AND_INCREASE(positiveNumber,8,exponent)
        CHECK_AND_INCREASE(positiveNumber,4,exponent)
        CHECK_AND_INCREASE(positiveNumber,2,exponent)
        CHECK_AND_INCREASE(positiveNumber,1,exponent)	
        CHECK_AND_INCREASE(positiveNumber,0,exponent)	
	}
}

/**
 * converts a couple of normalizedNumber/exponent (or any other equivalent) to a string using fixed format
 * normalizedNumber is not 0 nor Nan nor Inf and is positive
 * sizeLeft is the buffer size left
 * pBuffer is a writable area of memory of at least sizeLeft
 * precision determines the number of significative digits and is always not 0
 */
template <typename T> void FPToFixed(char *&pBuffer,int16 &sizeLeft,T normalizedNumber, int16 exponent,uint8 precision){
	// numbers below 1.0
	if (exponent < 0){

		if (sizeLeft--) *pBuffer++ = '0';
		if (sizeLeft--) *pBuffer++ = '.';
		
		// loop and add zeros
		int i;
		for (i = 0; i < -(exponent+1); i++){
			if (sizeLeft--) *pBuffer++ = '0';
		}
		// exponent has only the job of marking where to put the '.'
		// here is lowered by 1 to avoid adding a second '.' in the following code
		exponent--;
	} 
	
	// loop to fulfil precision 
	// also must reach the end of the integer part thus exponent is checked
	while ((exponent > 0) || (precision > 0) ){
		// before outputting the fractional part add a '.'
		if (exponent == -1){
			if (sizeLeft--) *pBuffer++ = '.';
		}
		
		// get a digit and shift the number
		uint8 digit = (uint8)(normalizedNumber * 1.000000000001);
		normalizedNumber -= digit;
		normalizedNumber *= 10.0;

		if (sizeLeft--) *pBuffer++ = '0'+ digit;

		// update precision and exponent
		if (precision > 0) precision--;
		exponent--;
	}
}

/**
Encodes the exponent in the classical form E+nn
 */
static inline void ExpToDecimal(char *&pBuffer,int16 &sizeLeft,int16 exponent){
    // output exponent if exists
    if (exponent != 0){
		if (sizeLeft--) *pBuffer++ = 'E';
        char buffer2[7];
        uint16 stringSize;
        const char *expNumber = NumberToDecimal(stringSize,buffer2,sizeof(buffer2),exponent,true);
        while (*expNumber != 0){
            *pBuffer++ = *expNumber++;
        }
    }
}

template <typename T> bool BasicFloatConversion(uint16 &stringSize,char *&pBuffer,int16 &sizeLeft,T number, uint8 precision){
	if (isnan(number)) {
		stringSize = 3;
		pBuffer = "NaN";	
        return false;
	}

	if (isinf(number)) {
		stringSize = 3;
		pBuffer = "Inf";		
        return false;
	}

	if (number == 0) {
		stringSize = 1;
		pBuffer = "0";		
        return false;
	}

	// 0 not allowed
	if (precision == 0) precision = 1;

	// not space even for "0"!
	if (sizeLeft < 1) {
		stringSize = 1;
		pBuffer = "?"; 
        return false;
	}
	// stringSize is also an index within the buffer for the next free space
	stringSize = 0;

	// flip sign if necessary;
	if (number < 0){
		number = -number;
		if (sizeLeft--) *pBuffer++ = '-';
	}
    
    return true;
}

static inline uint8 FixedFormatSize(int16 exponent,uint16 precision){    

	// fixed notation 
	uint8 fixedNotationSize = precision;
    if (exponent > (precision-1)) fixedNotationSize = exponent;
    if (exponent < (precision-1)) fixedNotationSize++;
    return fixedNotationSize;
}

/**
 * converts a float/double (or any other equivalent) to a string using fixed format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan to a const char)
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToFixed(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}

static inline uint16 NumberOfDigitsOfExponent(int16 exponent){
	// workout the size of exponent
	// the size of exponent is 2+expNDigits 
	int16 exponentNumberOfDigits = 3;// E+nn
	uint16 absExponent = exponent;
	if(absExponent<0){
		absExponent*=-1;
	}
	while (absExponent > 10){
		exponentNumberOfDigits++;
		absExponent /= 10;
	}
    return exponentNumberOfDigits;
}

static inline uint8 ExponentialFormatSize(int16 exponent,uint16 precision){    
	//	exponential notation number size
	uint8 exponentNotationSize = precision+NumberOfDigitsOfExponent(exponent);
    // add space for .
    if (1 < precision) exponentNotationSize++;
    return exponentNotationSize;
}

/**
 * converts a float/double (or any other equivalent) to a string using exponential format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan to a const char)
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToExponential(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, 0,precision);
    
    // writes exponent
    ExpToDecimal(pBuffer,sizeLeft, exponent);

	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}

static inline int16 ExponentToEngineering(int16 &exponent){
    int16 engineeringExponent = exponent / 3;
    if (engineeringExponent < 0) engineeringExponent = (exponent-2)/3;
    engineeringExponent *= 3;
    exponent -= engineeringExponent;
    return engineeringExponent;
}

static inline uint8 EngineeringFormatSize(int16 exponent,uint16 precision){    
	uint8 engineeringNotationSize = precision;
	int16 exponentCopy=exponent;
    engineeringNotationSize += NumberOfDigitsOfExponent(ExponentToEngineering(exponentCopy));
        
    if (exponent > (precision-1)) engineeringNotationSize = exponentCopy;
    if (exponent < (precision-1)) engineeringNotationSize++;
    return engineeringNotationSize;
}

/**
 * converts a float/double (or any other equivalent) to a string using engineering format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToEngineering(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // partitions the exponent between engineering part and residual 
    int16 engineeringExponent = ExponentToEngineering(exponent);
    
    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, exponent,precision);
    
    // output exponent if exists
    if (exponent != 0){
		if (sizeLeft--) *pBuffer++ = 'E';
        char buffer2[7];
        uint16 stringSize;
        const char *expNumber = NumberToDecimal(stringSize,buffer2,sizeof(buffer2),exponent,true);
        while (*expNumber != 0){
            *pBuffer++ = *expNumber++;
        }
    }

    	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}

static inline uint8 SmartFormatSize(int16 exponent,uint16 precision){    
	uint8 smartNotationSize = precision;
    int16 exponentCopy = exponent;
	uint16 engineeringNotationSize=precision;
    int16 engineeringExponent = ExponentToEngineering(exponentCopy);
        
    if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
        engineeringNotationSize++;
    } else {
        engineeringNotationSize += NumberOfDigitsOfExponent(engineeringExponent);
    }
        
    if (exponent > (precision-1)) smartNotationSize = exponentCopy;
    if (exponent < (precision-1)) smartNotationSize++;
    
    return smartNotationSize;
}	
/**
 * converts a float/double (or any other equivalent) to a string using engineering format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToSmart(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // partitions the exponent between engineering part and residual 
    int16 engineeringExponent = ExponentToEngineering(exponent);
    
    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

    if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
        static const char *symbols = "pnum kMGT";
		if (sizeLeft--) *pBuffer++ = symbols[engineeringExponent/3];        
    } else {
        // writes exponent
        ExpToDecimal(pBuffer,sizeLeft, engineeringExponent);
    }
      
   	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}


/**
 * converts a float/double (or any other equivalent) to a string using whatever format achieves best compact representation
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan) to a const char
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToCompact(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

    // deals with nan, 0 etc
    if (!BasicFloatConversion(stringSize,pBuffer,sizeLeft,number, precision)){
        return pBuffer;
    }

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);
    
    uint8 fs[4];
    fs[0] = FixedFormatSize(exponent,precision);
    fs[1] = ExponentialFormatSize(exponent,precision);
    fs[2] = EngineeringFormatSize(exponent,precision);
    fs[3] = SmartFormatSize(exponent,precision);
    
    int chosen = 0;
    int size = fs[0];
    for (int i = 1; i < 4;i++){
        if (((fs[i] <=  sizeLeft) && (fs[i] > fs[chosen])) || ((fs[i] < fs[chosen]) && (fs[chosen] > sizeLeft))) chosen = i;
    }
    if (fs[chosen] > sizeLeft){
        precision -= (fs[chosen] + sizeLeft);
    }
    
    if ( precision < 1) return "?";
    
    switch (chosen){
        case 0:{
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, exponent,precision);
            
        }break;
        case 1:{
    
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, 0,precision);

            // writes exponent
            ExpToDecimal(pBuffer,sizeLeft, exponent);
            
        }break;
        case 2:{
            // partitions the exponent between engineering part and residual 
            int16 engineeringExponent = ExponentToEngineering(exponent); 
    
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

            // writes exponent
            ExpToDecimal(pBuffer,sizeLeft, engineeringExponent);
            
        }break;
        case 3:{
            // partitions the exponent between engineering part and residual 
            int16 engineeringExponent = ExponentToEngineering(exponent); 
    
            // does all the work of conversion but for the sign and special cases
            FPToFixed(pBuffer,sizeLeft,number, exponent,precision);

            if ((engineeringExponent != 0) && (engineeringExponent<=12) && (engineeringExponent>=-12)){
                static const char *symbols = "pnum kMGT";
                if (sizeLeft--) *pBuffer++ = symbols[engineeringExponent/3];        
                } else {
                   // writes exponent
                ExpToDecimal(pBuffer,sizeLeft, engineeringExponent);
            }            
        }break;
    }

    	// no space to complete number exit
	if (sizeLeft < 0) {
		stringSize = 1;
		return "?";
	}
	
	// terminate string - space is guaranteed by check above  
	*pBuffer = 0;
	stringSize = (pBuffer - buffer);	

	return buffer;
}
#endif 

#endif	
