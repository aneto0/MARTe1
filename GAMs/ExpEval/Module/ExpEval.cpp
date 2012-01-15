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

#include "CDBVirtual.h"
#include "CDBExtended.h"
#include "ExpEval.h"

/// Constructor
ExpEval::ExpEval(FString equation) {
    
//    printf("ExpEval constructor called\n"); // DEBUG

    SetEquation(equation);
    eeep.SetSymbolContainer(&eesc);
    eeep.SetDataTable(&eedt);
    /// Careful: it is done like this to avoid getting
    /// out of scope after constructor "returns"
    GCRTemplate<UnaryOperation> mon(GCFT_Create);
    masterOperationNode = mon;
    if(!(masterOperationNode.IsValid())) {
	printf("masterOperationNode is not valid!!!\n");
    }
    
    if(!TeachMaths()) {
	printf("ExpEval: TeachMaths(): failed\n");
    }
}

/// Destructor
ExpEval::~ExpEval() {
};

///
void ExpEval::SetEquation(FString eq) {
    eeep.SetString(eq);
}

///
bool ExpEval::TeachMaths() {
    /// Teach the maths symbols
    GCRTemplate< SymbolTemplate< PlusOperation > >  plusOperationSymbol(GCFT_Create);
    plusOperationSymbol->SetSymbol("+", IS_ADDITION, PRIO_ADDITION);
    if(!eeep.sc->Insert(plusOperationSymbol)) {
	printf("Test: main(): unable to insert PlusSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< TimesOperation > > timesOperationSymbol(GCFT_Create);
    timesOperationSymbol->SetSymbol("*", IS_MULTIPLICATION, PRIO_MULTIPLICATION);
    if(!eeep.sc->Insert(timesOperationSymbol)) {
	printf("Test: main(): unable to insert timesSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< DivisionOperation > > divisionOperationSymbol(GCFT_Create);
    divisionOperationSymbol->SetSymbol("/", IS_DIVISION, PRIO_DIVISION);
    if(!eeep.sc->Insert(divisionOperationSymbol)) {
	printf("Test: main(): unable to insert divisionSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< AndOperation > > andOperationSymbol(GCFT_Create);
    andOperationSymbol->SetSymbol("&", IS_AND, PRIO_AND);
    if(!eeep.sc->Insert(andOperationSymbol)) {
	printf("Test: main(): unable to insert andSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< EqualOperation > > equalOperationSymbol(GCFT_Create);
    equalOperationSymbol->SetSymbol("==", IS_EQUAL, PRIO_EQUAL);
    if(!eeep.sc->Insert(equalOperationSymbol)) {
	printf("Test: main(): unable to insert equalSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< NotEqualOperation > > notEqualOperationSymbol(GCFT_Create);
    notEqualOperationSymbol->SetSymbol("!=", IS_NOTEQUAL, PRIO_NOTEQUAL);
    if(!eeep.sc->Insert(notEqualOperationSymbol)) {
	printf("Test: main(): unable to insert notEqualSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< PowerOperation > > powerOperationSymbol(GCFT_Create);
    powerOperationSymbol->SetSymbol("^", IS_POWER, PRIO_POWER);
    if(!eeep.sc->Insert(powerOperationSymbol)) {
	printf("Test: main(): unable to insert powerSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< SqrtOperation > > sqrtOperationSymbol(GCFT_Create);
    sqrtOperationSymbol->SetSymbol("Sqrt", IS_SQRT, PRIO_FUNCTION);
    if(!eeep.sc->Insert(sqrtOperationSymbol)) {
	printf("Test: main(): unable to insert sqrtSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< AbsOperation > > absOperationSymbol(GCFT_Create);
    absOperationSymbol->SetSymbol("Abs", IS_ABS, PRIO_FUNCTION);
    if(!eeep.sc->Insert(absOperationSymbol)) {
	printf("Test: main(): unable to insert absSymbol into the SymbolContainer\n");
	return False;
    }
    GCRTemplate< SymbolTemplate< SinOperation > > sineOperationSymbol(GCFT_Create);
    sineOperationSymbol->SetSymbol("Sin", IS_SINE, PRIO_FUNCTION);
    if(!eeep.sc->Insert(sineOperationSymbol)) {
    	printf("Test: main(): unable to insert sineSymbol into the SymbolContainer\n");
    	return False;
    }

    return True;
}

/// Parse equation and build operation object's tree
void ExpEval::ParseEquation() {
    if(!masterOperationNode.IsValid()) {
	printf("ExpEval: ParseEquation(): masterOperationNode is not valid!!!\n");
	return;
    }
    eeep.Parse(masterOperationNode);
}

///
BasicTypeData &ExpEval::Execute() {
    if(!masterOperationNode.IsValid()) {
        printf("ExpEval: Execute(): masterOperationNode is not valid!!!\n");
    }
    if(result.GetDataType() == BTDTNone){
        result = masterOperationNode->Execute();
    }
    else{
        result.UpdateData(masterOperationNode->Execute());
    }

    return (result);
}

bool ExpEval::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err) {

    if(!GCNamedObject::ObjectLoadSetup(cdbData,err)) {
	CStaticAssertErrorCondition(InitialisationError, "ExpEval::ObjectLoadSetup error on GCNamedObject::ObjectLoadSetup");
	return False;
    }
    
    CDBExtended cdb(cdbData);

    FString expressionString;
    if(!cdb.ReadFString(expressionString, "Expression", "")) {
	CStaticAssertErrorCondition(InitialisationError, "ExpEval::ObjectLoadSetup(): Expression not found");
	return False;
    }
    SetEquation(expressionString);

    return True;
}

OBJECTLOADREGISTER(ExpEval,"$Id: ExpEval.cpp,v 1.3 2011/07/08 13:50:58 aneto Exp $")
