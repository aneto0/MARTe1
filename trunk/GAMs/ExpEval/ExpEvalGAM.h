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
#ifndef __EXP_EVAL_GAM__
#define __EXP_EVAL_GAM__

#include "ExpEval.h"
#include "GAM.h"
#include "Symbol.h"
#include "SymbolTemplate.h"
#include "Node.h"

OBJECT_DLL(ExpEvalGAM)
class ExpEvalGAM : public GAM {
OBJECT_DLL_STUFF(ExpEvalGAM)

private:

    DDBInputInterface    **input;

    DDBOutputInterface   **output;

    GCRTemplate<ExpEval>   gcr;

    uint32                *numberOfInputSignals;

    int32                **tableIndexes;

    /**
     * Array of ExpEvals to execute in real-time
     */
    GCRTemplate<ExpEval> *executableList;

    /**
     * Local data to store the execution results
     */
    BasicTypeData data;
    BasicTypeData outBtd;

public:

    /// Constructor
    ExpEvalGAM();

    /// Destructor
    ~ExpEvalGAM();

    ///
    bool  Initialise(ConfigurationDataBase &cdb);

    ///
    bool  Execute(GAM_FunctionNumbers execFlag);

};

#endif

