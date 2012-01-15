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

#ifndef __SYMBOL_CONTAINER__
#define __SYMBOL_CONTAINER__

#include "FString.h"
#include "GCReferenceContainer.h"
#include "GCRTemplate.h"
#include "Symbol.h"

OBJECT_DLL(SymbolContainer)
class SymbolContainer : public GCReferenceContainer {
OBJECT_DLL_STUFF(SymbolContainer)

public:

    uint32 IdentifyChar(char symChar);
    
    uint32 IdentifySymbol(FString symStr);
    
    uint32 IdentifySymbol(const char *symStr);
    
    uint32 GetSymbolPriority(FString symStr);
    
    uint32 GetSymbolPriority(const char *symStr);


    GCRTemplate<Node> BuildOperation(FString opStr, GCRTemplate<Node> parentNode);

    GCRTemplate<Node> BuildOperation(const char *opStr, GCRTemplate<Node> parentNode);
};

#endif
