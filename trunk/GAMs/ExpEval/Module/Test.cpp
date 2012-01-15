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
#include "Symbol.h"
#include "SymbolTemplate.h"
#include "GCRTemplate.h"
#include "ExpressionParser.h"
#include "PlusOperation.h"
#include "TimesOperation.h"
#include "DivisionOperation.h"
#include "PowerOperation.h"
#include "AndOperation.h"
#include "EqualOperation.h"
#include "NotEqualOperation.h"
#include "PowerOperation.h"
#include "SqrtOperation.h"
#include "AbsOperation.h"

#include "ExpEval.h"

int main(int argc, char *argv[]) {
#if 0
    /// Initialise parser object
    // ExpressionParser  pp("2-1");
    // ExpressionParser  pp("-2*3-1");
    // ExpressionParser  pp("1-3*3+1");
    // ExpressionParser  pp("1+2*3-4/4.0+1");
    // ExpressionParser  pp("Abs(1+2*3-4/4.0-7.1)*2");
    ExpressionParser  pp("Abs(1+2*3-4/4.0-7.1)*2 * (1 != 0)");
    // ExpressionParser pp("2^2+3-4/2");
    // ExpressionParser pp("Sqrt(4)-6/2.0");
    // ExpressionParser pp("Sqrt(-2)");
    // ExpressionParser pp("Abs(-4)");
    // ExpressionParser  pp("(1+2*3-4/4.0-7.1)*2");
    // ExpressionParser  pp("1+2*3-4/4.0-7.1");
    // ExpressionParser  pp("1+2");
    // ExpressionParser  pp("Abs(1-3)");
    // ExpressionParser  pp("-1");
    // ExpressionParser  pp("((((2*3)/2)))");
    // ExpressionParser pp("1 & 0");
    SymbolContainer symbolContainer;
    ExpressionParser::sc = &symbolContainer;
    
    /// Teach the maths symbols
    GCRTemplate< SymbolTemplate< PlusOperation > >  plusOperationSymbol(GCFT_Create);
    plusOperationSymbol->SetSymbol("+", IS_ADDITION, PRIO_ADDITION);
    if(!symbolContainer.Insert(plusOperationSymbol)) {
    	printf("Test: main(): unable to insert PlusSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< TimesOperation > > timesOperationSymbol(GCFT_Create);
    timesOperationSymbol->SetSymbol("*", IS_MULTIPLICATION, PRIO_MULTIPLICATION);
    if(!symbolContainer.Insert(timesOperationSymbol)) {
    	printf("Test: main(): unable to insert timesSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< DivisionOperation > > divisionOperationSymbol(GCFT_Create);
    divisionOperationSymbol->SetSymbol("/", IS_DIVISION, PRIO_DIVISION);
    if(!symbolContainer.Insert(divisionOperationSymbol)) {
    	printf("Test: main(): unable to insert divisionSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< AndOperation > > andOperationSymbol(GCFT_Create);
    andOperationSymbol->SetSymbol("&", IS_AND, PRIO_AND);
    if(!symbolContainer.Insert(andOperationSymbol)) {
    	printf("Test: main(): unable to insert andSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< EqualOperation > > equalOperationSymbol(GCFT_Create);
    equalOperationSymbol->SetSymbol("==", IS_EQUAL, PRIO_EQUAL);
    if(!symbolContainer.Insert(equalOperationSymbol)) {
    	printf("Test: main(): unable to insert equalSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< NotEqualOperation > > notEqualOperationSymbol(GCFT_Create);
    notEqualOperationSymbol->SetSymbol("!=", IS_NOTEQUAL, PRIO_NOTEQUAL);
    if(!symbolContainer.Insert(notEqualOperationSymbol)) {
    	printf("Test: main(): unable to insert notEqualSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< PowerOperation > > powerOperationSymbol(GCFT_Create);
    powerOperationSymbol->SetSymbol("^", IS_POWER, PRIO_POWER);
    if(!symbolContainer.Insert(powerOperationSymbol)) {
    	printf("Test: main(): unable to insert powerSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< SqrtOperation > > sqrtOperationSymbol(GCFT_Create);
    sqrtOperationSymbol->SetSymbol("Sqrt", IS_SQRT, PRIO_FUNCTION);
    if(!symbolContainer.Insert(sqrtOperationSymbol)) {
    	printf("Test: main(): unable to insert sqrtSymbol into the SymbolContainer\n");
    	return -1;
    }
    GCRTemplate< SymbolTemplate< AbsOperation > > absOperationSymbol(GCFT_Create);
    absOperationSymbol->SetSymbol("Abs", IS_ABS, PRIO_FUNCTION);
    if(!symbolContainer.Insert(absOperationSymbol)) {
    	printf("Test: main(): unable to insert absSymbol into the SymbolContainer\n");
    	return -1;
    }
    
    /// Build operations tree
    GCRTemplate<UnaryOperation> masterOperationNode(GCFT_Create);
    pp.Parse(masterOperationNode);

    /// Evaluate expression
    masterOperationNode->Execute();

    /// Show result
    float bubu;
    masterOperationNode->result.GetData(BTDFloat, &bubu);
    printf("result = %f\n", bubu);
#endif

#if 1

    // ExpEval expEval("1+2");
    // ExpEval expEval("Abs(1+2*3-4/4.0-7.1)*2 * (1 != 0)");
    // ExpEval expEval("1+4+2");
    ExpEval expEval("1+bubu");
    float val = 0;
    BasicTypeData value(BTDFloat, 1);
    value.UpdateData(&val);
    int32 idx;
    FString str = "bubu";
    idx = expEval.eedt.CreateAndAddDataTableEntry(str, value);
    printf("idx = %d\n", idx);

    expEval.ParseEquation();

    float out;
    for(int i = 1 ; i < 3 ; i++) {
    	val = i;
    	value.UpdateData(&val);
    	expEval.eedt.UpdateDataTableEntryValue(idx, value);
    	expEval.Execute();
    	expEval.result.GetData(BTDFloat, &out);
    	printf("out = %f\n", out);
    }

    // outcome = expEval.Execute();

    // outcome.GetData(BTDFloat, &out);
    // printf("out = %f\n", out);

    // float out1;
    // expEval.masterOperationNode->result.GetData(BTDFloat, &out1);
    // printf("out1 = %f\n", out1);

    // float out2;
    // expEval.result.GetData(BTDFloat, &out2);
    // printf("out2 = %f\n", out2);

#endif

#if 0
    uint32 uint32Bubu;
    int32  int32Bubu = 2;
    float  floatBubu;

    BTConvert(1, BTDFloat, &floatBubu, BTDInt32, &int32Bubu);           // Works!!!
    printf("int32Bubu = %d -> floatBubu = %f\n", int32Bubu, floatBubu);

    // BTConvert(1, BTDFloat, &floatBubu, BTDUint32, &uint32Bubu);         // Works!!!
    // printf("uint32Bubu = %d -> floatBubu = %f\n", uint32Bubu, floatBubu);

    // BTConvert(1, BTDInt32, &int32Bubu, BTDFloat, &floatBubu);           // Works!!!
    // printf("floatBubu = %f -> int32Bubu = %d\n", floatBubu, int32Bubu);

    // BTConvert(1, BTDUint32, &uint32Bubu, BTDFloat, &floatBubu);         // Works!!!
    // printf("floatBubu = %f -> uint32Bubu = %d\n", floatBubu, uint32Bubu);
#endif
    return 0;
}
