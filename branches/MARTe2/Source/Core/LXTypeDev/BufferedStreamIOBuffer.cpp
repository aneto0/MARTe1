#include "BufferedStreamIOBuffer.h"

/**
    sets the readBufferFillAmount to 0
    adjust the seek position
*/
bool BufferedStreamIOBuffer::Resync(TimeoutType         msecTimeout){
	// empty!
    if (maxAmount == 0) {
    	return true;
    }
    
    // distance to end 
    uint32 deltaToEnd =  amountLeft - fillLeft;
    
    // adjust seek position
    // in read mode the actual stream 
    // position is to the character after the buffer end
    if (!stream->Seek (stream->Position()-deltaToEnd)) {
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
	
    // load next batch of data
    bufferPtr = BufferReference();    	
    	

    maxAmount  = BufferSize();
	amountLeft = maxAmount;
	// just use this as a temp variable
    fillLeft   = maxAmount; 
    if (stream->Read(BufferReference(),fillLeft)){
    	fillLeft   = maxAmount - fillLeft; 
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
    uint32 writeSize = maxAmount - fillLeft;
    
    // write
    if (!stream->Write(Buffer(),writeSize,msecTimeout,true)) {
    	return False;
    }

    bufferPtr=BufferReference(); 

    maxAmount = BufferSize();
    Empty();
    return True;  
}


