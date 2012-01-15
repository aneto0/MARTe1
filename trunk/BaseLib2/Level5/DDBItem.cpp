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

#include "DDBItem.h"
#include "CDBExtended.h"

bool DDBItem::ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err){
    CDBExtended cdb(info);

    cdb.WriteString(signalName.Buffer(),"SignalName");

    FString typeName;
    typeName = "Unknown";

    if      (typeCode==BTDInt32)  {typeName="int32"   ;}
    else if (typeCode==BTDInt64)  {typeName="int64"   ;}
    else if (typeCode==BTDUint32) {typeName="uint32"  ;}
    else if (typeCode==BTDUint64) {typeName="uint64"  ;}
    else if (typeCode==BTDFloat)  {typeName="float"   ;}
    else if (typeCode==BTDDouble) {typeName="double"  ;}

    cdb.WriteString(typeName.Buffer(),"Type");
    cdb.WriteInt32(size,"SignalSize");

    InterfaceDescriptorShowIterator interfaceDescriptorShowIterator(cdb);
    listOfInterfaceDescriptors.ListIterate(&interfaceDescriptorShowIterator);

    return True;
}

bool DDBItem::ObjectLoadSetup(ConfigurationDataBase &cdb,StreamInterface *err){
        if(err != NULL) err->Printf("DDBItem::ObjectLoadSetup: Not Implemented.");
        AssertErrorCondition(FatalError,"DDBItem::ObjectLoadSetup: Not Implemented.");
        return False;
}

void DDBItem::UpdateStatus(const DDBInterface& ddbInterface){

    // Check if the Item is already being written by another interface
    if (accessMode.CheckMask(DDB_WriteMode) && ddbInterface.GetInterfaceDescriptor().AccessMode().CheckMask(DDB_WriteMode)){
        status |= DDB_WriteConflictError;
    }

    // Update access mode
    accessMode |= ddbInterface.GetInterfaceDescriptor().AccessMode();

    // Check if Missing Source
    if ((accessMode.CheckMask(DDB_ReadMode)) && (!accessMode.CheckMask(DDB_WriteMode))){
        status |= DDB_MissingSourceError;
    }else{
        status&=~DDB_MissingSourceError;
    }

    // Check if Patching Exsclusive signal
    if ((accessMode.CheckMask(DDB_ExclusiveWriteBit)) && (accessMode.CheckMask(DDB_PatchMode))){
        status|=DDB_WriteExclusiveViolation;
    }else{
        status&=~DDB_WriteExclusiveViolation;
    }

}

bool DDBItem::AddInterfaceDescriptor(const DDBInterface& ddbInterface){

    DDBInterfaceDescriptor* interfaceDescriptor = Find(ddbInterface);
    if (interfaceDescriptor == NULL){
        // Creates new descriptor and add it to the listOFInterfaceDescriptors.
        DDBInterfaceDescriptor* newInterfaceDescriptor = new DDBInterfaceDescriptor(ddbInterface.GetInterfaceDescriptor());
        listOfInterfaceDescriptors.ListAdd(newInterfaceDescriptor);
        UpdateStatus(ddbInterface);
        return True;

    }
    return True;
}

void DDBItem::Print(StreamInterface& s, bool showUsers){
    ConfigurationDataBase cdb;
    ObjectSaveSetup(cdb,NULL);
    cdb->WriteToStream(s);
}


bool DDBItemBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData){
    DDBItem *item = (DDBItem *)userData;
    if(item == NULL) return False;
    in.Printf("\n");
    item->Print(in, False);
    return True;
}


OBJECTREGISTER(DDBItem,"$Id: DDBItem.cpp,v 1.8 2008/06/18 16:33:31 fpiccolo Exp $")

