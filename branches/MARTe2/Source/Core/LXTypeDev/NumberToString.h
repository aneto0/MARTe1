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

template <typename T> void NtoStringPriv(char *buffer,int &ix,T number){       
	while (number > 0){
        if (ix <= 0) return ;
		ix--;
		unsigned short  digit = number % 10;
		number                = number / 10;
		buffer[ix] = '0' + digit;
	}
}


template <typename T> const char *NtoString(char *buffer,int &size,T number){       
    if (size <= 0) {
        return "";
    }
	buffer[size--] = 0;
	int ix = size;
	if (number == 0){
        if (ix <= 0) return "";
        ix--;
	    buffer[ix] = '0';
	}
    if (number < 0) {
        number = -number;

        NtoStringPriv(buffer,ix,number);       

        if (ix <= 0) return "";
        ix--;
        buffer[ix] = '-';
    } else {

        NtoStringPriv(buffer,ix,number);       
    }
    
    size -= ix;
	return buffer + ix;
}


char toHex(uint8_t nibble){
	if (nibble < 9) return '0'+nibble;
	if (nibble > 15) return '?';
	return 'A'+nibble-10;
}

static const char PROGMEM hexLabel[] = "0x";

template <typename T> const char *NtoHex(char *buffer,int &size,T number){       
    int n = sizeof(number);   
    if (size <= (2*n+1)) return "??";
    buffer[size-1] = 0;
    int ix = size-1;

    for (int i = 0;i < 2*n; i++ ){
        unsigned short digit = number & 0xF;
        ix--;
        if (digit < 10)   buffer[ix] = '0' + digit;
        else              buffer[ix] = 'A' + digit - 10;
        number >>=4;
    }
    
    size =2*n;
    return buffer + ix;
}




#if 0   /// from arduino dev

#include <stdlib.h>
#include "StreamAux.h"
#include <math.h>




uint8_t Putn_s(Print &stream,const char *s, uint8_t size){
	while (*s!= 0){
		if (size==0) return size;
		stream.print(s[0]);
		s++;
		size--;
	}
	return size;
}

// progmem string
uint8_t Putn_ps(Print &stream,const prog_char *s, uint8_t size){
	if (s == NULL) return size;
	char c = pgm_read_byte(s);
	while (c != 0){
		if (size==0) return size;
		stream.print(c);
		s++;
		size--;
		c = pgm_read_byte(s);
	}
	return size;
}

/// move labels to match data and return how far it had to move - should be = data 
uint8_t enumFind(uint8_t data,const prog_char *&labels){
	uint8_t index = 0;
	char c = pgm_read_byte(labels);
	while ((index < data) && (c != 0)){
		if (c == ';') index++;
		labels++;
		c = pgm_read_byte(labels);
	}
	return index;
}

///
uint8_t Putn_enum(Print &stream,uint8_t data,const prog_char *labels,uint8_t maxPrint){ 
	if (labels == NULL) return maxPrint;
	if (maxPrint == 0) return maxPrint;

	// move labels to match data and return how far it had to move - should be = data 
	uint8_t index = enumFind(data,labels); 

	if (index == data) {
		uint16_t labelSize = 0;
		char c = pgm_read_byte(labels+labelSize);	
		while ((c != 0) && (c != ';')){
			labelSize++;
			c = pgm_read_byte(labels+labelSize);
		}

		if (labelSize <= maxPrint){
			Putn_ps(stream, labels,labelSize);
			return maxPrint - labelSize;
		}
	}
	
	stream.print('?');
	return maxPrint -1;

}

// up to size or ?
uint8_t Putn_u32(Print &stream,uint32_t number,uint8_t maxPrint){
	char buffer[11];
	char *ptr = toBuffer_u32(buffer,number);
	int8_t neededSize = (&buffer[10] - ptr);
	if (neededSize > maxPrint){
		if (maxPrint > 0){
			stream.print('!');
			maxPrint--;
		}
    	return maxPrint;
	}
	return Putn_s(stream,ptr,maxPrint);
}

/// up to size or ?
uint8_t Putn_u16(Print &stream,uint16_t number,uint8_t maxPrint){
	char buffer[6];
	char *ptr = toBuffer_u16(buffer,number);
	int8_t neededSize = (&buffer[5] - ptr);
	if (neededSize > maxPrint){
		if (maxPrint > 0){
			stream.print('!');
			maxPrint--;
		}
    	return maxPrint;
	}
	return Putn_s(stream,ptr,maxPrint);

}

// up to size or ?
uint8_t Putn_u8(Print &stream,uint8_t number,uint8_t maxPrint){
	char buffer[4];
	char *ptr = toBuffer_u8(buffer,number);
	int8_t neededSize = (&buffer[3] - ptr);
	if (neededSize > maxPrint){
		if (maxPrint > 0){
			stream.print('!');
			maxPrint--;
		}
    	return maxPrint;
	}
	return Putn_s(stream,ptr,maxPrint);

}

char toHex(uint8_t nibble){
	if (nibble < 9) return '0'+nibble;
	if (nibble > 15) return '?';
	return 'A'+nibble-10;
}

static const char PROGMEM hexLabel[] = "0x";

// up to size or ?
uint8_t Putn_u8x(Print &stream,uint8_t number,uint8_t maxPrint){
	if (maxPrint == 0) return maxPrint;
	if (maxPrint< 4) {
		stream.print('!');
		maxPrint--;
	} else {
		stream.print((const __FlashStringHelper *)(hexLabel));
		stream.print(toHex(number >> 4) );
		stream.print(toHex(number & 0xF));
		maxPrint-=4;
	}
	return maxPrint;
}

// up to size or ?
uint8_t Putn_u16x(Print &stream,uint16_t number,uint8_t maxPrint){
	if (maxPrint == 0) return maxPrint;
	if (maxPrint < 6) {
		stream.print('!');
		maxPrint--;
	} else {
		stream.print((const __FlashStringHelper *)(hexLabel));
		stream.print(toHex( number >> 12)      );
		stream.print(toHex((number >>  8)& 0xF));
		stream.print(toHex((number >>  4)& 0xF));
		stream.print(toHex( number       & 0xF));
		maxPrint-=6;
	}
	return maxPrint;
}

// up to size or ?
uint8_t Putn_u32x(Print &stream,uint32_t number,uint8_t maxPrint){
	if (maxPrint == 0) return maxPrint;
	if (maxPrint < 10) {
		stream.print('!');
		maxPrint--;
	} else {
		stream.print((const __FlashStringHelper *)(hexLabel));
		stream.print(toHex( number >> 28)      );
		stream.print(toHex((number >> 24)& 0xF));
		stream.print(toHex((number >> 20)& 0xF));
		stream.print(toHex((number >> 16)& 0xF));
		stream.print(toHex((number >> 12)& 0xF));
		stream.print(toHex((number >>  8)& 0xF));
		stream.print(toHex((number >>  4)& 0xF));
		stream.print(toHex( number       & 0xF));
		maxPrint-=10;
	}
	return maxPrint;
}

// up to size or ?
uint8_t Putn_i32(Print &stream,int32_t number,uint8_t maxPrint){
	if (maxPrint == 0) return 0;
	if (number < 0){
		stream.print('-');
		maxPrint--;
		number = -number;
	}
	return Putn_u32(stream,number,maxPrint);
}

// up to size or ?
uint8_t Putn_i16(Print &stream,int16_t number,uint8_t maxPrint){
	if (maxPrint== 0) return 0;
	if (number < 0){
		stream.print('-');
		maxPrint--;
		number = -number;
	}
	return Putn_u16(stream,number,maxPrint);
}
// up to size or ?
uint8_t Putn_i8(Print &stream,int8_t number,uint8_t maxPrint){
	if (maxPrint == 0) return 0;
	if (number < 0){
		stream.print('-');
		maxPrint--;
		number = -number;
	}
	return Putn_u8(stream,number,maxPrint);
}

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
