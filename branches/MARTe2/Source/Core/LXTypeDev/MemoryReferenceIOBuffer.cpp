#include "MemoryReferenceIOBuffer.h"

MemoryReferenceIOBuffer::~MemoryReferenceIOBuffer(){

}

// read buffer private methods
/// if buffer is full this is called 
bool 		MemoryReferenceIOBuffer::Flush(TimeoutType         msecTimeout){
	// nothing to do.
	return true;
}


/** 
 * loads more data into buffer and sets amountLeft and bufferEnd
 * READ OPERATIONS 
 * */
bool 		MemoryReferenceIOBuffer::Refill(TimeoutType         msecTimeout){
		// nothing to do.
		return true;
}

/**
        sets amountLeft to 0
        adjust the seek position of the stream to reflect the bytes read from the buffer
 * READ OPERATIONS 
 */
bool 		MemoryReferenceIOBuffer::Resync(TimeoutType         msecTimeout){
	// nothing to do.
	return true;
}    

