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

#include "CDBDataTypes.h"

static const CDBTYPE CDBTypes[12] = {
    CDBTYPE_float,
    CDBTYPE_double,
    CDBTYPE_int32,
    CDBTYPE_uint32,
    CDBTYPE_char,
    CDBTYPE_String,
    CDBTYPE_BString,
    CDBTYPE_FString,
    CDBTYPE_CDBStyle,
    CDBTYPE_CDBEval,
    CDBTYPE_Interpret,
    CDBTYPE_Content
};

/** finds a suitable CDBTYPE for a given CDBDataType */
const CDBTYPE *FindCDBTYPEFromCDBDataType(CDBDataType cdt){
    int nOfTypes = sizeof(CDBTypes)/sizeof(CDBTYPE);

    for (int i =0;i<nOfTypes;i++){
        if (CDBTypes[i].dataType == cdt){
            return CDBTypes + i;
        }
    }
    return &CDBTYPE_NULL;
}



