/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
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
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

/** 
 * @file
 * Base64 encoding/decoding of strings
 */
#if !defined (BASE64_CODEC)
#define BASE64_CODEC

#include "FString.h"
#include "System.h"


extern "C" {

    /**
     * Base64 encoding of a string
     * @param input the string to encode
     * @param output the encoded string
     * @return True if the encoding was successful
     */
    bool B64Encode(FString &input, FString &output);

    /** */
    bool B64Decode(FString &input, FString &output);

}



#endif

