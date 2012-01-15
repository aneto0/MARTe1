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
#include "GCReferenceContainer.h"
#include "OperationInterface.h"
#include "ConditionInterface.h"
#include "CDBExtended.h"

#if !defined(_OPERATION_CONDITION_)
#define _OPERATION_CONDITION_

OBJECT_DLL(OperationCondition)
class OperationCondition : public ConditionInterface, public GCReferenceContainer {
OBJECT_DLL_STUFF(OperationCondition)

private:

    bool                              conditionResult;

    GCRTemplate<OperationInterface>   operation;

    float                            *excludingLowerThresh;

    float                            *includingLowerThresh;

    float                            *excludingUpperThresh;

    float                            *includingUpperThresh;

    float                            *equalTo;

public:

    OperationCondition() {
        conditionResult = False;

        excludingLowerThresh = NULL;
        includingLowerThresh = NULL;
        excludingUpperThresh = NULL;
        includingUpperThresh = NULL;
        equalTo              = NULL;
    };
    
    ~OperationCondition() {
        if(excludingLowerThresh != NULL) {
            free((void*&)excludingLowerThresh);
            excludingLowerThresh = NULL;
        }
        if(includingLowerThresh != NULL) {
            free((void*&)includingLowerThresh);
            includingLowerThresh = NULL;
        }
        if(excludingUpperThresh != NULL) {
            free((void*&)excludingUpperThresh);
            excludingUpperThresh = NULL;
        }
        if(includingUpperThresh != NULL) {
            free((void*&)includingUpperThresh);
            includingUpperThresh = NULL;
        }
        if(equalTo != NULL) {
            free((void*&)equalTo);
            equalTo = NULL;
        }
    };

    bool ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err);

    void Reset();
    
    bool IsTrue();
    
};

#endif
