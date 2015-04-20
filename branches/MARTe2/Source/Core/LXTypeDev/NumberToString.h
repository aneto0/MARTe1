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
#include "math.h"

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
template <typename T> const char *NumberToDecimal(uint16 &stringSize,char *buffer,uint16 bufferSize,T number,bool addPositiveSign=false){
	// 5/4,7/6,12/11,22/21 depending on the byte size of number - include 0 and eventual sign
	int neededSize = (sizeof(number)*5 +3)/2;	
	if (number<0)neededSize++; 
	
	// there must be space for the number in the worst case
    if (bufferSize < neededSize)   {
    	stringSize = 1;
    	return "?";
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
template <typename T> const char *NumberToHexadecimal(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, bool putTrailingZeros,bool addHeader=false){       
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
    	return "?";
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
template <typename T> const char *NumberToOctal(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, bool putTrailingZeros,bool addHeader=false){       
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
    	return "?";
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
template <typename T> const char *NumberToBinary(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, bool putTrailingZeros,bool addHeader=false){       

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
		uint8 digit = (uint8)number;
		number -= digit;
		number *= 10.0;

		if (sizeLeft--) *pBuffer++ = '0'+ digit;

		// update precision and exponent
		if (precision > 0) precision--;
		exponent--;
	}

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
	if (isnan(number)) {
		stringSize = 3;
		return "NaN";	
	}

	if (isinf(number)) {
		stringSize = 3;
		return "Inf";		
	}

	if (number == 0) {
		stringSize = 1;
		return "0";		
	}

	// 0 not allowed
	if (precision == 0) precision = 1;

	// not space even for "0"!
	if (bufferSize <= 2) {
		stringSize = 1;
		return "?"; 
	}
	// stringSize is also an index within the buffer for the next free space
	stringSize = 0;
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

	// flip sign if necessary;
	if (number < 0){
		number = -number;
		if (sizeLeft--) *pBuffer++ = '-';
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

/**
 * converts a float/double (or any other equivalent) to a string using fixed format
 * bufferSize is the buffer size and includes the space for the 0 terminator
 * buffer is a writable area of memory of at least bufferSize
 * returns pointer to the string either within the buffer or (in case of errors or inf/nan to a const char)
 * returns stringSize with the actual length of the string
 * precision determines the number of significative digits
 */
template <typename T> const char *FloatToExponent(uint16 &stringSize,char *buffer,uint16 bufferSize,T number, uint8 precision){
	if (isnan(number)) {
		stringSize = 3;
		return "NaN";	
	}

	if (isinf(number)) {
		stringSize = 3;
		return "Inf";		
	}

	if (number == 0) {
		stringSize = 1;
		return "0";		
	}

	// 0 not allowed
	if (precision == 0) precision = 1;

	// not space even for "0"!
	if (bufferSize <= 2) {
		stringSize = 1;
		return "?"; 
	}
	// stringSize is also an index within the buffer for the next free space
	stringSize = 0;
	// we will use sizeLeft to control use of buffer
	// 16 bit and signed so that we can count negatively 
	// (max exponent is -310 and max precision is 255 so the sum of the two is the max number of digits) 
	int16 sizeLeft = bufferSize-1;
	// we will also use pBuffer to mark the current active location in the buffer
	char *pBuffer = buffer;

	// flip sign if necessary;
	if (number < 0){
		number = -number;
		if (sizeLeft--) *pBuffer++ = '-';
	}

	// normalize number
	int16 exponent = 0;
	NormalizeNumber(number,exponent);

    // does all the work of conversion but for the sign and special cases
    FPToFixed(pBuffer,sizeLeft,number, 0,precision);
    
    // output exponent if exists
    if (exponent != 0){
		if (sizeLeft--) *pBuffer++ = 'E';
        char buffer2[7];
        uint16 stringSize;
        char *expNumber = NumberToDecimal(stringSize,buffer2,sizeof(buffer2),exponent,true);
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


	
	
// up to size or ?
/*uint8 Putn_float(Print &stream,float number,uint8_t numberOfSignificantFigures,uint8_t maxPrint){
	if (isnan(number)){
		if (maxPrint > 3) {
			maxPrint-=3;
			stream.print(F("NaN"));
		} else {
			if (maxPrint > 0) {
				maxPrint--;
				stream.print('!');
			}
		}
		return maxPrint;
	}
	if (isinf(number)){
		if (maxPrint > 3) {
			maxPrint-=3;
			stream.print(F("Inf"));
		} else {
			if (maxPrint > 0) {
				maxPrint--;
				stream.print('!');
			}
		}
		return maxPrint;
	}
	
	if (maxPrint == 0) return 0;
	if (number < 0){
		maxPrint--;
		stream.print('-');
		number = -number;
	}
	if (maxPrint == 0) return 0;

	
	int16_t nDigits = normalizeNumber(number);

	// workout the size of exponent
	// the size of exponent is 2+expNDigits+1 
	int16_t expNDigits = 0;
	uint16_t absNDigits = abs (nDigits);
	while (absNDigits > 10){
		expNDigits++;
		absNDigits /= 10;
	}
	
	//	scientific notation number size 3= .E+
	uint8_t expNotationSize = numberOfSignificantFigures+3+expNDigits+1;

	// fractional notation just number . number
	uint8_t fractNotationSize = numberOfSignificantFigures+1;
	// add zeroes if number below 1.0
	if (nDigits < 0)fractNotationSize-=nDigits;
	if ((nDigits+1) > numberOfSignificantFigures)fractNotationSize=nDigits+1;
	
	// choose fractional if enough space and if it is shorter then integer
	if ((maxPrint >= fractNotationSize)&&(fractNotationSize < expNotationSize)) {
		
		maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,nDigits);
	} else
    // choose exp notation if enough space 
    if (maxPrint >= expNotationSize) {
    	
    	maxPrint = Putn_scientific(stream,number,maxPrint,numberOfSignificantFigures,nDigits,expNDigits);    
    } else
	// choose super compact notation if enough space and if exponent within range (p,n,u,m,k,M,G,T)
  	if ((maxPrint >= (2+nDigits % 3)) && (nDigits <= 12) && (nDigits >= -12)){
  		int8_t symbolIndex = (nDigits + 12)/3;
  		if (symbolIndex == 4){
  			maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,nDigits);
  		} else {
  			nDigits = (nDigits + 12)%3;
  			static const char *symbols = "pnum kMGT";
  			if (maxPrint > 0){
  				maxPrint--;
  				maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,nDigits);
  				stream.print(symbols[symbolIndex]);
  			}
  		}
  	} else 
    // choose compact exp notation if enough space 
    if (maxPrint >= (3+expNDigits)) {
    	
    	maxPrint = Putn_scientific(stream,number,maxPrint,numberOfSignificantFigures,nDigits,expNDigits);
  	} else {
  		stream.print('!');
  		maxPrint--;
  	}

	return maxPrint;
}*/




template <typename T> const char *FloatToFixed(char *buffer,int &size,T number, bool putTrailingZeros,bool addHeader=false){
	
	
	
	
	
	
}



#if 0   /// from arduino dev

#include <stdlib.h>
#include "StreamAux.h"
#include <math.h>





// for internal use only - number must be normalised fabs(number)<10 - exponent is carried separately
static uint8_t Putn_fixed(Print &stream,float number,uint8_t maxPrint,uint8_t numberOfSignificantFigures,int16_t exponent){
	
	// suffix 0.000 in case of negative exponent
	if (exponent < 0){
		if (maxPrint > 0){
			stream.print('0');
			maxPrint--;
		}
		if (maxPrint > 0){
			stream.print ('.');
			maxPrint--;
		}
		int i;
		for (i=exponent;i<-1;i++){
			if (maxPrint > 0){
				stream.print('0');
				maxPrint--;
			}
		}
		exponent--;
	}
	
	// write all the significative figures
	while ((numberOfSignificantFigures > 0) || (exponent >= 0)){
	    // at the crossing of 0 exponent put a .
		if (exponent==-1){
			if (maxPrint > 0){
				stream.print('.');
				maxPrint--;
			}
		}
		uint8_t digit = (uint8_t)number;
		if (maxPrint > 0){
			stream.print((char)('0'+digit));
			maxPrint--;
		}
		float digitF = digit;
		number = number - digitF;
		number *= 10.0;			
		exponent--;
		numberOfSignificantFigures--;
	}
    return maxPrint;
}

// for internal use only - number must be normalised fabs(number)<10 - exponent is carried separately
static uint8_t Putn_scientific(Print &stream,float number,uint8_t maxPrint,uint8_t numberOfSignificantFigures,int16_t decimalExponent,int16_t expNDigits){
	if (maxPrint > (expNDigits+3)){
		maxPrint -= expNDigits+3; // 3 = E+ and at least one digit (expNDigit=0 ==> 1 digit)
		maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,0);
		maxPrint += expNDigits+3;
		if (maxPrint >0){
			stream.print('E');
			maxPrint--;
		}
		if (maxPrint >=2)
			if (decimalExponent >= 0){
				stream.print('+');
				maxPrint--;
				maxPrint = Putn_u16(stream,decimalExponent,maxPrint);
			}
			else {
				stream.print('-');
				maxPrint--;
				maxPrint = Putn_u16(stream,-decimalExponent,maxPrint);
			}
	}
	
	return maxPrint;
}

// used internally 
static int16_t normalizeNumber(float &number){
	if (number == 0.0) return 0;
	int16_t powerShift = 0;
	while (number >= 10.0){
		powerShift++;
		number *= 0.1;
	}
	while (number < 1.0){
		powerShift--;
		number *= 10.0;
	}
	return powerShift;
}

// up to size or ?
uint8_t Putn_float(Print &stream,float number,uint8_t numberOfSignificantFigures,uint8_t maxPrint){
	if (isnan(number)){
		if (maxPrint > 3) {
			maxPrint-=3;
			stream.print(F("NaN"));
		} else {
			if (maxPrint > 0) {
				maxPrint--;
				stream.print('!');
			}
		}
		return maxPrint;
	}
	if (isinf(number)){
		if (maxPrint > 3) {
			maxPrint-=3;
			stream.print(F("Inf"));
		} else {
			if (maxPrint > 0) {
				maxPrint--;
				stream.print('!');
			}
		}
		return maxPrint;
	}
	
	if (maxPrint == 0) return 0;
	if (number < 0){
		maxPrint--;
		stream.print('-');
		number = -number;
	}
	if (maxPrint == 0) return 0;

	
	int16_t nDigits = normalizeNumber(number);

	// workout the size of exponent
	// the size of exponent is 2+expNDigits+1 
	int16_t expNDigits = 0;
	uint16_t absNDigits = abs (nDigits);
	while (absNDigits > 10){
		expNDigits++;
		absNDigits /= 10;
	}
	
	//	scientific notation number size 3= .E+
	uint8_t expNotationSize = numberOfSignificantFigures+3+expNDigits+1;

	// fractional notation just number . number
	uint8_t fractNotationSize = numberOfSignificantFigures+1;
	// add zeroes if number below 1.0
	if (nDigits < 0)fractNotationSize-=nDigits;
	if ((nDigits+1) > numberOfSignificantFigures)fractNotationSize=nDigits+1;
	
	// choose fractional if enough space and if it is shorter then integer
	if ((maxPrint >= fractNotationSize)&&(fractNotationSize < expNotationSize)) {
		
		maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,nDigits);
	} else
    // choose exp notation if enough space 
    if (maxPrint >= expNotationSize) {
    	
    	maxPrint = Putn_scientific(stream,number,maxPrint,numberOfSignificantFigures,nDigits,expNDigits);    
    } else
	// choose super compact notation if enough space and if exponent within range (p,n,u,m,k,M,G,T)
  	if ((maxPrint >= (2+nDigits % 3)) && (nDigits <= 12) && (nDigits >= -12)){
  		int8_t symbolIndex = (nDigits + 12)/3;
  		if (symbolIndex == 4){
  			maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,nDigits);
  		} else {
  			nDigits = (nDigits + 12)%3;
  			static const char *symbols = "pnum kMGT";
  			if (maxPrint > 0){
  				maxPrint--;
  				maxPrint = Putn_fixed(stream,number,maxPrint,numberOfSignificantFigures,nDigits);
  				stream.print(symbols[symbolIndex]);
  			}
  		}
  	} else 
    // choose compact exp notation if enough space 
    if (maxPrint >= (3+expNDigits)) {
    	
    	maxPrint = Putn_scientific(stream,number,maxPrint,numberOfSignificantFigures,nDigits,expNDigits);
  	} else {
  		stream.print('!');
  		maxPrint--;
  	}

	return maxPrint;
}



#endif

#endif
