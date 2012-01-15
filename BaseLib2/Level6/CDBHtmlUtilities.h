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
 * @brief Utility implementations to provide the interface between a CDB and HTML
 */
#if !defined (CDB_HTML_UTILITIES)
#define CDB_HTML_UTILITIES

#include "ConfigurationDataBase.h"
#include "HttpStream.h"


/** to specify what part of the webpage to produce */
typedef  int CDBHUMode;

/** all the header */
const CDBHUMode  CDBHUHOSUV_Header = 0x1;
/** just the script part of the header */
const CDBHUMode  CDBHUHOSUV_Script = 0x2;
/** the body part without BODY statement */
const CDBHUMode  CDBHUHOSUV_Body   = 0x4;
/** the body part with BODY statement */
const CDBHUMode  CDBHUHOSUV_FullBody = 0x8;
/** the body part has no Back statement */
const CDBHUMode  CDBHUHOSUV_NoBack = 0x10;


extern "C" {
/** allows implementing a browsing functionality for
CDBVirtual derived databases */
bool CDBHUHtmlObjectSubView(ConfigurationDataBase &cdb_,const char *title_,HttpStream &hStream, CDBHUMode mode);

}

#endif
