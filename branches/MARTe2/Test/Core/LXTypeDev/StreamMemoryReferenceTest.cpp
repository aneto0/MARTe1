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
#include "StreamMemoryReferenceTest.h"
#include "StringHelper.h"
#include "StreamTestHelper.h"
#include "DoubleInteger.h"
#include "stdio.h"



void cleanBuffer(char* buffer,int32 size){
	for(int32 i=0; i<size; i++){
		buffer[i]=0;
	}
}


bool StreamMemoryReferenceTest::TestGetC(){
	char buffer[10];
	StreamMemoryReference mBReference(buffer, 10);
	const char* inputString="Hello World";
	uint32 size=StringHelper::Length(inputString);	

	mBReference.Write(inputString,size);
	mBReference.Seek(0);

	char retC;

	for(int32 i=0; i<10; i++){
		mBReference.GetC(retC);
		if(retC!=inputString[i]){
			return False;
		}
	}

	//out of range, retC remains the same!
	mBReference.GetC(retC);

	if(retC!='l'){
		return False;
	}

	return True;	
}

bool StreamMemoryReferenceTest::TestPutC(){

	char buffer[10];	
	StreamMemoryReference mBReference(buffer,10);
	const char* toPut="Hello World";

	//Put all inputString on the StreamMemoryReference
	for(int32 i=0; i<10; i++){
		mBReference.PutC(*toPut);
		toPut++;
	}

	//Size finished	
	mBReference.PutC(*toPut);

	mBReference.Seek(0);

	//Check the result.	
	if(StringHelper::Compare(mBReference.Buffer(),"Hello Worl")!=0){
		return False;
	}

	return True;
}



bool StreamMemoryReferenceTest::TestRead(){


	char buffer[10];
	StreamMemoryReference mBReference(buffer,10);


	const char* inputString="Hello World";

	//Write only 7
	uint32 tempSize=7;
	mBReference.Write(inputString,tempSize);
	
	mBReference.Seek(0);
	char outputBuffer[32];
	cleanBuffer(outputBuffer,32);

	tempSize=32;
	mBReference.Read(outputBuffer,tempSize);

	if(StringHelper::Compare(outputBuffer,"Hello W")!=0){
		return False;
	}

	cleanBuffer(outputBuffer,32);

	//reWrite
	tempSize=32;
	mBReference.Seek(0);
	mBReference.Write(inputString, tempSize);

	//reRead
	tempSize=32;
	mBReference.Seek(0);
	mBReference.Read(outputBuffer,tempSize);

	//read all.
	mBReference.Read(outputBuffer,tempSize);
	if(StringHelper::Compare(outputBuffer, "Hello Worl")!=0){
		return False;
	}


	return True;
}

/*

bool StreamMemoryReferenceTest::TestWrite(const char* inputString){
	//UnBuffered Way
	StreamMemoryReference mBReference;
	char test[128];
	StringHelper::Copy(test, inputString);

	//size equal to the inputString length	
	uint32 size=StringHelper::Length(inputString);	
	
	mBReference.Write(inputString, size);

	if(StringHelper::Compare(test, mBReference.Buffer())!=0){
		return False;
	}
	
	
	//size greater than the inputString length
	size+=3;
	

	//create a buffer to avoid possible segfault.
	char inputBuffer[32];
	cleanBuffer(inputBuffer,32);
	StringHelper::Copy(inputBuffer, inputString);
	StringHelper::Concatenate(test, inputString);
	printf("\n%s\n", test);


	mBReference.Write(inputBuffer, size);	

	printf("\n%s\n", mBReference.Buffer());

	if(StringHelper::Compare(test, mBReference.Buffer())!=0){
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
	mBReference.RelativeSeek(-3);
	mBReference.Write(inputString, size);
	
	if(StringHelper::Compare(test, mBReference.Buffer())!=0){
		return False;
	}

	return True;
}





bool StreamMemoryReferenceTest::TestSeek(const char *inputString){
	StreamMemoryReference mBReference;
	mBReference=inputString;
	
	//Test position
	uint32 size= StringHelper::Length(inputString);
	if(mBReference.Position()!=size){
		return False;
	}

	//seek to zero
	if(!mBReference.Seek(0)){
		return False;
	}

	if(StringHelper::Compare(inputString, mBReference.Buffer())!=0){
		return False;
	}

	//seek greater than the dimension
	if(mBReference.Seek(size+10)){
		return False;
	}

	if(mBReference.Position()!=size){
		return False;
	}

	//relative seek
	if(!mBReference.RelativeSeek(-1)){
		return False;
	}

	char outputBuffer[32];
	uint32 dummySize=1;
	mBReference.Read(outputBuffer,dummySize);

	//read a char. It increments the position.
	if(mBReference.Position()!=size || (*outputBuffer)!=inputString[size-1] ){
		return False;
	}

	//position under 0
	if(mBReference.RelativeSeek(-size-1)){
		return False;
	}

	if(StringHelper::Compare(inputString, mBReference.Buffer())!=0){
		return False;
	}

	//position over the size
	if(mBReference.RelativeSeek(size+10)){
		return False;
	}

	if(mBReference.Position()!=size){
		return False;
	}

	return True;
}



bool StreamMemoryReferenceTest::TestOperators(const char* firstString, const char* secondString){
	StreamMemoryReference mBReference1;
	StreamMemoryReference mBReference2;
	
	//string parameters must be different each other


	//Assignment operator
	mBReference1=firstString;

	//return at the beginning.
	mBReference1.Seek(0);
	
	//Is different operator.
	if(mBReference1!=firstString || !(mBReference1==firstString) || mBReference1==secondString){
		return False;
	}

	char* s=NULL;
	
	//Null assignment return false	
	if(mBReference1=s){
		return False;
	}

	if(mBReference1==s){
		return False;
	}

	//Different sizes.
	mBReference2=secondString;
	if(mBReference1 == mBReference2){
		return False;
	}

	mBReference2=firstString;
	mBReference1.BufferReference()[0]='a';

	//Same sizes but different strings
	if(mBReference1 == mBReference2){
		return False;
	}

	//Another assignment
	mBReference1=secondString;
	
	mBReference1.Seek(0);	
	//Is equal operator
	if(mBReference1==firstString || mBReference1!=secondString){
		return False;
	}
	
	//Same operators between two StreamMemoryReferences
	mBReference1=firstString;	
	mBReference2=mBReference1;

	mBReference1.Seek(0);
	mBReference1.Seek(0);
	if(mBReference1!=mBReference2 || !(mBReference1==mBReference2)){
		return False;
	}

	//Append operation
	mBReference1=secondString;
	mBReference1+=firstString;
	char test[100];
	
	StringHelper::Copy(test, secondString);
	StringHelper::Concatenate(test, firstString);

	mBReference1.Seek(0);
	if(mBReference1!=test){
		return False;
	}

	//Append another StreamMemoryReference
	mBReference2=firstString;
	
	//Try to make it fail with a seek.
	mBReference1.Seek(0);

	mBReference1+=mBReference2;
	StringHelper::Concatenate(test, firstString);
	
	mBReference1.Seek(0);
	if(mBReference1!=test){
		return False;
	}
	
	//Use char
	char c='c';
	mBReference2=c;
	mBReference2+=c;
	if(mBReference2!="cc"){
		return False;
	}
	
	//[] operator

	mBReference1=secondString;
	if(secondString[1]!=mBReference1[1]){
		return False;
	}

	//Should return 0 if the begin is greater than size.
	uint32 size=StringHelper::Length(secondString);
	if(mBReference1[size+1]!=0){
		return False;
	}


	//Size function
	if(mBReference1.Size()!=size){
		return False;
	}



	return True;
}



bool StreamMemoryReferenceTest::TestUseless(){
	StreamMemoryReference mBReference;
	StreamMemoryReferenceIOBuffer mBReferenceBuffer;
	mBReference="Hello";
	uint32 dummy=1;

	//Resync return true
	if(!mBReferenceBuffer.Resync()){
		return False;
	}

	//Refill return true
	if(!mBReferenceBuffer.Refill()){
		return False;
	}

	//Flush allocate again the minimum granularity defined and 
        //return true in case of success.
	if(!mBReferenceBuffer.Flush()){
		return False;
	}

	if(mBReference.NOfStreams()!=0){
		return False;
	}

	if(mBReference.Switch("a") || mBReference.Switch(dummy)){
		return False;
	}	

	if(mBReference.SelectedStream()!=0){
		return False;
	}
	
	if(mBReference.StreamName(dummy, NULL, 1)){
		return False;
	}

	if(mBReference.AddStream(NULL)){
		return False;
	}

	if(mBReference.RemoveStream(NULL)){
		return False;
	}
	
	if(!mBReference.CanSeek() || !mBReference.CanRead() || !mBReference.CanWrite()){
		return False;
	}


	if(!mBReference.SetSize(dummy)){
		return True;
	}

	return True;
}


bool StreamMemoryReferenceTest::TestPrint(){

	//Create a read&write streamable.
	StreamMemoryReference mBReference;

	//make writable the string.
	mBReference="a";
	mBReference.Seek(0);

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
	mBReference.Printf("% 3u % 3f % 3d % 3x\n", sbit8, fbit32, sbit16, sbit8);



	//Use the unbuffered PutC, 4 parameters.	
	//For integer the letter is useless
	mBReference.Printf("% 3u % 3f % 3d % 3x\n", ubit64, fbit32, ubit16, sbit8);

	//Use the unbuffered PutC, 3 parameters.
	mBReference.Printf("% 3u % 3c % 3d\n", sbit64, ubit32, fbit32);

	//Use the unbuffered PutC, 3 parameters.
	mBReference.Printf("% 3u % 3c % 3d\n", sbit128, ubit128, fbit32);

	//Use the unbuffered PutC, 2 parameters.
	mBReference.Printf("% 3c % 3F\n", ubit8, dbit64);
	//Use the unbuffered PutC, 2 parameters.
	mBReference.Printf("% 3c % 3F\n", ubit8, dbit64);


	//Use the unbuffered PutC, 1 parameter.
	mBReference.Printf("% 3o\n", sbit32);
	printf("\n%s\n", mBReference.Buffer());

	if(StringHelper::Compare(" -1 1.2  -2  FF\n  4 1.2   2  FF\n -4   3 1.2\n  0   5 1.2\n  1 3.4\n  1 3.4\n  ?\n", mBReference.Buffer())!=0){
		return False;
	}
	//Unsupported 128 float numbers
	if(mBReference.Printf("%f", fbit128)){
		return False;
	}

	//structured data anytype
	sbit128.dataDescriptor.isStructuredData=true;
	if(mBReference.Printf("% 3d", sbit128)){
		return False;
	}
	
	//wrong type in the format
	if(mBReference.Printf("%3l",fbit32)){
		return False;
	}
	
	return True;
}		



	
bool StreamMemoryReferenceTest::TestToken(){

	const char* inputString="Nome::: Giuseppe. Cognome: Ferrò:)";
	uint32 size=StringHelper::Length(inputString);

	StreamMemoryReference mBReference;
	mBReference=inputString;	


	char buffer[32];
	char saveTerminator;


	mBReference.Seek(0);

    //TESTS ON TOKEN FUNCTIONS BETWEEN A STREAM AND A STRING
    mBReference.GetToken(buffer, ".:", size, &saveTerminator, NULL);
    if (StringHelper::Compare(buffer, "Nome")!=0 || saveTerminator != ':') {
        return False;
    }

    //Without skip chars, the function skips consecutive terminators.
    mBReference.GetToken(buffer, ".:", size, &saveTerminator, NULL);
    if (StringHelper::Compare(buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }

    //return before the :::
    mBReference.Seek(4);
    
    //The function skips correctly the second ":"
    mBReference.GetToken(buffer, ".:", size, &saveTerminator, ":");
    if (StringHelper::Compare( buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }

    //The function does not skips because the terminator character is not correct
    mBReference.Seek(4);

    mBReference.GetToken(buffer, ".:", size, &saveTerminator, ".");
    if (StringHelper::Compare(buffer, "")!=0 || saveTerminator != ':') {
        return False;
    }

    //Test with a minor maximum size passed by argument. The terminated char is calculated in the size
    mBReference.Seek(4);  

    size = 4;
    mBReference.GetToken(buffer, ".:", size, &saveTerminator, ":");
    if (StringHelper::Compare( buffer,  " Gi")!=0 || saveTerminator != 'i') {
        return False;
    }
    size = StringHelper::Length(inputString);

	

    //Arrive to the next terminator
    mBReference.GetToken(buffer, ".:", size, &saveTerminator, ":");


    //Test if the functions terminate when the string is terminated
    for (uint32 i = 0; i < 3; i++) {
        mBReference.GetToken(buffer, ".:", size, &saveTerminator, ":");
    }
    if (StringHelper::Compare(buffer, ")")!=0) {
        return False;
    }

	mBReference.Seek(0);    

    mBReference.SkipTokens(3, ":.");
    mBReference.GetToken(buffer, ")", size, &saveTerminator, NULL);
    if (StringHelper::Compare(buffer, " Ferrò:")!=0) {
        return False;
    }
 


//TESTS ON TOKEN FUNCTIONS BETWEEN A STRING AND STREAM

	mBReference.Seek(0);

    SimpleStreamable outputStream;
    outputStream.doubleBuffer=True;

    uint32 buffOutputSize=1;
    if(!outputStream.SetBuffered(buffOutputSize)){
	return False;
    }	
    

    mBReference.GetToken(outputStream, ".:", &saveTerminator, NULL);
    outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, "Nome")!=0 || saveTerminator != ':') {
        return False;
    }
    outputStream.Clear();

    //Without skip chars, the function skips consecutive terminators.
    mBReference.GetToken(outputStream, ".:", &saveTerminator, NULL);
	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }
	outputStream.Clear();
    //return before the :::
    mBReference.Seek(4);
    
    //The function skips correctly the second ":"
    mBReference.GetToken(outputStream, ".:", &saveTerminator, ":");
	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, " Giuseppe")!=0
            || saveTerminator != '.') {
        return False;
    }
	outputStream.Clear();
    //The function does not skips because the terminator character is not correct
    mBReference.Seek(4);

    mBReference.GetToken(outputStream, ".:", &saveTerminator, ".");
 	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, "")!=0 || saveTerminator != ':') {
        return False;
    }



    //Test if the functions terminate when the string is terminated
    for (uint32 i = 0; i < 3; i++) {
        mBReference.GetToken(outputStream, ".:",  &saveTerminator, ":");
    }

	outputStream.Seek(0);
    if (StringHelper::Compare(outputStream.buffer, " Giuseppe Cognome Ferrò")!=0) {
        return False;
    }


    return True;


}



*/
