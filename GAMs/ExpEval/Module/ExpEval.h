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

#ifndef __EXPRESSION_EVALUATOR_OBJECT__
#define __EXPRESSION_EVALUATOR_OBJECT__

#include "FString.h"

#include "ExpressionParser.h"
#include "SymbolContainer.h"
#include "SymbolTemplate.h"

#include "BinaryOperation.h"
#include "UnaryOperation.h"

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
#include "SinOperation.h"

OBJECT_DLL(ExpEval)
class ExpEval : public GCNamedObject {
OBJECT_DLL_STUFF(ExpEval)

public:

    ///
    SymbolContainer              eesc;

    ///
    ExpressionParser             eeep;

    ///
    DataTable                    eedt;
    
    ///
    BasicTypeData                result;

    ///
    GCRTemplate<UnaryOperation>  masterOperationNode;

    /// Constructor
    ExpEval(FString equation = "");

    /// Destructor
    ~ExpEval();

    /// Set the equation string in the expression parser object
    void SetEquation(FString eq);

    /// Load symbol definitions into the symbol container
    bool TeachMaths();

    /// Parse equation and build operation object's tree
    void ParseEquation();

    /// Execute the operations to evaluate the expression
    BasicTypeData &Execute();

    /// 
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);
    
};
#endif
