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
    Implements an integer number of double the size of T
    Uses operators of T to implement operators  
 */
template<typename T>
class DoubleInteger{
public:
	// least significative
	T lower;
	// most significative
	T upper;
public:
	// default constructor
	DoubleInteger(){
		lower = 0;
		upper = 0;
	}
	// copy constructor
	DoubleInteger(const DoubleInteger<T> &n){
		lower = n.lower;
		upper = n.upper;
	}
	// smart copy constructor
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
/*	
	DoubleInteger(T *memory){
		lower = *memory++;
		upper = *memory;
	}
	*/
	
	// safe left shift
	void operator<<=(int16 shift){
		// shift of sizeof(T)*8 is treated as shift 0
		// for this reason exit here to avoid this pitfall
		if (shift <= 0) return;
		// calculates n of bits of T
		int bitSize = (sizeof(T)*8);
		// shift within one half
		if (shift < bitSize){
			// shift upper first
			upper = upper << shift;
			// add overflow from lower
			// this would fail if shift is 0
			upper |= (lower >> (bitSize - shift));
			// complete the lower
			lower = lower << shift;
		} else { // more than half!			
			// remove half
			shift -= bitSize;
			// swap lower -> upper and shift with the remainder
			upper = lower << shift;
			// lower is 0
			lower = 0;
		}
	}	
	// safe right shift
	void operator>>=(int16 shift){
		// shift of sizeof(T)*8 is treated as shift 0
		// for this reason exit here to avoid this pitfall
		if (shift <=0) return;
		// calculates n of bits of T
		int bitSize = (sizeof(T)*8);
		// shift within one half
		if (shift < bitSize){
			// shift lower first
			lower = lower >> shift;		
			// add overflow from upper
			// this would fail if shift is 0
			lower |= (upper << (bitSize-shift));
			// complete the upper
			upper = upper >> shift;
		} else { // more than half!			
			// remove half
			shift -= bitSize;
			// swap upper -> lower and shift with the remainder
			lower = upper >> shift;
			// upper is 0
			upper = 0;
		}
	}	
	// bitwise and
	void operator&=(const DoubleInteger<T> &n){
		lower &= n.lower;		
		upper &= n.upper;
	}	
	// bitwise or
	void operator|=(const DoubleInteger<T> &n){
		lower |= n.lower;		
		upper |= n.upper;
	}	
	// logical !=
	bool operator!=(const DoubleInteger<T> &n)const{
		return ((upper != n.upper) || (lower != n.lower)); 
	}
	// logical >
	bool operator>(const DoubleInteger<T> &n)const{
		return ((upper > n.upper) || ((upper == n.upper) && (lower > n.lower))); 
	}
	// bitwise invert
	DoubleInteger<T>  operator~()const{
		DoubleInteger<T>output;
		output.upper = ~upper;
		output.lower = ~lower;
		return output;
	}
	// bitwise and 
	DoubleInteger<T>  operator&(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		output.upper = upper & n.upper;
		output.lower = lower & n.lower;
		return output;
	}
	// bitwise or 
	DoubleInteger<T>  operator|(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		output.upper = upper | n.upper;
		output.lower = lower | n.lower;
		return output;
	}
	// math subtract binary op 
	DoubleInteger<T>  operator-(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		// subtract upper
		output.upper = upper - n.upper;
		// if there will be an underflow borrow from upper
		if (n.lower > lower) output.upper--;
		// subtract lower
		output.lower = lower - n.lower;
		return output;
	}
	// math add binary op
	DoubleInteger<T>  operator+(const DoubleInteger<T> &n)const{
		DoubleInteger<T>output;
		// add upper
		output.upper = upper + n.upper;
		// add lower
		output.lower = lower + n.lower;
		// handle overflow
        if ((output.lower < lower) && (output.lower < n.lower)) output.upper++;  			
		return output;
	}
	// right shift binary op
	DoubleInteger<T>  operator>>(uint8 shift)const{
		DoubleInteger<T>output(*this);
		// uses unary op
		output >>= shift;		
		return output;
	}
	// left shift binary op
	DoubleInteger<T>  operator<<(uint8 shift)const{
		DoubleInteger<T>output(*this);
		// uses unary op
		output <<= shift;		
		return output;
	}
};

#endif
