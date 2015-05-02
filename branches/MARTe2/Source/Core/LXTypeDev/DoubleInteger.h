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

#if !defined DOUBLE_INTEGER
#define DOUBLE_INTEGER

#include "GeneralDefinitions.h"
#include <math.h>



/**
    !!!!! Implementation is endianity dependant 
 */
template<typename T>
class DoubleInteger{
public:
	T lower;
	T upper;
public:
	DoubleInteger(){
		lower = 0;
		upper = 0;
	}
	DoubleInteger(const DoubleInteger<T> &n){
		lower = n.lower;
		upper = n.upper;
	}
	template<typename T2>
	DoubleInteger(T2 n){
		if (n==0){
			lower = 0;
			upper = 0;
		} else
		if (n>0){
			lower = (T)n;
			upper = 0;
			if (sizeof (T2) > sizeof(T)){
				uint8 shift = (sizeof(T)*8); 
				n >>= shift;
				upper = (T)n;
			} 
		} else {
			lower = (T)n;
			upper = -1;
			if (sizeof (T2) > sizeof(T)){
				uint8 shift = (sizeof(T)*8); 
				n >>= shift;
				upper = (T)n;
			} 
			
		}
	}
	DoubleInteger(T *memory){
		lower = *memory++;
		upper = *memory;
	}
	void operator<<=(int16 shift){
		if (shift <=0) return;
		int bitSize = (sizeof(T)*8); 
		if (shift < bitSize){
			upper = upper << shift;		
			upper |= (lower >> (bitSize - shift));
			lower = lower << shift;
		} else {
			shift -= bitSize;
			upper = lower << shift;
			lower = 0;
		}
	}	
	void operator>>=(int16 shift){
		if (shift <=0) return;
		int bitSize = (sizeof(T)*8); 
		if (shift < bitSize){
			lower = lower >> shift;		
			lower |= (upper << (bitSize-shift));
			upper = upper >> shift;
		} else {
			shift -= bitSize;
			lower = upper >> shift;
			upper = 0;
		}
	}	
	void operator&=(const DoubleInteger<T> &n){
		lower &= n.lower;		
		upper &= n.upper;
	}	
	void operator|=(const DoubleInteger<T> &n){
		lower |= n.lower;		
		upper |= n.upper;
	}	
	bool operator!=(const DoubleInteger<T> &n)const{
		return ((upper != n.upper) || (lower != n.lower)); 
	}
	bool operator>(const DoubleInteger<T> &n)const{
		return ((upper > n.upper) || ((upper == n.upper) && (lower > n.lower))); 
	}
	DoubleInteger<T>  operator~()const{
		DoubleInteger<T>output;
		output.upper = ~upper;
		output.lower = ~lower;
		return output;
	}
	DoubleInteger<T>  operator&(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		output.upper = upper & n.upper;
		output.lower = lower & n.lower;
		return output;
	}
	DoubleInteger<T>  operator|(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		output.upper = upper | n.upper;
		output.lower = lower | n.lower;
		return output;
	}
	DoubleInteger<T>  operator-(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		output.upper = upper - n.upper;
		if (n.lower > lower) output.upper--;
		output.lower = lower - n.lower;
		return output;
	}
	DoubleInteger<T>  operator>>(uint8 shift)const{
		DoubleInteger<T>output(*this);
		output >>= shift;		
		return output;
	}
	DoubleInteger<T>  operator<<(uint8 shift)const{
		DoubleInteger<T>output(*this);
		output <<= shift;		
		return output;
	}
};

#endif
