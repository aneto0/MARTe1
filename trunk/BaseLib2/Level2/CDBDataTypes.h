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
 * Additional data type definitions for CDB
 */
#if !defined(CONFIGURATION_DATABASE_DATATYPES)
#define CONFIGURATION_DATABASE_DATATYPES

#include "CDBTypes.h"
#include "Streamable.h"
#include "FString.h"

/** constant to request the transformation to and from a FString
    data is the address of a vector of FStrings */
static const CDBTYPE CDBTYPE_FString     (CDB_FString  ,sizeof(FString)         ,NULL);

/** constant to request the formatted data.
    data is the address of a single FString
    vararg parameters: uint32 indentChars,uint32 maxElements,CDBWriteMode writeMode */
static const CDBTYPE CDBTYPE_CDBStyle    (CDB_CDBStyle ,sizeof(Streamable *)    ,NULL);

/** constant to request the formatted data after being evaluated.
    data is the address of a single FString
    vararg parameters: uint32 indentChars,uint32 maxElements,CDBWriteMode writeMode */
static const CDBTYPE CDBTYPE_CDBEval     (CDB_CDBEval  ,sizeof(Streamable *)    ,NULL);
/** constant to get the raw data interpreted by the node  */
static const CDBTYPE CDBTYPE_Interpret   (CDB_Interpret,sizeof(char *)          ,NULL);
/** constant to get the raw data stored in the node  */
static const CDBTYPE CDBTYPE_Content     (CDB_Content  ,sizeof(FString *)       ,NULL);


extern "C" {

    /** finds a suitable CDBTYPE for a given CDBDataType */
    const CDBTYPE *FindCDBTYPEFromCDBDataType(CDBDataType cdt);
}

#endif

