/* Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they
 will be approved by the European Commission - subsequent
 versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 Licence.
 * You may obtain a copy of the Licence at:
 *
 * http: //ec.europa.eu/idabc/eupl
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

#include "GeneralDefinitions.h"
#include "BitSetToIntegerTest.h"
#include "StringTestHelper.h"
#include "StreamTestHelper.h"
#include "stdio.h"

bool BitSetToIntegerTest::TestBitSetToBitSet(){

	//128 bit source
	int32 source[]={0xffffffff, 0x0, 0x01, 0x0};
	//128 bit dest
	int32 dest[4];

	uint8 destShift=0;
	uint8 sourceShift=0;
	uint8 destSize=32;
	uint8 sourceSize=32;
	int32 *sPointer=source;
	int32 *dPointer=dest;

	//Copy the first position in the first position
	BitSetToBitSet(dPointer, destShift, destSize, true, sPointer, sourceShift, sourceSize, true);
	if(dest[0]!=source[0]){
		return False;
	}

	destShift=32;
	sourceShift=0;
	sPointer=source;
	dPointer=dest;
	//Copy the first position in the second position
	BitSetToBitSet(dPointer, destShift, destSize, true, sPointer, sourceShift, sourceSize, true);
	printf("\n%d %d\n", dest[1], source[0]);
	if(dest[1]!=source[0]){
		return False;
	}

	//Copy 0x0000ffff (between first and second, the second is up) in the third position.
	destShift=64;
	sourceShift=16;
	sPointer=source;
	dPointer=dest;
	int32 test=(int32)0x0000ffff;
	BitSetToBitSet(dPointer, destShift, destSize, true, sPointer, sourceShift, sourceSize, true);
	printf("\n%d %d \n", dest[2], test);
	if(dest[2]!=test){
		return False;
	}

	return True;
}



bool BitSetToIntegerTest::TestBitSetToInteger(){
	return True;
}


bool BitSetToIntegerTest::TestIntegerToBitSet(){
	return True;
}
