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

#include "ExceptionInformation.h"

void ExceptionInformation::CleanUp(){
    exceptionType       = XH_ET_Unknown;
    exceptionAddress    = 0;
    exceptionAccessAddress = 0;
    infoStr.SetSize(0);
}

const char *ExceptionInformation::ExplainExceptionType(uint32 type){

    switch (type){
        case XH_ET_Terminate                : return "Process Termination";
        case XH_ET_AsyncTerminate           : return "Process Async Termination";
        case XH_ET_ReadAccessViolation      : return "Read Access Violation";
        case XH_ET_WriteAccessViolation     : return "Write Access Violation";
        case XH_ET_ExecuteAccessViolation   : return "Execute Violation";
        case XH_ET_AccessViolation          : return "Access Violation";
        case XH_ET_IntDivideByZero          : return "Integer Divide by 0";
        case XH_ET_FloatDivideByZero        : return "Float Divide by 0";
        case XH_ET_UnWind                   : return "Unwind";
        case XH_ET_GuardPageViolation       : return "Page Violation";
        case XH_ET_Unknown                  : return "Unknown Exception";
        case XH_ET_PriviledgedInstruction   : return "Priviledged Instruction";
        case XH_ET_IllegalInstruction       : return "Illegal Instruction";

    };
    return "Unmapped Exception";
}


#if defined(_WIN32)
ExceptionInformation::ExceptionInformation(
    _EXCEPTION_RECORD *report,
    _CONTEXT          *context){

    CleanUp();
    exceptionAddress  = (uint32)report->ExceptionAddress;

    switch( report->ExceptionCode ) {
        case EXCEPTION_ACCESS_VIOLATION:
            exceptionAccessAddress  = (uint32)report->ExceptionInformation[1];
            switch (report->ExceptionInformation[0]){
                case 0:
                    exceptionType = XH_ET_ReadAccessViolation;
                break;
                case 1:
                    exceptionType = XH_ET_WriteAccessViolation;
                break;
                // from http://www.microsoft.com/technet/prodtechnol/winxppro/maintain/sp2mempr.mspx
                case 8:
                    exceptionType = XH_ET_ExecuteAccessViolation;
                break;
                default:
                    exceptionType = XH_ET_AccessViolation;
            }
        break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:{
            exceptionType = XH_ET_IntDivideByZero;
        } break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:{
            exceptionType = XH_ET_FloatDivideByZero;
        } break;
        case EXCEPTION_IN_PAGE_ERROR:{
            exceptionType = XH_ET_GuardPageViolation;
        }break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:{
            exceptionType = XH_ET_IllegalInstruction;
        }
        case EXCEPTION_PRIV_INSTRUCTION:{
            exceptionType = XH_ET_PriviledgedInstruction;
        }

        default:
            exceptionType = XH_ET_Unknown;
    }
    infoStr.Printf("%s\n",ExplainExceptionType(exceptionType));
    infoStr.Printf("Exception %08x at location %08x access %08x\n",report->ExceptionCode,exceptionAddress,exceptionAccessAddress);
    infoStr.Printf("GS  = %08x FS  = %08x ES  = %08x DS  = %08x\n",context->SegGs,context->SegFs,context->SegEs,context->SegDs);
    infoStr.Printf("EAX = %08x EBX = %08x ECX = %08x EDX = %08x\n",context->Eax,context->Ebx,context->Ecx,context->Edx);
    infoStr.Printf("EDI = %08x ESI = %08x EBP = %08x EIP = %08x\n",context->Edi,context->Esi,context->Ebp,context->Eip);
}

#elif defined(_OS2)
ExceptionInformation::ExceptionInformation(
    EXCEPTIONREPORTRECORD *report,
    CONTEXTRECORD         *context){

    CleanUp();
    exceptionAddress=report->ExceptionInfo[1];

    switch( report->ExceptionNum ) {
        case XCPT_PROCESS_TERMINATE:
            exceptionType = XH_ET_Terminate;
        break;
        case XCPT_ASYNC_PROCESS_TERMINATE:
            exceptionType = XH_ET_AsyncTerminate;
        break;
        case XCPT_ACCESS_VIOLATION:
           switch (exceptionReport->ExceptionInfo[0]){
                case XCPT_READ_ACCESS:
                    exceptionType = XH_ET_ReadAccessViolation;
                break;
                case XCPT_WRITE_ACCESS:
                    exceptionType = XH_ET_WriteAccessViolation;
                break;
                case XCPT_EXECUTE_ACCESS:
                    exceptionType = XH_ET_ExecuteAccessViolation;
                break;
                default:
                    exceptionType = XH_ET_AccessViolation;
            }
        break;
        case XCPT_INTEGER_DIVIDE_BY_ZERO:{
            exceptionType = XH_ET_IntDivideByZero;
        } break;
        case XCPT_FLOAT_DIVIDE_BY_ZERO:{
            exceptionType = XH_ET_FloatDivideByZero;
        } break;
        case XCPT_UNWIND:{
            exceptionType = XH_ET_UnWind;
        } break;
        case XCPT_GUARD_PAGE_VIOLATION:{
            exceptionType = XH_ET_GuardPageViolation;
        }break;
        default:
            exceptionType = XH_ET_Unknown;
    }
    infoStr.Printf("%s\n",ExplainExceptionType(exceptionType));
    infoStr.Printf("Exception %08x flags %x at location %08x\n"   ,report->ExceptionNum,report->fHandlerFlags,report->ExceptionAddress);
    infoStr.Printf("GS  = %08x FS  = %08x ES  = %08x DS  = %08x\n",context->ctx_SegGs,context->ctx_SegFs,context->ctx_SegEs,context->ctx_SegDs);
    infoStr.Printf("EAX = %08x EBX = %08x ECX = %08x EDX = %08x\n",context->ctx_RegEax,context->ctx_RegEbx,context->ctx_RegEcx,context->ctx_RegEdx);
    infoStr.Printf("EDI = %08x ESI = %08x EBP = %08x EIP = %08x\n",context->ctx_RegEdi,context->ctx_RegEsi,context->ctx_RegEbp,context->ctx_RegEip);
}

#endif





#if 0
#if defined(_OS2)
// OS2 specific version
ULONG __thread _ExceptionHandler_Handler(void *p1,void *p2,void *p3,void *p4){
    ExceptionHandler *handler=(ExceptionHandler *)p2;
    ExceptionInformation ei;
    EXCEPTIONREPORTRECORD *report  = (EXCEPTIONREPORTRECORD *)p1;
    CONTEXTRECORD         *context = (CONTEXTRECORD *)p3;

    ei.exceptionAddress=report->ExceptionInfo[1];

    switch( report->ExceptionNum ) {
        case XCPT_PROCESS_TERMINATE:
            ei.exceptionType = XH_ET_Terminate;
        break;
        case XCPT_ASYNC_PROCESS_TERMINATE:
            ei.exceptionType = XH_ET_AsyncTerminate;
        break;
        case XCPT_ACCESS_VIOLATION:
           switch (exceptionReport->ExceptionInfo[0]){
                case XCPT_READ_ACCESS:
                    ei.exceptionType = XH_ET_ReadAccessViolation;
                break;
                case XCPT_WRITE_ACCESS:
                    ei.exceptionType = XH_ET_WriteAccessViolation;
                break;
                case XCPT_EXECUTE_ACCESS:
                    ei.exceptionType = XH_ET_ExecuteAccessViolation;
                break;
                default:
                    ei.exceptionType = XH_ET_AccessViolation;
            }
        break;
        case XCPT_INTEGER_DIVIDE_BY_ZERO:{
            ei.exceptionType = XH_ET_IntDivideByZero;
        } break;
        case XCPT_FLOAT_DIVIDE_BY_ZERO:{
            ei.exceptionType = XH_ET_FloatDivideByZero;
        } break;
        case XCPT_UNWIND:{
            ei.exceptionType = XH_ET_UnWind;
        } break;
        case XCPT_GUARD_PAGE_VIOLATION:{
            ei.exceptionType = XH_ET_GuardPageViolation;
        }break;
        default:
            ei.exceptionType = XH_ET_Unknown;
    }
    ei.InfoStr.Printf("Exception %08x flags %x at location %08x\n"   ,report->ExceptionNum,report->fHandlerFlags,report->ExceptionAddress);
    ei.InfoStr.Printf("GS  = %08x FS  = %08x ES  = %08x DS  = %08x\n",context->ctx_SegGs,context->ctx_SegFs,context->ctx_SegEs,context->ctx_SegDs);
    ei.InfoStr.Printf("EAX = %08x EBX = %08x ECX = %08x EDX = %08x\n",context->ctx_RegEax,context->ctx_RegEbx,context->ctx_RegEcx,context->ctx_RegEdx);
    ei.InfoStr.Printf("EDI = %08x ESI = %08x EBP = %08x EIP = %08x\n",context->ctx_RegEdi,context->ctx_RegEsi,context->ctx_RegEbp,context->ctx_RegEip);


    ULONG ret = XCPT_CONTINUE_SEARCH;

    if (handler != NULL){
        if (handler->object!=NULL)
        uint32 ret2 = handler->object->Handle(ei);
        if (ret2 == XH_ContinueExec) ret = XCPT_CONTINUE_EXECUTION;
    }
    return ret;
}

#elif defined(_WIN32)

#elif defined(_VXWORKS)

#else

#endif


///void ExceptionHandler::Return()  { longjmp(jmpWorker, 0x55 ); }

bool ExceptionHandler::InstallHandler(ExceptionHandlable *object) {
    this->object = object;
#if defined(_OS2)
    err.pNext       = NULL;
    err.pfnHandler  = (PFN)_ExceptionHandler_Handler;
    return (DosSetExceptionHandler( &err ) == 0);
#elif defined(_WIN32)

#elif defined(_VXWORKS)

#else

#endif
}

bool ExceptionHandler::DeInstallHandler(){
    object = NULL;
#if defined(_OS2)
    return (DosUnsetExceptionHandler( &err ) == 0);
#elif defined(_WIN32)

#elif defined(_VXWORKS)

#else

#endif
}


#endif
