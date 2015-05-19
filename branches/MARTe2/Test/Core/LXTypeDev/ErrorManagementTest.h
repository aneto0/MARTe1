/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id:$
 *
 **/
/**
 * @class ErrorManagementTest
 * @brief Tests the ErrorManagement functions.
 *
 * The test consists in observing the results of the conversions from float (or equivalent) numbers and strings and their correct
 * print on a generic stream which implements a PutC(c) function.
 */

#ifndef ERROR_MANAGEMENT_TEST_H
#define ERROR_MANAGEMENT_TEST_H

#include "AdvancedErrorManagement.h"
#include "StreamString.h"
#include "StringHelper.h"
#include "Threads.h"
#include "stdio.h"

#define BUFF_SIZE 128


void ReportTestFunction(ErrorInformation& errorInfo, const char* description);	
void ReportParametersTestFunction(ErrorInformation& errorInfo, const char* description);


class ErrorManagementTest {

public:

	/** Use static members only for an access from the extern report error function. */
	static ErrorType errCode;
	static const char* errDescription;
	static const char* errFilename;
	static uint32 errLine;
	static const char* errFunction;
	static uint32 nThreads;
	static bool retVal;
	static const char* expectedErrorName;
	char buffer[BUFF_SIZE];


public:

	ErrorManagementTest(){
		retVal=false;
		for(int32 i=0; i<BUFF_SIZE; i++) buffer[i]=0;
	}

	

	void CheckParameters(ErrorInformation& errorInfo, const char* description){
		if(errorInfo.header.errorType!=errCode){
			retVal=False;
			return;
		}
		if(errorInfo.header.lineNumber!=errLine){
			retVal=False;
			return;
		}
		if(StringHelper::Compare(errorInfo.fileName,errFilename)!=0){
			retVal=False;
			return;
		}
		if(StringHelper::Compare(errorInfo.functionName, errFunction)!=0){
			retVal=False;
			return;
		}	
		if(StringHelper::Compare(description, errDescription)!=0){
			retVal=False;
			return;
		}	
		if(nThreads>0){
			if(errorInfo.threadId!=Threads::Id()){
				retVal=False;
				return;
			}
		}
	
		if(StringHelper::Compare(ErrorManagement::ErrorName(errCode), expectedErrorName)!=0){
			retVal=False;
			return;
		}
	
		retVal=True;
	}

	bool TestReportError(ErrorType code, const char* expected, const char* errorDescription, const char* filename="", uint32 lineNumber=0, const char* functionName="", int32 nOfThreads=0){
		errCode=code;
		expectedErrorName=expected;
		errDescription=errorDescription;
		errFilename=filename;
		errLine=lineNumber;
		errFunction=functionName;
		nThreads=nOfThreads;
		


		ErrorManagement_SetErrorMessageProcessFunction(ReportParametersTestFunction);

		

		REPORT_ERROR_PARAMETERS(code, "Type:%s File:%s Line:%d Fun:%s", expected, filename, lineNumber, functionName);

		if(!retVal) return False;


   		ErrorManagement_SetErrorMessageProcessFunction(ReportTestFunction);
	
         	for(int32 i=0; i<nOfThreads; i++){
			retVal=False;
			ErrorManagement::ReportError(code, errorDescription, filename, lineNumber, functionName);
			ErrorManagement::ReportErrorFullContext(code, errorDescription, filename, lineNumber, functionName);
			
			if(!retVal) return False;
			
		}		
		return retVal;
	}
	


};



#endif
