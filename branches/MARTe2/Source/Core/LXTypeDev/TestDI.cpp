#include <stdio.h>
#include "FormatDescriptor.h"
#include "FloatToStream.h"
#include "IntegerToStream.h"
#include "BitSetToInteger.h"


#define MAX_DIMENSION 64


class MyStream{
	
public:
	void PutC(char c){
		putchar(c);		
	}
	
}myStream;

void print128(DoubleInteger<uint64>n){
	FormatDescriptor	format,format2;
	const char *pFormat;
	pFormat = "#0x";	
	format.InitialiseFromString(pFormat);
	pFormat = "0x";	
	format2.InitialiseFromString(pFormat);
	
	IntegerToStream(myStream,n.upper,format);
	IntegerToStream(myStream,n.lower,format2);
	putchar('\n');		
}

void TestShiftR(DoubleInteger<uint64>n){
	for (int shiftAmount=1;shiftAmount<80;shiftAmount++){
		int shifted = 0;
		DoubleInteger<uint64>nn = n;
		printf("\nshifting step = %i\n",shiftAmount);		
		print128(nn);
		while(shifted<128){
			nn >>=  shiftAmount;
			shifted += shiftAmount; 
			print128(nn);
		}		
	}
}
void TestShiftL(DoubleInteger<uint64>n){
	for (int shiftAmount=1;shiftAmount<80;shiftAmount++){
		int shifted = 0;
		DoubleInteger<uint64>nn = n;
		printf("\nshifting step = %i\n",shiftAmount);		
		print128(nn);
		while(shifted<128){
			nn <<=  shiftAmount;
			shifted += shiftAmount; 
			print128(nn);
		}		
	}
}



int main(int argc, char **argv){
	DoubleInteger<uint64> dd;
	dd.upper=0x123456789ABCDEF0;
	dd.lower=0x0FEDCBA987654321;

	TestShiftR(dd);
	TestShiftL(dd);

	/*
	FormatDescriptor	format,format2;
	const char *pFormat;
	pFormat = "#0x";	
	format.InitialiseFromString(pFormat);
	pFormat = "0x";	
	format2.InitialiseFromString(pFormat);

	DoubleInteger<uint64> dd;
	dd=0x123456789ABCDEF0;
	dd=0;
	IntegerToStream(myStream,dd.upper,format);
	IntegerToStream(myStream,dd.lower,format2);
	putchar('\n');		
	dd=~dd;
	IntegerToStream(myStream,dd.upper,format);
	IntegerToStream(myStream,dd.lower,format2);
	putchar('\n');		
	dd>>=33;
	IntegerToStream(myStream,dd.upper,format);
	IntegerToStream(myStream,dd.lower,format2);
	putchar('\n');		
	dd>>=33;
	IntegerToStream(myStream,dd.upper,format);
	IntegerToStream(myStream,dd.lower,format2);
	putchar('\n');		
	dd<<=33;
	IntegerToStream(myStream,dd.upper,format);
	IntegerToStream(myStream,dd.lower,format2);
	putchar('\n');		
	dd<<=33;
	IntegerToStream(myStream,dd.upper,format);
	IntegerToStream(myStream,dd.lower,format2);
	putchar('\n');		

	uint64 upper = 0x123456789ABCDEF0;
	uint64 lower = 0x0FEDCBA987654321;
	IntegerToStream(myStream,upper,format);
	IntegerToStream(myStream,lower,format2);
	putchar('\n');		
	
	uint8 shift = 1;
	uint8 bitSize = 64;
	lower = lower >> shift;
	uint64 tmp = upper << (bitSize-shift);
	IntegerToStream(myStream,tmp,format);
	putchar('\n');		
	
	lower |= tmp;
	upper = upper >> shift;

	IntegerToStream(myStream,upper,format);
	IntegerToStream(myStream,lower,format2);
	putchar('\n');		
*/	
	return 0;
}

