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
 * @class DoubleIntegerTest
 * @brief Tests the DoubleInteger functions.
 *
 * The test consists in observing the results of the conversion from integer-bitSet to a string, and its corrent print
 * on a generic stream which implements a generic PutC(c) function.
 */

#ifndef DOUBLE_INTEGER_TEST_H
#define DOUBLE_INTEGER_TEST_H

#include "DoubleInteger.h"
#include "FormatDescriptor.h"

class DoubleIntegerTest{

public:

	bool TestShift();
	bool TestLogicalOperators();
	bool TestMathematicOperators();

};

#endif


