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
#include "IntegerToStreamTest.h"
#include "StringTestHelper.h"
#include "StreamTestHelper.h"
#include "StringPortable.h"
#include "stdio.h"
bool IntegerToStreamTest::TestDecimalMagnitude() {
    uint8 bit8 = 255;

    if (GetOrderOfMagnitude(bit8) != 2) {
        return False;
    }

    uint16 bit16 = 12345;

    if (GetOrderOfMagnitude(bit16) != 4) {
        return False;
    }

    uint32 bit32 = 123456789;
    if (GetOrderOfMagnitude(bit32) != 8) {
        return False;
    }

    uint64 bit64 = 12345678912345678;
    if (GetOrderOfMagnitude(bit64) != 16) {
        return False;
    }

    return True;
}

bool IntegerToStreamTest::TestHexadecimalMagnitude() {
    uint8 bit8 = 0x81;

    if (GetNumberOfDigitsHexNotation(bit8) != 2) {
        return False;
    }

    uint16 bit16 = 0xf123;

    if (GetNumberOfDigitsHexNotation(bit16) != 4) {
        return False;
    }

    uint32 bit32 = 0xaf010a31;
    if (GetNumberOfDigitsHexNotation(bit32) != 8) {
        return False;
    }

    uint64 bit64 = 0xedab12348976aef2;
    if (GetNumberOfDigitsHexNotation(bit64) != 16) {
        return False;
    }

    return True;
}

bool IntegerToStreamTest::TestOctalMagnitude() {
    uint8 bit8 = 0x81;

    if (GetNumberOfDigitsOctalNotation(bit8) != 3) {
        return False;
    }

    uint16 bit16 = 0xf123;

    if (GetNumberOfDigitsOctalNotation(bit16) != 6) {
        return False;
    }

    uint32 bit32 = 0xff010a31;
    if (GetNumberOfDigitsOctalNotation(bit32) != 11) {
        return False;
    }

    uint64 bit64 = 0xadab12348976aef2;
    if (GetNumberOfDigitsOctalNotation(bit64) != 22) {
        return False;
    }

    bit64 = 0x7dab12348976aef2;

    if (GetNumberOfDigitsOctalNotation(bit64) != 21) {
        return False;
    }
    return True;
}

bool IntegerToStreamTest::TestBinaryMagnitude() {
    uint8 bit8 = 0x81;

    if (GetNumberOfDigitsBinaryNotation(bit8) != 8) {
        return False;
    }

    bit8 = 0x71;

    if (GetNumberOfDigitsBinaryNotation(bit8) != 7) {
        return False;
    }

    uint16 bit16 = 0xf123;

    if (GetNumberOfDigitsBinaryNotation(bit16) != 16) {
        return False;
    }

    uint32 bit32 = 0xff010a31;
    if (GetNumberOfDigitsBinaryNotation(bit32) != 32) {
        return False;
    }

    uint64 bit64 = 0xedab12348976aef2;
    if (GetNumberOfDigitsBinaryNotation(bit64) != 64) {
        return False;
    }

    return True;
}

bool IntegerToStreamTest::TestDecimalStream() {

    MyStream thisStream;
    uint8 ubit8 = 255;

    IntegerToStreamDecimalNotation(thisStream, ubit8);

    if (!StringTestHelper::Compare("255", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //LeftAligned and not padded (nothing happen)
    IntegerToStreamDecimalNotation(thisStream, ubit8, 5, false, true);

    if (!StringTestHelper::Compare("255", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();
    int8 sbit8 = -127;
    IntegerToStreamDecimalNotation(thisStream, sbit8);

    if (!StringTestHelper::Compare("-127", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //LeftAligned and padded "...  "
    IntegerToStreamDecimalNotation(thisStream, sbit8, 6, true, true);
    if (!StringTestHelper::Compare("-127  ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //Max int8
    sbit8 = 0x80;
    IntegerToStreamDecimalNotation(thisStream, sbit8);

    if (!StringTestHelper::Compare("-128", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    uint16 ubit16 = 12345;
    IntegerToStreamDecimalNotation(thisStream, ubit16);

    if (!StringTestHelper::Compare("12345", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //LeftAligned and padded with positive sign "+...  "
    IntegerToStreamDecimalNotation(thisStream, ubit16, 8, true, true, true);

    if (!StringTestHelper::Compare("+12345  ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    int16 sbit16 = -12345;
    IntegerToStreamDecimalNotation(thisStream, sbit16);

    if (!StringTestHelper::Compare("-12345", thisStream.Buffer())) {

        return False;
    }
    thisStream.Clear();

    //Max int16
    sbit16 = 0x8000;
    IntegerToStreamDecimalNotation(thisStream, sbit16);

    if (!StringTestHelper::Compare("-32768", thisStream.Buffer())) {

        return False;
    }

    thisStream.Clear();

    uint32 ubit32 = 123456789;
    IntegerToStreamDecimalNotation(thisStream, ubit32);

    if (!StringTestHelper::Compare("123456789", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //RightAligned and padded with sign " +..."
    IntegerToStreamDecimalNotation(thisStream, ubit32, 11, true, false, true);
    if (!StringTestHelper::Compare(" +123456789", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    int32 sbit32 = -123456789;

    IntegerToStreamDecimalNotation(thisStream, sbit32);

    if (!StringTestHelper::Compare("-123456789", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //Max int32
    sbit32 = 0x80000000;

    IntegerToStreamDecimalNotation(thisStream, sbit32);

    if (!StringTestHelper::Compare("-2147483648", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();


    uint64 ubit64 = 12345678912345678;
    IntegerToStreamDecimalNotation(thisStream, ubit64);
    if (!StringTestHelper::Compare("12345678912345678", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    int64 sbit64 = -12345678912345678;
    IntegerToStreamDecimalNotation(thisStream, sbit64);

    if (!StringTestHelper::Compare("-12345678912345678", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Max int64
    sbit64 = 0x8000000000000000;
    IntegerToStreamDecimalNotation(thisStream, sbit64);

    if (!StringTestHelper::Compare("-9223372036854775808", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //? if the maxSize is incorrect	
    IntegerToStreamDecimalNotation(thisStream, sbit64, 11, true, false, true);

    if (!StringTestHelper::Compare("          ?", thisStream.Buffer())) {
        return False;
    }

    return True;

}

bool IntegerToStreamTest::TestHexadecimalStream() {

    MyStream thisStream;
    uint8 ubit8 = 0xea;

    IntegerToStreamExadecimalNotation(thisStream, ubit8);
    if (!StringTestHelper::Compare("EA", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //Check if works also for negative signed numbers.
    int8 sbit8 = 0xea;

    IntegerToStreamExadecimalNotation(thisStream, sbit8);
    if (!StringTestHelper::Compare("EA", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();


    //LeftAligned and not padded (nothing happen)
    IntegerToStreamExadecimalNotation(thisStream, ubit8, 5, false, true);
    if (!StringTestHelper::Compare("EA", thisStream.Buffer())) {
        return False;
    }

    //Add Header and trailing zeros
    thisStream.Clear();
    IntegerToStreamExadecimalNotation(thisStream, ubit8, 0, false, false, true,
                                      true);

    if (!StringTestHelper::Compare("0xEA", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit8 = 0xf;
    //Add only trailing zeros
    thisStream.Clear();
    IntegerToStreamExadecimalNotation(thisStream, ubit8, 0, false, false, sizeof(ubit8)*8,
                                      false);
    if (!StringTestHelper::Compare("0F", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //All true with a space more... it must print only a space after.
    uint16 ubit16 = 0xabcd;
    IntegerToStreamExadecimalNotation(thisStream, ubit16, 7, true, true, sizeof(ubit16)*8,
                                      true);

    if (!StringTestHelper::Compare("0xABCD ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    ubit16 = 0xcd;
    //With zeros and 3 as number of digits (without header).
    IntegerToStreamExadecimalNotation(thisStream, ubit16, 5, true, true, sizeof(ubit16)*8,
                                      true);
    if (!StringTestHelper::Compare("0x0CD", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //Only right aligned with header
    uint32 ubit32 = 0xabcdef78;
    IntegerToStreamExadecimalNotation(thisStream, ubit32, 12, true, false,
                                      0, true);
    if (!StringTestHelper::Compare("  0xABCDEF78", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    ubit32 = 0x00abcd0f;
    //Right align with zero and header
    IntegerToStreamExadecimalNotation(thisStream, ubit32, 11, true, false, sizeof(ubit32)*8,
                                      true);
    if (!StringTestHelper::Compare(" 0x00ABCD0F", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //Right align without zeros and header
    IntegerToStreamExadecimalNotation(thisStream, ubit32, 11, true, false,
                                      0, false);
    if (!StringTestHelper::Compare("     ABCD0F", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //padded=false
    uint64 ubit64 = 0x89abcdef0123fff0;
    IntegerToStreamExadecimalNotation(thisStream, ubit64, 120, false);
    if (!StringTestHelper::Compare("89ABCDEF0123FFF0", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();
    //?
    ubit64 = 0x123fff0;
    IntegerToStreamExadecimalNotation(thisStream, ubit64, 5, true);
    if (!StringTestHelper::Compare("    ?", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    ubit64 = 0xfff0;
    IntegerToStreamExadecimalNotation(thisStream, ubit64, 7, true, true, sizeof(ubit64)*8);
    if (!StringTestHelper::Compare("000FFF0", thisStream.Buffer())) {
        return False;
    }

    return True;

}

bool IntegerToStreamTest::TestOctalStream() {

    MyStream thisStream;
    uint8 ubit8 = 0xea;

    IntegerToStreamOctalNotation(thisStream, ubit8);
    if (!StringTestHelper::Compare("352", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Test if works also for negative signed numbers.
    int8 sbit8 = -22;

    IntegerToStreamOctalNotation(thisStream, sbit8);
    if (!StringTestHelper::Compare("352", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();
   
    //LeftAligned and not padded (nothing happen)
    IntegerToStreamOctalNotation(thisStream, ubit8, 5, false, true);
    if (!StringTestHelper::Compare("352", thisStream.Buffer())) {
        return False;
    }

    //Add Header and trailing zeros
    thisStream.Clear();
    IntegerToStreamOctalNotation(thisStream, ubit8, 0, false, false, sizeof(ubit8)*8,
                                 true);
    if (!StringTestHelper::Compare("0o352", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit8 = 0xf;
    //Add only trailing zeros
    thisStream.Clear();
    IntegerToStreamOctalNotation(thisStream, ubit8, 0, false, false, sizeof(ubit8)*8,
                                 false);
    if (!StringTestHelper::Compare("017", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //All true with a space more... it must print a space after.
    uint16 ubit16 = 0x7bcd; //6 is the maximum now are 5
    IntegerToStreamOctalNotation(thisStream, ubit16, 9, true, true, sizeof(ubit16)*8, true);
    if (!StringTestHelper::Compare("0o075715 ", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit16 = 0xcd;
    //With zeros and 5 as number of digits (without header).
    IntegerToStreamOctalNotation(thisStream, ubit16, 7, true, true, sizeof(ubit16)*8, true);
if (!StringTestHelper::Compare("0o00315", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Only right aligned with header
    uint32 ubit32 = 0xabcdef78;
    IntegerToStreamOctalNotation(thisStream, ubit32, 15, true, false, 0,
                                 true);
    if (!StringTestHelper::Compare("  0o25363367570", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit32 = 0xcd0f;
    //Right align with zero and header
    IntegerToStreamOctalNotation(thisStream, ubit32, 15, true, false, sizeof(ubit32)*8,
                                 true);
    if (!StringTestHelper::Compare("  0o00000146417", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Right align without zeros and header
    IntegerToStreamOctalNotation(thisStream, ubit32, 15, true, false, 0,
                                 false);
    if (!StringTestHelper::Compare("         146417", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //padded=false
    uint64 ubit64 = 0x89abcdef01240000;
    IntegerToStreamOctalNotation(thisStream, ubit64, 120, false);
    if (!StringTestHelper::Compare("1046536336740111000000",
                                   thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();
    //?
    ubit64 = 0x123fff0;
    IntegerToStreamOctalNotation(thisStream, ubit64, 5, true);
    if (!StringTestHelper::Compare("    ?", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit64 = 0xfff0;
    IntegerToStreamOctalNotation(thisStream, ubit64, 7, true, true, sizeof(ubit64)*8);
    if (!StringTestHelper::Compare("0177760", thisStream.Buffer())) {
        return False;
    }

    return True;

}

bool IntegerToStreamTest::TestBinaryStream() {

    MyStream thisStream;
    uint8 ubit8 = 0xea;

    IntegerToStreamBinaryNotation(thisStream, ubit8);
    if (!StringTestHelper::Compare("11101010", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Test if works also for negative signed numbers
    int8 sbit8 = 0xea;

    IntegerToStreamBinaryNotation(thisStream, sbit8);
    if (!StringTestHelper::Compare("11101010", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();
    //LeftAligned and not padded (nothing happen)
    IntegerToStreamBinaryNotation(thisStream, ubit8, 50, false, true);
    if (!StringTestHelper::Compare("11101010", thisStream.Buffer())) {
        return False;
    }

    //Add trailing zeros
    thisStream.Clear();
    IntegerToStreamBinaryNotation(thisStream, ubit8, 0, false, false, sizeof(ubit8)*8,
                                  true);
    if (!StringTestHelper::Compare("0b11101010", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit8 = 0xf;
    //Add only trailing zeros
    thisStream.Clear();
    IntegerToStreamBinaryNotation(thisStream, ubit8, 0, false, false, sizeof(ubit8)*8,
                                  false);
    if (!StringTestHelper::Compare("00001111", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //All true with a space more... it must print a space after.
    uint16 ubit16 = 0x7bcd; //
    IntegerToStreamBinaryNotation(thisStream, ubit16, 19, true, true, sizeof(ubit16)*8,
                                  true);
    if (!StringTestHelper::Compare("0b0111101111001101 ",
                                   thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit16 = 0xcd;
    //With zeros and 10 as number of digits (without header).
    IntegerToStreamBinaryNotation(thisStream, ubit16, 12, true, true, sizeof(ubit16)*8,
                                  true);
    if (!StringTestHelper::Compare("0b0011001101", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Only right aligned with header
    uint32 ubit32 = 0xabcdef78;
    IntegerToStreamBinaryNotation(thisStream, ubit32, 34, true);
    if (!StringTestHelper::Compare("  10101011110011011110111101111000",
                                   thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit32 = 0xcd0f;
    //Right align with zero and header
    IntegerToStreamBinaryNotation(thisStream, ubit32, 34, true, false, sizeof(ubit32)*8);
    if (!StringTestHelper::Compare("  00000000000000001100110100001111",
                                   thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Right align without zeros and header
    IntegerToStreamBinaryNotation(thisStream, ubit32, 34, true, false, 0);
    if (!StringTestHelper::Compare("                  1100110100001111",
                                   thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    //padded=false
    uint64 ubit64 = 0x8000000000000001;
    IntegerToStreamBinaryNotation(thisStream, ubit64, 120, false);
    if (!StringTestHelper::Compare(
            "1000000000000000000000000000000000000000000000000000000000000001",
            thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();
    //?
    ubit64 = 0x123fff0;
    IntegerToStreamBinaryNotation(thisStream, ubit64, 5, true);
    if (!StringTestHelper::Compare("    ?", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    ubit64 = 0xff0;
    IntegerToStreamBinaryNotation(thisStream, ubit64, 14, true, true, sizeof(ubit64)*8);
    if (!StringTestHelper::Compare("00111111110000", thisStream.Buffer())) {
        return False;
    }
    return True;

}

bool IntegerToStreamTest::TestIntegerToStream() {
    int8 sbit8 = 0xea;
    FormatDescriptor format;
    const char *pFormat;
    MyStream myStream;

    pFormat = "- i";
    format.InitialiseFromString(pFormat);
    IntegerToStream(myStream, sbit8, format);
    if (!StringTestHelper::Compare("-106", myStream.Buffer())) {
        return False;
    }

    int16 sbit64 = 0xf0;
    pFormat = " #0x";
    format.InitialiseFromString(pFormat);
    IntegerToStream(myStream, sbit64, format);
    if (!StringTestHelper::Compare("0x00000000000000f0", myStream.Buffer())) {
        return False;
    }

    int32 sbit32 = 0x18;

    pFormat = "- #0o";
    format.InitialiseFromString(pFormat);
    IntegerToStream(myStream, sbit32, format);
    if (!StringTestHelper::Compare("0o00000011000", myStream.Buffer())) {
        return False;
    }

    int16 sbit16 = 0x71;
    pFormat = " #b";
    format.InitialiseFromString(pFormat);
    IntegerToStream(myStream, sbit16, format);
    if (!StringTestHelper::Compare("0b1110001", myStream.Buffer())) {
        return False;
    }
    return True;
}





bool IntegerToStreamTest::TestBitSetToStream() {
	uint64 data[5] =   {0x13579BDF02468ACE,0x13579BDF02468ACE,0x123456789ABCDEF0,0xDEADBABEBAB00111};
	const char streamString[] = "DEADBABEBAB00111123456789ABCDEF013579BDF02468ACE13579BDF02468ACE";
	int32 sizeStr=63;   
	int32 dataBitSize=256;
	MyStream myStream;

	FormatDescriptor	format;
	const char *pFormat;
	pFormat = "0x";	
	format.InitialiseFromString(pFormat);
	uint32*	source = (uint32*) data;

	//from size =1 to size = 64
	for(int size=4; size<64; size+=4){
		int32 beg=0;
		int32 end=0;
		
		for(int myShift=0; myShift<dataBitSize; myShift+=size){
			//source and shift are automatically modified by the function.
			BitSetToStream(myStream, source , myShift, size, false, format);
			char buffer[128];
			
			end=sizeStr-myShift/4;
			beg=end-size/4 + 1;
			StringPortable::Substr(beg, end, streamString, buffer);
//			printf("\n%s",buffer);

			if(!StringTestHelper::Compare(buffer, myStream.Buffer())){
				return False;
			}
			myStream.Clear();

			//Avoids to print shit. (it remains less than size)
			if((dataBitSize-myShift)<(2*size))
				break;			
		}
	}

        return True;
}
