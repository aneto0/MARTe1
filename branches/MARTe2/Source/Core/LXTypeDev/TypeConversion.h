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
 * $Id: $
 *
 **/
/**
 * @file
 */
#ifndef TYPED_CONVERSION_H
#define TYPED_CONVERSION_H

#include "AnyType.h"
#include "FormatDescriptor.h"
#include "CStream.h"

/** 
*/
bool TypeConvert(const AnyType &destination, const AnyType& source, const FormatDescriptor &conversionFormat ){
    return False;
}


/** 
*/
bool TypePrint(CStream &output, const char *format, const AnyType& par1){
    return False;
}

/** 
*/
bool TypePrint(CStream &output, const char *format, const AnyType& par1, const AnyType& par2 ){
    return False;
}


#endif
