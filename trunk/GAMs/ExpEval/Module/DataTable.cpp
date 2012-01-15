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

#include "DataTable.h"

int32 DataTable::AddDataTableEntry(StructDataTable *dt) {

    StructDataTable **oldPtrs2DataEntries = ptrs2DataEntries;
    
    if((ptrs2DataEntries = (StructDataTable **)malloc((tableSize+1)*sizeof(StructDataTable *))) == NULL) {
        printf("DataTable: AddDataTableEntry(): unable to allocate memory\n");
        return 0;
    }
    
    if(oldPtrs2DataEntries) {
        memcpy(ptrs2DataEntries, oldPtrs2DataEntries, tableSize*sizeof(StructDataTable *));
        free((void*&)oldPtrs2DataEntries);
    }
    
    ptrs2DataEntries[tableSize] = dt;

    return (tableSize++);
}
    
///
StructDataTable *DataTable::CreateDataTableEntry(FString varName, BasicTypeData &varValue, bool varLocked) {

    StructDataTable *data    = new StructDataTable();
	
    data->key                = varName;
    data->value              = varValue;
    data->isLockedForWriting = varLocked;
	
    return data;
}
    
///
int32 DataTable::CreateAndAddDataTableEntry(FString varName, BasicTypeData varValue, bool varLocked) {

    StructDataTable *dataPtr;
    dataPtr = CreateDataTableEntry(varName, varValue, varLocked);
	
    return AddDataTableEntry(dataPtr);
}

///
int32 DataTable::GetIndex(FString varName) {
    for(int i = 0 ; i < tableSize ; i++) {
        if(ptrs2DataEntries[i]->key == varName) {
            return i;
        }
    }

    return -1;
}
        
///
bool DataTable::UpdateDataTableEntryValue(uint32 index, BasicTypeData &val) {
    if(index >= tableSize) { 
        printf("DataTable: UpdateDataTableEntryValue(): index beyond table size\n");
        return False;
    } else {
        if(ptrs2DataEntries[index]->isLockedForWriting) {
            printf("DataTable: UpdateDataTableEntryValue(): error, attempt to update locked value\n");
            return False;
        } else {
            ptrs2DataEntries[index]->value.UpdateData(val);
        }
    }

    return True;
}

///
bool DataTable::UpdateDataTableEntryValue(FString varName, BasicTypeData &val) {

    int32 index;
    
    if((index = GetIndex(varName)) == -1) {
        printf("Error\n");
        return False;
    }
    
    return UpdateDataTableEntryValue(index, val);
}

///
bool DataTable::GetDataTableEntryValue(uint32 index, BasicTypeData &val) {
    if(index >= tableSize) { 
        printf("DataTable: GetDataTableEntryValue(): index beyond table size\n");
        return False;
    } else {
        if(val.GetDataType() == BTDTNone){
            val = ptrs2DataEntries[index]->value;
        }
        else{
            val.UpdateData(ptrs2DataEntries[index]->value);
        }
    }

    return True;
}

///
bool DataTable::GetDataTableEntryValue(FString varName, BasicTypeData &val) {

    int32 index;
    
    if((index = GetIndex(varName)) == -1) {
        printf("Error\n");
        return False;
    }
    
    return GetDataTableEntryValue(index, val);
}
