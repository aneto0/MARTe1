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

#ifndef NUMBER_TO_STRING_TEST_H
#define NUMBER_TO_STRING_TEST_H

#include "NumberToString.h"
#define MAX_DIMENSION 128 


class myStream{
private:
	char buffer[MAX_DIMENSION];
	int32 size;
public:

	myStream(){
		size=0;
	}
	
	void PutC(char c){
		size%=(MAX_DIMENSION-1);
			
		
		buffer[size]=c;
		buffer[size+1]=0;
		size++;		
	}
	
	char* Buffer(){
		return buffer;
	}

	void Clear(){
		size=0;
	}
};


class NumberToStringTest{

private:

public:

	bool TestDecimalMagnitude();
	bool TestHexadecimalMagnitude();
	bool TestOctalMagnitude();
	bool TestBinaryMagnitude();
	bool TestDecimalStream();
	bool TestHexadecimalStream();
	bool TestOctalStream();
	bool TestBinaryStream();
	bool TestDecimalPrint();
	bool TestHexadecimalPrint();
};

#endif