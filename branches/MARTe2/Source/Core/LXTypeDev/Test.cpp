#include <stdio.h>
#include "FormatDescriptor.h"
#include "FloatToStream.h"
#include "IntegerToStream.h"

/*
#include "TypeConversion.h"
#include "TypeDescriptor.h"
#include "AnyType.h"
#include "CStream.h"
#include "CharBuffer.h"
#include "Streamable.h"
//#include "StreamInterface.h"
*/

#define MAX_DIMENSION 64

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
};



/*struct TestPattern{
    // the character in the printf format
    const char *format;
    // the set of flags
    FormatDescriptor fd;
};

/// 0 terminated vector of FDLookups 
static const TestPattern testPatterns[] = {
    { "d",          FormatDescriptor(0, 0,     False, False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { "5d",	        FormatDescriptor(5, 0,     False, False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { ".2d",        FormatDescriptor(0, 2,     False, False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { "5.2d",       FormatDescriptor(5, 2,     False, False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " d", 	    FormatDescriptor(0, 0,     True,  False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " 5d",	    FormatDescriptor(5, 0,     True,  False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " .2d",       FormatDescriptor(0, 2,     True,  False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " 5.2d",      FormatDescriptor(5, 2,     True,  False, Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { "-d",         FormatDescriptor(0, 0,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { "-5d",	    FormatDescriptor(5, 0,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { "-.2d",       FormatDescriptor(0, 2,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { "-5.2d",      FormatDescriptor(5, 2,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " -d", 	    FormatDescriptor(0, 0,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " -5d",	    FormatDescriptor(5, 0,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " -.2d",      FormatDescriptor(0, 2,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " -5.2d",     FormatDescriptor(5, 2,     True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " -15.2d",    FormatDescriptor(15, 2,    True,  True,  Notation::FixedPointNotation, Notation::NormalNotation, False, False) },
    { " -15.2a",    FormatDescriptor(15, 2,    True,  True,  Notation::FixedPointNotation, Notation::HexNotation, False, False) },
    { " -15.2x",    FormatDescriptor(15, 2,    True,  True,  Notation::FixedPointNotation, Notation::HexNotation, False, False) },
    { " -15.2p",    FormatDescriptor(15, 2,    True,  True,  Notation::FixedPointNotation, Notation::HexNotation, False, False) },
    { " -15o",      FormatDescriptor(15, 0,    True,  True,  Notation::FixedPointNotation, Notation::OctalNotation, False, False) },
    { " -15o.",     FormatDescriptor(15, 0,    True,  True,  Notation::FixedPointNotation, Notation::OctalNotation, False, False) },
    { " -15.2b",    FormatDescriptor(15, 2,    True,  True,  Notation::FixedPointNotation, Notation::BitNotation, False, False) }

};

void PrintError(FormatDescriptor &fd, const FormatDescriptor expected){
    printf(" ERROR ");
    if(fd.width != expected.width){
        printf(" width:8 = %d (expected %d)", fd.width, expected.width);
    }
    if(fd.precision != expected.precision){
        printf(" precision:8 = %d (expected %d)", fd.precision, expected.precision);
    }
    if(fd.pad != expected.pad){
        printf(" pad:1 = %d (expected %d)", fd.pad, expected.pad);
    }
    if(fd.leftAlign != expected.leftAlign){
        printf(" leftAlign:1 = %d (expected %d)", fd.leftAlign, expected.leftAlign); 
    }
    if(fd.floatNotation != expected.floatNotation){
        printf(" floatNotation:2 = %d (expected %d)", fd.floatNotation, expected.floatNotation); 
    }
    if(fd.binaryNotation != expected.binaryNotation){
        printf(" binaryNotation:2 = %d (expected %d)", fd.binaryNotation, expected.binaryNotation); 
    }
    if(fd.binaryPadded != expected.binaryPadded){
        printf(" binaryPadded:1 = %d (expected %d)", fd.binaryPadded, expected.binaryPadded);
    }
    if(fd.fullNotation != expected.fullNotation){
        printf(" fullNotation:1 = %d (expected %d)", fd.fullNotation, expected.fullNotation);
    }
    if(fd.spareBits != expected.spareBits){
        printf(" spareBits:8 = %d (expected %d)", fd.spareBits, expected.spareBits);
    }
    printf("\n");
}

void TestAll(){
    int32 n = sizeof(testPatterns)/sizeof(TestPattern);
    int32 i = 0;
    for(i=0; i<n; i++){
        printf("Testing \"%s\"", testPatterns[i].format);
        FormatDescriptor fd;
        bool ok = fd.InitialiseFromString((const char *&)testPatterns[i].format);
        if(!ok){
            PrintError(fd, testPatterns[i].fd);
        }
        else if(fd != testPatterns[i].fd){
            PrintError(fd, testPatterns[i].fd);
        }
        else{
            printf(" OK!\n");
        }
    }
    
}*/

/*

bool TestMagnitude(){
	int number=12345;
	uint8 order=0;
	
	if((order=GetOrderOfMagnitude(number))!=4){
		printf("F order: %c", '0'+order);	
		return False;
	}
	
	printf("\nT order: %c\n", '0'+order);

}


bool TestToDecimalStreamPrivate(){
	myStream thisStream;
	int32 number=40;
	NToDecimalStreamPrivate(thisStream, number);
	printf("\nT buffer1: %s\n", thisStream.Buffer());
	return True;
} 
	

bool TestToDecimalStream(){
	myStream thisStream;
	int32 number=-40;
	NumberToDecimalStream(thisStream, number);
	printf("\nT buffer2: %s\n", thisStream.Buffer());
	return True;
}


bool TestToHexadecimalStream(){
	myStream thisStream;
	uint8 number=0x8f;
	NumberToHexadecimalStream(thisStream, number);
	printf("\nT buffer3: %s\n", thisStream.Buffer());
	return True;
}

bool TestToOctalStream(){
	myStream thisStream;
	uint8 number=0xfd;
	NumberToOctalStream(thisStream, number);
	printf("\nT buffer4: %s\n", thisStream.Buffer());
	return True;
}

bool TestToBinaryStream(){
	myStream thisStream;
	uint8 number=0xa1;
	uint8 size=GetOrderOfMagnitudeBin(number); 
	NumberToBinaryStream(thisStream, number);
	printf("\nT buffer5: %s\n", thisStream.Buffer());
	return True;
}


*/

class MyStream{
	
public:
	void PutC(char c){
		putchar(c);		
	}
	
}myStream;

template <typename T> void Test_FloatToStream(T number){

	FormatDescriptor	format;
	const char *pFormat;
	
	for (int i = 0;i<30;i++){
		pFormat= "- 10.7f";	
		format.InitialiseFromString(pFormat);	
		putchar('>');		
		FloatToStream(myStream,number,format);
		putchar('<');		
		
		pFormat = " 10.7e";	
		format.InitialiseFromString(pFormat);	
		putchar('>');		
		FloatToStream(myStream,number,format);
		putchar('<');		
		
		pFormat = "- 10.7E";	
		format.InitialiseFromString(pFormat);
		putchar('>');		
		FloatToStream(myStream,number,format);
		putchar('<');		

		pFormat = " 10.7g";	
		format.InitialiseFromString(pFormat);	
		putchar('>');		
		FloatToStream(myStream,number,format);
		putchar('<');		
		
		putchar('\n');	
		number = number/10;
	}

}


int main(int argc, char **argv){
	
	
	float n1 = 12345678;	
	Test_FloatToStream(n1);

	double n2 = 12345678;	
	Test_FloatToStream(n2);
	
	__float128 n3 = 12345678;
	Test_FloatToStream(n3);

/*	
    char buffer[100];
    double d = 1.2345;
    int32  n = 12345;
    uint16 size = 0;
	
	TestMagnitude();
	TestToDecimalStreamPrivate();
	TestToDecimalStream();
	TestToHexadecimalStream();
	TestToOctalStream();
	TestToBinaryStream();

    printf("%s\n", NumberToDecimal(size, buffer, sizeof(buffer), n, false));
    printf("%s\n", NumberToHexadecimal(size, buffer, sizeof(buffer), n, false));
    printf("%s\n", NumberToOctal(size, buffer, sizeof(buffer), n, false));
    printf("%s\n", NumberToBinary(size, buffer, sizeof(buffer), n, false));
   
    printf("%s\n", NumberToDecimal(size, buffer, sizeof(buffer), n, false));
    printf("%s\n", NumberToHexadecimal(size, buffer, sizeof(buffer), n, false, true));
    printf("%s\n", NumberToOctal(size, buffer, sizeof(buffer), n, false, true));
    printf("%s\n", NumberToBinary(size, buffer, sizeof(buffer), n, false, true));

    const char *retString = FloatToFixed(size, buffer, sizeof(buffer), d, 8);
    printf("->%s<-size=%d\n", retString, size);

//    TestAll();
//    TestFormatDescriptor("d");

//    FormatDescriptor expected();
//    TestFormatDescriptor("5d");
	*/
    return 0;
}

