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
		if(retC!=myStream.buffer[i++]){
			return false;
		}
	}
	//Buffered way. 
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
		if(retC!=myStream.buffer[i++]){
			return False;
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
		//printf("\n%c\n",retC);
		if(retC!=(*toPut)){
			return False;
		}
		toPut++;
	}


	if(StringHelper::Compare(myStream.buffer, inputString)!=0){
		return False;
	}

	//Buffered way. 
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
	myStream.FlushAndResync();

	if(StringHelper::Compare(myStream.buffer, inputString)!=0){
		return False;
	}

	//printf("\n%s\n", myStream.buffer);
	return True;
}



bool StreamableTest::TestGetCAndPutC(const char *inputString){
	
	SimpleStreamable myStream;
	myStream.doubleBuffer=True;
	
	//Buffered operations.
	uint32 buffSize=StringHelper::Length(inputString);
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}

	StringHelper::Copy(myStream.buffer, inputString);
	
	char c;
	myStream.GetC(c);
	
	if(c!=*inputString){
		return False;
	}

	//useless but is the only with read buffer not empy in tests
	myStream.FlushAndResync();


	myStream.PutC(*inputString);
	
	myStream.FlushAndResync();

	if(myStream.buffer[*(myStream.sizePtr)-1]!=*inputString){
		return False;
	}

	


	return True;
}




bool StreamableTest::TestRead(const char* inputString){
	//UnBuffered way
	SimpleStreamable myStream;
	char outputBuffer[32];
	StringHelper::Copy(myStream.buffer, inputString);
	uint32 size=StringHelper::Length(inputString);
	myStream.Read(outputBuffer, size);
	outputBuffer[size]=0;
	//printf("\n%s %d\n", outputBuffer, size);

	//Buffered way.
	myStream.Clear();

	//The buffer must have a size at least 4 times greater than the size to read	
	
	uint32 buffSize=size*4+1;

	if(!myStream.SetBuffered(buffSize)){
		return False;
	}

	//Read one char at once.
	size=1;

	//Put inputString in the stream
	StringHelper::Copy(myStream.buffer, inputString);
	
	char result[128];
	StringHelper::Copy(result,"");
	uint32 sumSize=0;
	while(sumSize<buffSize){
		myStream.Read(outputBuffer, size);
		outputBuffer[size]=0;
		//Fill the result for each read operation.
		StringHelper::Concatenate(result,outputBuffer);
		sumSize+=size;
	}
	result[sumSize]=0;

	//Compare the result with the stream.
	if(StringHelper::Compare(result, myStream.buffer)!=0){
		//printf("\n%s, %s\n", result, inputString);
		return False;
	}

	//printf("\n%s\n", outputBuffer);

	return True;
}



bool StreamableTest::TestWrite(const char* inputString){
	//UnBuffered Way
	SimpleStreamable myStream;
	myStream.doubleBuffer=True;
	const char *toWrite=inputString;
	uint32 size=StringHelper::Length(inputString);
	myStream.Write(toWrite, size);
	//printf("\n%s\n", myStream.buffer);

	//Buffered way
	myStream.Clear();
	
	//The buffer must have at least a size 4 times greater than the write size.
	uint32 buffSize=size*4+1;
	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	
	//Put the input string in the stream
	StringHelper::Copy(myStream.buffer, inputString);
	
	char result[128];
	
	StringHelper::Copy(result,"");
	
	uint32 sumSize=0;
	while(sumSize<buffSize){
		myStream.Write(toWrite,size);
		//On the stream input string should be added.
		StringHelper::Concatenate(result, inputString);
		sumSize+=size;
		if(myStream.Position()!=sumSize){
			//printf("\n%d %d %d \n",sumSize, myStream.Position(), myStream.UnBufferedPosition());
			return False;
		}
	}
	
	result[sumSize]=0;

	if(myStream.Size()!=sumSize){
		//printf("\n%d %d\n", myStream.Size(), sumSize);
		return False;
	}

	//Compare the expected result with the stream.
	if(StringHelper::Compare(result, myStream.buffer)!=0){
		return False;
	}
	
	//printf("\n%s\n", myStream.buffer);
	return True;
}

bool StreamableTest::TestReadAndWrite(const char *stringToRead, const char *stringToWrite){
	SimpleStreamable myStream;
	myStream.doubleBuffer=True;
	//Upload the string to write in the stream. The buffer size is small so it chooses the unbuffered read/write.
	StringHelper::Copy(myStream.buffer, stringToRead);
	
	uint32 readSize=StringHelper::Length(stringToRead);
	uint32 writeSize=StringHelper::Length(stringToWrite);

	//The size of the buffer is the minor of the two sizes.
	uint32 buffSize=readSize;
	if(readSize>writeSize){
		buffSize=writeSize;
	}

	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	//output buffer for reading.
	char outputBuffer[32];
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;
	
	if(StringHelper::Compare(outputBuffer, myStream.buffer)!=0){
		return False;
	}

	//write on the stream
	myStream.Write(stringToWrite, writeSize);
		
	//This should be on the stream.
	StringHelper::Concatenate(outputBuffer, stringToWrite);

	if(StringHelper::Compare(outputBuffer, myStream.buffer)!=0){
		return False;
	}

	

	//printf("\n%s %s\n", myStream.buffer, outputBuffer);


	//Increase the buffer size to allow buffered read/write.
	SimpleStreamable myNewStream;
	myNewStream.doubleBuffer=true;
	StringHelper::Copy(myNewStream.buffer, stringToRead);
	
	//Buffer size enough large to allow buffered operations.
	buffSize=readSize*4;
	if(writeSize>readSize){
		buffSize=writeSize*4;
	}
	buffSize++;
		
	
	if(!myNewStream.SetBuffered(buffSize)){
		return False;
	}

	//output buffer for reading.
	myNewStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;

	if(StringHelper::Compare(outputBuffer, myNewStream.buffer)!=0){
		return False;
	}
	//printf("\nhere\n");
	myNewStream.Write(stringToWrite, writeSize);

	myNewStream.FlushAndResync();

	StringHelper::Concatenate(outputBuffer, stringToWrite);
		//printf("\n%s %s\n", myNewStream.buffer, outputBuffer);
	if(StringHelper::Compare(outputBuffer, myNewStream.buffer)!=0){
		return False;
	}


	return True;
}




bool StreamableTest::TestSeek(const char *stringToRead, const char *stringToWrite){
	SimpleStreamable myStream;
	
	//canSeek is false so the seek should return false
	if(myStream.Seek(1)){
		return False;
	}
	
	myStream.doubleBuffer=True;
	
	//Upload the string to write in the stream. The buffer size is small so it chooses the unbuffered seek.
	StringHelper::Copy(myStream.buffer, stringToRead);
	uint32 buffSize=StringHelper::Length(stringToRead);

	if(!myStream.SetBuffered(buffSize)){
		return False;
	}
	uint32 readSize=buffSize;

	char outputBuffer[32];

	//Unbuffered read
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;
	//printf("\n%s\n", outputBuffer);
	if(StringHelper::Compare(outputBuffer, myStream.buffer)!=0){
		return False;
	}
	//Unbuffered seek
	myStream.Seek(2);
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;
	//printf("\n%s\n", outputBuffer);

	if(StringHelper::Compare(outputBuffer, myStream.buffer+2)!=0){
		return False;
	}

	//Buffered operations
	myStream.Clear();
	StringHelper::Copy(myStream.buffer, stringToRead);
	uint32 writeSize=StringHelper::Length(stringToWrite);

	buffSize=readSize*4+1;

	if(writeSize>readSize){
		buffSize=writeSize*4+1;
	}

	if(!myStream.SetBuffered(buffSize)){
		return False;
	}

	//Buffered read
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;
	//printf("\n%s\n", outputBuffer);

	//The position falls in the buffer	
	myStream.Seek(2);
	myStream.Read(outputBuffer, readSize);
	if(myStream.Position()!=(readSize+2)){
		//printf("\n%d %d\n",myStream.UnBufferedPosition(), buffSize);
		return False;
	}

	//printf("\n%s\n", outputBuffer);

	//Buffered write
	myStream.Write(stringToWrite, writeSize);
	myStream.Seek(readSize);

	if(myStream.Position()!=readSize){
		return False;
	}

	
	//Buffered operations
	myStream.Clear();
	StringHelper::Copy(myStream.buffer, stringToRead);
	
	//initial seek	
	myStream.Seek(3);
	

	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;

	if(StringHelper::Compare(outputBuffer, stringToRead+3)!=0){
		return False;
	}

	//printf("\n%s\n", outputBuffer);
	
	//Seek not in the range of the buffer.
	myStream.Seek(2);
	myStream.Read(outputBuffer, readSize);
	outputBuffer[readSize]=0;

	if(StringHelper::Compare(outputBuffer, stringToRead+2)!=0){
		return False;
	}
	//printf("\n%s\n", outputBuffer);

	


	//Relative seek

	//Append the write string.
	myStream.Seek(readSize);
	myStream.Write(stringToWrite, writeSize);


	//The write buffer is still full, then before the seek it should be flushed.
	myStream.RelativeSeek(-readSize-writeSize);

	//Return at the beginning
	if(myStream.Position()!=0){
		return False;
	}

	//Return at readSize.
	myStream.Seek(readSize);
	
	//Read buffered operation.
	myStream.Read(outputBuffer, writeSize);
	outputBuffer[writeSize]=0;
	
	if(StringHelper::Compare(outputBuffer, stringToWrite)!=0){
		return False;
	}

	//The read buffer is still full, the position falls in the buffer.
	myStream.RelativeSeek(-22);
	
	if(myStream.Position()!=readSize){
		return False;
	}

	//The position falls out of the buffer.
	myStream.RelativeSeek(writeSize);
	uint32 dummySize=1;
	myStream.Read(outputBuffer,dummySize);
	outputBuffer[dummySize]=0;
	if(*outputBuffer!=0){
		return False;
	}

	myStream.RelativeSeek(-writeSize-2);
	if(myStream.Position()!=(readSize-1)){
		//printf("\n%d %d\n", readSize-1, myStream.Position());
		return False;
	}

	
	return True;
}



bool StreamableTest::TestSwitch(const char* onStream1, const char* onStream2){
	SimpleStreamable myStream;

	StringHelper::Copy(myStream.buffer,onStream1);

	myStream.Switch(2);
	StringHelper::Copy(myStream.buffer,onStream2);

	const char a='1';
	myStream.Switch(&a);
	//We have two streams.
	if(myStream.NOfStreams()!=2){
		return False;
	}

	char outputBuffer[32];
	uint32 readSize = StringHelper::Length(onStream1);
	myStream.Read(outputBuffer,readSize);
	outputBuffer[readSize]=0;
	//printf("\n%s\n", outputBuffer);

	const char b='2';
	myStream.Switch(&b);
	readSize=StringHelper::Length(onStream2);
	myStream.Read(outputBuffer,readSize);
	outputBuffer[readSize]=0;
	//printf("\n%s\n",outputBuffer);
	
	myStream.RemoveStream(&b);

	myStream.Seek(0);
	myStream.Read(outputBuffer,readSize);
	outputBuffer[readSize]=0;
//	printf("\n%s\n", outputBuffer);

	//the seclected stream is the first
	if(myStream.SelectedStream()!=1){
		return False;
	}

	//set the size, but is false if !canSeek
	if(myStream.SetSize(1)){
		return False;
	}
	
	//enable canSeek
	myStream.doubleBuffer=True;
	uint32 fakeSize=2;
	myStream.SetBuffered(fakeSize);

	//now it is true
	if(!myStream.SetSize(1)){
		return False;
	}


	return True;
}




		



	
	
		









