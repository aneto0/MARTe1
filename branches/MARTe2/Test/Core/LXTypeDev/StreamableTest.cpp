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


bool StreamableTest::TestGetC(const char* inputString){
	SimpleStreamable myStream;
	StringHelper::Copy(myStream.buffer, inputString);
	char retC;
	int32 i=0;
	//Unbuffered way
	while(myStream.buffer[i]!=0){
		myStream.GetC(retC);
		printf("\n%c\n", retC);	
		if(retC!=myStream.buffer[i++]){
			return false;
		}
	}
	//Buffered way. it works but gives virtual fault because the reference (in the Flush() function).
	//read 2 bytes at once.
	uint32 buffSize=2;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	i=0;
	myStream.Clear();
	StringHelper::Copy(myStream.buffer, inputString);
	while(myStream.buffer[i]!=0){
		myStream.GetC(retC);
		printf("\n%c\n", retC);	
		if(retC!=myStream.buffer[i++]){
			return false;
		}
	}

	return True;	
}

bool StreamableTest::TestPutC(const char* inputString){
	SimpleStreamable myStream;
	const char* toPut=inputString;

	//Unbuffered way.
	int32 i=0;
	while((*toPut)!=0){
		myStream.PutC(*toPut);
		char retC= myStream.buffer[i++];
		printf("\n%c\n",retC);
		if(retC!=(*toPut)){
			return False;
		}
		toPut++;
	}
	//Buffered way. it works but gives virtual fault because the reference (in the Flush() function).
	//write 2 bytes at once
	uint32 buffSize=2;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	i=0;
	myStream.Clear();
	toPut=inputString;
	while((*toPut)!=0){
		myStream.PutC(*toPut);
		toPut++;
	}

	//Flush the buffer on the stream
	myStream.Flush();
	printf("\n%s\n", myStream.buffer);
	return True;
}


bool StreamableTest::TestRead(const char* inputString){
	//UnBuffered way
	SimpleStreamable myStream;
	char outputBuffer[32];
	StringHelper::Copy(myStream.buffer, inputString);
	uint32 size=StringHelper::Length(inputString)+1;
	myStream.Read(outputBuffer, size);
	printf("\n%s %d\n", outputBuffer, size);

	//Buffered way.
	myStream.Clear();
	uint32 buffSize=2;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	StringHelper::Copy(myStream.buffer, inputString);
	size=12;
	myStream.Read(outputBuffer, size);
	myStream.Flush();
	printf("\n%s\n", outputBuffer);

	return True;
}



bool StreamableTest::TestWrite(const char* inputString){
	//UnBuffered Way
	SimpleStreamable myStream;
	const char *toWrite=inputString;
	uint32 size=StringHelper::Length(inputString)+1;
	myStream.Write(toWrite, size);
	printf("\n%s\n", myStream.buffer);

	//Buffered way
	myStream.Clear();
	uint32 buffSize=2;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	StringHelper::Copy(myStream.buffer, inputString);
	size=12;
	myStream.Write(toWrite, size);
	printf("\n%s\n", myStream.buffer);
	return True;
}

bool StreamableTest::TestReadAndWrite(const char *stringToRead, const char *stringToWrite){
	SimpleStreamable myStream;
	myStream.doubleBuffer=True;
	//Upload the string to write in the stream
	StringHelper::Copy(myStream.buffer, stringToRead);
	uint32 buffSize=4;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	uint32 readSize=StringHelper::Length(stringToRead);
	uint32 writeSize=StringHelper::Length(stringToWrite);

	//output buffer for reading.
	char outputBuffer[32];
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize+1]=0;
	myStream.Write(stringToWrite, writeSize);
	printf("\n%s %s\n", myStream.buffer, outputBuffer);

	myStream.Clear();
	StringHelper::Copy(myStream.buffer, stringToRead);
	
	if(readSize>writeSize){
		buffSize=readSize;
	}
	else{
		buffSize=writeSize;
	}

	buffSize+=10;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	//output buffer for reading.
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize+1]=0;
	myStream.Write(stringToWrite, writeSize);
	printf("\n%s %s\n", myStream.buffer, outputBuffer);
	return True;
}








