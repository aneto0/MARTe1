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
 * Logging management functions
 */
#ifndef _ERROR_MANAGEMENT_H
#define _ERROR_MANAGEMENT_H

#include "System.h"

/** values to be used in ErrorManagementFunction */
enum EMFErrorType {
    /** Debug Information (should never be found in production code) */
    Debug               =   2,
    /** Application Information to be used to understand the working of the code */
    Information         =   1,
    /** Application warns of suspicious conditions */
    Warning             =   0,
    /** Application reports a fatal error */
    FatalError          =  -1,
    /** Application reports an error that allows recovery */
    RecoverableError    =  -2,
    /** Application reports an error during initialisation */
    InitialisationError =  -3,
    /** Error while calling an operating system function */
    OSError             =  -4,
    /** Unexpected parameter value that was passed to a function */
    ParametersError     =  -5,
    /** The operation was illegal in the run time context */
    IllegalOperation    =  -6,
    /** The operation failed because of a sharing problen */
    ErrorSharing        =  -7,
    /** The operation failed because of a sharing problen */
    ErrorAccessDenied   =  -8,
    /** an exception has occurred */
    Exception           =  -9,
    /** a Timeout has occurred */
    Timeout             = -10,
    /** error during a communication */
    CommunicationError  = -11,
    /** error while parsing */
    SyntaxError         = -12
};


/** what to do in case of error */
enum EMFErrorBehaviour {
/** on error loop infinitely */
    onErrorSulkES           = 0x0001,
/** on error just exit */
    onErrorQuitES           = 0x0002,
/** on error remember the error code */
    onErrorRememberES       = 0x0004,
/** on error print the error on a remote error-console non UDP.. not supported now */
    onErrorReportES         = 0x0008,
/** on error print the error on the = console */
    onErrorReportConsoleES  = 0x0010,
/** on error print the error on a log file */
    onErrorLogES            = 0x0020,
/** Remote debugging support option : on error sends an UDP packet with the error content */
    onErrorRemoteLogES      = 0x0040
};

/** the type of an user provided ErrorProcessing function */
typedef void (*AssembleErrorMessageFunctionType)(const char *errorDescription,va_list argList,const char *errorHeader,...);

extern "C" {

    /**
        Sets the error status and depending on setup does appropriate action
        This call is to be called from static members
    */
    void VCAssertErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList);

    /** Sets the error status and depending on setup does appropriate action
        This call is to be called from Interrupt Service Routines
    */
    void VCISRAssertErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList);

    /** Sets the error status and depending on setup does appropriate action
        This call is to be called from Interrupt Service Routines
    */
    void CISRStaticAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,...);

    #if defined(_OS2)
    /** Sets the error status and depending on setup does appropriate action */
    void VCAssertPlatformErrorCondition(EMFErrorType errorCode,const void *object,const char *className,int32 os2ErrorCode,const char *errorDescription,va_list argList);
    #else
    /** Sets the error status and depending on setup does appropriate action */
    void VCAssertPlatformErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList);
    #endif

    /** Sets the error status and depending on setup does appropriate action */
    void VCAssertSocketErrorCondition(EMFErrorType errorCode,const void *object,const char *className,const char *errorDescription,va_list argList);

    /** Sets the error status and depending on setup does appropriate action
        This call is to be called from static members
    */
    void VCStaticAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,va_list argList);

    /** Sets the error status and depending on setup does appropriate action
        This call is to be called from static members
    */
    void CStaticAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,...);

    #if defined(_OS2)
    /** Sets the error status and depending on setup does appropriate action */
    void CStaticAssertPlatformErrorCondition(EMFErrorType errorCode,int32 os2ErrorCode,const char *errorDescription,...);
    #else
    /** Sets the error status and depending on setup does appropriate action */
    void CStaticAssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription,...);
    #endif

    /** set the handler for error messages */
    void LSSetUserAssembleErrorMessageFunction(AssembleErrorMessageFunctionType userFun=NULL);

    /** get the handler for error messages */
    void LSGetUserAssembleErrorMessageFunction(AssembleErrorMessageFunctionType &userFun);

    /** sets the handler for error messages coming from interrupts */
    void LSSetUserAssembleISRErrorMessageFunction(AssembleErrorMessageFunctionType userFun);

    /** translate EMFErrorType to ErrorName */
    const char *EMFErrorName(EMFErrorType errorCode);
}
#endif

