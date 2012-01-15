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

#ifndef __DATA_TABLE__
#define __DATA_TABLE__

#include "FString.h"
#include "BasicTypeData.h"
#include "GCNamedObject.h"

class StructDataTable {
public:
    FString           key;
    BasicTypeData     value;
    bool              isLockedForWriting;
};

class DataTable : public GCNamedObject {

private:

    /// Array containing pointers to StructDataTable entries
    StructDataTable **ptrs2DataEntries;

    /// Number of entries in the data table
    uint32 tableSize;

private:

    ///
    int32 AddDataTableEntry(StructDataTable *dt);
    
    ///
    StructDataTable *CreateDataTableEntry(FString varName, BasicTypeData &varValue, bool varLocked = False);
    
public:
    
    /// Constructor
    DataTable() {
	ptrs2DataEntries  = NULL;
	tableSize         = 0;
    }
    
    /// Destructor
    ~DataTable() {
	if(ptrs2DataEntries != NULL) {
	    for(int i = 0 ; i < tableSize ; i++) {
		if(ptrs2DataEntries[i] != NULL) {
		    delete ptrs2DataEntries[i];
		}
	    }
	    free((void*&)ptrs2DataEntries);
	}
    }
    
    ///
    int32 CreateAndAddDataTableEntry(FString varName, BasicTypeData varValue, bool varLocked = False);

    int32 GetIndex(FString varName);

    ///
    bool UpdateDataTableEntryValue(uint32 index, BasicTypeData &val);

    ///
    bool UpdateDataTableEntryValue(FString varName, BasicTypeData &val);
    
    ///
    bool GetDataTableEntryValue(uint32 index, BasicTypeData &val);

    ///
    bool GetDataTableEntryValue(FString varName, BasicTypeData &val);

    ///
    uint32 GetDataTableSize() {
	return tableSize;
    }
    
};
#endif
