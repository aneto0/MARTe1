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
 */
#if !defined (OBJECT_REGISTRY_TOOLS_H)
#define OBJECT_REGISTRY_TOOLS_H

#include "StreamInterface.h"

extern "C" {
    /**
        Evaluates (*((type *)address)).expression into the address of the last element and its type
        address is modified to the address of the specified item and dataType points to the type information
        of the specified item. expression can only contain '.' separated elements
    */
    bool ObjectRegistryEvaluateAddress(const char *type,char *&address,const char *expression, ClassStructureEntry &dataType,StreamInterface *error);
}

#endif

