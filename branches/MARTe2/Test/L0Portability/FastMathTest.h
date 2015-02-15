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
 * $Id$
 *
**/

#ifndef FASTMATH_TEST_H
#define FASTMATH_TEST_H

#include "FastMath.h"

class FastMathTest {
private:
    static const float SIN_1    = 0.841470984807;
    static const float SIN_PI_2 = 1.0;
    static const float SIN_PI_4 = 0.707106781186;

    static const float COS_1    = 0.540302305868;
    static const float COS_PI_2 = 0.0;
    static const float COS_PI_4 = 0.707106781186;

    static const float EPSILON  = 0.000001;
    


public:
    FastMathTest(){
    }

    /**
     * Tests the fast float to int32 conversion
     */
    bool TestFloatToInt32(float testFloat, int32 expectedValue){
        int32 testInt32 = 0;

        testInt32 = FastMath::FloatToInt(testFloat);

        return (testInt32 == expectedValue);
    }

    /**
     * Tests the fast Sin function
     */
    bool TestSin(float angle, float expectedValue){
	float result = 0.0;
        bool testResult = false;

        result = FastMath::Sin(angle);

        if ( (result >= expectedValue-EPSILON) && (result <= expectedValue+EPSILON) ) testResult = true;

        return testResult;
    }

    /**
     * Tests the fast Cos function
     */
    bool TestCos(float angle, float expectedValue){
	float result = 0.0;
        bool testResult = false;

        result = FastMath::Cos(angle);

        if ( (result >= expectedValue-EPSILON) && (result <= expectedValue+EPSILON) ) testResult = true;

        return testResult;
    }

    


    /**
     * Executes all the tests
     */
    bool All(){
        bool ok = TestFloatToInt32(42.24, 42);
        ok      = ok && TestFloatToInt32(-42.24, -42);
        ok      = ok && TestSin(1, SIN_1);
        ok      = ok && TestSin(FastMath::PI_2, SIN_PI_2);
        ok      = ok && TestSin(FastMath::PI_4, SIN_PI_4);
        ok      = ok && TestCos(1, COS_1);
        ok      = ok && TestCos(FastMath::PI_2, COS_PI_2);
        ok      = ok && TestCos(FastMath::PI_4, COS_PI_4);
        return ok;
    }
};

#endif

