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
 * Tests the streamable (for example a file) functions, testing the read, write and seek operation in buffered/unbuffered modes
 * and on a read/write/read&write stream. */

#ifndef STREAMABLE_TEST_H
#define STREAMABLE_TEST_H

#include "Streamable.h"
#include "FormatDescriptor.h"


/** @brief Class for testing of Streamable functions. */
class StreamableTest {

private:

public:

  	/**
	 * @brief Tests the streamable GetC function.
	 * @param inputString is the string on the stream.
         * @return true if the read string is equal to the parameter.
         *
         * The inputString is copied on the stream and then read using GetC in unbuffered and
         * buffered modes. */ 
	bool TestGetC(const char* inputString);

	/** 
         * @brief Test the streamable PutC function.
	 * @param inputString is the string to put on the stream. 
         * @return true if on the stream is copied the parameter.
         *
         * The inputString is written on the stream using the PutC function in unbuffered and
         * buffered modes. */
	bool TestPutC(const char* inputString);


	/** 
         * @brief Tests the GetC and PutC streamable functions on a read&write stream.
 	 * @param inputString is the string parameter.
      	 * @return true if the test goes fine.
 	 *
 	 * The inputString is copied on the stream and a character is read using buffered GetC.
 	 * Then using PutC a character of inputString is written on the stream.
	 * The test return true if on the stream the first two chars are equal to the first char of inputString. */
	bool TestGetCAndPutC(const char* inputString);

	/**
	 * @brief Tests the streamable read function.
	 * @param inputString is the string on the stream.
 	 * @return true if the read string is equal to inputString using Read function in buffered and unbuffered modes. */ 
	bool TestRead(const char* inputString);

	/**
	 * @brief Tests the streamable write function.
	 * @param inputString is the string to write on the stream.
 	 * @return true if the string written on the stream using Write function in buffered and unbuffered modes is equal to inputString.*/
	bool TestWrite(const char* inputString);

	/**
	 * @brief Tests the Read And Write functions on a read&write stream.
 	 * @param stringToRead is the stream already on the stream.
	 * @param stringToWrite is the string to write on the stream.
	 * @return true if tests goes fine.
	 *
	 * Testing Read and Write in buffered mode on a read&write stream, checking that the read string
	 * is equal to stringToRead and the string written on the stream is equal to stringToWrite. */
	bool TestReadAndWrite(const char* stringToRead, const char* stringToWrite);

	/**
 	 * @brief Tests the streamable Seek and RelativeSeek.
         * @param stringToRead is the string already on the stream.
	 * @param stringToWrite is the string to write on the stream.
	 * @return true if the seek operations returns the correct result. 
	 *
	 * Test the seek functions in different conditions, for example using a positions which falls in
	 * the buffer and none. */	 
	bool TestSeek(const char* stringToRead, const char* stringToWrite);

	/** 
	 * @brief Tests the streamable Switch, RemoveStream and other functions mostly implemented in the derived class of Streamable. 
         * @param onStream1 is a string on the first stream.
  	 * @param onStream2 is a string on the second stream.
 	 * @return true if tests goes fine.
         *
 	 * For an use case of these functions see the derived class implementation (SimpleStreamable class in StreamTestHelper.h) 
	 * where we have two different stream buffers. */
	bool TestSwitch(const char* onStream1, const char* onStream2);


	bool TestPrint();

	bool TestToken();
};

#endif
