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

#ifndef SAFE_SHIFT_TEST_H
#define SAFE_SHIFT_TEST_H

#include "GeneralDefinitions.h"
#include "SafeShift.h"

class SafeShiftTest{

private:
	SafeShift number;
public:

    /**
     * @param testValue the value to be tested by the all
     * the test functions
     */
    SafeShiftTest() {
    }

  
    bool TestLogicalRightShift() {
	int8 sbit8=(int8)0xf0;
	number.Set(sbit8);
	
	if(number>>9 != 0){
		return False;
	}

	if(number>>4 !=(int8) 0x0f){
		return False;
	}

	int16 sbit16=(int16)0xf000;
	number.Set(sbit16);
	
	if(number>>16 != 0){
		return False;
	}

	if(number>>4 !=(int16) 0xf00){
		return False;
	}
	int32 sbit32=(int32)0xf0000000;
	number.Set(sbit32);
	
	if(number>>33 != 0){
		return False;
	}

	if(number>>4 !=(int32) 0xf000000){
		return False;
	}

	int64 sbit64=(int64)0xf000000000000000;
	number.Set(sbit64);
	
	if(number>>63 != 1){
		return False;
	}

	if(number>>0 !=(int64) 0xf000000000000000){
		return False;
	}

	return True;

    }

    bool TestLogicalLeftShift(){
	int8 sbit8=(int8)0xf;
	number.Set(sbit8);
	
	if(number<<9 != 0){
		return False;
	}

	if(number<<4 !=(int8) 0xf0){
		return False;
	}

	int16 sbit16=(int16)0xf;
	number.Set(sbit16);
	
	if(number<<16 != 0){
		return False;
	}

	if(number<<4 !=(int16) 0xf0){
		return False;
	}
	int32 sbit32=(int32)0xf;
	number.Set(sbit32);
	
	if(number<<33 != 0){
		return False;
	}

	if(number<<4 !=(int32) 0xf0){
		return False;
	}

	int64 sbit64=(int64)0x0f0000000000000;
	number.Set(sbit64);
	
	if(number<<8 != 1){
		return False;
	}

	if(number>>0 !=(int64) 0x0f00000000000000){
		return False;
	}
	
	return True;

    }

    bool TestMatematicRightShift() {
	return True;
    }

    bool TestMatematicRightShift() {
	return True;
    }
   

};

#endif
