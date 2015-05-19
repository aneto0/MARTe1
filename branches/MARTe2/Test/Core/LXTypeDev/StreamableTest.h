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
 * @class StreamableTest.h
 * @brief Tests the Streamable functions.
 *
 * Tests the Streamable functions and operators. */

#ifndef STREAM_STRING_TEST_H
#define STREAM_STRING_TEST_H

#include "Streamable.h"
#include "FormatDescriptor.h"


/** @brief Class for testing of Streamable functions. */
class StreamableTest {

private:

public:

	/**
 	 * @brief Tests the streamString Seek and RelativeSeek.
         * @param stringToRead is the string already on the stream.
	 * @param stringToWrite is the string to write on the stream.
	 * @return true if the seek operations returns the correct result. 
	 *
	 * Test the seek functions in different conditions, for example using a positions which falls out of bounds. */	 
	bool TestSeek();


	bool TestPrint();
	
	bool TestToken();

};

#endif
