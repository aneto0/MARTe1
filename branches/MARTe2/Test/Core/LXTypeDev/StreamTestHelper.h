

#include "Streamable.h"
#include "Memory.h"
#define MAX_DIMENSION 128

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
	char buffer[MAX_DIMENSION];
	uint32 size;
	bool doubleBuffer;
public:


	SimpleStreamable(){
		size=0;
		doubleBuffer=false;
		operatingModes.canSeek=true;
		for(int i=0; i<MAX_DIMENSION; i++) buffer[i]=0;
	}

	~SimpleStreamable(){
		Flush();
	}
	

	void Clear(){
		size=0;
		for(int i=0; i<MAX_DIMENSION; i++) buffer[i]=0;
	}

	//Copy buffer in inBuffer for the read from stream.
	bool UnBufferedRead(char* inBuffer, uint32 &inSize, TimeoutType timeout=TTDefault, bool complete=false){
	
		if((size+inSize) >= MAX_DIMENSION){
			size=0;
		}
	
		if(!MemoryCopy(inBuffer, &buffer[size], inSize)){
			return false;
		}
	
		size+=inSize;
		return true;

	}		

	//Copy outBuffer in buffer for the write to stream.
	bool UnBufferedWrite(const char* outBuffer, uint32 &outSize, TimeoutType timeout=TTDefault, bool complete=false){
		if((size+outSize) >= MAX_DIMENSION){
			size=0;
		}
		if(!MemoryCopy(&buffer[size], outBuffer, outSize)){
			return false;
		}
		size+=outSize;
		return true;
	}


	bool SetBuffered(uint32 defSize){
		return SetBufferSize(defSize, defSize);			
	}


	int64 UnBufferedSize()const{
		return size;
	}
	
	bool UnBufferedSeek(int64 seek){
		size=seek;	
		return true;
	}

	int64 UnBufferedPosition()const{
		return size;
	}

	bool UnBufferedSetSize(int64 desSize){
		return true;
	}	

	bool UnBufferedSwitch(uint32 a){
		return true;
	}

	bool UnBufferedSwitch(const char* a){
		return true;
	}
	

	bool UnBufferedRemoveStream(const char* a){
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
		return 1;
	}

	bool StreamName(uint32 a, char* some, int b)const{
		return false;
	}

	uint32 SelectedStream(){
		return 1;
	}
		
	bool AddStream(const char* h){
		return true;
	}

	bool GetToken(StreamInterface &a, const char* b, char* c, const char* d){
		return true;
	}

	

};
