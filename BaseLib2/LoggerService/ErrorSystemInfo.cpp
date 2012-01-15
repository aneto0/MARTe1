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

#include "ErrorSystemInfo.h"
#include "Threads.h"
#include "Object.h"
//#include "ProcessClass.h"
#include "Endianity.h"
#include "ErrorSystemInstructionItem.h"
#include "ObjectRegistryDataBase.h"
#include "ObjectRegistryItem.h"

void ESIInit(ErrorSystemInfo &esi){
    esi.errorAction           = 0;
    esi.errorCode             = 0;
    esi.errorTime             = 0;
    esi.tid                   = 0;
    esi.pid                   = 0;
    esi.object                = NULL;
    esi.errorMessage.SetSize(0);
    esi.className.SetSize(0);
    esi.threadName.SetSize(0);
}

void ESILoad(ErrorSystemInfo &esi,StreamInterface &in){
    esi.errorAction           = 0;
    esi.errorCode             = 0;
    esi.errorTime             = 0;
    esi.tid                   = 0;
    esi.pid                   = 0;
    esi.object                = NULL;
    esi.errorMessage.SetSize(0);
    esi.className.SetSize(0);
    esi.threadName.SetSize(0);

    FString token;
    FString value;
    FString OSErrorMsg;
    uint32 osErrorCode= 0;
    FString msg;

    char saveSep=0;
    while (in.GetToken(token,"|=",&saveSep,NULL)){
        if (saveSep == '='){
            value="";
            if (token=="TM"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x",&esi.errorTime);
            } else
            if (token=="C"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') {
                    esi.className = value.Buffer();
                }
            } else
            if (token=="N"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') {
                    esi.threadName = value.Buffer();
                }
            } else
            if (token=="O"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x",&esi.object);
            } else
            if (token=="T"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x",&esi.tid);
            } else
            if (token=="P"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x",&esi.pid);
            } else
            if (token=="E"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x",&esi.errorCode);
            } else
            if (token=="EX"){
                in.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x",&osErrorCode);
            } else
            if (token=="EDX"){
                in.GetToken(OSErrorMsg,"|",&saveSep,NULL);
                if (saveSep != '|') OSErrorMsg="";
            } else
            if (token=="D"){
                in.GetToken(msg,"|",&saveSep,NULL);
            }
        }
        token = "";
    }

    if (osErrorCode !=0)
        esi.errorMessage.Printf("OSErr=%i=>%s::%s",osErrorCode,OSErrorMsg.Buffer(),msg.Buffer());
    else
        esi.errorMessage.Printf("%s",msg.Buffer());

    esi.errorAction = NOT_FOUND;

    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind((char *)esi.ClassName());
    if (ori!=NULL){
        esi.errorAction = ori->classInstructions.SearchErrorAction(esi.object,esi.ClassName(),esi.errorCode);
    }if (esi.errorAction == NOT_FOUND)
        esi.errorAction = OBJGetGlobalInstructions()->SearchErrorAction(esi.object,esi.ClassName(),esi.errorCode);
    if (esi.errorAction == NOT_FOUND) esi.errorAction = OBJGetGlobalErrorAction();
}

void  ESISend(ErrorSystemInfo &esi,StreamInterface &out){
    const int bufSize = 256;
    char buffer[bufSize];
    int left = bufSize-1;

    // Need to do this initialisation to avoid optimizer to
    // mess up in its particular way!!!
    memset(buffer,0,bufSize);

    // 5 numbers
    int size = 20;
    Endianity::MemCopyToMotorola((uint32 *)buffer,(uint32 *)&esi.errorCode,size/4);
    char *pb = &buffer[20];
    left -= 20;

    // allow space for the : in any case
    left--;
    strncat(pb,esi.ThreadName(),left);
    strncat(pb,":",left);
    // adjust size of left
    left -= strlen(esi.ThreadName());
    if (left<0) left = 0;

    // allow space for the : in any case
    left--;
    strncat(pb,esi.ClassName(),left);
    strncat(pb,":",left);
    // adjust size of left
    left -= strlen(esi.ClassName());
    if (left<0) left = 0;

    strncat(pb,esi.errorMessage.Buffer(),left);
    left -= strlen(esi.errorMessage.Buffer());
    if (left<0) left = 0;

    uint32 sz = bufSize-left;
    out.Write(buffer,sz);
}

void ESIFromBuffer(ErrorSystemInfo &esi,char *buffer,uint32 size){
    int n = size / 4;
    if (n > 5) n = 5;

    Endianity::MemCopyFromMotorola((uint32 *)&esi.errorCode,(uint32 *)buffer,n);
    size -= n*4;
    buffer+=n*4;

    esi.errorAction  = 0;
    esi.errorMessage.SetSize(0);
    esi.className.SetSize(0);
    esi.threadName.SetSize(0);

    FString temp;
    temp = buffer;

    temp.GetToken(esi.threadName,":",NULL,"");
    temp.GetToken(esi.className,":",NULL,"");
    esi.errorMessage = temp.Buffer() + temp.Position();
//    temp.GetToken(esi.errorMessage,"",NULL,False);

}


void ESIComposeToText(ErrorSystemInfo &esi,StreamInterface &out){
    uint32 Time = esi.ErrorTime();
    char stime[64];
#ifndef _RTAI
    sprintf(stime,"%s",ctime((time_t *)&Time));
#else
    char t[256];
    ctime(t,256,(time_t *)&Time);
    sprintf(stime,"%s",t);
#endif
    stime[strlen(stime)-1]=0;

    FString buffer;

    buffer.Printf("%s:",stime);
    const char *errorName = EMFErrorName((EMFErrorType)esi.ErrorCode());
    if (errorName[0] != 0){
        buffer.Printf("%s:",errorName);
    } else {
        buffer.Printf("ERROR %i:",esi.ErrorCode());
    }
    buffer.Printf("tid=0x%x ",esi.Tid());
    if (esi.threadName.Size()>0){
        buffer.Printf("(%s)",esi.threadName.Buffer());
    }
    if (esi.Object() != 0){
        buffer.Printf("obj=");
        if (esi.className.Size()>0){
            buffer.Printf("(%s *)",esi.className.Buffer());
        }
        buffer.Printf("0x%x",esi.Object());
    }
    buffer.Printf(":%s",esi.ErrorMessage());


    switch(esi.ErrorCode()){
        case Information          :  out.SSPrintf(ColourStreamMode,"%i %i",Grey     ,Black); break;
        case Warning              :  out.SSPrintf(ColourStreamMode,"%i %i",White    ,Black); break;
        case FatalError           :  out.SSPrintf(ColourStreamMode,"%i %i",Red      ,Black); break;
        case RecoverableError     :  out.SSPrintf(ColourStreamMode,"%i %i",DarkYellow,Black); break;
        case InitialisationError  :  out.SSPrintf(ColourStreamMode,"%i %i",Green    ,Black); break;
        case OSError              :  out.SSPrintf(ColourStreamMode,"%i %i",DarkRed  ,Black); break;
        case ParametersError      :  out.SSPrintf(ColourStreamMode,"%i %i",DarkGreen,Black); break;
        case IllegalOperation     :  out.SSPrintf(ColourStreamMode,"%i %i",Blue     ,Black); break;
        case ErrorSharing         :  out.SSPrintf(ColourStreamMode,"%i %i",Cyan     ,Black); break;
        case ErrorAccessDenied    :  out.SSPrintf(ColourStreamMode,"%i %i",Purple   ,Black); break;
        case Exception            :  out.SSPrintf(ColourStreamMode,"%i %i",Cyan     ,Black); break;
        case Timeout              :  out.SSPrintf(ColourStreamMode,"%i %i",Yellow   ,Black); break;
        case CommunicationError   :  out.SSPrintf(ColourStreamMode,"%i %i",DarkCyan ,Black); break;
        case SyntaxError          :  out.SSPrintf(ColourStreamMode,"%i %i",DarkBlue ,Black); break;
    };

    out.Printf("%s\n",buffer.Buffer());

    out.SSPrintf(ColourStreamMode,"%i %i",Grey ,Black);
}


