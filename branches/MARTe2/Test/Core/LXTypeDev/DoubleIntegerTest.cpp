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
#include "DoubleIntegerTest.h"
#include "StringTestHelper.h"
#include "StreamTestHelper.h"
#include "stdio.h"




bool DoubleIntegerTest::TestShift(){
	//init a 64 bit integer.
	DoubleInteger<int32> zero;
	
	//== operator
	if(!(zero == 0)){
		return False;
	}

	//= operator
	DoubleInteger<int32> sbit64((int64)0xf000000000000000);

	//!= operator
	if(sbit64 != (int64)0xf000000000000000){
		return False;
	}

	//Math shift with sign extension.
	if((sbit64 >> 60) != -1){
		printf("\n%d %d\n",sbit64.upper,sbit64.lower);
		return False;
	}

	//Copy bit a bit.
	DoubleInteger<uint32> ubit64(0xf000000000000000);

	//Math shift without sign extension.
	if((ubit64 >> 60) != (int64)0xf){
		return False;
	}

	if((sbit64 << 3) != (int64)0x8000000000000000){
		return False;
	}


	ubit64=0xffffffff2;
	uint32 i=2;
	printf("\n%u %u\n", ubit64.upper, ubit64.lower);
	DoubleInteger<uint32> two(2);

	ubit64-=two;	
	printf("\n%u %u\n", ubit64.upper, ubit64.lower);
	return True;
}

bool DoubleIntegerTest::TestLogicalOperators(){
	return True;
}

bool DoubleIntegerTest::TestMathematicOperators(){
	return True;
}
