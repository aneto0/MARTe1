#include "StreamStringIOBuffer.h"

StreamStringIOBuffer::~StreamStringIOBuffer(){

}

/** 
 *  desiredSize is the desired buffer size
 *  strings will fit upto desiredSize-1 
 *  sets the size of the buffer to be desiredSize or greater up next granularity
 */
bool  StreamStringIOBuffer::SetBufferAllocationSize(
		uint32 			desiredSize,
		uint32 			allocationGranularityMask){
	
    bool ret = SetBufferHeapMemory(desiredSize,allocationGranularityMask,1);
    Terminate();
    return ret;
    
}

/** copies buffer of size size at the end of writeBuffer
 * before calling check that bufferPtr is not NULL
 */ 
void StreamStringIOBuffer::Write(const char *buffer, uint32 &size){
	
	// clip to spaceLeft
	if (size > AmountLeft()) {
		SetBufferAllocationSize(UsedSize() + size);
	}
	
	IOBuffer::Write(buffer,size);

}


// read buffer private methods
/// if buffer is full this is called 
bool 		StreamStringIOBuffer::Flush(TimeoutType         msecTimeout){

	// reallocate buffer
	// uses safe version of the function
	// implemented in this class
	if (!SetBufferAllocationSize(BufferSize()+1)) {
		return false;
	}

	return true;
}


/** 
 * loads more data into buffer and sets amountLeft and bufferEnd
 * READ OPERATIONS 
 * */
bool 		StreamStringIOBuffer::Refill(TimeoutType         msecTimeout){
		// nothing to do.
		return true;
}

/**
        sets amountLeft to 0
        adjust the seek position of the stream to reflect the bytes read from the buffer
 * READ OPERATIONS 
 */
bool 		StreamStringIOBuffer::Resync(TimeoutType         msecTimeout){
	// nothing to do.
	return true;
}    

/**
 * position is set relative to start of buffer
 */
/*
bool        StreamStringIOBuffer::Seek(uint32 position){
	if (position > UsedSize()) {
		return false;
	}
	bufferPtr = BufferReference() + position;
	amountLeft = MaxUsableAmount() - position; 
	return true;
}
*/

/**
 * position is set relative to start of buffer
 */
/*
bool        StreamStringIOBuffer::RelativeSeek(int32 delta){
	bool ret = true;
	if (delta >= 0){
		uint32 actualLeft = amountLeft-fillLeft;
		//cannot seek beyond fillLeft
		if ((uint32)delta > actualLeft){
			delta = actualLeft;
			/// maybe saturate at the end?
			ret =  false;
		}
	} else {
		// cannot seek below 0
		if ((uint32)(-delta) > Position()){
			/// maybe saturate at the beginning?
			ret =  false;
			delta = -Position();
		}
	}
	amountLeft -= delta;
	bufferPtr += delta;
	return ret;
}    
*/

