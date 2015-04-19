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
 * size contains the size of the buffer (including space for terminator 0)
 * size returns the size of the string excluding the trailing 0
 * if there is enough space a pointer is returned to the start of the number in buffer.
 * buffer is filled from the end backwards
 */
template <typename T> const char *NumberToDecimal(char *buffer,int &size,T number){
	// 5/4,7/6,12/11,22/21 depending on the byte size of number - include 0 and eventual sign
	int neededSize = (sizeof(number)*5 +3)/2;	
	if (number<0)neededSize++; 
	
	// there must be space for the number in the worst case
    if (size < neededSize)   {
    	size = 1;
    	return "?";
    }

	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = size-1;
    
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
    }
    
	// nextFree is the next location to be used in the buffer going backwards
	// firstUsed is the first location used in the buffer 
	int firstUsed = nextFree+1; 
	
	// size is the space available for the number in the buffer including terminator 0
	// size = 10 and first used = 0 ==> 9 characters
    size -= firstUsed;
    size --;
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
template <typename T> const char *NumberToHexadecimal(char *buffer,int &size,T number, bool putTrailingZeros){       
	int totalNumberSize = sizeof(number) * 2;
	// Even if trailing zeroes are skipped
	// we need this buffer size to process number
	// from now on we do need to check for buffer size anymore!
	if (size < (totalNumberSize+3)) {
    	size = 1;
    	return "?";
    }
	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = size-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;

    for (int i = 0;i < totalNumberSize  i++ ){
        unsigned short digit = number & 0xF;
        if (digit < 10)   buffer[nextFree--] = '0' + digit;
        else              buffer[nextFree--] = 'A' + digit - 10;
        
        // maybe better /=16??? check with signed numbers!
        number >>=4;
        
        // this code is alternative to the one commented below 
        if ((number == 0) && !putTrailingZeros) break; 
    }

    // now use a pointer to the first digit
	int firstUsed = nextFree+1; 

	// alternative option
//    if (!putTrailingZeros){
    	// if firstUsed is not the first digit and is 0 then skip it
//    	while ((firstUsed < (size-1)) && (buffer[firstUsed]=='0')) firstUsed++;    			
//    }

    // the space is guaranteed by the check at the beginning!
    buffer[firstUsed-1] = 'x';
    buffer[firstUsed-2] = '0';
    
    size -= firstUsed;
    size--;
    return buffer + firstUsed-2;
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
template <typename T> const char *NumberToOctal(char *buffer,int &size,T number, bool putTrailingZeros){       
	int totalNumberSize = (sizeof(number) * 8 + 2 ) / 3;
	// Even if trailing zeroes are skipped
	// we need this buffer size to process number
	// from now on we do need to check for buffer size anymore!
	if (size < (totalNumberSize+3)) {
    	size = 1;
    	return "?";
    }
	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = size-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;

    for (int i = 0;i < totalNumberSize  i++ ){
        unsigned short digit = number & 0x7;
        buffer[nextFree--] = '0' + digit;

        // maybe better /=8??? check with signed numbers!
        number >>=3;
        
        // this code is alternative to the one commented below 
        if ((number == 0) && !putTrailingZeros) break; 
    
    }

    // now use a pointer to the first digit
	int firstUsed = nextFree+1; 

//    if (!putTrailingZeros){
    	// if firstUsed is not the first digit and is 0 then skip it
//    	while ((firstUsed < (size-1)) && (buffer[firstUsed]=='0')) firstUsed++;    			
//    }

    // the space is guaranteed by the check at the beginning!
    buffer[firstUsed-1] = 'o';
    buffer[firstUsed-2] = '0';
    
    size -= firstUsed;
    size--;
    return buffer + firstUsed-2;
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
template <typename T> const char *NumberToBinary(char *buffer,int &size,T number, bool putTrailingZeros){       
	int totalNumberSize = sizeof(number) * 8;
	// Even if trailing zeroes are skipped
	// we need this buffer size to process number
	// from now on we do need to check for buffer size anymore!
	if (size < (totalNumberSize+3)) {
    	size = 1;
    	return "?";
    }
	// size is now the avilable size for the number
	// nextFree is an index within the buffer for the next free space
	int nextFree = size-1;
    
    // terminate string and then fill backwards
	buffer[nextFree--] = 0;

    for (int i = 0;i < totalNumberSize  i++ ){
        unsigned short digit = number & 0x1;
        buffer[nextFree--] = '0' + digit;
        // maybe better /=2??? check with signed numbers!
        number >>=1;
        
        // this code is alternative to the one commented below 
        if ((number == 0) && !putTrailingZeros) break; 
        
    }

    // now use a pointer to the first digit
	int firstUsed = nextFree+1; 

//    if (!putTrailingZeros){
    	// if firstUsed is not the first digit and is 0 then skip it
//    	while ((firstUsed < (size-1)) && (buffer[firstUsed]=='0')) firstUsed++;    			
//    }

    // the space is guaranteed by the check at the beginning!
    buffer[firstUsed-1] = 'o';
    buffer[firstUsed-2] = '0';
    
    size -= firstUsed;
    size--;
    return buffer + firstUsed-2;
}


template <typename T> const char *NumberToFormat(char *buffer, int &size,T number,Notation::Binary binaryNotation= Notation::NormalNotation){
	const char *convertedNumber;
	
	switch (fd.binaryNotation){
	case NormalNotation:{
		convertedNumber= NumberToDecimal(buffer,size,number);
	}break;
	case HexNotation{
		convertedNumber= NumberToHexadecimal(buffer,size,number,fd.binaryPadded);
	}break;
	case OctalNotation{
		convertedNumber= NumberToOctal(buffer,size,number,fd.binaryPadded);
	}break;
	case BitNotation{
		convertedNumber= NumberToBinary(buffer,size,number,fd.binaryPadded);
	}break;
	}

	// if it does not fit than produce a  ?
    if (size > fd.width) {
    	size = 1;
    	convertedNumber = '?';
    }
    
    // check padding style
    if (fd.pad){
    	if (fd.leftAlign)    	
    }
    
	
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
