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
#include "LoggerMessage.h"
#include "Endianity.h"
#include "SXMemory.h"

OBJECTREGISTER(LoggerMessage,"$Id$")
bool LoggerMessage::LoadFromMessage(const char *sourceAddress, char *buffer, uint32 size){
    Reset();    
    if(!LoadFromText(buffer, size)){
        Reset();
        LoadFromBinary(buffer, size);
    }
    this->sourceAddress = sourceAddress;
    return True;
}

void LoggerMessage::LoadFromBinary(char *buffer, uint32 size){
    int n = size / 4;
    if (n > 5) 
        n = 5;
    

    Endianity::MemCopyFromMotorola((uint32 *)&errorCode,(uint32 *)buffer,n);
    size -= n*4;
    buffer += n*4;

    errorMessage.SetSize(0);
    className.SetSize(0);
    threadName.SetSize(0);

    SXMemory sxm(buffer, size);
    
    sxm.GetToken(threadName, ":", NULL, "");
    sxm.GetToken(className, ":", NULL, "");  
    errorMessage = buffer + sxm.Position();
}

bool LoggerMessage::LoadFromText(char *buffer, uint32 size){
    bool ret = False;    
    
    FString token;
    FString value;
    FString OSErrorMsg;
    uint32 osErrorCode= 0;
    SXMemory sxm(buffer, size);

    char saveSep=0;
    while (sxm.GetToken(token,"|=", &saveSep, NULL)){        
        if (saveSep == '='){
            ret = True;
            value="";
            if (token=="TM"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x", &errorTime);
            } else
            if (token=="C"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') {
                    className = value.Buffer();
                }
            } else
            if (token=="N"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') {
                    threadName = value.Buffer();
                }
            } else
            if (token=="I"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') {
                    sourceAddress = value.Buffer();
                }                
            } else
            if (token=="O"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x", &object);
            } else
            if (token=="T"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x", &tid);
            } else
            if (token=="P"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x", &pid);
            } else
            if (token=="E"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x", &errorCode);
            } else
            if (token=="EX"){
                sxm.GetToken(value,"|",&saveSep,NULL);
                if (saveSep == '|') sscanf(value.Buffer(),"%x", &osErrorCode);
            } else            
            if (token=="EDX"){
                sxm.GetToken(OSErrorMsg,"|",&saveSep,NULL);
                if (saveSep != '|') OSErrorMsg="";
            } else                            
            if (token=="D"){
                sxm.GetToken(errorMessage,"|",&saveSep,NULL);
                if (saveSep != '|') errorMessage="";
            } 
            else{                
                ret = False;
                break;
            }                
        }
        token = "";
    }    
    
    return ret;
}

void LoggerMessage::EncodeToText(StreamInterface &buffer){
    buffer.Printf("|TM=%x", errorTime);
    if(className.Size() > 0)
        buffer.Printf("|C=%s", className.Buffer());
    if(threadName.Size() > 0)
        buffer.Printf("|N=%s", threadName.Buffer());
    if(sourceAddress.Size() > 0)
        buffer.Printf("|I=%s", sourceAddress.Buffer());
    if(object != 0)
        buffer.Printf("|O=%x", object);
    
    buffer.Printf("|T=%x", tid);
    buffer.Printf("|P=%x", pid);
    buffer.Printf("|E=%x", errorCode);
    buffer.Printf("|EX=%x", errorCode);
    if(osErrorMessage.Size() > 0)
        buffer.Printf("|EDX=%s", osErrorMessage.Buffer());
    if(errorMessage.Size() > 0)
        buffer.Printf("|D=%s", errorMessage.Buffer());    
}

void LoggerMessage::FormatMessage(StreamInterface &out){
    uint32 Time = errorTime;
    char stime[64];
    sprintf(stime, "%s", ctime((time_t *)&Time));
    stime[strlen(stime)-1]=0;

    FString buffer;    
    if(((formattedMsgFields & LM_SOURCE_ADDR) == LM_SOURCE_ADDR) && (sourceAddress.Size() > 0)){
        buffer.Printf("%s:", sourceAddress.Buffer());
    }
    if((formattedMsgFields & LM_ERROR_TIME) == LM_ERROR_TIME){
        buffer.Printf("%s:", stime);
    }
    if((formattedMsgFields & LM_ERROR_NAME) == LM_ERROR_NAME){
        const char *errorName = EMFErrorName((EMFErrorType)errorCode);
        if (errorName[0] != 0){
            buffer.Printf("%s:",errorName);
        } else {
            buffer.Printf("ERROR %i:", errorCode);
        }
    }
    if((formattedMsgFields & LM_PID) == LM_PID){
        buffer.Printf("pid=0x%x:", tid);
    }
    if((formattedMsgFields & LM_TID) == LM_TID){
        buffer.Printf("tid=0x%x ", tid);
        if((formattedMsgFields & LM_THREAD_NAME) == LM_THREAD_NAME){
            if (threadName.Size()>0){
                buffer.Printf("(%s)",threadName.Buffer());
            }
        }
    }
    if((formattedMsgFields & LM_OBJECT) == LM_OBJECT){
        if (object != 0){
            buffer.Printf("obj=");
            if((formattedMsgFields & LM_CLASS_NAME) == LM_CLASS_NAME){
                if (className.Size()>0){
                    buffer.Printf("(%s *)",className.Buffer());
                }
            }
            buffer.Printf("0x%x", object);
        }
    }
    if((formattedMsgFields & LM_ERROR_MSG) == LM_ERROR_MSG){
        buffer.Printf(":%s", errorMessage.Buffer());
    }


    switch(errorCode){
        case Information          :  out.SSPrintf(ColourStreamMode,"%i %i",Green     ,White); break;
        case Warning              :  out.SSPrintf(ColourStreamMode,"%i %i",Black    ,White); break;
        case FatalError           :  out.SSPrintf(ColourStreamMode,"%i %i",Red      ,White); break;
        case RecoverableError     :  out.SSPrintf(ColourStreamMode,"%i %i",DarkYellow,White); break;
        case InitialisationError  :  out.SSPrintf(ColourStreamMode,"%i %i",Green    ,White); break;
        case OSError              :  out.SSPrintf(ColourStreamMode,"%i %i",DarkRed  ,White); break;
        case ParametersError      :  out.SSPrintf(ColourStreamMode,"%i %i",DarkGreen,White); break;
        case IllegalOperation     :  out.SSPrintf(ColourStreamMode,"%i %i",Blue     ,White); break;
        case ErrorSharing         :  out.SSPrintf(ColourStreamMode,"%i %i",Cyan     ,White); break;
        case ErrorAccessDenied    :  out.SSPrintf(ColourStreamMode,"%i %i",Purple   ,White); break;
        case Exception            :  out.SSPrintf(ColourStreamMode,"%i %i",Cyan     ,White); break;
        case Timeout              :  out.SSPrintf(ColourStreamMode,"%i %i",Yellow   ,White); break;
        case CommunicationError   :  out.SSPrintf(ColourStreamMode,"%i %i",DarkCyan ,White); break;
        case SyntaxError          :  out.SSPrintf(ColourStreamMode,"%i %i",DarkBlue ,White); break;
    };

    out.Printf("%s\n",buffer.Buffer());

    out.SSPrintf(ColourStreamMode,"%i %i",Grey ,Black);
}
