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
 * $Id: ErrorManagement.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

/** 
 * @file
 * Logging management functions
 */
#ifndef _ERROR_MANAGEMENT_H
#define _ERROR_MANAGEMENT_H

//#include "System.h"

struct ErrorInformation;
/** values to be used in ErrorManagementFunction */
enum ErrorType {
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
	SyntaxError         = -12,
	/**	something that should be possible but still it is not supported	 */
	UnsupportedError    = -13
};

/** 
 	 Information associated with the error
 	 to better be able to debug what happened and/or 
 	 to disambiguate among possible sources 
 */
struct ErrorInformation{
	
	/// first to be transmitted is the header;
	struct {
		///
		ErrorType 	errorType:8;

		///
		uint16      lineNumber;

		///
		bool        isObject:1;

		///
		bool        hasProcessId:1;

		///
		bool        hasObjectPointer:1;

		///
		bool        hasClassName:1;

	} header;

	/// msecs from start of time -  
	uint64     	msecTime;

	///
	char *      fileName;
	
	/// thread ID
	TID		    threadId;
	
	///
//	PID         processId;
	
	///
	void *      objectPointer;

	///
	char *      className;
	
	/// 
	ErrorInformation(){
		
	}

};

/** the type of an user provided ErrorProcessing function */
typedef void (*AssembleErrorMessageFunctionType)(ErrorInformation &information,const char *errorDescription);


extern "C" {

	const char *ErrorManagement_ErrorName(ErrorType errorCode);

	void ErrorManagement_ReportError(ErrorType code, const char *errorDescription);

	void ErrorManagement_ReportErrorWithinInterrupt(ErrorType errorCode,const char *errorDescription);

	void ErrorManagement_SetUserAssembleErrorMessageFunction(AssembleErrorMessageFunctionType userFun=NULL);

	void ErrorManagement_SetUserAssembleISRErrorMessageFunction(AssembleErrorMessageFunctionType userFun);

}

/**
 * Collection of functions and  types to manage error reporting
 */
class ErrorManagement{

	
public:

public:    
    /** translate ErrorManagement::ErrorType to ErrorName */
	static inline const char *ErrorName(ErrorType errorCode){
		return ErrorManagement_ErrorName(errorCode);
	}
	

public:
	/** what to do in case of error */
	enum ErrorBehaviour {
		/** on error loop infinitely */
		onErrorSulk           = 0x0001,
		/** on error just exit the thread*/
		onErrorQuit           = 0x0002,
		/** on error remember the error code */
//		onErrorRemember       = 0x0004,
		/** on error print the error on a remote error-console non UDP.. not supported now */
		onErrorReport         = 0x0008,
		/** on error print the error on the = console */
		onErrorReportConsole  = 0x0010,
		/** on error print the error on a log file */
//		onErrorLog            = 0x0020,
		/** Remote debugging support option : on error sends an UDP packet with the error content */
//		onErrorRemoteLog      = 0x0040
	};
	
public:
	
public:

    /** Sets the error status and depending on setup does appropriate action
        Most simple error function
        Must be non blocking.
        Must be thread safe
	 */
	static inline void ReportError(ErrorType code, const char *errorDescription){
		ErrorManagement_ReportError(code, errorDescription);
	}

    /** Sets the error status and depending on setup does appropriate action
        This call is to be called from Interrupt Service Routines
        Most simple error function
        Must be non blocking.
        Must be thread and interrupt safe
    */
	static inline void ReportErrorWithinInterrupt(ErrorType errorCode,const char *errorDescription){
		ErrorManagement_ReportErrorWithinInterrupt(errorCode,errorDescription);
	}


public:    
    /** set the handler for error messages */
	static inline void SetUserAssembleErrorMessageFunction(AssembleErrorMessageFunctionType userFun=NULL){
		ErrorManagement_SetUserAssembleErrorMessageFunction(userFun);
	}

    /** sets the handler for error messages coming from interrupts */
	static inline void SetUserAssembleISRErrorMessageFunction(AssembleErrorMessageFunctionType userFun){
		ErrorManagement_SetUserAssembleISRErrorMessageFunction(userFun);
	}

};


#endif
