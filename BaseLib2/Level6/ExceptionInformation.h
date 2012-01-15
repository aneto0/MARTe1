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

/**
 * @file
 */
#if !defined (EXCEPTION_INFORMATION)
#define EXCEPTION_INFORMATION

#include "System.h"
#include "FString.h"

/** */
const int32 XH_ET_Terminate                 = 1;
/** */
const int32 XH_ET_AsyncTerminate            = 2;
/** */
const int32 XH_ET_ReadAccessViolation       = 3;
/** */
const int32 XH_ET_WriteAccessViolation      = 4;
/** */
const int32 XH_ET_ExecuteAccessViolation    = 5;
/** */
const int32 XH_ET_AccessViolation           = 6;
/** */
const int32 XH_ET_IntDivideByZero           = 7;
/** */
const int32 XH_ET_FloatDivideByZero         = 8;
/** */
const int32 XH_ET_UnWind                    = 9;
/** */
const int32 XH_ET_GuardPageViolation        = 10;
/** */
const int32 XH_ET_PriviledgedInstruction     = 11;
/** */
const int32 XH_ET_IllegalInstruction        = 12;
/** */
const int32 XH_ET_Unknown                   = 0;


/** platform specific exception information */
class ExceptionInformation{
public:
    /** */
    uint32 exceptionType;

    /** */
    uint32 exceptionAddress;

    /** where we were trying to access to */
    uint32 exceptionAccessAddress;

    /** */
    FString infoStr;

    /** */
    void CleanUp();

public:
    /** */
    static const char *ExplainExceptionType(uint32 type);

    /** */
    ExceptionInformation(){ CleanUp(); }

#if defined(_WIN32)
    /** */
    ExceptionInformation(
        _EXCEPTION_RECORD     *report,
        _CONTEXT              *context);
#elif defined(_OS2)
    ExceptionInformation(
        EXCEPTIONREPORTRECORD *report,
        CONTEXTRECORD         *context);

#endif
};



#endif
























#if 0
/** */
class ExceptionHandlable {
    //
    friend uint32 __thread _ExceptionHandler_Handler(void *p1,void *p2,void *p3,void *p4 );

    /** */ may be overridden to add specific handling
    virtual uint32 Handle(ExceptionInformation &info,ExceptionHandler *h){
        return XH_NotHandled;
    }

};

/** */
class ExceptionHandler {
#if defined(_OS2)
    // OS2 specific version
    EXCEPTIONREGISTRATIONRECORD err;

    //
    friend uint32 __thread _ExceptionHandler_Handler(void *p1,void *p2,void *p3,void *p4 );

#elif defined(_WIN32)

#else

#endif
protected:

    /** */
    ExceptionHandlable     *object;

    /** */ called back when an exception occurs
    uint32                  Handle(ExceptionInformation *ei);

public:
//    jmp_buf                 jmpWorker;

    /** */
    ExceptionHandler(){
        object = NULL;
    }

    /** */
    virtual ~ExceptionHandler(){
        if (object != NULL)
            DeInstallHandler();
    }

    /** */
    bool InstallHandler(ExceptionHandlable *object);

    /** */
    bool DeInstallHandler();

    /** */
//    void Return();

};

/** */ call this to mark the return point
//#define Try(handler) (setjmp( handler.jmpWorker )!=0x55)


#endif
