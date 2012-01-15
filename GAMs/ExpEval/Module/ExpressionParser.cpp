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

#include "ExpressionParser.h"

///
void ExpressionParser::SetSymbolContainer(SymbolContainer *symCont) {
    sc = symCont;
}

///
void ExpressionParser::SetDataTable(DataTable *datTab) {
    dataTable = datTab;
}

/// 
void ExpressionParser::SetString(FString eqStr) {
    equation = eqStr;
    length   = equation.Size();
    position = 0;
}

/// Print equation string
void ExpressionParser::ShowEquation() {
    printf("Equation -> |%s|\n", equation.Buffer());
}

/// Return the length of the equation string
uint32 ExpressionParser::GetEquationLength() {
    return equation.Size();
}

/// Get char in equation string
char ExpressionParser::GetChar() {
    char c = *(equation.Buffer()+position);

    return (c);
}

///
bool ExpressionParser::MoveParserPositionByOffset(int32 offset) {
    uint32 posAux = position + offset;
    if((posAux >= 0) && (posAux < length)) {
	position = posAux;
	return True;
    } else {
	return False;
    }
}

/// Peek char at position "offset" from current parser position
char ExpressionParser::PeekChar(int32 offset) {
    if(position+offset < length) {
	return (*(equation.Buffer()+position+offset));
    } else {
	return '\0';
    }
}
    
/// Get parser's current char position
uint32 ExpressionParser::GetParserPosition() {
    return position;
}
    
///
void ExpressionParser::ResetPosition() {
    position = 0;
}

/// Remove spaces and update equation string
void ExpressionParser::RemoveSpaces() {
    FString output;
    while(equation.GetToken(output, " "));
    output.Seek(0);
	
    equation = output;
    position = 0;
    length   = equation.Size();
}
    
bool ExpressionParser::RemovedOutsideBrackets() {
    char  br;
    int32 brC = 0;
    while(br = GetChar()) {
	// If the equation starts and ends with the same 
	// matching parenthesis then remove the parenthesis
	if((GetParserPosition() == 0) && (br != '(')) {
	    break;
	} else if(br == '(') {
	    brC++;
	} else if(br == ')') {
	    brC--;
	} else {
	    ;
	}
	if((brC == 0) && (position == length-1)) {
	    FString eq;
	    eq = equation.Buffer()+1;
	    eq.SetSize(length-2);
	    SetString(eq);
	    return False;
	    break;
	} else if(brC == 0) {
	    break;
	} else {
	    ;
	}
	    
	// Move parser postion one char forward
	if(!MoveParserPositionByOffset(1))
	    break;
    }

    return True;
}
    
/// 
uint32 ExpressionParser::GetLowestPriorityPosition(FString &op, int32 &prio) {
    char   c3[3] = {'\0', '\0', '\0'};
    char   c2[2] = {'\0', '\0'};
    int32  pr    = PRIO_CLOSE_PARENTHESIS + 1;
    int32  pr2;
    int32  pr3;
    uint32 pos   = 0;
    int32  bracketCounter = 0;

    ResetPosition();
    while(!RemovedOutsideBrackets());
    ResetPosition();

    /// Remove '-' operations/signs
    if(firstParse) {
	FString noMinusEquation;
	char mC;
	while(mC = GetChar()) {
	    if(((mC == '-') && (position == 0)) || ((mC == '-') && (PeekChar(-1) == '('))) {
		noMinusEquation += "(-1)*";
	    } else if(mC == '-') {
		noMinusEquation += "+(-1)*";
	    } else {
		noMinusEquation.PutC(mC);
	    }
	    // Move parser postion one char forward	    
	    if(!MoveParserPositionByOffset(1))
		break;
	}
	SetString(noMinusEquation);
	firstParse = False;
    }

    ResetPosition();
    // Get lowest priority char or string (2 chars max)
    while(c2[0] = GetChar()) {
	if(c2[0] == '(') {
	    bracketCounter++;
	} else if(c2[0] == ')') {
	    bracketCounter--;
	} else if(bracketCounter == 0) {
	    c3[0] = c3[1];
	    c3[1] = c2[0];
		
	    pr2 = sc->GetSymbolPriority((const char *)(&c2));
	    pr3 = sc->GetSymbolPriority((const char *)(&c3));
		
	    if((pr2 < pr) && (pr2 > PRIO_DIGIT)) {
		pr  = pr2;
		pos = position;
		op  = c2;
	    }
	    if((pr3 < pr) && (pr3 > PRIO_DIGIT)) {
		pr  = pr3;
		pos = position-1;
		op  = c3;
	    }
	} else {
	    ;
	}

	// Move parser postion one char forward	    
	if(!MoveParserPositionByOffset(1))
	    break;
    }

    // Check if all parenthesis match by the end of the equation string
    if(bracketCounter != 0) {
	printf("ExpressionParser: GetLowestPriorityPosition(): parenthesis mismatch\n");
	prio = -1;
	op   = "";
	pos  = 0;
    }

    // Check if no operation was found
    if((prio = pr) == (PRIO_CLOSE_PARENTHESIS + 1)) {
	ResetPosition();
	equation.GetToken(op, "(");
	if(op == equation) { // parameter/signal found
	    //printf("ExpressionParser: GetLowestPriorityPosition(): no operation found\n");
	    prio = -1;
	    op   = "";
	    pos  = 0;
	} else { // function found
	    prio = sc->GetSymbolPriority(op);
	    pos  = 0;
	}
    }
	
    return pos;
}
    
/// Parse equation
void ExpressionParser::Parse(GCRTemplate<UnaryOperation> previousUnaryNode) {
	
    FString operation;
    int32   prio;
    uint32  opPos;

    if(!previousUnaryNode.IsValid()) {
	printf("ExpressionParser: Parse(): previousUnaryNode not valid!!!\n");
	return;
    }

    ResetPosition();
    RemoveSpaces();

//    ShowEquation(); //DEBUG

    // Get the details for the lowest priority operation in the equation
    opPos = GetLowestPriorityPosition(operation, prio);

//    printf("operation = %s\tprio = %d\n", operation.Buffer(), prio); //DEBUG

    FString newEquation;
    ExpressionParser next("", sc, dataTable, False);
    if(prio >= PRIO_OPEN_PARENTHESIS) { // Only one side ... unary operation
	newEquation = equation.Buffer() + opPos + operation.Size();
	next.SetString((const char *)(newEquation.Buffer()));
	GCRTemplate<UnaryOperation> unaryLocal = sc->BuildOperation(operation.Buffer(), previousUnaryNode);
	next.Parse(unaryLocal);
    } else if((prio != -1) && (prio != 0)) { // Binary operation

	GCRTemplate<BinaryOperation> binLocal = sc->BuildOperation(operation.Buffer(), previousUnaryNode);
	    
	// Left side
	GCRTemplate<UnaryOperation> ulo(GCFT_Create);
	binLocal->leftSink = ulo;
	newEquation = equation;
	newEquation.SetSize(opPos);
	next.SetString((const char *)(newEquation.Buffer()));
	next.Parse(ulo);
	    
	// Rigth side
	GCRTemplate<UnaryOperation> uro(GCFT_Create);
	binLocal->rightSink = uro;
	newEquation = equation.Buffer() + opPos + operation.Size();
	next.SetString((const char *)(newEquation.Buffer()));
	next.Parse(uro);

    } else if(prio == -1) {
	if((sc->IdentifyChar(*equation.Buffer()) == IS_DIGIT)) {
	    float value = atof(equation.Buffer());
	    BasicTypeData data(BTDFloat, 1);
	    data.UpdateData(&value);
	    previousUnaryNode->data   = data;
	    previousUnaryNode->isLeaf = True;
	} else if((sc->IdentifyChar(*equation.Buffer()) == IS_ALPHA)) {
	    previousUnaryNode->isLeaf    = True;
	    previousUnaryNode->dataTable = dataTable;
	    previousUnaryNode->dataTableIndex = dataTable->GetIndex(equation);
	} else {
	    printf("Error\n");
	}
    }
}

OBJECTLOADREGISTER(ExpressionParser, "$Id: ExpressionParser.cpp,v 1.1 2010/08/20 06:32:54 dalves Exp $")
