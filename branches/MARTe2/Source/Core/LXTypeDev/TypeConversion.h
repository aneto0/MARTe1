#if !defined (TYPED_CONVERSION)
#define TYPED_CONVERSION

#include "TypedPointer.h"
#include "FormatDescriptor.h"

class CStream{
};
/** 
*/
bool TypeConvert(const TypedPointer &destination, const TypedPointer& source, const FormatDescriptor &conversionFormat ){

    return false;
}


/** 
*/
bool TypePrint(CStream &output, const char *format, const TypedPointer& par1){
    printf(format, *((char *)par1.dataPointer));
    return false;
}

/** 
*/
bool TypePrint(CStream &output, const char *format, const TypedPointer& par1, const TypedPointer& par2 ){

    return false;
}


#endif
