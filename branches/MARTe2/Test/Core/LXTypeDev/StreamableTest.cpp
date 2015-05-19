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
#include "StringHelper.h"
#include "StreamTestHelper.h"
#include "DoubleInteger.h"
#include "stdio.h"
#include "StreamWrapperIOBuffer.h"






bool StreamableTest::TestSeek(){
	SimpleStreamable myStream(true);
	uint32 size=64;
	char buffer[64];

	const char *toWrite="HelloWorld.";
	uint32 toWriteSize=11;

	StringHelper::Copy(myStream.buffer, toWrite);
	char output[32];
	uint32 sizeOutput=32;	

	myStream.GetToken(output,".",sizeOutput,0,0); 

			
	
	printf("\n%d\n",myStream.Position());	
	//After write position is 64
	if(myStream.Position()!=size){
		return False;
	}	
	
	myStream.Sync();
	printf("\n%d\n",myStream.Position());
	
	if(myStream.Position()!=toWriteSize){
		return False;
	}
	

	return True;
}






bool StreamableTest::TestPrint(){

	//Create a read&write streamable.
	SimpleStreamable myStream;
	
	const char *tooShort="Hello World";
	const char *string64bit="HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorld";
	
	//Use the Print function.
	myStream.Print("%d",10);
	

	//not flushed
	myStream.Printf("%s",tooShort);
	
	printf("\n%s\n",myStream.buffer);

	if(StringHelper::Compare("", myStream.buffer)!=0){
		return False;
	}

	//Now flush the string is greater than 64 chars.
	myStream.Printf("%s",string64bit);
	
	printf("\n%s\n",myStream.buffer);

	
	char test[65];
	uint32 sizeToCopy=64;
	
	StringHelper::CopyN(test,string64bit,sizeToCopy);
	test[64]=0;

	printf("\n%s\n",test);
	if(StringHelper::Compare(test, myStream.buffer)!=0){
		return False;
	}

	return True;
}		



	
bool StreamableTest::TestToken(){


	SimpleStreamable myStream1;
	SimpleStreamable myStream2;
	const char *inputString="HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHello.Word";
	StreamString myString;
	myString=inputString;
	myString.Seek(0);

	char saveTerminator;

	//Get token from streamstring to streamable
	myString.GetToken(myStream1, ":.", &saveTerminator,0);


	printf("\n%s\n", myStream1.buffer);
	const char* test="HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHello";
	const char* test64bit="HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHell";
	//flushes after 64 chars.
	if(StringHelper::Compare(myStream1.buffer,test64bit)!=0){
		return False;
	}


	//Get token from streamable to streamstring
	myStream1.Clear();
        myString="";	
	StringHelper::Copy(myStream1.buffer, inputString);
	myStream1.GetToken(myString, ":.", &saveTerminator, 0);
	

	if(myString != test){
		return False;
	}

	//Get token from streamable to streamable

	myStream1.Seek(0);
	myStream1.GetToken(myStream2, ":.", &saveTerminator, 0);
	
	//flushes after 64 chars.
	if(StringHelper::Compare(myStream2.buffer,test64bit)!=0){
		return False;
	}
		
	//Get token with a buffer.
	myStream1.Seek(0);
	char outputBuffer[100];
	uint32 outputSize=100;
	for(uint32 i=0; i<outputSize; i++) outputBuffer[i]=0;

	myStream1.GetToken(outputBuffer, ":.",outputSize, &saveTerminator, 0);

	if(StringHelper::Compare(outputBuffer,test)!=0){
		return False;
	}


    return True;


}
