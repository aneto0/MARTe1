#if !defined (TYPED_CONVERSION)
#define TYPED_CONVERSION

#include "AnyType.h"
#include "FormatDescriptor.h"
#include "CStream.h"

/** 
*/
bool TypeConvert(const AnyType &destination, const AnyType& source, const FormatDescriptor &conversionFormat ){

    return false;
}


/** 
*/
bool TypePrint(CStream &output, const char *format, const AnyType& par1){
    return false;
}

/** 
*/
bool TypePrint(CStream &output, const char *format, const AnyType& par1, const AnyType& par2 ){

    return false;
}


#endif
