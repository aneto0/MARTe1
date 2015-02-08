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
/**
 * @file
 * Macro definition of some mathematical constanst and some 
 * optimised mathematical operations.
 */
#ifndef FAST_MATH_H
#define FAST_MATH_H

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_ARCHITECTURE(ARCHITECTURE,FastMathA.h)

class FastMath {
public:
    // Math constants
    /** e */
    static const double E        = 2.7182818284590452354;
    /** log2(e) */
    static const double LOG2E    = 1.4426950408889634074;
    /** log10(e) */
    static const double LOG10E   = 0.43429448190325182765;
    /** log(2) */
    static const double LN2      = 0.69314718055994530942;
    /** log(10) */
    static const double LN10     = 2.30258509299404568402;
    /** pi */
    static const double PI       = 3.14159265358979323846;
    /** pi/2 */
    static const double PI_2     = 1.57079632679489661923;
    /** pi/4 */
    static const double PI_4     = 0.78539816339744830962;
    /** 1/pi */
    static const double _1_PI     = 0.31830988618379067154;
    /** 2/pi */
    static const double _2_PI     = 0.63661977236758134308;
    /** 2/sqrt(pi)*/
    static const double _2_SQRTPI = 1.12837916709551257390;
    /** sqrt(2) */
    static const double SQRT2    = 1.41421356237309504880;
    /** sqrt(1/2) */
    static const double SQRT1_2  = 0.70710678118654752440;

    /**     
     * Converts a float to an integer using a processor instruction. 
     * @param input the value to convert
     * @return the input as an integer
     */
    static inline int32 FloatToInt(float input){
        return FastMathFloatToInt(input);
    }

    /**     
     * Computes the cosine of an angle using a processor instruction. 
     * @param param the angle to compute 
     * @return the cos(angle)
     */
    static inline float Cos(float angle){
        return FastMathCos(angle);
    }

    /**     
     * Computes the sine of an angle using a processor instruction. 
     * @param param the angle to compute 
     * @return the sin(angle)
     */
    static inline float Sin(float angle){
        return FastMathSin(angle);
    }
};

#endif

