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

#ifndef __PLUS_OPERATION__
#define __PLUS_OPERATION__

#include "BinaryOperation.h"

class PlusOperation : public BinaryOperation {

public:

    PlusOperation() {
        defaultType = BTDFloat;
        result.SetType(defaultType);
    }
    
    virtual BasicTypeData &ExecuteLocal() {

        float         lv, rv, v;

        if(!leftData.GetData(defaultType, &lv)) {
            printf("Plus: ExecuteLocal(): unable to get left value\n");
        }
        if(!rightData.GetData(defaultType, &rv)) {
            printf("Plus: ExecuteLocal(): unable to get right value\n");
        }
        
        v = lv + rv;

        if(!result.UpdateData(&v)) {
            printf("Plus: ExecuteLocal(): unable to update BasicTypeData value\n");
        }
        
        return (result);
    }
    
};

#endif
