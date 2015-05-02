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
#if !defined INTEGER_TO_STREAM
#define INTEGER_TO_STREAM

#include "GeneralDefinitions.h"
#include "FormatDescriptor.h"
#include <math.h>

// returns the exponent
// positiveNumber is the abs (number)
/** @brief Calculates the order of a number, namely its number of digits minus one.
  * @param positiveNumber is the number argument which must be positive.
  * @return the number of digits minus one.
  * 
  * The function operates by comparing with 10**N with converging by bisection to the correct value. */
template <typename T> uint8 GetOrderOfMagnitude(T positiveNumber){       
    T tenToExponent = 1;
    uint8 exp = 0;
    // check whether exponent greater than 10  
    if (sizeof(T)>=8){ // max 19
        T temp = tenToExponent * 10000000000; // 10 zeros 
        if (positiveNumber >= temp )  {
            tenToExponent = temp;
            exp += 10;
        }
    }
    
    // check whether exponent greater than 5  
    if (sizeof(T)>=4){ // max 9 
        T temp = tenToExponent * 100000; // 5 zeros
        if (positiveNumber >= temp ) {
            tenToExponent = temp;
            exp += 5;
        }
    }
    
    // check whether exponent greater than 2  
    if (sizeof(T)>=2){ // max 4 zeros
        T temp = tenToExponent * 100; // 2 zeros
        if (positiveNumber >= temp ) {
            tenToExponent = temp;
            exp += 2;
        }
    }
    
    // check whether exponent greater than 1  
    T temp = tenToExponent * 10; // 1 
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
/** @brief Calculates the number of digits for a hexadecimal representation.
  * @param number is the number argument.
  * @return the minimum number of digits for an hexadecimal print.
  *
  * This function operates comparing the number with 16**N numbers with N=1,2,4,8 converging by 
  * bisection to the correct value. */ 

template <typename T> uint8 GetNumberOfDigitsHexNotation(T number){
	uint8 exp = 1;
	
	// negative numbers are 2 complements and have therefore all bits
	if (number < 0) return sizeof(T) * 2;

	uint8 shift = 0;
	// check if larger than 2**32
	if (sizeof(T)==8)
		if  (number >= 0x100000000){
			exp += 8;
			shift = 32; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		}

	// check if larger than 2**16
	if (sizeof(T)>=4)
		if  (number >= 0x10000){
			exp += 4;
			shift = 16; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		}

	// check if larger than 2**8
	if (sizeof(T)>=2)
		if  (number >= 0x100){
			exp += 2;
			number >>= 8;
		}

	// check if larger than 2**4
	if  (number >= 0x10){
		exp += 1;
		number >>= 4;
	}

    return exp;
}

// returns the number of digits necessary to represent this number -1 
/** @brief Calculates the number of digits for an octal representation.
  * @param number is the number argument.
  * @return the minimum number of digits for an octal print.
  *
  * This function operates comparing the number with 8**N numbers with N=1,2,4,8,16 converging by 
  * bisection to the correct value. */ 

template <typename T> uint8 GetNumberOfDigitsOctalNotation(T number){
	// negative numbers are 2 complements and have therefore all bits
	if (number < 0) return (sizeof(T) * 8 + 2) / 3;

	uint8 shift = 0;
	uint8 exp = 1;
	if (sizeof(T)==8)
		if  (number >= 0x1000000000000){
			exp += 16;
			shift = 48; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		} 

	if (sizeof(T)>=4)
		if  (number >= 0x1000000){
			exp += 8;
			shift = 24; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		}

	// check if larger than 2**12
	if (sizeof(T)>=2)
		if  (number >= 0x1000){
			exp += 4;
			shift = 12; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		}

	// check if larger than 2**6
	if  (number >= 0x40){
		exp += 2;
		number >>= 6;
	}

	// check if larger than 2**2
	if  (number >= 0x8){
		exp += 1;
		number >>= 3;
	}

    return exp;
}


// returns the number of digits necessary to represent this number -1 
/** @brief Calculates the number of digits for a binary representation.
  * @param number is the number argument.
  * @return the minimum number of digits for a binary print.
  *
  * This function operates comparing the number with 2**N numbers with N=1,2,4,8,16,32 converging by 
  * bisection to the correct value. */ 

template <typename T> uint8 GetNumberOfDigitsBinaryNotation(T number){
	// negative numbers are 2 complements and have therefore all bits
	if (number < 0) return sizeof(T) * 8 ;

	uint8 exp = 1;
	uint8 shift = 0;
	// check if larger than 2**32
	// if so shift 
	if (sizeof(T)==8)
		if  (number >= 0x100000000){
			exp += 32;
			shift = 32; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		}

	// check if larger than 2**16
	if (sizeof(T)>=4)
		if  (number >= 0x10000){
			exp += 16;
			shift = 16; // used a variable to trick the compiler warning not to issue a warning of too long shift
			number >>= shift;
		}

	// check if larger than 2**8
	if (sizeof(T)>=2)
		if  (number >= 0x100){
			exp += 8;
			number >>= 8;
		}
	
	// check if larger than 2**4
	if  (number >= 0x10){
		exp += 4;
		number >>= 4;
	}

	// check if larger than 2**2
	if  (number >= 0x4){
		exp += 2;
		number >>= 2;
	}

	// check if larger than 2**1
	if  (number >= 0x2){
		exp += 1;
		number >>= 1;
	}

    return exp;
}

/** @brief Prints an integer number on a general stream in decimal notation.
  * @param s is a general stream class which implements a putC() function.
  * @param positiveNumber is the number to print (it must be positive the '-' is added a part).
  * @param numberFillLength is the minimum number of digits requested for each 16 bit number (<5 because 2**16 has 5 digits) and 
  * the function fillw the different between it and the minimum necessary space with zeros.
  * 
  * This function implements a 2 step conversion - step1 32/64 to 16bit step2 10bit to decimal
  * this way the number of 32/64 bit operations are reduced
  * numberFillLength is used to specify how many digits to prints at least (this would include trailingzeros)
  * it will never print more trailing zeros than the maximum size of a number of that format
  * streamer must have a PutC(char) method. It will be used to output the digits. */
template <typename T, class streamer> 
static inline void Number2StreamDecimalNotationPrivate(streamer &s, T positiveNumber,int16 numberFillLength=0){

	// no negative!
	if (numberFillLength < 0) numberFillLength=0;

	// 64 bits
	if (sizeof(T)==8){  
		// treat 64 bit numbers dividing them into 5 blocks of max 4 digits
		// 16 12 8 4 zeroes
		const int64 tests[4] = {10000000000000000,1000000000000,100000000,10000};

		uint8 i;
		// how many figures are below the current test point
		uint16 figures = 16;
		for (i=0;i<4;i++){
			// enter if a big number or if zero padding required			
			if ((positiveNumber > (T)tests[i]) || 
			    (numberFillLength > figures))  {
				// call this template with 16 bit number
				// otherwise infinite recursion!
				uint16 x       = positiveNumber / tests[i];
				positiveNumber = positiveNumber % tests[i];
				
				// process the upper part as uint16
				// recurse into this function
				Number2StreamDecimalNotationPrivate(s,x,numberFillLength-figures);
				
				// print all the blocks in full from now on 
				numberFillLength = figures;
			}
			// update
			figures -= 4;
		}
		// call this template with 16 bit number
		// otherwise infinite recursion!
		uint16 x = positiveNumber;
		// recurse into this function
		Number2StreamDecimalNotationPrivate(s,x,numberFillLength);
		return;
	}  

	// treat 32 bit numbers dividing them into 3 blocks of max 4 digits
	if (sizeof(T)==4){  
		// 8 4 zeroes
		const int32 tests[2] = {100000000,10000};
		// how many figures are below the current test point
		int16 figures = 8;
		uint8 i;
		for (i=0;i<2;i++){
			if ((positiveNumber > (T)tests[i]) || 
			    (numberFillLength > figures))  {
				// call this template with 16 bit number
				// otherwise infinite recursion!
				uint16 x       = positiveNumber / tests[i];
				positiveNumber = positiveNumber % tests[i];

				// process the upper part as uint16
				// recurse into this function
				Number2StreamDecimalNotationPrivate(s,x,numberFillLength-figures);

				// print all the blocks in full from now on 
				numberFillLength = figures;
			} // after this 11 max
			figures -= 4;
		}
		// call this template with 16 bit number
		// otherwise infinite recursion!
		uint16 x = positiveNumber;
		// recurse into this function
		Number2StreamDecimalNotationPrivate(s,x,numberFillLength);
		return;
	}

	// 16 bit code 
	if (sizeof(T)<=2){
		// sufficient for  a 16 - 8 bit number NO terminator needed
		char buffer[5]; 

		int index = sizeof(buffer)-1;
	
		// if not zero extract digits backwards
		do {
			uint8 digit    = positiveNumber % 10;
			positiveNumber = positiveNumber / 10;
			buffer[index--] = '0' + digit;
		} while(positiveNumber > 0);
		
		// first fill in all necessary zeros 
		int i = 0;		
		if (numberFillLength > 0){
			// clamp to 5
			if (numberFillLength > 5)numberFillLength = 5;
			// fill up with zeros
			for (i=(5-numberFillLength);i<=index;i++) s.PutC('0');
		}
		// then complete by outputting all digits 
		for (i=index+1;i<=4;i++) s.PutC(buffer[i]);
	}
}

template <class streamer>
static inline void PutS(streamer & stream,const char *s){
	while (*s != 0){
		stream.PutC(*s);
		*s++;
	}
}

/** @brief Prints an integer number on a general stream in decimal notation.
  * @param stream is a general stream class which implements a PutC() function.
  * @param maximumSize is the maximum requested space for the number print. 
  * @param padded specifies if the difference between maximumSize and the necessary space for the number must be filled by spaces ' '.
  * @param leftAligned specifies if the number must be print with left or right alignment.
  * @param addPositiveSign specifies if we want print the '+' before positive numbers.
  * @return true.
  *
  * Converts any integer type, signed and unsigned to a sequence of characters inserted into the stream stream by mean of a method PutC
  * respects maximumSize and number integrity. 
  * if not possible (maximum size minor than the minimum space for the number print) outputs is ? */
template <typename T, class streamer> 
bool IntegerToStreamDecimalNotation(
			streamer &		stream,                        // must have a GetC(c) function where c is of a type that can be obtained from chars  
			T 				number,			
			uint8 			maximumSize			= 0,       // 0 means that the number is printed in its entirety
			bool 			padded				= false,   // if maximumSize!=0 & align towards the right or the left
			bool 			leftAligned			= false,   // if padded and maximumSize!=0 align towards the left
			bool 			addPositiveSign		= false)   // prepend with + not just with - for negative numbers
{

	// if no limits set the numberSize as big enough for the largest integer
	if (maximumSize==0) maximumSize=20;
	
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

	// 0x800000....
	if (positiveNumber < 0){
		if ((sizeof(T)==8) && (maximumSize >= 20 )){
			PutS(stream,"-9223372036854775808");
			return true;
		}
		if ((sizeof(T)==4) && (maximumSize >= 10 )){
			PutS(stream,"-2147483648");
			return true;
		}
		if ((sizeof(T)==2) && (maximumSize >= 6 )){
			PutS(stream,"-32768");
			return true;
		}
		if ((sizeof(T)==1) && (maximumSize >= 4 )){
			PutS(stream,"-128");
			return true;
		}			
		
		// does not fit
		numberSize = maximumSize+1; 
	} else {
	
		// add the number of figures beyond the first 
		numberSize += GetOrderOfMagnitude(positiveNumber);
	}

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
		if (number <  0)     stream.PutC('-');
		else 
        if (addPositiveSign) stream.PutC('+');

		// put number
		Number2StreamDecimalNotationPrivate(stream, positiveNumber);
	}
	
	// fill up from numberSize to maximumSize with ' '
	if (padded && leftAligned){
		for (int i=numberSize;i < maximumSize;i++) stream.PutC(' ');
	}
    return true;	
}


/** @brief Prints an integer number on a general stream in hexadecimal notation.
  * @param stream is a general stream class which implements a PutC() function.
  * @param maximumSize is the maximum requested space for the number print. 
  * @param padded specifies if the difference between maximumSize and the necessary space for the number must be filled by spaces ' '.
  * @param leftAligned specifies if the number must be print with left or right alignment.
  * @param putTrailingZeros specifiec if we want fill with zeros the digits before the minimum necessary digits (ex 16 bit: ab -> 00ab).
  * @param addHeader specifies if we want to add the hex header '0x' before the number.
  * @return true.
  *
  * Converts any integer type, signed and unsigned to a sequence of characters inserted into the stream stream by mean of a method PutC
  * uses hexadecimal notation 
  * respects maximumSize and number integrity 
  * if not possible (maximum size minor than the minimum space for the number print) output is ? */
template <typename T, class streamer> 
bool IntegerToStreamExadecimalNotation(
			streamer &		stream, 
			T 				number,
			uint16 			maximumSize			= 0,       // 0 means that the number is printed in its entirety
			bool 			padded				= false,   // if maximumSize!=0 & align towards the right or the left
			bool 			leftAligned			= false,   // if padded and maximumSize!=0 align towards the left
			bool 			putTrailingZeros	= false,   // trailing zeroes are not omitted (unless breaking the maximumSize)
			bool 			addHeader    		= false)   // prepend with 0x
{       
	// put here size of number
	uint8 headerSize       = 0;

	// adding two chars 0x header
	if (addHeader) headerSize =2;

	// actual meaningful digits
	uint8 numberOfDigits   = GetNumberOfDigitsHexNotation(number);

	// add header for total size if padded
	uint8 numberSize  = headerSize + numberOfDigits;

	// 1000 = no limits
	if (maximumSize == 0) maximumSize = 1000;

	// cannot fit the number even without trailing zeroes
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
			// if we add the trailing zeroes
			// sizeof(number) * 8 = totalBits
			// divided by 4 = number of digits	
			uint8 numberOfDigitsPadded= sizeof (T) * 2;

			// add header for total size if padded
			uint8 fullNumberSize  = headerSize + numberOfDigitsPadded;

			// check if adding all zeros number will not fit
			if (fullNumberSize > maximumSize){
				// how much is exceeding?
				uint8 excess   = fullNumberSize - maximumSize;
				// number is made to fit the available space
				numberSize     = maximumSize;
				// we cannot print all the zeros, remove excess
				numberOfDigits = numberOfDigitsPadded - excess; 
			} else {	
				// we will use the full number size
				numberSize     = fullNumberSize;
				// we will print all digits
				numberOfDigits = numberOfDigitsPadded; 
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
		int bits=(numberOfDigits-1)*4;	
	
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



/** @brief Prints an integer number on a general stream in octal notation.
  * @param stream is a general stream class which implements a PutC() function.
  * @param maximumSize is the maximum requested space for the number print. 
  * @param padded specifies if the difference between maximumSize and the necessary space for the number must be filled by spaces ' '.
  * @param leftAligned specifies if the number must be print with left or right alignment.
  * @param putTrailingZeros specifiec if we want fill with zeros the digits before the minimum necessary digits.
  * @param addHeader specifies if we want to add the oct header '0o' before the number.
  * @return true.
  *
  * Converts any integer type, signed and unsigned to a sequence of characters inserted into the stream stream by mean of a method PutC
  * uses octal notation 
  * respects maximumSize and number integrity 
  * if not possible (maximum size minor than the minimum space for the number print) output is ?  */
template <typename T, class streamer> 
bool IntegerToStreamOctalNotation(       
	streamer &		stream, 
	T 				number,
	uint16 			maximumSize			= 0,       // 0 means that the number is printed in its entirety
	bool 			padded				= false,   // if maximumSize!=0 & align towards the right or the left
	bool 			leftAligned			= false,   // if padded and maximumSize!=0 align towards the left
	bool 			putTrailingZeros	= false,   // trailing zeroes are not omitted (unless breaking the maximumSize)
	bool 			addHeader    		= false)   // prepend with 0o
{
	
	// put here size of number
	uint8 headerSize       = 0;

	// adding two chars 0x header
	if (addHeader) headerSize =2;

	// actual meaningful digits
	uint8 numberOfDigits   = GetNumberOfDigitsOctalNotation(number);

	// add header for total size if padded
	uint8 numberSize  = headerSize + numberOfDigits;

	// 1000 = no limits
	if (maximumSize == 0) maximumSize = 1000;

	// cannot fit the number even without trailing zeroes
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
			// if we add the trailing zeroes
			// sizeof(number) * 8 = totalBits
			// divided by 4 = number of digits	
			uint8 numberOfDigitsPadded= (sizeof (T) * 8+2)/3;

			// add header for total size if padded
			uint8 fullNumberSize  = headerSize + numberOfDigitsPadded;

			// check if adding all zeros number will not fit
			if (fullNumberSize > maximumSize){
				// how much is exceeding?
				uint8 excess   = fullNumberSize - maximumSize;
				// number is made to fit the available space
				numberSize     = maximumSize;
				// we cannot print all the zeros, remove excess
				numberOfDigits = numberOfDigitsPadded - excess; 
			} else {	
				// we will use the full number size
				numberSize     = fullNumberSize;
				// we will print all digits
				numberOfDigits = numberOfDigitsPadded; 
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
		int bits=(numberOfDigits-1)*3;	
	
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
		for (int i = numberSize;i < maximumSize;i++) stream.PutC(' ');
	}
    return true;	
}


/** @brief Prints an integer number on a general stream in binary notation.
  * @param stream is a general stream class which implements a PutC() function.
  * @param maximumSize is the maximum requested space for the number print. 
  * @param padded specifies if the difference between maximumSize and the necessary space for the number must be filled by spaces ' '.
  * @param leftAligned specifies if the number must be print with left or right alignment.
  * @param putTrailingZeros specifiec if we want fill with zeros the digits before the minimum necessary digits.
  * @param addHeader specifies if we want to add the bin header '0b' before the number.
  * @return true.
  *
  * Converts any integer type, signed and unsigned to a sequence of characters inserted into the stream stream by mean of a method PutC
  * uses binary notation 
  * respects maximumSize and number integrity 
  * if not possible (maximum size minor than the minimum space for the number print) output is ?  */
template <typename T, class streamer> 
bool IntegerToStreamBinaryNotation(
		streamer &		stream, 
		T 				number,
		uint16 			maximumSize			= 0,       // 0 means that the number is printed in its entirety
		bool 			padded				= false,   // if maximumSize!=0 & align towards the right or the left
		bool 			leftAligned			= false,   // if padded and maximumSize!=0 align towards the left
		bool 			putTrailingZeros	= false,   // trailing zeroes are not omitted (unless breaking the maximumSize)
		bool 			addHeader    		= false)   // prepend with 0b
{
	
	// put here size of number
	uint8 headerSize       = 0;

	// adding two chars 0x header
	if (addHeader) headerSize =2;

	// actual meaningful digist
	uint8 numberOfDigits   = GetNumberOfDigitsBinaryNotation(number);

	// add header for total size if padded
	uint8 numberSize  = headerSize + numberOfDigits;

	// 1000 = no limits
	if (maximumSize == 0) maximumSize = 1000;

	// cannot fit the number even without trailing zeroes
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
			// if we add the trailing zeroes
			// sizeof(number) * 8 = totalBits
			// divided by 4 = number of digits	
			uint8 numberOfDigitsPadded= sizeof (T) * 8;

			// add header for total size if padded
			uint8 fullNumberSize  = headerSize + numberOfDigitsPadded;

			// check if adding all zeros number will not fit
			if (fullNumberSize > maximumSize){
				// how much is exceeding?
				uint8 excess   = fullNumberSize - maximumSize;
				// number is made to fit the available space
				numberSize     = maximumSize;
				// we cannot print all the zeros, remove excess
				numberOfDigits = numberOfDigitsPadded - excess; 
			} else {	
				// we will use the full number size
				numberSize     = fullNumberSize;
				// we will print all digits
				numberOfDigits = numberOfDigitsPadded; 
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
		int bits=numberOfDigits-1;	

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

/** @brief Print on a general stream using a specific format.
  * @param stream is a general stream class which implements a PutC() function.
  * @param number is the integer to print.
  * @param format is the desired format.
  * @return true if the format is correct, false otherwise.
  *
  * Converts any integer type, signed and unsigned to a sequence of characters inserted into the stream stream by mean of a method PutC
  * uses notation specified in format
  * also respects all relevant format parameters 
  * respects format.size and number integrity 
  * if not possible output is ? */
template <typename T, class streamer> 
bool IntegerToStream(
		streamer &			stream, 
		T 					number,
		FormatDescriptor	format){
	switch (format.binaryNotation){
	case Notation::DecimalNotation:{
		return IntegerToStreamDecimalNotation(stream,number,format.size,format.padded,format.leftAligned,format.fullNotation);
	}break;
	case Notation::HexNotation:{
		return IntegerToStreamExadecimalNotation(stream,number,format.size,format.padded,format.leftAligned,format.binaryPadded,format.fullNotation); 
	}break;
	case Notation::OctalNotation:{
		return IntegerToStreamOctalNotation(stream,number,format.size,format.padded,format.leftAligned,format.binaryPadded,format.fullNotation); 
	}break;
	case Notation::BitNotation:{
		return IntegerToStreamBinaryNotation(stream,number,format.size,format.padded,format.leftAligned,format.binaryPadded,format.fullNotation); 
	}break;
	}
	
	return false;
}


#endif	
