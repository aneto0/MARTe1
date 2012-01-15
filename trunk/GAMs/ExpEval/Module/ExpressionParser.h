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

#ifndef __EXPRESSION_PARSER__
#define __EXPRESSION_PARSER__

#include "FString.h"
#include "SymbolContainer.h"
#include "DataTable.h"
#include "BinaryOperation.h"
#include "UnaryOperation.h"

OBJECT_DLL(ExpressionParser)
class ExpressionParser : public GCNamedObject {
OBJECT_DLL_STUFF(ExpressionParser)

private:

    /// Equation string to parse
    FString equation;
    
    /// Equation length
    uint32  length;
    
    /// Position in equation
    uint32  position;

    /// Flag to check whether to remove '-' signs or not
    bool    firstParse;

public:

    /// Container of maths' symbols
    SymbolContainer *sc;

    /// Container of variable/parameter values
    DataTable       *dataTable;

    /// Constructor
    ExpressionParser(FString eq = "", SymbolContainer *symCont = NULL, DataTable *datTab = NULL, bool firstParseOfOriginalEquation = True) {
 	SetString(eq);
	SetSymbolContainer(symCont);
	SetDataTable(datTab);
 	firstParse = firstParseOfOriginalEquation;
// 	if(firstParse)
// 	    ShowEquation();
    }

    /// Destructor
    ~ExpressionParser() {
    }

    ///
    void SetSymbolContainer(SymbolContainer *symCont);

    ///
    void SetDataTable(DataTable *datTab);

    /// 
    void SetString(FString eqStr);

    /// Print equation string
    void ShowEquation();

    /// Return the length of the equation string
    uint32 GetEquationLength();

    /// Get char in equation string
    char GetChar();

    ///
    bool MoveParserPositionByOffset(int32 offset);

    /// Peek char at position "offset" from current parser position
    char PeekChar(int32 offset);
    
    /// Get parser's current char position
    uint32 GetParserPosition();
    
    /// Remove spaces and update equation string
    void RemoveSpaces();
    
    ///
    void ResetPosition();

    ///
    bool RemovedOutsideBrackets();

    ///
    uint32 GetLowestPriorityPosition(FString &op, int32 &prio);
    
    /// Parse equation
    void Parse(GCRTemplate<UnaryOperation> previousUnaryNode);
    
};
#endif
