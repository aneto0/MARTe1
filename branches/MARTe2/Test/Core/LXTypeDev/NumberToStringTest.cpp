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
#include "NumberToStringTest.h"
#include "StringTestHelper.h"
#include "stdio.h"


bool NumberToStringTest::TestDecimalMagnitude(){
	uint8 bit8=255;
	uint8 order;
	
	if((order=GetOrderOfMagnitude(bit8))!=2){
		printf("\n ret1=%d", order);		
		return False;
	}

	uint16 bit16=12345;

	if((order=GetOrderOfMagnitude(bit16))!=4){
		printf("\n ret2=%d", order);
		return False;
	}

	uint32 bit32=123456789;
	if((order=GetOrderOfMagnitude(bit32))!=8){
		printf("\n ret3=%d", order);
		return False;
	}
	
	uint64 bit64= 12345678912345678;
	if((order=GetOrderOfMagnitude(bit64))!=16){
		printf("\n ret4=%d %lld", order, bit64);
		return False;
	}	

	return True;
}



bool NumberToStringTest::TestHexadecimalMagnitude(){
	uint8 bit8=0x81;
	uint8 order;
	
	if((order=GetOrderOfMagnitudeHex(bit8))!=1){
		printf("\n ret1=%d", order);		
		return False;
	}

	uint16 bit16=0xf123;

	if((order=GetOrderOfMagnitudeHex(bit16))!=3){
		printf("\n ret2=%d", order);
		return False;
	}

	uint32 bit32=0xaf010a31;
	if((order=GetOrderOfMagnitudeHex(bit32))!=7){
		printf("\n ret3=%d", order);
		return False;
	}
	
	uint64 bit64= 0xedab12348976aef2;
	if((order=GetOrderOfMagnitudeHex(bit64))!=15){
		printf("\n ret4=%d %lld", order, bit64);
		return False;
	}	

	return True;
}



bool NumberToStringTest::TestOctalMagnitude(){
	uint8 bit8=0x81;
	uint8 order;
	
	if((order=GetOrderOfMagnitudeOct(bit8))!=2){
		printf("\n ret1=%d", order);		
		return False;
	}

	uint16 bit16=0xf123;

	if((order=GetOrderOfMagnitudeOct(bit16))!=5){
		printf("\n ret2=%d", order);
		return False;
	}

	uint32 bit32=0xff010a31;
	if((order=GetOrderOfMagnitudeOct(bit32))!=10){
		printf("\n ret3=%d", order);
		return False;
	}
	
	uint64 bit64= 0xadab12348976aef2;
	if((order=GetOrderOfMagnitudeOct(bit64))!=21){
		printf("\n ret4=%d %lld", order, bit64);
		return False;
	}	
	
	bit64= 0x7dab12348976aef2;

	if((order=GetOrderOfMagnitudeOct(bit64))!=20){
		printf("\n ret1=%d", order);		
		return False;
	}
	return True;
}



bool NumberToStringTest::TestBinaryMagnitude(){
	uint8 bit8=0x81;
	uint8 order;
	
	if((order=GetOrderOfMagnitudeBin(bit8))!=7){
		printf("\n ret1=%d", order);		
		return False;
	}

	bit8=0x71;

	if((order=GetOrderOfMagnitudeBin(bit8))!=6){
		printf("\n ret1=%d", order);		
		return False;
	}

	uint16 bit16=0xf123;

	if((order=GetOrderOfMagnitudeBin(bit16))!=15){
		printf("\n ret2=%d", order);
		return False;
	}

	uint32 bit32=0xff010a31;
	if((order=GetOrderOfMagnitudeBin(bit32))!=31){
		printf("\n ret3=%d", order);
		return False;
	}
	
	uint64 bit64= 0xedab12348976aef2;
	if((order=GetOrderOfMagnitudeBin(bit64))!=63){
		printf("\n ret4=%d %lld", order, bit64);
		return False;
	}	

	return True;
}



bool NumberToStringTest::TestDecimalStream(){

	myStream thisStream;
	uint8 ubit8=255;
	NumberToDecimalStream(thisStream, ubit8);
	printf("\nT buffer u8: %s\n", thisStream.Buffer());

	thisStream.Clear();

	//LeftAligned and not padded (nothing happen)
	NumberToDecimalStream(thisStream, ubit8, 5, false, true);
	printf("\nT buffer u8: %s!\n", thisStream.Buffer());
	
	thisStream.Clear();
	int8 sbit8=-127;
	NumberToDecimalStream(thisStream, sbit8);
	printf("\nT buffer s8: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	//LeftAligned and padded "...  "
	NumberToDecimalStream(thisStream, sbit8, 6, true, true);
	printf("\nT buffer s8 (...  ): %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	uint16 ubit16=12345;
	NumberToDecimalStream(thisStream, ubit16);
	printf("\nT buffer u16: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	//LeftAligned and padded with positive sign "+...  "
	NumberToDecimalStream(thisStream, ubit16, 8, true, true, true);
	printf("\nT buffer u16 (+...  ): %s!\n", thisStream.Buffer());

	thisStream.Clear();
		
	uint16 sbit16=-12345;
	NumberToDecimalStream(thisStream, sbit16);
	printf("\nT buffer s16: %s!\n", thisStream.Buffer());

	thisStream.Clear();

	uint32 ubit32=123456789;
	NumberToDecimalStream(thisStream, ubit32);
	printf("\nT buffer u32: %s!\n", thisStream.Buffer());

	thisStream.Clear();

	//RightAligned and padded with sign " +..."
	NumberToDecimalStream(thisStream, ubit32, 11, true, false, true);
	printf("\nT buffer u32 ( +...): %s!\n", thisStream.Buffer());
	
	thisStream.Clear();

	int32 sbit32=-123456789;

	NumberToDecimalStream(thisStream, sbit32);
	printf("\nT buffer s32: %s!\n", thisStream.Buffer());
	
	thisStream.Clear();
	uint64 ubit64= 12345678912345678;
	NumberToDecimalStream(thisStream, ubit64);
	printf("\nT buffer u64: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	int64 sbit64= -12345678912345678;
	NumberToDecimalStream(thisStream, sbit64);
	printf("\nT buffer s64: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	//? if the maxSize is incorrect	
	NumberToDecimalStream(thisStream, sbit64, 11, true, false, true);
	printf("\nT buffer s64 ?: %s!\n", thisStream.Buffer());
	
	return True;

}




bool NumberToStringTest::TestHexadecimalStream(){

	myStream thisStream;
	uint8 ubit8=0xea;
	
	NumberToHexadecimalStream(thisStream, ubit8);
	printf("\nT buffer u8 EA: %s\n", thisStream.Buffer());

	thisStream.Clear();

	//LeftAligned and not padded (nothing happen)
	NumberToHexadecimalStream(thisStream, ubit8, 5, false, true);
	printf("\nT buffer u8 EA: %s!\n", thisStream.Buffer());
	

	//Add Header and trailing zeros
	thisStream.Clear();
	NumberToHexadecimalStream(thisStream, ubit8, 0, false, false, true, true);
	printf("\nT buffer u8 0xEA: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	ubit8=0xf;
	//Add only trailing zeros
	thisStream.Clear();
	NumberToHexadecimalStream(thisStream, ubit8, 0, false, false, true, false);
	printf("\nT buffer u8 0F: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	//All true with a space more... it must print only a space after.
	uint16 ubit16=0xabcd;
	NumberToHexadecimalStream(thisStream, ubit16, 7, true, true, true, true);
	printf("\nT buffer u16 0xABCD !: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	ubit16=0xcd;
	//With zeros and 3 as number of digits (without header).
	NumberToHexadecimalStream(thisStream, ubit16, 5, true, true, true, true);
	printf("\nT buffer u16 0x0CD: %s!\n", thisStream.Buffer());

	thisStream.Clear();
	
	//Only right aligned with header
	uint32 ubit32=0xabcdef78;
	NumberToHexadecimalStream(thisStream, ubit32, 12, true, false, false, true);
	printf("\nT buffer u32 !  0xABCDEF78: %s!\n", thisStream.Buffer());

	thisStream.Clear();

	ubit32= 0x00abcd0f;
	//Right align with zero and header
	NumberToHexadecimalStream(thisStream, ubit32, 11, true, false, true, true);
	printf("\nT buffer u32 ! 0x00ABCD0F: !%s!\n", thisStream.Buffer());
	
	thisStream.Clear();

	//Right align without zeros and header
	NumberToHexadecimalStream(thisStream, ubit32, 11, true, false, false, false);
	printf("\nT buffer u32 !     ABCD0F!: !%s!\n", thisStream.Buffer());

	
	thisStream.Clear();

	//padded=false
	uint64 ubit64= 0x89abcdef0123fff0;
	NumberToHexadecimalStream(thisStream, ubit64, 120, false);
	printf("\nT buffer u64 !89ABCDEF0123FFF0!: !%s!\n", thisStream.Buffer());
	thisStream.Clear();
	//?
	ubit64= 0x123fff0;
	NumberToHexadecimalStream(thisStream, ubit64, 5, true);
	printf("\nT buffer u64 ?: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	ubit64= 0xfff0; 
	NumberToHexadecimalStream(thisStream, ubit64, 7, true, true, true);
	printf("\nT buffer u64 000FFF0: !%s!\n", thisStream.Buffer());

	return True;

}




bool NumberToStringTest::TestOctalStream(){

	myStream thisStream;
	uint8 ubit8=0xea;
	

	NumberToOctalStream(thisStream, ubit8);
	printf("\nT buffer u8 352: %s\n", thisStream.Buffer());

	thisStream.Clear();

	//LeftAligned and not padded (nothing happen)
	NumberToOctalStream(thisStream, ubit8, 5, false, true);
	printf("\nT buffer u8 352: %s!\n", thisStream.Buffer());
	

	//Add Header and trailing zeros
	thisStream.Clear();
	NumberToOctalStream(thisStream, ubit8, 0, false, false, true, true);
	printf("\nT buffer u8 0o352: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	ubit8=0xf;
	//Add only trailing zeros
	thisStream.Clear();
	NumberToOctalStream(thisStream, ubit8, 0, false, false, true, false);
	printf("\nT buffer u8 017: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	//All true with a space more... it must print a space after.
	uint16 ubit16=0x7bcd;//6 is the maximum now are 5
	NumberToOctalStream(thisStream, ubit16, 9, true, true, true, true);
	printf("\nT buffer u16 0o075715 !: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	ubit16=0xcd;
	//With zeros and 5 as number of digits (without header).
	NumberToOctalStream(thisStream, ubit16, 7, true, true, true, true);
	printf("\nT buffer u16 0o00315!: %s!\n", thisStream.Buffer());

	thisStream.Clear();
	
	//Only right aligned with header
	uint32 ubit32=0xabcdef78;
	NumberToOctalStream(thisStream, ubit32, 15, true, false, false, true);
	printf("\nT buffer u32 !  0o25363367570: %s!\n", thisStream.Buffer());

	thisStream.Clear();

	ubit32= 0xcd0f;
	//Right align with zero and header
	NumberToOctalStream(thisStream, ubit32, 15, true, false, true, true);
	printf("\nT buffer u32 !  0o00000146417: !%s!\n", thisStream.Buffer());
	
	thisStream.Clear();

	//Right align without zeros and header
	NumberToOctalStream(thisStream, ubit32, 15, true, false, false, false);
	printf("\nT buffer u32 !         146417: !%s!\n", thisStream.Buffer());

	
	thisStream.Clear();

	//padded=false
	uint64 ubit64= 0x89abcdef01240000;
	NumberToOctalStream(thisStream, ubit64, 120, false);
	printf("\nT buffer u64 !1046536336740111000000!: !%s!\n", thisStream.Buffer());
	thisStream.Clear();
	//?
	ubit64= 0x123fff0;
	NumberToOctalStream(thisStream, ubit64, 5, true);
	printf("\nT buffer u64 ?: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	ubit64= 0xfff0; 
	NumberToOctalStream(thisStream, ubit64, 7, true, true, true);
	printf("\nT buffer u64 0177760: !%s!\n", thisStream.Buffer());

	return True;

}


bool NumberToStringTest::TestBinaryStream(){

	myStream thisStream;
	uint8 ubit8=0xea;
	
	NumberToBinaryStream(thisStream, ubit8);
	printf("\nT buffer u8 11101010: %s\n", thisStream.Buffer());

	thisStream.Clear();

	//LeftAligned and not padded (nothing happen)
	NumberToBinaryStream(thisStream, ubit8, 50, false, true);
	printf("\nT buffer u8 11101010: %s!\n", thisStream.Buffer());
	

	//Add trailing zeros
	thisStream.Clear();
	NumberToBinaryStream(thisStream, ubit8, 0, false, false, true);
	printf("\nT buffer u8 11101010: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	ubit8=0xf;
	//Add only trailing zeros
	thisStream.Clear();
	NumberToBinaryStream(thisStream, ubit8, 0, false, false, true);
	printf("\nT buffer u8 00001111: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	//All true with a space more... it must print a space after.
	uint16 ubit16=0x7bcd;//
	NumberToBinaryStream(thisStream, ubit16, 17, true, true, true);
	printf("\nT buffer u16 0111101111001101 !: %s!\n", thisStream.Buffer());
	thisStream.Clear();

	ubit16=0xcd;
	//With zeros and 10 as number of digits (without header).
	NumberToBinaryStream(thisStream, ubit16, 10, true, true, true);
	printf("\nT buffer u16 0011001101!: %s!\n", thisStream.Buffer());

	thisStream.Clear();
	
	//Only right aligned with header
	uint32 ubit32=0xabcdef78;
	NumberToBinaryStream(thisStream, ubit32, 34, true);
	printf("\nT buffer u32 !  10101011110011011110111101111000: !%s!\n", thisStream.Buffer());

	thisStream.Clear();

	ubit32= 0xcd0f;
	//Right align with zero and header
	NumberToBinaryStream(thisStream, ubit32, 34, true, false, true);
	printf("\nT buffer u32 !  00000000000000001100110100001111: !%s!\n", thisStream.Buffer());
	
	thisStream.Clear();

	//Right align without zeros and header
	NumberToBinaryStream(thisStream, ubit32, 34, true, false, false);
	printf("\nT buffer u32 !                  1100110100001111: !%s!\n", thisStream.Buffer());

	
	thisStream.Clear();

	//padded=false
	uint64 ubit64= 0x8000000000000001;
	NumberToBinaryStream(thisStream, ubit64, 120, false);
	printf("\nT buffer u64 !1000000000000000000000000000000000000000000000000000000000000001!: !%s!\n", thisStream.Buffer());
	thisStream.Clear();
	//?
	ubit64= 0x123fff0;
	NumberToBinaryStream(thisStream, ubit64, 5, true);
	printf("\nT buffer u64 ?: %s!\n", thisStream.Buffer());
	thisStream.Clear();
	
	ubit64= 0xff0; 
	NumberToBinaryStream(thisStream, ubit64, 14, true, true, true);
	printf("\nT buffer u64 00111111110000: !%s!\n", thisStream.Buffer());

	return True;

}



bool NumberToStringTest::TestDecimalPrint(){

	char thisStream[70];
	char* retPointer;
	uint16 numberSize=0;
	uint8 ubit8=255;
	uint16 bufferSize=70;
	
	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, ubit8);
	retPointer[numberSize]=0;
	printf("\nT buffer u8: %s %d\n", retPointer, numberSize);


	int8 sbit8=-127;
	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, sbit8);
	printf("\nT buffer s8: %s!\n", retPointer);
	

	int16 ubit16=12345;
	retPointer=NumberToDecimal(numberSize, thisStream,bufferSize, ubit16, true);
	printf("\nT buffer u16: %s!\n", retPointer);

		
	int16 sbit16=-12345;
	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, sbit16, true );
	printf("\nT buffer s16: %s!\n", retPointer);


	uint32 ubit32=123456789;
	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, ubit32);
	printf("\nT buffer u32: %s!\n", retPointer);

	int32 sbit32=-123456789;

	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, sbit32);
	printf("\nT buffer s32: %s!\n", retPointer);
	
	int64 ubit64= 12345678912345678;
	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, ubit64, true);
	printf("\nT buffer u64: %s!\n", retPointer);
	
	int64 sbit64= -12345678912345678;
		retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, sbit64);
	printf("\nT buffer s64: %s!\n", retPointer);

	bufferSize=4;
	//? if the maxSize is incorrect	
	retPointer=NumberToDecimal(numberSize, thisStream, bufferSize, sbit64);
	printf("\nT buffer s64 ?: %s!\n", retPointer);
	return True;

}


bool NumberToStringTest::TestHexadecimalPrint(){

	char thisStream[70];
	char* retPointer;
	uint16 numberSize;
	uint16 bufferSize=70;

	uint8 ubit8=0xea;
	
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit8);
	printf("\nT buffer u8 EA: %s\n", retPointer);


	//Add Header and trailing zeros
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit8, true, true);
	printf("\nT buffer u8 0xEA: %s!\n", retPointer);
	
	ubit8=0xf;
	//Add only trailing zeros
 	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit8, true);
	printf("\nT buffer u8 0F: %s!\n", retPointer);

	
	uint16 ubit16=0xabcd;
 	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit16, false, true);
	printf("\nT buffer u16 0xABCD !: %s!\n",retPointer);

	ubit16=0xcd;
	//With zeros and 3 as number of digits (without header).
 	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit16, true, true);
	printf("\nT buffer u16 0x00CD: %s!\n", retPointer);

	
	//Only header
	uint32 ubit32=0xabcdef78;
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit32, true, true);
	printf("\nT buffer u32 !0xABCDEF78: %s!\n", retPointer);


	ubit32= 0x00abcd0f;
	//Right align with zero and header
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit32, true, true);
	printf("\nT buffer u32 ! 0x00ABCD0F: !%s!\n", retPointer);
	

	//Right align without zeros and header
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit32, false, false);
	printf("\nT buffer u32 !     ABCD0F!: !%s!\n", retPointer);

	

	//padded=false
	uint64 ubit64= 0x89abcdef0123fff0;
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit64, true, false);
	printf("\nT buffer u64 !89ABCDEF0123FFF0!: !%s!\n", retPointer);
	//?
	ubit64= 0x123fff0;
	bufferSize=5;
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit64, false, true);
	printf("\nT buffer u64 ?: %s!\n", retPointer);
	
	bufferSize=70;
	ubit64= 0xfff0; 
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit64, true, false);
	printf("\nT buffer u64 000000000000FFF0: !%s!\n", retPointer);
	
	ubit64= 0xfff0; 
	retPointer=NumberToHexadecimal(numberSize, thisStream, bufferSize, ubit64, false, true);
	printf("\nT buffer u64 0xFFF0: !%s!\n", retPointer);

	return True;

}



/*
//Write on stream an int32 number with different formats
bool CStreamTest::TestPrintInt32(const char* sDec, const char* uDec,
                                 const char* hex, const char* oct,
                                 int32 number) {

    //Allocate new memory
    myCStream.NewBuffer(&myCStream);
    char* begin = myCStream.bufferPtr;

    //Print the maximum negative number 0x8000000. In complement 2 it's -0x7fffffff = -2^31.

    //The '-' going before the space requested for the desired size
    CPrintInt32(&myCStream, -2147483648, 12, 0, 'd');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "- 2147483648")) {
        FreeAll(&myCStream);
        return False;
    }

    //In hexadecimal mode.
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, -2147483648, 9, 0, 'x');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) " 80000000")) {
        FreeAll(&myCStream);
        return False;
    }

    //In octal mode.
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, -2147483648, 13, 0, 'o');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "  20000000000")) {
        FreeAll(&myCStream);
        return False;
    }

    //The '-' going before the padding
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, -2147483648, 12, '*', 'd');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "-*2147483648")) {
        FreeAll(&myCStream);
        return False;
    }

    //In hexadecimal mode.
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, -2147483648, 9, 'x', 'x');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "x80000000")) {
        FreeAll(&myCStream);
        return False;
    }

    //In octal mode.
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, -2147483648, 12, 'x', 'o');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "x20000000000")) {
        FreeAll(&myCStream);
        return False;
    }

    //Write an integer in signed mode on the stream
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, 0, ' ', 'd');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) sDec)) {
        FreeAll(&myCStream);
        return False;
    }

    //Write an integer in unsigned mode on the stream
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, 0, ' ', 'u');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) uDec)) {
        FreeAll(&myCStream);
        return False;
    }

    //Write an integer in hex mode on the stream
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, 0, ' ', 'X');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) hex)) {
        FreeAll(&myCStream);
        return False;
    }

    //Write an integer in oct mode	
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, 0, ' ', 'o');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) oct)) {
        FreeAll(&myCStream);
        return False;
    }

    //Test the zero write on stream for different formats
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, 0, 0, ' ', 'd');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, 0, 0, ' ', 'u');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, 0, 0, ' ', 'X');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, 0, 0, ' ', 'o');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }

    //Test the padding modality. The padding is added before the number and the final size is >= of desiredSize
    uint32 desiredSize = 12;

    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, desiredSize, '*', 'd');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, desiredSize, '*', 'u');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, desiredSize, '*', 'X');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt32(&myCStream, number, desiredSize, '*', 'o');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }
    FreeAll(&myCStream);

    return True;
}

//Write on stream an int64 number with different formats
bool CStreamTest::TestPrintInt64(const char* sDec, const char* hex,
                                 const char* oct, int64 number) {

    //Write a long in signed mode
    myCStream.NewBuffer(&myCStream);
    char* begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, number, 0, ' ', 'd');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) sDec)) {
        FreeAll(&myCStream);
        return False;
    }

    //Write a long in hex mode
    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, number, 0, ' ', 'X');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) hex)) {
        FreeAll(&myCStream);
        return False;
    }

    //Write a long in oct mode
    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, number, 0, 0, 'o');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) oct)) {
        FreeAll(&myCStream);
        return False;
    }

    //Test zero write on the stream
    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, 0, 0, ' ', 'd');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }

    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, 0, 0, ' ', 'X');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, 0, 0, ' ', 'o');
    if (begin[0] != '0') {
        FreeAll(&myCStream);
        return False;
    }

    //Test the padding modality. The padding is added before the number.
    uint32 desiredSize = 23;

    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, number, desiredSize, '*', 'd');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }
    CPrintInt64(&myCStream, number, desiredSize, '*', 'X');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintInt64(&myCStream, number, desiredSize, '*', 'o');
    CPut(&myCStream, '\0');
    if (StringTestHelper::Size(begin) != desiredSize) {
        FreeAll(&myCStream);
        return False;
    }

    FreeAll(&myCStream);
    return True;
}

bool CStreamTest::TestPrintDouble() {

    //Allocare memory and initialize a big and a small double
    myCStream.NewBuffer(&myCStream);

    //Test the default printing
    double myDouble = 2.321;
    const char* string = "2.3e+000";
    char* begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, myDouble, 0, 1, 0, 'e');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //Test the approximation
    myDouble = 99.9999;
    string = "1.0e+002";
    begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, myDouble, 0, 1, 0, 'e');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    double myBigDouble = 139665.99554;
    double myLittleDouble = 0.0395;

    //Test the padding and the e-modality with the big number. Furthermore it tests also the 
    //approximation at the 7th number after the point
    string = "*****1.3966600e+005";
    begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, myBigDouble, 19, 7, '*', 'e');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //Test the padding and the e-modality with the small number. It tests also the approximation at the 1st number
    //after the point
    string = "**4.0e-002";
    begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, myLittleDouble, 10, 1, '*', 'e');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //Create an Inf number.
    double inf = 1.0 / 0.0;

    //Check if it recognizes the inf number. The padding is not considered, but it's added a space for the desired size.
    begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, inf, 5, 7, '*', 'e');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "+Inf ")) {
        FreeAll(&myCStream);
        return False;
    }

    //With a negative sign.
    inf = -1.0 / 0.0;

    begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, inf, 6, 7, '*', 'e');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "-Inf  ")) {
        FreeAll(&myCStream);
        return False;
    }

    //Create a Nan number.
    double nan = 0.0 / 0.0;

    //Test the print of a Nan number.
    begin = myCStream.bufferPtr;
    CPrintDouble(&myCStream, nan, 5, 7, '*', 'f');
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) "NaN  ")) {
        FreeAll(&myCStream);
        return False;
    }

    FreeAll(&myCStream);
    return True;

}

//Test the write of strings on the stream
bool CStreamTest::TestPrintString(const char* string) {

    //Allocate new memory
    myCStream.NewBuffer(&myCStream);
    char* begin = myCStream.bufferPtr;

    //Write string on stream with right and left justify but no padding. It must be the same
    CPrintString(&myCStream, string, 0, 0, True);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }
    begin = myCStream.bufferPtr;
    CPrintString(&myCStream, string, 0, 0, False);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //Add three stars as padding and test right and left justify
    uint32 size = StringTestHelper::Size((char*) string);

    uint32 desiredSize = size + 3;
    char result[32];
    StringTestHelper::Append((char*) string, (char*) "***", result);

    begin = myCStream.bufferPtr;
    CPrintString(&myCStream, string, desiredSize, '*', False);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) result)) {
        FreeAll(&myCStream);
        return False;
    }
    StringTestHelper::Append((char*) "***", (char*) string, result);

    begin = myCStream.bufferPtr;
    CPrintString(&myCStream, string, desiredSize, '*', True);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) result)) {
        FreeAll(&myCStream);
        return False;
    }
    FreeAll(&myCStream);
    return True;
}

//Test the Cprintf function. There are other tests on double
bool CStreamTest::TestCPrintf() {

    //print an int32 on stream
    myCStream.NewBuffer(&myCStream);
    char* begin = myCStream.bufferPtr;
    const char* string = "Int32 number:   13    d   15";
    CPrintf(&myCStream, "Int32 number: %4i %4x %4o", 13, 13, 13);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //print an int64 on stream
    begin = myCStream.bufferPtr;
    string = "Int64 number: 1099500000000 FFFF4E9300 37723511400";
    CPrintf(&myCStream, "Int64 number: %lli %6Lx %3lo", 1099500000000,
            1099500000000, 1099500000000);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //print a double on stream. The default numbers after the point are 6
    string = "Double number: 199.900000";
    begin = myCStream.bufferPtr;
    CPrintf(&myCStream, "Double number: %f", 199.9);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //approximation with f-modality of double
    string = "Double number: 200.0";
    begin = myCStream.bufferPtr;
    CPrintf(&myCStream, "Double number: %.1f", 199.9989);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //if the desired numbers after the point are greather than nine they returns to the default (6)
    //it tests also the % print
    string = "Double number: 199.900000 %";
    begin = myCStream.bufferPtr;
    CPrintf(&myCStream, "Double number: %10f %%", 199.9);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //space and numbers after the point changes for each new entry
    string = "Double number: @@@@@@@@@199.90 1.999000e+002";
    begin = myCStream.bufferPtr;
    CPrintf(&myCStream, "Double number: %@15.2f %e", 199.9, 199.9);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //characters are not affected by desired space. Default left justify for strings
    begin = myCStream.bufferPtr;
    const char* origin = "print this string?";	//18
    char c = 'y';
    string = "   print this string? y";
    CPrintf(&myCStream, "%21s %9c", origin, c);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //Padding on the left
    begin = myCStream.bufferPtr;
    string = "***print this string? y";	//18
    CPrintf(&myCStream, "%*21s %9c", origin, c);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //To impose the right justify for strings add -
    begin = myCStream.bufferPtr;
    string = "print this string?    y";	//18
    CPrintf(&myCStream, "%-21s %9c", origin, c);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //A double - impose '-' as padding
    begin = myCStream.bufferPtr;
    string = "print this string?--- y";	//18
    CPrintf(&myCStream, "%--21s %9c", origin, c);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    //Padding on the right
    begin = myCStream.bufferPtr;
    string = "%--- print this string?***     r y";	//18
    CPrintf(&myCStream, "%--- %*-21s %5s %9c", origin, "r", c);
    CPut(&myCStream, '\0');
    if (!StringTestHelper::Compare(begin, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    string = "print this string?    y";	//18
    //Test the sprintf function with size passed by argument (incluse the terminated char)
    char myBuffer[32];
    bl2_snprintf(myBuffer, 18, "%-21s %9c", origin, c);
    if (!StringTestHelper::Compare(myBuffer, (char*) "print this string")) {
        FreeAll(&myCStream);
        return False;
    }

    //Test the sprintf function without size.
    bl2_sprintf(myBuffer, "%-21s %9c", origin, c);
    if (!StringTestHelper::Compare(myBuffer, (char*) string)) {
        FreeAll(&myCStream);
        return False;
    }

    FreeAll(&myCStream);
    return True;
}

bool CStreamTest::TokenTest() {

    //TESTS ON TOKEN FUNCTIONS BETWEEN STRINGS

    //Define a string (caution: don't use const char to avoid segmentation fault for the destructiveToken function
    char input1[] = "Nome::: Giuseppe. Cognome: Ferrò:)";
    char* input = input1;
    char* begin = input;
    uint32 size = 15;
    char buffer[15];

    //This function returns the token using as terminators chars passed in the argument (the terminator is a char)
    CGetCStringToken((const char*&) input, buffer, ":.", size);

    if (!StringTestHelper::Compare((char*) buffer, (char*) "Nome")) {
        return False;
    }
    CGetCStringToken((const char*&) input, buffer, ":.", size);
    if (!StringTestHelper::Compare((char*) buffer, (char*) " Giuseppe")) {
        return False;
    }
    CGetCStringToken((const char*&) input, buffer, ":.", size);
    if (!StringTestHelper::Compare((char*) buffer, (char*) " Cognome")) {
        return False;
    }
    char saveTerminator;

    //return to the initial string and get the token with the distructive function. A '\0' char is written at the position of the terminator
    input = begin;
    char* p = CDestructiveGetCStringToken((char *&) input, ":.",
                                          &saveTerminator, ":");
    if (!StringTestHelper::Compare((char*) p, (char*) "Nome") || saveTerminator != ':') {
        return False;
    }

    //Restore the terminator char in the string
    input--;
    if (*input != 0) {
        return False;
    }
    *input = saveTerminator;
    input++;

    //Save this point of the string to test the skip
    char* newBegin = input;

    //Impose null skip. The function skips consecutive terminators automatically
    p = CDestructiveGetCStringToken((char *&) input, ":.", &saveTerminator,
                                    NULL);
    if (!StringTestHelper::Compare((char*) p, (char*) " Giuseppe") || saveTerminator != '.') {
        return False;
    }
    input--;
    *input = saveTerminator;
    input++;

    //Imposing ":" as skip char, the function skips the second ":" terminator
    input = newBegin;
    p = CDestructiveGetCStringToken((char *&) input, ":.", &saveTerminator,
                                    ":");
    if (!StringTestHelper::Compare((char*) p, (char*) " Giuseppe") || saveTerminator != '.') {
        return False;
    }

    //Imposing "." as skip char, the function doesn't skips because it finds ":" and return an empty string
    input--;
    *input = saveTerminator;
    input++;
    input = newBegin;
    p = CDestructiveGetCStringToken((char *&) input, ":.", &saveTerminator,
                                    ".");
    if (!StringTestHelper::Compare((char*) p, (char*) "")) {
        return False;
    }

    //Test if the functions terminate when the string is terminated
    for (uint32 i = 0; i < 4; i++) {
        input--;
        *input = saveTerminator;
        input++;
        p = CDestructiveGetCStringToken((char *&) newBegin, ":.",
                                        &saveTerminator, NULL);
    }
    if (!StringTestHelper::Compare((char*) p, (char*) ")")) {
        return False;
    }

    //TESTS ON TOKEN FUNCTIONS BETWEEN A CSTREAM AND A STRING
    myCStream.NewBuffer = FakeNewBuffer;
    myCStream.NewBuffer(&myCStream);
    myCStream.bufferPtr = (char*) "Nome:: Giuseppe. Cognome: Ferrò:)";

    begin = myCStream.bufferPtr;
    CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, NULL);
    if (!StringTestHelper::Compare((char*) buffer, (char*) "Nome") || saveTerminator != ':') {
        FreeAll(&myCStream);
        return False;
    }

    //Without skip chars, the function skips consecutive terminators.
    newBegin = myCStream.bufferPtr;

    CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, NULL);
    if (!StringTestHelper::Compare((char*) buffer, (char*) " Giuseppe")
            || saveTerminator != '.') {
        FreeAll(&myCStream);
        return False;
    }
    myCStream.bufferPtr = newBegin;

    //The function skips correctly the second ":"
    CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, ":");
    if (!StringTestHelper::Compare((char*) buffer, (char*) " Giuseppe")
            || saveTerminator != '.') {
        FreeAll(&myCStream);
        return False;
    }

    //The function does not skips because the terminator character is not correct
    myCStream.bufferPtr = newBegin;

    CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, ".");
    if (!StringTestHelper::Compare((char*) buffer, (char*) "") || saveTerminator != ':') {
        FreeAll(&myCStream);
        return False;
    }

    //Test with a minor maximum size passed by argument. The terminated char is calculated in the size
    myCStream.bufferPtr = newBegin;
    size = 4;
    CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, ":");
    if (!StringTestHelper::Compare((char*) buffer, (char*) " Gi") || saveTerminator != 'i') {
        FreeAll(&myCStream);
        return False;
    }
    size = 15;
    //Arrive to the next terminator
    CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, ":");

    //Test if the functions terminate when the string is terminated
    for (uint32 i = 0; i < 3; i++) {
        CGetToken(&myCStream, (char*) buffer, ".:", size, &saveTerminator, ":");
    }
    if (!StringTestHelper::Compare((char*) buffer, (char*) ")")) {
        FreeAll(&myCStream);
        return False;
    }

    //TESTS ON TOKEN FUNCTIONS BETWEEN TWO CSTREAMS

    //Create two CStreams. The first in input does not allocate memory because points to a string, the second allocates memory
    struct CStream inputCStream;
    struct CStream outputCStream;
    struct CStreamContext inputContext;
    struct CStreamContext outputContext;

    inputContext.counter = 0;
    outputContext.counter = 0;

    inputContext.buffGranularity = 0;
    outputContext.buffGranularity = 100;

    inputCStream.context = (void*) (&inputContext);
    inputCStream.bufferPtr = NULL;
    //The input CStream does not allocate memory
    inputCStream.NewBuffer = FakeNewBuffer;
    inputCStream.sizeLeft = 0;

    outputCStream.context = (void*) (&outputContext);
    outputCStream.bufferPtr = NULL;
    outputCStream.NewBuffer = CreateNewBuffer;
    outputCStream.sizeLeft = 0;

    inputCStream.NewBuffer(&inputCStream);
    outputCStream.NewBuffer(&outputCStream);

    //Create the string to tokenize
    inputCStream.bufferPtr = (char*) "Nome:: Giuseppe. Cognome:: Ferrò:)";
    begin = inputCStream.bufferPtr;

    uint32 totalSize = 0;
    char* output = outputCStream.bufferPtr;

    //Use a string as a terminator  with this function
    CGetStringToken(&inputCStream, &outputCStream, ":: ", totalSize);
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) "Nome")) {
        FreeAll(&outputCStream);
        return False;
    }

    output = outputCStream.bufferPtr;
    CGetStringToken(&inputCStream, &outputCStream, ":: ", totalSize);
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) "Giuseppe. Cognome")) {
        FreeAll(&outputCStream);
        return False;
    }

    //If the sizeLeft of inputCStream (3 because the fake newBuffer function for inputCStream increases sizeleft of buffGranularity and CStrinToken calls the newBuffer function)
    //is different than totalSize passed by argument, the function write on output sizeLeft chars.
    output = outputCStream.bufferPtr;
    inputCStream.sizeLeft = 0;
    inputContext.buffGranularity = 3;
    totalSize = 5;
    CGetStringToken(&inputCStream, &outputCStream, ":: ", totalSize);
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) "Fer")) {
        FreeAll(&outputCStream);
        return False;
    }

    //Test the Token with chars as terminators between two strings
    inputCStream.bufferPtr = begin;
    output = outputCStream.bufferPtr;
    CGetCSToken(&inputCStream, &outputCStream, ":.", &saveTerminator, ":");
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) "Nome")) {
        FreeAll(&outputCStream);
        return False;
    }

    //The function skips the second ":"
    output = outputCStream.bufferPtr;
    CGetCSToken(&inputCStream, &outputCStream, ":.", &saveTerminator, ":");
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) " Giuseppe")) {
        FreeAll(&outputCStream);
        return False;
    }
    CGetCSToken(&inputCStream, &outputCStream, ":", &saveTerminator, NULL);

    //The function does not skips because the terminator is different than the second ":"
    output = outputCStream.bufferPtr;
    CGetCSToken(&inputCStream, &outputCStream, ":.", &saveTerminator, ".");
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) "")) {
        FreeAll(&outputCStream);
        return False;
    }

    //Tests if the functions write the final of the string without terminators.
    output = outputCStream.bufferPtr;
    CGetCSToken(&inputCStream, &outputCStream, "", &saveTerminator, NULL);
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) " Ferrò:)")) {
        FreeAll(&outputCStream);
        return False;
    }

    //Test the skip function that can skip a number of terminetors. Consecutive terminetors are calculated once.
    inputCStream.bufferPtr = begin;
    CSkipTokens(&inputCStream, 3, ":.");
    output = outputCStream.bufferPtr;
    CGetCSToken(&inputCStream, &outputCStream, ")", &saveTerminator, NULL);
    CPut(&outputCStream, '\0');
    if (!StringTestHelper::Compare((char*) output, (char*) ": Ferrò:")) {
        FreeAll(&outputCStream);
        return False;
    }
    FreeAll(&outputCStream);
    return True;
}
*/
