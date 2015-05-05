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
 * $Id:$
 *
 **/
/**
 * @class FloatToStreamTest
 * @brief Tests the Float To stream functions.
 *
 * The test consists in observing the results of the conversions from float (or equivalent) numbers and strings and their correct
 * print on a generic stream which implements a PutC(c) function.
 */

#ifndef FLOAT_TO_STREAM_TEST_H
#define FLOAT_TO_STREAM_TEST_H

#include "FloatToStream.h"
#include "FormatDescriptor.h"

class FloatToStreamTest {

private:

public:

    /** Tests the print of a float on a stream in fixed point notation (precision = #decimals) */
    bool TestFixedPoint();
 
    /** Tests the print of a float on a stream in fixed point relative notation (precision = #significative digits). */
    bool TestFixedRelativePoint();
    
    /** Tests the print of a float on a stream in engineering notation (precision = #significative digits). */
    bool TestEngeneering();
    
    /** Tests the print of a float on a stream in smart notation (precision = #significative digits). */
    bool TestSmart();

    /** Tests the print of a float on a stream in exponential notation (precision = #significative digits). */
    bool TestExponential();
    
    /** Tests the print of a float on a stream in compact notation (precision = #significative digits). */
    bool TestCompact();
};

#endif
