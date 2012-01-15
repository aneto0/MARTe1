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

#include "ErrorManagement.h"
#include "Threads.h"
#include "Sleep.h"
#include "Processes.h"


void NULLAssembleErrorMessageFunction(const char *errorDescription,va_list argList,const char *errorHeader,...){


    if (errorHeader!=NULL){
        va_list argList2;
        va_start(argList2,errorHeader);
        vprintf(errorHeader,argList2);
        va_end(argList2);
    }

    vprintf(errorDescription,argList);
    printf("\n");
}


// selects the error processing function
AssembleErrorMessageFunctionType assembleErrorMessage = NULLAssembleErrorMessageFunction;

// selects the error processing function
AssembleErrorMessageFunctionType assembleISRErrorMessage = NULLAssembleErrorMessageFunction;

void LSSetUserAssembleErrorMessageFunction(AssembleErrorMessageFunctionType userFun){
    if (userFun != NULL)  assembleErrorMessage = userFun;
    else assembleErrorMessage = NULLAssembleErrorMessageFunction;
}

void LSGetUserAssembleErrorMessageFunction(AssembleErrorMessageFunctionType &userFun){
    userFun = assembleErrorMessage;
}

void LSSetUserAssembleISRErrorMessageFunction(AssembleErrorMessageFunctionType userFun){
    assembleISRErrorMessage = userFun;
}




void VCStaticAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,va_list argList){
#if (defined(_VXWORKS) || defined (_LINUX) || defined(_RTAI) || defined(_MACOSX))
    assembleErrorMessage(errorDescription,argList,"|TM=%x|T=%08x|E=%08x|D=",GetDateSeconds(),Threads::ThreadId(),errorCode);
#else
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|T=%08x|P=%08x|E=%08x|D=",Threads::GetName(),GetDateSeconds(),Threads::ThreadId(),Processes::GetThisPid(),errorCode);
#endif
}

/**
   Sets the error status and depending on setup does appropriate action
   This call is to be called from static members
*/
void VCAssertErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
#if (defined(_VXWORKS) || defined (_LINUX) || defined(_RTAI) || defined(_MACOSX))
    assembleErrorMessage(errorDescription,argList,"|TM=%x|C=%s|O=%08x|T=%08x|E=%08x|D=",GetDateSeconds(),className,object,Threads::ThreadId(),errorCode);
#else
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|P=%08x|E=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),Processes::GetThisPid(),errorCode);
#endif
}

void CStaticAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,...){
    va_list argList;
    va_start(argList,errorDescription);
    VCStaticAssertErrorCondition(errorCode,errorDescription,argList);
    va_end(argList);
}


void VCISRAssertErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
    assembleISRErrorMessage(errorDescription,argList,"|TM=%x|C=%s|O=%08x|E=%08x|D=",GetDateSeconds(),className,object,errorCode);
}

void CISRStaticAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,...){
    va_list argList;
    va_start(argList,errorDescription);
    assembleISRErrorMessage(errorDescription,argList,"|TM=%x|E=%08x|D=",GetDateSeconds(),errorCode);
    va_end(argList);
}

#if defined(_OS2)
void VCAssertPlatformErrorCondition(EMFErrorType errorCode,const void *object,const char *className,int32 os2ErrorCode,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|TM=%x|C=%s|O=%08x|T=%08x|P=%08x|E=%08x|EX=%08x|EDX=%s|D=",GetDateSeconds(),className,object,ThreadClass::ThreadId(),ProcessClass::GetThisPid(),errorCode,os2ErrorCode,DosErrorMessage(os2ErrorCode));
}
void CStaticAssertPlatformErrorCondition(EMFErrorType errorCode,int32 os2ErrorCode,const char *errorDescription,...){
    va_list argList;
    va_start(argList,errorDescription);
    assembleErrorMessage(errorDescription,argList,"|TM=%x|T=%08x|P=%08x|E=%08x|EX=%08x|EDX=%s|D=",GetDateSeconds(),ThreadClass::ThreadId(),ProcessClass::GetThisPid(),errorCode,os2ErrorCode,DosErrorMessage(os2ErrorCode));
    va_end(argList);
}
void VCAssertSocketErrorCondition(EMFErrorType errorCode,const void *object,const char *className,int32 os2ErrorCode,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|TM=%x|C=%s|O=%08x|T=%08x|P=%08x|E=%08x|EX=%08x|D=",GetDateSeconds(),className,object,ThreadClass::ThreadId(),ProcessClass::GetThisPid(),errorCode,sock_errno());
}

#endif

#if defined(_WIN32)
/// Sets the error status and depending on setup does appropriate action
void VCAssertPlatformErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|P=%08x|E=%08x|EX=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),Processes::GetThisPid(),errorCode,GetLastError());
}
void CStaticAssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription,...){
    va_list argList;
    va_start(argList,errorDescription);
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|T=%08x|P=%08x|E=%08x|EX=%08x|D=",Threads::GetName(),GetDateSeconds(),Threads::ThreadId(),Processes::GetThisPid(),errorCode,GetLastError());
    va_end(argList);
}
void VCAssertSocketErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|P=%08x|E=%08x|EX=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),Processes::GetThisPid(),errorCode,WSAGetLastError());
}
#endif

#if defined(_VXWORKS)
void VCAssertPlatformErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|E=%08x|OE=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),errorCode,errnoGet());
}
void CStaticAssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription,...){
    va_list argList;
    va_start(argList,errorDescription);
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|T=%08x|E=%08x|OE=%08x|D=",Threads::GetName(),GetDateSeconds(),Threads::ThreadId(),errorCode,errnoGet());
    va_end(argList);
}
void VCAssertSocketErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|E=%08x|OE=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),errorCode,sock_errno());
}
#endif

#if (defined(_LINUX) || defined(_SOLARIS) || defined(_RTAI) || defined(_MACOSX))
/// Sets the error status and depending on setup does appropriate action
void VCAssertPlatformErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|E=%08x|EX=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),errorCode,strerror(errorCode));
}
void CStaticAssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription,...){
    va_list argList;
    va_start(argList,errorDescription);
    assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|T=%08x|E=%08x|EX=%08x|D=",Threads::GetName(),GetDateSeconds(),Threads::ThreadId(),errorCode,strerror(errorCode));
    va_end(argList);
}
void VCAssertSocketErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList){
  assembleErrorMessage(errorDescription,argList,"|N=%s|TM=%x|C=%s|O=%08x|T=%08x|E=%08x|EX=%08x|D=",Threads::GetName(),GetDateSeconds(),className,object,Threads::ThreadId(),errorCode,sock_errno());
}

#endif

typedef struct {
    const char *name;
    EMFErrorType error;
} EMFErrorNameItem;

static const EMFErrorNameItem errorNames[] = {
    {"Information",           Information },
    {"Warning",               Warning },
    {"FatalError",            FatalError },
    {"RecoverableError",      RecoverableError },
    {"InitialisationError",   InitialisationError },
    {"OSError",               OSError },
    {"ParametersError",       ParametersError },
    {"IllegalOperation",      IllegalOperation },
    {"ErrorSharing",          ErrorSharing },
    {"ErrorAccessDenied",     ErrorAccessDenied},
    {"Exception",             Exception},
    {"Timeout",               Timeout},
    {"CommunicationError",    CommunicationError},
    {"SyntaxError",           SyntaxError},
    {NULL,                    SyntaxError},
    };



/** translate EMFErrorType to ErrorName */
const char *EMFErrorName(EMFErrorType errorCode){
    int i=0;
    while ( errorNames[i].name != NULL){
        if (errorNames[i].error == errorCode) return errorNames[i].name;
        i++;
    }
    return "";
}

