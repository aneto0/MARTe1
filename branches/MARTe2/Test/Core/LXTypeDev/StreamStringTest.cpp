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
#include "StreamStringTest.h"
#include "StringHelper.h"
#include "StreamTestHelper.h"
#include "DoubleInteger.h"
#include "stdio.h"

static void cleanOutputBuffer(char* buffer,int32 size){
	for(int32 i=0; i<size; i++){
		buffer[i]=0;
	}
}




bool StreamStringTest::TestGetC(const char* inputString){
	StreamString myString;

	//The buffer pointer is at the end!
	myString=inputString;

	//To read from the beginning a seek is necessary.
	myString.Seek(0);

	char retC;
	int32 i=0;
	while(inputString[i]!=0){
		myString.GetC(retC);
		if(retC!=inputString[i++]){
			return False;
		}
	}

	return True;	
}

bool StreamStringTest::TestPutC(const char* inputString){
	StreamString myString;
	const char* toPut=inputString;

	//Put all inputString on the StreamString
	while((*toPut)!=0){
		myString.PutC(*toPut);
		toPut++;
	}

	myString.Seek(0);

	//Check the result.	
	if(myString!=inputString){
		return False;
	}

	return True;
}



bool StreamStringTest::TestRead(const char* inputString){
	//UnBuffered way
	StreamString myString;
	myString=inputString;

	myString.Seek(0);
	
	char outputBuffer[32];
	cleanOutputBuffer(outputBuffer,32);


	//size equal to inputString length (+1 for the terminated char).	
	uint32 size=StringHelper::Length(inputString);
	myString.Read(outputBuffer, size);


	//Use Buffer() for comparison.
	if(StringHelper::Compare(myString.Buffer(), outputBuffer)){
		printf("\n%s %s\n", outputBuffer, myString.Buffer());
		return False;
	}
	

	cleanOutputBuffer(outputBuffer,32);
	//size greater than inputString length.	
	size+=10;
	

	//return at the beginning
	myString.Seek(0);
	myString.Read(outputBuffer, size);
	myString.Seek(0);

	if(myString!=outputBuffer){
		printf("\n%s %s\n", outputBuffer, myString.Buffer());
		return False;
	}


	//size minor than inputString length.	
	size=StringHelper::Length(inputString)-1;
	if(size<=0){
		size++;
	}

	//Create a test string clipping the string at size.
	outputBuffer[size]=0;
	char test[32];
	
	StringHelper::Copy(test, outputBuffer);	

	//clean the output buffer.
	cleanOutputBuffer(outputBuffer,32);

	myString.Seek(0);
	myString.Read(outputBuffer, size);

	if(StringHelper::Compare(test, outputBuffer)!=0){
		printf("\n%s %s\n", outputBuffer, test);
		return False;
	}


	return True;
}



bool StreamStringTest::TestWrite(const char* inputString){
	//UnBuffered Way
	StreamString myString;
	char test[128];
	StringHelper::Copy(test, inputString);

	//size equal to the inputString length	
	uint32 size=StringHelper::Length(inputString);	
	
	myString.Write(inputString, size);

	if(StringHelper::Compare(test, myString.Buffer())!=0){
		return False;
	}
	
	
	//size greater than the inputString length
	size+=3;
	

	//create a buffer to avoid possible segfault.
	char inputBuffer[32];
	cleanOutputBuffer(inputBuffer,32);
	StringHelper::Copy(inputBuffer, inputString);
	StringHelper::Concatenate(test, inputString);
	printf("\n%s\n", test);


	myString.Write(inputBuffer, size);	

	printf("\n%s\n", myString.Buffer());

	if(StringHelper::Compare(test, myString.Buffer())!=0){
		return False;
	}

	//size minor than the inputString length
	size=StringHelper::Length(inputString)-1;
	if(size<=0){
		size++;
	}

	StringHelper::ConcatenateN(test, inputString, size);
	printf("\n%s\n", test);

	//padding zeros because of the precedent operation, adjust seek.
	myString.RelativeSeek(-3);
	myString.Write(inputString, size);
	
	if(StringHelper::Compare(test, myString.Buffer())!=0){
		return False;
	}

	return True;
}





bool StreamStringTest::TestSeek(const char *inputString){
	StreamString myString;
	myString=inputString;
	
	//Test position
	uint32 size= StringHelper::Length(inputString);
	if(myString.Position()!=size){
		return False;
	}

	//seek to zero
	if(!myString.Seek(0)){
		return False;
	}

	if(StringHelper::Compare(inputString, myString.Buffer())!=0){
		return False;
	}

	//seek greater than the dimension
	if(myString.Seek(size+10)){
		return False;
	}

	if(myString.Position()!=size){
		return False;
	}

	//relative seek
	if(!myString.RelativeSeek(-1)){
		return False;
	}

	char outputBuffer[32];
	uint32 dummySize=1;
	myString.Read(outputBuffer,dummySize);

	//read a char. It increments the position.
	if(myString.Position()!=size || (*outputBuffer)!=inputString[size-1] ){
		return False;
	}

	//position under 0
	if(myString.RelativeSeek(-size-1)){
		return False;
	}

	if(StringHelper::Compare(inputString, myString.Buffer())!=0){
		return False;
	}

	//position over the size
	if(myString.RelativeSeek(size+10)){
		return False;
	}

	if(myString.Position()!=size){
		return False;
	}

	return True;
}



bool StreamStringTest::TestOperators(const char* firstString, const char* secondString){
	StreamString myString1;
	StreamString myString2;
	
	//string parameters must be different each other


	//Assignment operator
	myString1=firstString;

	//return at the beginning.
	myString1.Seek(0);
	
	//Is different operator.
	if(myString1!=firstString || !(myString1==firstString) || myString1==secondString){
		return False;
	}

	char* s=NULL;
	
	//Null assignment return false	
	if(myString1=s){
		return False;
	}

	if(myString1==s){
		return False;
	}

	//Different sizes.
	myString2=secondString;
	if(myString1 == myString2){
		return False;
	}

	myString2=firstString;
	myString1.BufferReference()[0]='a';

	//Same sizes but different strings
	if(myString1 == myString2){
		return False;
	}

	//Another assignment
	myString1=secondString;
	
	myString1.Seek(0);	
	//Is equal operator
	if(myString1==firstString || myString1!=secondString){
		return False;
	}
	
	//Same operators between two StreamStrings
	myString1=firstString;	
	myString2=myString1;

	myString1.Seek(0);
	myString1.Seek(0);
	if(myString1!=myString2 || !(myString1==myString2)){
		return False;
	}

	//Append operation
	myString1=secondString;
	myString1+=firstString;
	char test[100];
	
	StringHelper::Copy(test, secondString);
	StringHelper::Concatenate(test, firstString);

	myString1.Seek(0);
	if(myString1!=test){
		return False;
	}

	//Append another StreamString
	myString2=firstString;
	
	//Try to make it fail with a seek.
	myString1.Seek(0);

	myString1+=myString2;
	StringHelper::Concatenate(test, firstString);
	
	myString1.Seek(0);
	if(myString1!=test){
		return False;
	}
	
	//Use char
	char c='c';
	myString2=c;
	myString2+=c;
	if(myString2!="cc"){
		return False;
	}
	
	//[] operator

	myString1=secondString;
	if(secondString[1]!=myString1[1]){
		return False;
	}

	//Should return 0 if the begin is greater than size.
	uint32 size=StringHelper::Length(secondString);
	if(myString1[size+1]!=0){
		return False;
	}


	//Size function
	if(myString1.Size()!=size){
		return False;
	}



	return True;
}



bool StreamStringTest::TestUseless(){
	StreamString myString;
	StreamStringIOBuffer myStringBuffer;
	myString="Hello";
	uint32 dummy=1;

	//Resync return true
	if(!myStringBuffer.Resync()){
		return False;
	}

	//Refill return true
	if(!myStringBuffer.Refill()){
		return False;
	}

	//Flush allocate again the minimum granularity defined and 
        //return true in case of success.
	if(!myStringBuffer.Flush()){
		return False;
	}

	if(myString.NOfStreams()!=0){
		return False;
	}

	if(myString.Switch("a") || myString.Switch(dummy)){
		return False;
	}	

	if(myString.SelectedStream()!=0){
		return False;
	}
	
	if(myString.StreamName(dummy, NULL, 1)){
		return False;
	}

	if(myString.AddStream(NULL)){
		return False;
	}

	if(myString.RemoveStream(NULL)){
		return False;
	}
	
	if(!myString.CanSeek() || !myString.CanRead() || !myString.CanWrite()){
		return False;
	}


	if(!myString.SetSize(dummy)){
		return True;
	}

	return True;
}


bool StreamStringTest::TestPrint(){

	//Create a read&write streamable.
	StreamString myString;

	//make writable the string.
	myString="a";
	myString.Seek(0);

	uint8 ubit8=1;
	uint16 ubit16=2;
	uint32 ubit32=3;
	uint64 ubit64=4;
	DoubleInteger<uint64> ubit128Tmp=5;	
	AnyType ubit128;
	ubit128.dataPointer=&ubit128Tmp;
	ubit128.dataDescriptor={False,False,{{TypeDescriptor::UnsignedInteger, 128}}};
	ubit128.bitAddress=0;


	int8 sbit8=-1;
 	int16 sbit16=-2;
	int32 sbit32=-3;
	int64 sbit64=-4;


	DoubleInteger<int64> sbit128Tmp;
	sbit128Tmp=-5;
	AnyType sbit128;
	sbit128.dataPointer=&sbit128Tmp;
	sbit128.dataDescriptor={False,False,{{TypeDescriptor::SignedInteger, 128}}};
	sbit128.bitAddress=0;
	float fbit32=1.2;
	double dbit64=3.4;
	__float128 fbit128Tmp=5.6Q;
	AnyType fbit128;
	fbit128.dataPointer=&fbit128Tmp;
	fbit128.dataDescriptor={False,False,{{TypeDescriptor::Float, 128}}};
	fbit128.bitAddress=0;






	//Use the unbuffered PutC, 4 parameters.	
	//For integer the letter is useless
	myString.Printf("% 3u % 3f % 3d % 3x\n", sbit8, fbit32, sbit16, sbit8);



	//Use the unbuffered PutC, 4 parameters.	
	//For integer the letter is useless
	myString.Printf("% 3u % 3f % 3d % 3x\n", ubit64, fbit32, ubit16, sbit8);

	//Use the unbuffered PutC, 3 parameters.
	myString.Printf("% 3u % 3c % 3d\n", sbit64, ubit32, fbit32);

	//Use the unbuffered PutC, 3 parameters.
	myString.Printf("% 3u % 3c % 3d\n", sbit128, ubit128, fbit32);

	//Use the unbuffered PutC, 2 parameters.
	myString.Printf("% 3c % 3F\n", ubit8, dbit64);
	//Use the unbuffered PutC, 2 parameters.
	myString.Printf("% 3c % 3F\n", ubit8, dbit64);


	//Use the unbuffered PutC, 1 parameter.
	myString.Printf("% 3o\n", sbit32);
	printf("\n%s\n", myString.Buffer());

	if(StringHelper::Compare(" -1 1.2  -2  FF\n  4 1.2   2  FF\n -4   3 1.2\n  0   5 1.2\n  1 3.4\n  1 3.4\n  ?\n", myString.Buffer())!=0){
		return False;
	}
	//Unsupported 128 float numbers
	if(myString.Printf("%f", fbit128)){
		return False;
	}

	//structured data anytype
	sbit128.dataDescriptor.isStructuredData=true;
	if(myString.Printf("% 3d", sbit128)){
		return False;
	}
	
	//wrong type in the format
	if(myString.Printf("%3l",fbit32)){
		return False;
	}
	
	return True;
}		



	
bool StreamStringTest::TestToken(){

	const char* inputString="Nome::: Giuseppe. Cognome: Ferrò:)";
	uint32 size=StringHelper::Length(inputString);

	StreamString myString;
	myString=inputString;	


	char buffer[32];
	char saveTerminator;


	myString.Seek(0);

    //TESTS ON TOKEN FUNCTIONS BETWEEN A STREAM AND A STRING
    myString.GetToken(buffer, ".:", size, &saveTerminator, NULL);
    if (StringHelper::Compare(buffer, "Nome")!=0 || saveTerminator != ':') {
        return False;
    }

    //Without skip chars, the function skips consecutive terminators.
    myString.GetToken(buffer, ".:", size, &saveTerminator, NULL);
    if (StringHelper::Compare(buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }

    //return before the :::
    myString.Seek(4);
    
    //The function skips correctly the second ":"
    myString.GetToken(buffer, ".:", size, &saveTerminator, ":");
    if (StringHelper::Compare( buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }

    //The function does not skips because the terminator character is not correct
    myString.Seek(4);

    myString.GetToken(buffer, ".:", size, &saveTerminator, ".");
    if (StringHelper::Compare(buffer, "")!=0 || saveTerminator != ':') {
        return False;
    }

    //Test with a minor maximum size passed by argument. The terminated char is calculated in the size
    myString.Seek(4);  

    size = 4;
    myString.GetToken(buffer, ".:", size, &saveTerminator, ":");
    if (StringHelper::Compare( buffer,  " Gi")!=0 || saveTerminator != 'i') {
        return False;
    }
    size = StringHelper::Length(inputString);

	

    //Arrive to the next terminator
    myString.GetToken(buffer, ".:", size, &saveTerminator, ":");


    //Test if the functions terminate when the string is terminated
    for (uint32 i = 0; i < 3; i++) {
        myString.GetToken(buffer, ".:", size, &saveTerminator, ":");
    }
    if (StringHelper::Compare(buffer, ")")!=0) {
        return False;
    }

	myString.Seek(0);    

    myString.SkipTokens(3, ":.");
    myString.GetToken(buffer, ")", size, &saveTerminator, NULL);
    if (StringHelper::Compare(buffer, " Ferrò:")!=0) {
        return False;
    }
 


//TESTS ON TOKEN FUNCTIONS BETWEEN A STRING AND STREAM

	myString.Seek(0);

    SimpleStreamable outputStream;
    outputStream.doubleBuffer=True;

    uint32 buffOutputSize=1;
    if(!outputStream.SetBuffered(buffOutputSize)){
	return False;
    }	
    

    myString.GetToken(outputStream, ".:", &saveTerminator, NULL);
    outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, "Nome")!=0 || saveTerminator != ':') {
        return False;
    }
    outputStream.Clear();

    //Without skip chars, the function skips consecutive terminators.
    myString.GetToken(outputStream, ".:", &saveTerminator, NULL);
	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }
	outputStream.Clear();
    //return before the :::
    myString.Seek(4);
    
    //The function skips correctly the second ":"
    myString.GetToken(outputStream, ".:", &saveTerminator, ":");
	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }
	outputStream.Clear();
    //The function does not skips because the terminator character is not correct
    myString.Seek(4);

    myString.GetToken(outputStream, ".:", &saveTerminator, ".");
 	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, "")!=0 || saveTerminator != ':') {
        return False;
    }



    //Test if the functions terminate when the string is terminated
    for (uint32 i = 0; i < 3; i++) {
        myString.GetToken(outputStream, ".:",  &saveTerminator, ":");
    }

	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, " Giuseppe Cognome Ferrò")!=0) {
        return False;
    }


    return True;


}




