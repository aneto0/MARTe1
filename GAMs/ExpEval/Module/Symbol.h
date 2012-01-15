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

#ifndef __SYMBOL__
#define __SYMBOL__

#include "FString.h"
#include "GCNamedObject.h"
#include "GCRTemplate.h"
#include "Node.h"

/// Identifier's table
#define IS_UNKNOWN               0x0

#define IS_DIGIT                 0x1
#define IS_ALPHA                 0x2
#define IS_DOT                   0x4

#define IS_AND                   0x8
#define IS_OR                    0x10
#define IS_XOR                   0x20
#define IS_LTE                   0x40
#define IS_GTE                   0x80
#define IS_LT                    0x100
#define IS_GT                    0x200
#define IS_EQUAL                 0x400
#define IS_NOTEQUAL              0x800

#define IS_ADDITION              0x1000
#define IS_MULTIPLICATION        0x2000
#define IS_DIVISION              0x4000
#define IS_POWER                 0x8000

#define IS_NEGATION              0x10000

#define IS_OPEN_PARENTHESIS      0x20000
#define IS_CLOSE_PARENTHESIS     0x40000

#define IS_SQRT                  0x80000

#define IS_SINE                  0x100000

#define IS_ABS                   0x200000

/// Priority table
#define PRIO_UNKNOWN            -1

#define PRIO_DIGIT               0
#define PRIO_ALPHA               0
#define PRIO_DOT                 0

#define PRIO_AND                 1
#define PRIO_OR                  1
#define PRIO_XOR                 1
#define PRIO_LTE                 2
#define PRIO_GTE                 2
#define PRIO_LT                  2
#define PRIO_GT                  2
#define PRIO_EQUAL               2
#define PRIO_NOTEQUAL            2

#define PRIO_ADDITION            3
#define PRIO_MULTIPLICATION      4
#define PRIO_DIVISION            4

#define PRIO_POWER               5

#define PRIO_NEGATION            6

#define PRIO_OPEN_PARENTHESIS    7
#define PRIO_CLOSE_PARENTHESIS   7
#define PRIO_FUNCTION            7

class Symbol : public GCNamedObject {

private:

    FString     string;
    uint32      flag;
    int32       priority;
    
public:

    void       *argsPtr;

    /// Constructor
    Symbol(const char *symStr, uint32 symFlag, int32 symPriority, void *opArgs = NULL) {
	SetSymbol(symStr, symFlag, symPriority, opArgs);
    }
    
    Symbol() {
	SetSymbol("", IS_UNKNOWN, -1);
    }
    
    /// Destructor
    ~Symbol() {
    }
    
    /// Configure symbol
    void SetSymbol(const char *symStr, uint32 symFlag, int32 symPriority, void *opArgs = NULL) {
	string   = symStr;
	flag     = symFlag;
	priority = symPriority;
	argsPtr  = opArgs;
    }
    
    /// Return True if is symbol and False otherwise
    bool IsSymbol(uint32 symFlag) {
	return (symFlag == flag);
    }
    
    /// Return True if is symbol and False otherwise
    bool IsSymbol(const char *symStr) {
	if(string == symStr) {
	    return True;
	} else {
	    return False;
	}
    }

    /// Return True if is symbol and False otherwise
    bool IsSymbol(FString symStr) {
	return IsSymbol(symStr.Buffer());
    }
    
    /// Return symbol string
    const char *GetString() {
	return string.Buffer();
    }

    /// Return symbol flag
    uint32 GetFlag() {
	return flag;
    }

    /// Return symbol priority
    int32 GetPriority() {
	return priority;
    }

    /// Build the object that performs the actual operation
    virtual GCRTemplate<Node> BuildOperationObject(GCRTemplate<Node> parentNode) = 0;
};
#endif
