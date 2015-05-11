/* Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they
 will be approved by the European Commission - subsequent
 versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 Licence.
 * You may obtain a copy of the Licence at:
 *
 * http: //ec.europa.eu/idabc/eupl
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

#include "GeneralDefinitions.h"
#include "StreamableTest.h"
#include "StringTestHelper.h"
#include "StreamTestHelper.h"
#include "StringHelper.h"
#include "stdio.h"


bool StreamableTest::TestGetC(){
	SimpleStreamable myStream;
	StringHelper::Copy(myStream.buffer, "Hello World");
	char retC;
	while(myStream.buffer[myStream.size]!=0){
		myStream.GetC(retC);
		printf("\n%c\n", retC);	
		if(retC!=myStream.buffer[myStream.size-1]){
			return false;
		}
	}
	return True;	
}

bool StreamableTest::TestPutC(){
	SimpleStreamable myStream;
	const char* toPut="Hello World";
	while((*toPut)!=0){
		myStream.PutC(*toPut);
		char retC= myStream.buffer[myStream.size-1];
		printf("\n%c\n",retC);
		if(retC!=(*toPut)){
			return False;
		}
		toPut++;
	}

	return True;
}

