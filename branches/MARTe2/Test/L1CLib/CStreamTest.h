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
 * @class CStreamTest
 * @brief Tests the CStream functions.
 *
 * The test consists in observe the results of the various operators between BStrings, const char* and char.
 */

#ifndef CSTREAM_TEST_H
#define CSTREAM_TEST_H

#include "CStream.h"
#include "Memory.h"



struct CStreamContext{
	int32 counter;
	int32 buffGranularity;
	char* begin;	
};



void CreateNewBuffer(CStream* p);


class CStreamTest {

private:

public:
	struct CStreamContext myContext;
	struct CStream myCStream;

    CStreamTest() {
	myContext.counter=0;
	myContext.buffGranularity=500;
	myContext.begin=NULL;
	myCStream.context=(void*)(&myContext);
	myCStream.bufferPtr=NULL;
	myCStream.NewBuffer=CreateNewBuffer;
	myCStream.sizeLeft=0;
    }

    bool TestCPut(char c);	
    bool TestCGet(char c);
    bool TestCRead(const char* string);
    bool TestCWrite(const char* string);
    bool TestPrintInt32(const char* sDec, const char* uDec, const char* hex, const char* oct, int32 number);
    bool TestPrintInt64(const char* sDec, const char* hex, const char* oct, int64 number);
    bool TestPrintDouble();
    bool TestPrintString(const char* string);
    bool TestCPrintf();
    bool TokenTest();
};

#endif
