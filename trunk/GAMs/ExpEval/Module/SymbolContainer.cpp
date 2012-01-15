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

#include "SymbolContainer.h"

uint32 SymbolContainer::IdentifyChar(char symChar) {
    if(isdigit(symChar) || (symChar == '-'))
	return IS_DIGIT;
    if(isalpha(symChar))
	return IS_ALPHA;
    if(symChar == '.')
	return IS_DOT;
	
    return IS_UNKNOWN;
}
    
uint32 SymbolContainer::IdentifySymbol(FString symStr) {
    if(!(symStr == "-")) {
	for(int i = 0 ; i < Size() ; i++) {
	    GCRTemplate< Symbol > sym;
	    sym = Find(i);
	    if(sym.IsValid()) {
		if(sym->IsSymbol(symStr)) {
		    return (sym->GetFlag());
		}
	    } else {
		printf("SymbolContainer: IdentifySymbol(): object not valid\n");
	    }
	}
    }
	
    if(symStr.Size() == 1) {
	if(isdigit(*(symStr.Buffer())) || (strcmp(symStr.Buffer(), "-") == 0))
	    return IS_DIGIT;
	if(isalpha(*(symStr.Buffer())))
	    return IS_ALPHA;
	if((*(symStr.Buffer())) == '.')
	    return IS_DOT;
    } else {
	return IS_UNKNOWN;
    }
}
    
uint32 SymbolContainer::IdentifySymbol(const char *symStr) {
    if(strcmp(symStr, "-") != 0) {
	for(int i = 0 ; i < Size() ; i++) {
	    GCRTemplate< Symbol > sym;
	    sym = Find(i);
	    if(sym.IsValid()) {
		if(sym->IsSymbol(symStr)) {
		    return (sym->GetFlag());
		}
	    } else {
		printf("SymbolContainer: IdentifySymbol(): object not valid\n");
	    }
	}
    }

    if(strlen(symStr) == 1) {
	if(isdigit(*symStr) || (strcmp(symStr, "-") == 0))
	    return IS_DIGIT;
	if(isalpha(*symStr))
	    return IS_ALPHA;
	if((*symStr) == '.')
	    return IS_DOT;
    } else {
	return IS_UNKNOWN;
    }
}
    
uint32 SymbolContainer::GetSymbolPriority(FString symStr) {
    if(!(symStr == "-")) {
	for(int i = 0 ; i < Size() ; i++) {
	    GCRTemplate< Symbol > sym;
	    sym = Find(i);
	    if(sym.IsValid()) {
		if(sym->IsSymbol(symStr)) {
		    return (sym->GetPriority());
		}
	    } else {
		printf("SymbolContainer: IdentifySymbol(): object not valid\n");
	    }
	}
    }
	
    if(symStr.Size() == 1) {
	if(isdigit(*(symStr.Buffer())) || (strcmp(symStr.Buffer(), "-") == 0))
	    return PRIO_DIGIT;
	if(isalpha(*(symStr.Buffer())))
	    return PRIO_ALPHA;
	if((*(symStr.Buffer())) == '.')
	    return PRIO_DOT;
    } else {
	return PRIO_UNKNOWN;
    }
}
    
uint32 SymbolContainer::GetSymbolPriority(const char *symStr) {
    if(strcmp(symStr, "-") != 0) {
	for(int i = 0 ; i < Size() ; i++) {
	    GCRTemplate< Symbol > sym;
	    sym = Find(i);
	    if(sym.IsValid()) {
		if(sym->IsSymbol(symStr)) {
		    return (sym->GetPriority());
		}
	    } else {
		printf("SymbolContainer: IdentifySymbol(): object not valid\n");
	    }
	}
    }

    if(strlen(symStr) == 1) {
	if(isdigit(*symStr) || (strcmp(symStr, "-") == 0))
	    return PRIO_DIGIT;
	if(isalpha(*symStr))
	    return PRIO_ALPHA;
	if((*symStr) == '.')
	    return PRIO_DOT;
    } else {
	return PRIO_UNKNOWN;
    }
}

GCRTemplate<Node> SymbolContainer::BuildOperation(FString opStr, GCRTemplate<Node> parentNode) {
    return (BuildOperation(opStr.Buffer(), parentNode));
}

GCRTemplate<Node> SymbolContainer::BuildOperation(const char *opStr, GCRTemplate<Node> parentNode) {

    if(strcmp(opStr, "-") == 0) {
	printf("SymbolContainer: BuildOperation(): '-' operation not allowed\n");
    }
	
    for(int i = 0 ; i < Size() ; i++) {
	GCRTemplate< Symbol > sym;
	sym = Find(i);
	if(sym.IsValid()) {
	    if(sym->IsSymbol(opStr)) {
		return (sym->BuildOperationObject(parentNode));
	    }
	} else {
	    printf("SymbolContainer: BuildOperation(): object not valid\n");
	}
    }
	
}

OBJECTLOADREGISTER(SymbolContainer,"$Id: SymbolContainer.cpp,v 1.1 2010/08/20 06:33:05 dalves Exp $")
