#include "ErrorManagementTest.h"

void PrintErrorManagement(ErrorInformation& errorInfo, const char* description){
	ErrorManagementTest newEM;
	newEM.CheckParameters(errorInfo, description);
	
}

	ErrorType ErrorManagementTest::errCode;
	const char* ErrorManagementTest::errDescription;
	const char* ErrorManagementTest::errFilename;
	uint32 ErrorManagementTest::errLine;
	const char* ErrorManagementTest::errFunction;
	uint32 ErrorManagementTest::nThreads;
	bool ErrorManagementTest::retVal;
	const char* ErrorManagementTest::expectedErrorName;


