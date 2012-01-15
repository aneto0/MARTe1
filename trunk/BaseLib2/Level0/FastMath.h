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
 * $Id$
 *
**/

/**
 * @file
 * Macro definition of some mathematical constanst and some 
 * optimised mathematical operations.
 */
#ifndef _FAST_MATH_H
#define _FAST_MATH_H

#include "GenDefs.h"

// Maths constant macros
#ifndef M_E
/** e */
#define M_E             2.7182818284590452354
#endif
#ifndef M_LOG2E
/** log2(e) */
#define M_LOG2E         1.4426950408889634074
#endif
#ifndef M_LOG10E
/** log10(e) */
#define M_LOG10E        0.43429448190325182765
#endif
#ifndef M_LN2
/** log(2) */
#define M_LN2           0.69314718055994530942
#endif
#ifndef M_LN10
/** log(10) */
#define M_LN10          2.30258509299404568402
#endif
#ifndef M_PI
/** pi */
#define M_PI            3.14159265358979323846
#endif
#ifndef M_PI_2
/** pi/2 */
#define M_PI_2          1.57079632679489661923
#endif
#ifndef M_PI_4
/** pi/4 */
#define M_PI_4          0.78539816339744830962
#endif
#ifndef M_1_PI
/** 1/pi */
#define M_1_PI          0.31830988618379067154
#endif
#ifndef M_2_PI
/** 2/pi */
#define M_2_PI          0.63661977236758134308
#endif
#ifndef M_2_SQRTPI
/** 2/sqrt(pi)*/
#define M_2_SQRTPI      1.12837916709551257390
#endif
#ifndef M_SQRT2
/** sqrt(2) */
#define M_SQRT2         1.41421356237309504880
#endif
#ifndef M_SQRT1_2
/** sqrt(1/2) */
#define M_SQRT1_2       0.70710678118654752440
#endif

/** Converts a float to an integer using a processor instruction. */
static inline int32 FastFloat2Int(float input){
#if defined(_CINT)
    return 0;
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

    volatile int64 output;

    asm volatile(
        "fctiw %0,%1; " : "=f" (output) : "f" (input)
    );
    return output;

#elif defined(_CY32) || defined(_EMX_) || defined(_RSXNT)
    int32 output;
    asm(
        "flds %1; "
        "fistpl %0;"
        : "=m" (output) : "m" (input)
    );
    return output;

#elif defined(_MSC_VER)
    volatile int temp;
    __asm  {
        fld input;
        fistp temp;
    }
    return temp;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    volatile int32 output;
    __asm__ __volatile__(
        "fld   %1;\n"
        "fistpl %0;"
        : "=m" (output) : "m" (input)
        );
    return output;
#else
     // Get the largest integral value not greater than input
     double a = floor(input);

     if( (input-a) <= 0.5 ) return (int)a;
     else return (int)(a+1);
#endif
}

static inline float FastCos(float angle){
#if defined(_CINT)
    return 0;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    volatile float output;
    __asm__ __volatile__(
         "fcos;"
        : "=t" (output) : "0" (angle)
        );
    return output;
#else
     return cos(angle);
#endif
}

static inline float FastSin(float angle){
#if defined(_CINT)
    return 0;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    volatile float output;
    __asm__ __volatile__("fsin" : "=t" (output) : "0" (angle));
    return output;
#else
     return sin(angle);
#endif
}

#endif

