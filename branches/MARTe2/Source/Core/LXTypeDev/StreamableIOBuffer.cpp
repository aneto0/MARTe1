#include "Streamable.h"
#include "StreamableIOBuffer.h"



/**
    sets the readBufferFillAmount to 0
    adjust the seek position
*/
bool StreamableIOBuffer::Resync(TimeoutType         msecTimeout){
	// empty!
    if (MaxUsableAmount() == 0) {
    	return true; 
    }
    
    // distance to end 
    uint32 deltaToEnd =  AmountLeft() - fillLeft;
    
    // adjust seek position
    // in read mode the actual stream 
    // position is to the character after the buffer end
    if (!stream->UnBufferedSeek (stream->UnBufferedPosition()-deltaToEnd)) {
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
bool StreamableIOBuffer::Refill(TimeoutType         msecTimeout){
	// can we write on it?
    if (BufferReference() == NULL) {
        return false;
    }
	
    // move all pointers and indexes to empty status
    Empty();
    // load next batch of data
//    bufferPtr = BufferReference();    	
    	
//    maxAmount  = BufferSize();
//    amountLeft = MaxUsableAmount();
    // just use this as a temp variable
//    fillLeft   = MaxUsableAmount();
 
    if (stream->UnBufferedRead(BufferReference(),fillLeft)){
    	fillLeft   = MaxUsableAmount() - fillLeft; 
    	return true;
    }  

    Empty();
    return false;
    	
}

/**  
    empty writeBuffer
    only called internally when no more space available 
*/
bool StreamableIOBuffer::Flush(TimeoutType         msecTimeout  ){
    // no buffering!
    if (Buffer()== NULL) return true;
	
    // how much was written?
    uint32 writeSize = UsedSize();
    
    // write
    if (!stream->UnBufferedWrite(Buffer(),writeSize,msecTimeout,true)) {
    	return False;
    }

//    bufferPtr=BufferReference(); 
//    maxAmount = BufferSize();
    Empty();
    return True;  
}

/*
bool StreamableIOBuffer::Seek(uint32 position){
    if (position >= UsedSize()) return false; 
    amountLeft = MaxUsableAmount() - position;
    bufferPtr = BufferReference() + position;
    return true;
}

///
bool StreamableIOBuffer::RelativeSeek(int32 delta){
	if (delta > 0){
		if ((uint32)delta >= (amountLeft-fillLeft)) return false;
	}
	if (delta < 0){
		if ((amountLeft-delta) > MaxUsableAmount()) return false;
	}
	amountLeft -= delta;
	bufferPtr += delta;
	return true;
}
*/
