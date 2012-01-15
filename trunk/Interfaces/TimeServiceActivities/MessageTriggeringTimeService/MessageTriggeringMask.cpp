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
#include "MessageTriggeringMask.h"
#include "MessageDeliveryRequest.h"
#include "CDBExtended.h"

bool MessageTriggeringMask::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    if(!GCReferenceContainer::ObjectLoadSetup(info, err)){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: GCReferenceContainer::ObjectLoadSetup Failed",Name());
        return False;
    }
    if(Size() < 1){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: At least one message must be specified",Name());
        return False;
    }

    CDBExtended cdb(info);
    if(!cdb.ReadInt64(mask, "Mask")){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Mask value was not specified",Name());
        return False;
    }
    if(!cdb.ReadInt32(channelNumber, "ChannelNumber")){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: ChannelNumber was not specified",Name());
        return False;
    }
    FString tmp;
    if(!cdb.ReadFString(tmp, "Latched")){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Latched was not specified",Name());
        return False;
    }
    latched = (tmp == "True") || (tmp == "true");
    tmp.SetSize(0);
    if(!cdb.ReadFString(tmp, "Operation")){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Operation was not specified",Name());
        return False;
    }
    if(tmp == "AND"){
        operation = AND;
    }
    else if(tmp == "OR"){
        operation = OR;
    }
    else if(tmp == "XOR"){
        operation = XOR;
    }
    else if(tmp == "NAND"){
        operation = NAND;
    }
    else if(tmp == "NOR"){
        operation = NOR;
    }
    else if(tmp == "NXOR"){
        operation = NXOR;
    }
    else{
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Unrecognised operation %s",Name(), tmp.Buffer());
        return False;
    }
    tmp.SetSize(0);
    if(!cdb.ReadFString(tmp, "TestType")){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: TestType was not specified",Name());
        return False;
    }
    if(tmp == "OriginalInput"){
        testType = ORIGINAL_INPUT;
    }
    else if(tmp == "ChangedBitsAny"){
        testType = CHANGED_BITS_ANY;
    }
    else if(tmp == "ChangedBits0to1"){
        testType = CHANGED_BITS_0_1;
    }
    else if(tmp == "ChangedBits1to0"){
        testType = CHANGED_BITS_1_0;
    }
    else{
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Unrecognised test type %s",Name(), tmp.Buffer());
        return False;
    }


    if(!cdb.ReadInt64(result, "Result")){
        AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Result was not specified",Name());
        return False;
    }

    GCRTemplate<MessageDeliveryRequest> msg;
    int32 i=0;
    for(i=0; i<Size(); i++){
        msg = Find(i);
        if(!msg.IsValid()){
            AssertErrorCondition(InitialisationError,"MessageTriggeringMask::ObjectLoadSetup: %s: Only MessageDeliveryRequest objects are allowed. Invalid object in position %d",Name(), i);
            return False;
        }
    }

    return True;
}

OBJECTLOADREGISTER(MessageTriggeringMask, "$Id: MessageTriggeringMask.cpp,v 1.3 2011/12/06 09:29:52 aneto Exp $")

