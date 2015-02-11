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
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

#ifndef FAST_MATH_A_H
#define FAST_MATH_A_H

#include "math.h"

/**
 * @see FastMath::Sin
 */
static inline float FastMathSin(float angle){
	return sin(angle);
}

/**
 * @see FastMath::Cos
 */
static inline float FastMathCos(float angle){
	return cos(angle);
}

/** 
 * @see FastMath::FloatToInt 
 */
static inline int32 FastMathFloatToInt(float input){
	volatile int temp;
	__asm  {
		fld input;
		fistp temp;
	}
	return temp;
}
#endif

