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
#include "DDBInterfaceDescriptor.h"
#include "CDBExtended.h"


DDBInterfaceDescriptor::DDBInterfaceDescriptor(const DDBInterfaceDescriptor& descriptor){
        ownerName     = descriptor.OwnerName();
        interfaceName = descriptor.InterfaceName();
        accessMode    = descriptor.AccessMode();
}


void DDBInterfaceDescriptor::Print(StreamInterface& s) const{
    s.Printf("Name: %15s, Owner: %15s, ",interfaceName.Buffer(),ownerName.Buffer());
    accessMode.Print(s);
}

bool DDBIDObjectLoadSetup(DDBInterfaceDescriptor&     ddbid,
                          ConfigurationDataBase&      info,
                          StreamInterface *err){

    CDBExtended cdb(info);
    cdb->AddChildAndMove(ddbid.ownerName.Buffer());
    ddbid.accessMode.ObjectSaveSetup(cdb,err);
    cdb.WriteString(ddbid.interfaceName.Buffer(),"InterfaceName");
    cdb->MoveToFather();
    return True;
}




OBJECTREGISTER(DDBInterfaceDescriptor,"$Id$")

