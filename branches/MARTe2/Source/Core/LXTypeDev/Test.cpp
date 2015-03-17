#include <stdio.h>
#include "BasicTypeEnumerated.h"
#include "FormatDescriptor.h"
#include "TypeConversion.h"
#include "TypeDescriptor.h"
#include "TypedPointer.h"

int main(int argc, char **argv){
    TypedPointer tpint8 = 0xf; 
    CStream c;
    float x = 2;
    FormatDescriptor fd;
    TypePrint(c, "The value tpint8 is %d\n", (unsigned char)0x1);
    uint8 a = 1;
    uint16 b = 2;
    TypeConvert(a, x, fd);
    return 0;
}

