#include <stdio.h>
#include "FormatDescriptor.h"
#include "TypeConversion.h"
#include "TypeDescriptor.h"
#include "AnyType.h"
#include "CStream.h"
#include "CharBuffer.h"
#include "Streamable.h"
/*#include "StreamInterface.h"*/
#include "NumberToString.h"

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


int main(int argc, char **argv){
    char buffer[100];
    double d = 1.2345;
    int32  n = 12345;
    uint16 size = 0;
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
/*    TestFormatDescriptor("d");

    FormatDescriptor expected();
    TestFormatDescriptor("5d");*/
    return 0;
}

