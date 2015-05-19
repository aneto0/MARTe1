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

#include "GeneralDefinitions.h"
#include "HighResolutionTimer.h"
#include "Threads.h"

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

		/// implies that ObjectPointer and ClassName are sent as well
		bool        isObject:1;

	} header;


	/**
	 *  time as high resoution timer ticks
	*/  
	int64     		hrtTime;
	
	/** a pointer to a const char * which is persistent
	 * so a real constant, not a char * relabeled as const char *
	 * scope should be global to the application and persistent 
	 */ 
	const char * 	fileName;

	/** a pointer to a const char * which is persistent
	 * so a real constant, not a char * relabeled as const char *
	 * scope should be global to the application and persistent 
	 */ 
	const char * 	functionName;
	
	/// thread ID
	TID		    	threadId;
	
	/**
	 * address of the object that produced the error
	 * object may be temporary in memory
	 * as objectPointer will only be printed, not used
	 */
	void *      	objectPointer;

	/** a pointer to a const char * which is persistent
	 * so a real constant, not a char * relabeled as const char *
	 * scope should be global to the application and persistent 
	 */ 
	const char * 	className;
	
	/// initialise to 0 all parts that are not set by all the reporting functions
	ErrorInformation(){
		threadId 		= (TID)0;
		objectPointer 	= NULL;
		className       = NULL;
	}
};

/** the type of an user provided ErrorProcessing function */
typedef void (*ErrorMessageProcessFunctionType)(
		ErrorInformation &			errorInfo,
		const char *				errorDescription);

/**
 * pointer to the function that will process the errors
 */
extern ErrorMessageProcessFunctionType errorMessageProcessFunction; 

extern "C" {
	/// translates ErrorType to string 
	const char *ErrorManagement_ErrorName(ErrorType errorCode);

	/// installs a user defined Error Handling function
	void ErrorManagement_SetErrorMessageProcessFunction(ErrorMessageProcessFunctionType userFun=NULL);

}

/**
 * Collection of functions and  types to manage error reporting
 */
class ErrorManagement{

public:    
    /** translate ErrorManagement::ErrorType to ErrorName */
	static inline const char *ErrorName(ErrorType errorCode){
		return ErrorManagement_ErrorName(errorCode);
	}

    /** 
        Simplest error report function
        Non blocking. Thread and interrupt safe        
	 */
	static inline void ReportError(
				ErrorType 			code, 
				const char *		errorDescription,
				const char *        fileName			= NULL,
				unsigned int		lineNumber  		= 0,
				const char *		functionName        = NULL
				){
		ErrorInformation errorInfo;
		errorInfo.header.errorType  = code;
		errorInfo.header.lineNumber = lineNumber;
		errorInfo.fileName 	 		= fileName;
		errorInfo.functionName      = functionName;
		errorInfo.hrtTime 			= HighResolutionTimer::Counter();
#if !INTERRUPT_SUPPORTED		
		errorInfo.threadId 			= Threads::Id();
#endif		
		errorMessageProcessFunction(errorInfo, errorDescription);
	}

    /** 
        Simplest error report function
        Non blocking. Thread safe     
        Not Interrupt safe   
	 */
	static inline void ReportErrorFullContext(
			ErrorType 			code, 
			const char *		errorDescription,             /// memory volatile - will not be touched but is not expected to be persistent
			const char *        fileName			= NULL,   /// permanent in memory- accessible to all threads
			unsigned int		lineNumber  		= 0,
			const char *		functionName        = NULL    /// permanent in memory- accessible to all threads
			){

		ErrorInformation errorInfo;
		errorInfo.header.errorType  = code;
		errorInfo.header.lineNumber = lineNumber;
		errorInfo.fileName 	 		= fileName;
		errorInfo.functionName      = functionName;
		errorInfo.hrtTime 			= HighResolutionTimer::Counter();		
		errorInfo.threadId 			= Threads::Id();	
		errorMessageProcessFunction(errorInfo, errorDescription);
	}

public:    
    /** set the handler for error messages */
	static inline void SetErrorMessageProcessFunction(ErrorMessageProcessFunctionType userFun=NULL){
		ErrorManagement_SetErrorMessageProcessFunction(userFun);
	}
};

#ifndef __FUNCTION_NAME__
    #if  defined __FUNCTION__ 
        // Undeclared
        #define __FUNCTION_NAME__   __FUNCTION__  
    #elif defined __PRETTY_FUNCTION__
        // Undeclared
        #define __FUNCTION_NAME__   __PRETTY_FUNCTION__
    #else
        // Declared 
		#define __FUNCTION_NAME__   __func__ 
    #endif // __func__ 

#endif // __FUNCTION_NAME__

#define REPORT_ERROR(code,message)\
		ErrorManagement::ReportError(code,message,__FILE__,__LINE__,__FUNCTION_NAME__);

#define REPORT_ERROR_FULL(code,message)\
		ErrorManagement::ReportErrorFullContext(code,message,__FILE__,__LINE__,__FUNCTION_NAME__);

#endif
