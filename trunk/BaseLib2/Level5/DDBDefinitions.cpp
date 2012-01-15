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

#include "DDBDefinitions.h"
#include "FString.h"
#include "CDBExtended.h"

/** Bitwise-or operator. */
DDBStoringProperties operator|(const DDBStoringProperties& left,
                                         const DDBStoringProperties& right){
    DDBStoringProperties result(left);
    return result|=right;
}

/** Bitwise-and operator. */
DDBStoringProperties operator&(const DDBStoringProperties& left,
                                         const DDBStoringProperties& right){
    DDBStoringProperties result(left);
    return result&=right;
}

void DDBStoringProperties::Print(StreamInterface& s) const{
    uint32 size=2;
    s.Write("| ",size);

    if (CheckMask(DDB_FlatNamed)) s.Write("S ",size);
    else s.Write("  ",size);

    if (CheckMask(DDB_Consecutive)) s.Write("C ",size);
    else s.Write("  ",size);

    size=3;
    if (CheckMask(DDB_Unsized)) s.Write("U |",size);
    else s.Write("  |",size);
}

bool DDBIAMObjectSaveSetup(const DDBInterfaceAccessMode& ddbiam, ConfigurationDataBase &info, StreamInterface* s){
    uint32 size=2;
    FString temp;
    temp.Write("| ",size);

    if ((ddbiam.mode&0x1)!=0) temp.Write("R ",size);
    else temp.Write("  ",size);

    if ((ddbiam.mode&0x2)!=0) temp.Write("W ",size);
    else temp.Write("  ",size);

    if ((ddbiam.mode&0x4)!=0) temp.Write("P ",size);
    else temp.Write("  ",size);

    if ((ddbiam.mode&0x8)!=0) temp.Write("X ",size);
    else temp.Write("  ",size);

    size=1;
    temp.Write("|",size);

    CDBExtended cdb(info);
    cdb.WriteString(temp.Buffer(),"AccessMode");
    return True;
}

void DDBItemStatus::Print(StreamInterface& s) const{
    uint32 size=2;
    s.Write("| ",size);

    size=3;
    if ((status&0x01)!=0) s.Write("WC ",size);
    else s.Write("   ",size);

    if ((status&0x02)!=0) s.Write("XV ",size);
    else s.Write("   ",size);

    if ((status&0x04)!=0) s.Write("MS ",size);
    else s.Write("   ",size);

    if ((status&0x08)!=0) s.Write("CV ",size);
    else s.Write("   ",size);

    if ((status&0x10)!=0) s.Write("SM ",size);
    else s.Write("   ",size);

    if ((status&0x20)!=0) s.Write("US ",size);
    else s.Write("   ",size);

    if ((status&0x40)!=0) s.Write("TM ",size);
    else s.Write("   ",size);

    if ((status&0x80)!=0) s.Write("UT ",size);
    else s.Write("   ",size);

    size=1;
    s.Write("|",size);

}

