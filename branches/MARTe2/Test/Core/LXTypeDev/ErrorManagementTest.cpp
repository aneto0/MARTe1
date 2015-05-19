#include "ErrorManagementTest.h"

void ReportTestFunction(ErrorInformation& errorInfo, const char* description){
	

	ErrorManagementTest newEM;
	newEM.CheckParameters(errorInfo, description);
	
}

void ReportParametersTestFunction(ErrorInformation& errorInfo, const char* description){
	ErrorManagementTest newEM;

	const char* expected=newEM.expectedErrorName;
	uint32 lineNumber=newEM.errLine;
	const char* functionName=newEM.errFunction;
	const char* fileName=newEM.errFilename;

	StreamString test;
	test="";
	test.Printf("Type:%s File:%s Line:%d Fun:%s",expected , fileName, lineNumber, functionName);
	printf("\n%s\n------%s\n",test.Buffer(), description);

	if(test!=description){
		newEM.retVal=False;
		return;
	}
	
	newEM.retVal=True;
}

	ErrorType ErrorManagementTest::errCode;
	const char* ErrorManagementTest::errDescription;
	const char* ErrorManagementTest::errFilename;
	uint32 ErrorManagementTest::errLine;
	const char* ErrorManagementTest::errFunction;
	uint32 ErrorManagementTest::nThreads;
	bool ErrorManagementTest::retVal;
	const char* ErrorManagementTest::expectedErrorName;


