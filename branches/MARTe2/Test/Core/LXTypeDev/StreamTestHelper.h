
#include "StreamString.h"
#include "BufferedStream.h"
#include "Memory.h"
#include "StreamWrapperIOBuffer.h"
#define MAX_DIMENSION 128
#define MAX_STREAM_DIMENSION 256






//A generic stream class used for types print.
class MyStream : public StreamString{ 
private:
/*	char oBuffer[MAX_DIMENSION];
	int32 size;*/
public:
	IOBuffer *GetMyOutputBuffer(){
		return GetOutputBuffer();
	}

	bool Clear(){
		StreamString* me=this;
		return (*me)="";
	}

};






//A simple stream for streamable which implement unbuffered functions.
class SimpleBufferedStream : public BufferedStream{
public:
	char buffer1[MAX_STREAM_DIMENSION];
	char buffer2[MAX_STREAM_DIMENSION];
	char* buffer;
	uint32 size1;
	uint32 size2;
	uint32 *sizePtr;
	uint32 nBuffers;
	bool doubleBuffer;
public:


	SimpleBufferedStream(){
		size1=0;
		size2=0;
		nBuffers=2;
		buffer=buffer1;
		sizePtr=&size1;
		doubleBuffer=false;
		for(int i=0; i<MAX_DIMENSION; i++) buffer[i]=0;
	}

	~SimpleBufferedStream(){
		FlushAndResync();
	}
	

	void Clear(){
		(*sizePtr)=0;
		for(int i=0; i<MAX_DIMENSION; i++) buffer[i]=0;
	}

	//Copy buffer in inBuffer for the read from stream.
	bool UnBufferedRead(char* inBuffer, uint32 &inSize, TimeoutType timeout=TTDefault, bool complete=false){
	
		uint32 size=*sizePtr;
		if((size+inSize) >= MAX_DIMENSION){
			size=0;
		}
	
		if(!MemoryCopy(inBuffer, &buffer[size], inSize)){
			return false;
		}
	
		size+=inSize;
		*sizePtr=size;
		return true;

	}		

	//Copy outBuffer in buffer for the write to stream.
	bool UnBufferedWrite(const char* outBuffer, uint32 &outSize, TimeoutType timeout=TTDefault, bool complete=false){

		uint32 size=*sizePtr;
		if((size+outSize) >= MAX_DIMENSION){
			size=0;
		}
		if(!MemoryCopy(&buffer[size], outBuffer, outSize)){
			return false;
		}
		size+=outSize;
		*sizePtr=size;
		return true;
	}


	bool SetBuffered(uint32 defSize){
		return SetBufferSize(defSize, defSize);			
	}


	int64 UnBufferedSize()const{
		return *sizePtr;
	}
	
	bool UnBufferedSeek(int64 seek){
		(*sizePtr)=seek;	
		return true;
	}

	int64 UnBufferedPosition()const{
		return *sizePtr;
	}

	bool UnBufferedSetSize(int64 desSize){
		return true;
	}	

	bool UnBufferedSwitch(uint32 a){
		if(a==2){
			buffer=buffer2;
			sizePtr=&size2;
		}
		if(a==1){
			buffer=buffer1;
			sizePtr=&size1;
		}
		return true;
	}

	bool UnBufferedSwitch(const char* a){
		if(*a=='1'){
			sizePtr=&size1;
			buffer=buffer1;
		}
		if(*a=='2'){
			sizePtr=&size2;
			buffer=buffer2;
		}
		return true;
	}
	

	bool UnBufferedRemoveStream(const char* a){
		UnBufferedSwitch((*a-'0')%2+1);	
		nBuffers--;
		return true;
	}

///Useless functions	
	bool CanWrite()const{
		return true;
	}

	bool CanSeek()const{
		return doubleBuffer;
	}
	
	bool CanRead()const{
		return true;
	}

	uint32 NOfStreams(){
		return nBuffers;
	}

	bool StreamName(uint32 a, char* some, int b)const{
		return false;
	}

	uint32 SelectedStream(){
		if(buffer==buffer1)
			return 1;
		else 
			return 2;
	}
		
	bool AddStream(const char* h){
		nBuffers++;
		return true;
	}

};



//Uses the Wrapper IOBuffer
class SimpleStreamable : public Streamable{

public:
	uint32 size;
	char buffer[MAX_STREAM_DIMENSION];
	bool seekFlag;
	StreamWrapperIOBuffer outputBuff;
	char window[64];
protected:
	
	IOBuffer* GetInputBuffer(){
		if(seekFlag){
			return &outputBuff;	
		}	
		return NULL;
	}

	IOBuffer* GetOutputBuffer(){
		return NULL;
	}

public:

	SimpleStreamable(bool flag=false):outputBuff(this, window, sizeof(window)){
		for(int32 i=0; i<MAX_STREAM_DIMENSION; i++) buffer[i]=0;
		size=0;
		seekFlag=flag;
	}

	
	void Clear(){
		size=0;
		for(int32 i=0; i<MAX_STREAM_DIMENSION; i++) buffer[i]=0;
	}


	bool Sync(){
		if(seekFlag)	return outputBuff.Resync();
		else return true;
	}

	//Copy buffer in inBuffer for the read from stream.
	bool Read(char* inBuffer, uint32 &inSize, TimeoutType timeout=TTDefault, bool complete=false){
	
		if((size+inSize) >= MAX_STREAM_DIMENSION){
			size=0;
		}
	
		if(!MemoryCopy(inBuffer, &buffer[size], inSize)){
			return false;
		}
	
		size+=inSize;
		return true;

	}		

	//Copy outBuffer in buffer for the write to stream.
	bool Write(const char* outBuffer, uint32 &outSize, TimeoutType timeout=TTDefault, bool complete=false){

		if((size+outSize) >= MAX_STREAM_DIMENSION){
			size=0;
		}
		if(!MemoryCopy(&buffer[size], outBuffer, outSize)){
			return false;
		}
		size+=outSize;
		return true;
	}


	
	bool Seek(int64 seek){
		size=seek;	
		return true;
	}

	bool RelativeSeek(int32 delta){
		size+=delta;
		return true;
	}

	bool CanWrite()const{
		return true;
	}

	bool CanSeek()const{
		return true;
	}
	
	bool CanRead()const{
		return true;
	}

	int64 Size(){
		return MAX_DIMENSION;
	}	

	int64 Position(){
		return size;
	}

	bool SetSize(int64 s){
		return True;
	}
};


