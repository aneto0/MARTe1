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

#ifndef __UNARY_OPERATION__
#define __UNARY_OPERATION__

#include "Node.h"
#include "GCRTemplate.h"
#include "DataTable.h"

class UnaryOperation : public Node {

public:

    BasicTypeData        data;

    BasicTypeData        result;

    GCRTemplate<Node>    sink;

    DataTable           *dataTable;

    int32                dataTableIndex;

    UnaryOperation() {
        dataTable      = NULL;
        dataTableIndex = -1;
    }

    virtual BasicTypeData &Execute() {
        if(!isLeaf) {
            if(data.GetDataType() == BTDTNone){
                data = sink->Execute();
            }
            else{
                data.UpdateData(sink->Execute());
            }
        }
        if(result.GetDataType() == BTDTNone){
            result = ExecuteLocal();
        }
        else{
            result.UpdateData(ExecuteLocal());
        }

        return result;
    }

    virtual BasicTypeData &ExecuteLocal() {
        if((dataTable != NULL) && (dataTableIndex != -1)) {
            if(!dataTable->GetDataTableEntryValue(dataTableIndex, data)) {
                printf("Error: unable to read data\n");
            }
        }
	
        return data;
    }
};
#endif
