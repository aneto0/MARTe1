

#include "Streamable.h"
#include "Memory.h"
#define MAX_DIMENSION 128
#define MAX_STREAM_DIMENSION 256

//A generic stream class used for types print.
class MyStream{
private:
	char buffer[MAX_DIMENSION];
	int32 size;
public:

	MyStream(){
		size=0;
	}
	
	void PutC(char c){
		size%=(MAX_DIMENSION-1);
			
		
		buffer[size]=c;
		buffer[size+1]=0;
		size++;		
	}
	
	char* Buffer(){
		return buffer;
	}

	void Clear(){
		size=0;
	}
};



//A simple stream for streamable which implement unbuffered functions.
class SimpleStreamable : public Streamable{
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


	SimpleStreamable(){
		size1=0;
		size2=0;
		nBuffers=2;
		buffer=buffer1;
		sizePtr=&size1;
		doubleBuffer=false;
		for(int i=0; i<MAX_DIMENSION; i++) buffer[i]=0;
	}

	~SimpleStreamable(){
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
