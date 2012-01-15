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

#include "Message.h"
#include "FastPollingMutexSem.h"
#include "CDBExtended.h"

OBJECTLOADREGISTER(Message,"$Id: Message.cpp,v 1.8 2007/11/30 16:12:30 fpiccolo Exp $");

bool MSGObjectLoadSetup(
            Message &               msg,
            ConfigurationDataBase & info,
            StreamInterface *       err){

    CDBExtended cdbx(info);

    BString name;
    if (!info->NodeName(name)){
        msg.AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot retrieve name of node");
        return False;
    }
    msg.SetObjectName(name.Buffer());

    int32 code;
    int32 userCode;
    bool ret = True;
    bool ret1 = cdbx.ReadInt32(code,"Code",0);
    bool ret2 = cdbx.ReadInt32(userCode,"UserCode",0);
    bool ret3 = cdbx.ReadBString(msg.content,"Content","");
    if (!(ret1 || ret2 || ret3)){
        ret = False;
        msg.AssertErrorCondition(ParametersError,"ObjectLoadSetup:Code or UserCode or Content MUST be specified");
    }

    if (ret1 && ret2 ){
        msg.AssertErrorCondition(Warning,"ObjectLoadSetup:Code and UserCode have been specified, UserCode will be used");
        msg.messageCode = UserMessageCode + userCode;
    } else {
        if (ret2 ){
            msg.messageCode = UserMessageCode + userCode;
        } else {
            msg.messageCode = MessageCode(code);
        }
    }


    ret = msg.GCReferenceContainer::ObjectLoadSetup(info,err);


    if (!ret){
        msg.AssertErrorCondition(FatalError,"ObjectLoadSetup: GCReferenceContainer::ObjectLoadSetup failed");
    }

    return ret;
}

bool MSGObjectSaveSetup(
            Message &               msg,
            ConfigurationDataBase & info,
            StreamInterface *       err){


    CDBExtended cdbx(info);


    if (!cdbx.WriteInt32(msg.GetMessageCode().Code(),"Code")){
        msg.AssertErrorCondition(ParametersError,"ObjectSaveSetup:Cannot write Code for this Message");
        return False;
    }

    if (!cdbx.WriteString(msg.content.Buffer(),"Content")){
        msg.AssertErrorCondition(Warning,"ObjectSaveSetup:Cannot write Content for this Message");
    }

    bool ret = True;

    ret = msg.GCReferenceContainer::ObjectSaveSetup(info,err);

    if (!ret){
        msg.AssertErrorCondition(FatalError,"ObjectSaveSetup: GCReferenceContainer::ObjectSaveSetup failed");
    }

    return ret;
}

static uint32 uniqueId = 0;
static FastPollingMutexSem uidMux;

uint32 MSGGetUniqueId(){

    uint32 id;
    uidMux.FastLock();

    id = uniqueId++;

    uidMux.FastUnLock();

    return id;

}


