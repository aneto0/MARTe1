#include "BufferedStreamIOBuffer.h"

/**
    sets the readBufferFillAmount to 0
    adjust the seek position
*/
bool BufferedStreamIOBuffer::Resync(TimeoutType         msecTimeout){
	// empty!
    if (MaxUsableAmount() == 0) {
    	return true;
    }
    
    // distance to end 
    uint32 deltaToEnd = UsedSize() - Position();   
    
    // adjust seek position
    // in read mode the actual stream 
    // position is to the character after the buffer end
    if (!stream->Seek (stream->Position()-deltaToEnd)) {
        Empty();
    	return false;
    }
                                                                          
    // mark it as empty
    Empty();
    return true;
} 

/**  
    refill readBuffer
    assumes that the read position is now at the end of buffer
*/
bool BufferedStreamIOBuffer::Refill(TimeoutType         msecTimeout){
	// can we write on it?
	if (BufferReference() == NULL) {
		return false;
	}

	Empty();

    uint32 readSize = MaxUsableAmount();
	
    if (stream->Read(BufferReference(),readSize)){
    	SetUsedSize(readSize);
    	return true;
    }  

    Empty();
	return false;
    	
}

/**  
    empty writeBuffer
    only called internally when no more space available 
*/
bool BufferedStreamIOBuffer::Flush(TimeoutType         msecTimeout  ){
	// no buffering!
	if (Buffer()== NULL) return true;
	
	// how much was written?
    uint32 writeSize = UsedSize();
    
    // write
    if (!stream->Write(Buffer(),writeSize,msecTimeout,true)) {
    	return False;
    }

    Empty();
    return True;  
}


