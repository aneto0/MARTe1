#include <stdio.h>
#include "BasicType.h"
#include "FormatDescriptor.h"
#include "TypeConversion.h"
#include "TypeDescriptor.h"
#include "AnyType.h"
#include "CStream.h"

int main(int argc, char **argv){
    AnyType tpint8 = 0xf; 
    float x = 2;
    FormatDescriptor fd;
    CStream cs;
    TypePrint(cs, "The value tpint8 is %d\n", (unsigned char)0x1);
    return 0;
}

