#define MAX_DIMENSION 128


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
