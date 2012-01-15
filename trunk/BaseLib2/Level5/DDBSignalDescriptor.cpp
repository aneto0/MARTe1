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
#include "DDBSignalDescriptor.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"

void DDBSD(DDBSignalDescriptor *ddbsd, 
                const char*                 signalName,
                const int                   signalSize,
                const int                   from,
                const BasicTypeDescriptor   typeCode,
                const DDBStoringProperties& ddbss){

    ddbsd->typeCode = typeCode;
    ddbsd->storingProperties = ddbss;
            
    if ((signalName==NULL) || (strlen(signalName)==0)){
        ddbsd->signalName="Unnamed";
        ddbsd->AssertErrorCondition(ParametersError,"DDBSignalDescriptor::DDBSignalDescriptor() The name of the variable has not been specified.");
    }else{
        ddbsd->signalName = signalName;
    }

    ddbsd->signalSize = signalSize;
    ddbsd->from       = from;

    if(signalSize <= 0){
        ddbsd->AssertErrorCondition(ParametersError,"DDBSignalDescriptor::DDBSignalDescriptor() Signal Size for signal %s has to be larger than 0",signalName);
    }
}

void DDBSD2(DDBSignalDescriptor *ddbsd, const DDBSignalDescriptor& descriptor){
    ddbsd->signalName        = descriptor.SignalName();
    ddbsd->signalSize        = descriptor.SignalSize();
    ddbsd->from              = descriptor.SignalFromIndex();
    ddbsd->typeCode          = descriptor.SignalTypeCode();
    ddbsd->storingProperties = descriptor.SignalStoringProperties();
}

OBJECTREGISTER(DDBSignalDescriptor,"$Id: DDBSignalDescriptor.cpp,v 1.9 2008/06/04 16:25:02 fpiccolo Exp $")

void DDBSignalDescriptor::Print(StreamInterface& s) const {

    FString typeName;
    if (typeCode==BTDInt32)  {typeName="int32  ";}else
    if (typeCode==BTDInt64)  {typeName="int64  ";}else
    if (typeCode==BTDUint32) {typeName="uint32 ";}else
    if (typeCode==BTDUint64) {typeName="uint64 ";}else
    if (typeCode==BTDFloat)  {typeName="float32";}else
    if (typeCode==BTDDouble) {typeName="float64";}else{
	typeName="Unknown";
        AssertErrorCondition(ParametersError,"DDBSignalDescriptor::Print() Signal %s has an unknown type code",signalName.Buffer());
    }

    if(from == -1) {
        s.Printf("%s %10s[%02i]; "     , typeName.Buffer(),signalName.Buffer(),signalSize);
    }else{
        if(signalSize > 1)s.Printf("%s %10s(%02i:%02i); ", typeName.Buffer(),signalName.Buffer(),from,from+signalSize-1);
        else              s.Printf("%s %10s(%02i);      ", typeName.Buffer(),signalName.Buffer(),from);
    }

    storingProperties.Print(s);
}

bool DDBSignalDescriptor::ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err) const {

    CDBExtended cdb(info);
    cdb.WriteString(signalName.Buffer(),"SignalName");
    cdb.WriteInt32(signalSize,"Size");

    FString typeName;
    if (typeCode==BTDInt32)  {typeName="int32  ";}else
    if (typeCode==BTDInt64)  {typeName="int64  ";}else
    if (typeCode==BTDUint32) {typeName="uint32 ";}else
    if (typeCode==BTDUint64) {typeName="uint64 ";}else
    if (typeCode==BTDFloat)  {typeName="float32";}else
    if (typeCode==BTDDouble) {typeName="float64";}

    cdb.WriteString(typeName.Buffer(),"TypeCode");

    return True;
}

bool DDBSDBrowse(StreamInterface &in,StreamInterface &out,void *userData){
    DDBSignalDescriptor *ddbsd = (DDBSignalDescriptor *)userData;
    if(ddbsd == NULL) return False;

    ddbsd->Print(out);

    return True;

}


#if 0
bool DDBSignalDescriptor::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    if (!cdb.ReadFString(signalName,"SignalDescriptor.Name"))
        return False;

    if (!cdb.WriteInt32(signalSize,"SignalDescriptor.Size"))
        return False;

    FString stringValue;
    if (!cdb.ReadFString(stringValue,"SignalDescriptor.Structure.IsConsecutive")){
        AssertErrorCondition(InitialisationError,"DDBSignalDescriptor::ObjectLoadSetup: Failed reading entry IsConsecutive for signal %s", signalName.Buffer());
        return False;
    }else{
        if (strcmp(stringValue.Buffer(),"True")==0)
            storingProperties |=  DDB_Consecutive;
        else
            storingProperties &= ~DDB_Consecutive;
    }

    int32 typeCode;
    if (!cdb.ReadInt32(typeCode,"SignalDescriptor.Structure.TypeCode")){
        AssertErrorCondition(InitialisationError,"DDBSignalDescriptor::ObjectLoadSetup: Failed reading entry TypeCode for signal %s", signalName.Buffer());
        return False;
    }else{
        this->typeCode=(DDBTypeCode)typeCode;
    }
    return True;
}




#endif
