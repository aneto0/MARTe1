/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they
 will be approved by the European Commission - subsequent
 versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 Licence.
 * You may obtain a copy of the Licence at:
 *
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in
 writing, software distributed under the Licence is
 distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/

#include "StreamString.h"
#include "GeneralDefinitions.h"
#include "StringHelper.h"
#include "ErrorManagement.h"

StreamStringBuffer::~StreamStringBuffer(){

}

///
StreamStringBuffer::StreamStringBuffer(StreamString &s):string(s){
	bufferPtr = BufferReference();
	maxAmount = BufferSize();
	if (maxAmount > 0){
		// remove one for the terminator 0
		maxAmount--;
	} 
	amountLeft = maxAmount;	
	fillLeft   = maxAmount;
}


/** 
 *  desiredSize is the desired buffer size
 *  strings will fit upto desiredSize-1 
 *  sets the size of the buffer to be desiredSize or greater up next granularity
 */
bool  StreamStringBuffer::SetBufferAllocationSize(
		uint32 			desiredSize,
		uint32 			allocationGranularityMask){

	// save position    	
	uint32 position   = Position();
	uint32 stringSize = Size();
	
	// truncating
	if ((desiredSize-1) < stringSize){
		BufferReference()[desiredSize-1] = 0;
		stringSize = desiredSize-1;
	}
	// saturate index 
	if (position > stringSize) position = stringSize;  

	// reallocate buffer
	if (!CharBuffer::SetBufferAllocationSize(desiredSize,allocationGranularityMask)) {
		return false;
	}

	// update max size
	maxAmount  = BufferSize();
	if (maxAmount > 0){
		// remove one for the terminator 0
		maxAmount--;
	} 

	// update bufferPtr and amountLeft
	// stringSize is not changing here
	bufferPtr  = BufferReference() + position;
	amountLeft = maxAmount - position;
	fillLeft = maxAmount - stringSize;

	return true;

}

// read buffer private methods
/// if buffer is full this is called 
bool 		StreamStringBuffer::Flush(TimeoutType         msecTimeout){

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
bool 		StreamStringBuffer::Refill(TimeoutType         msecTimeout){
		// nothing to do.
		return true;
}

/**
        sets amountLeft to 0
        adjust the seek position of the stream to reflect the bytes read from the buffer
 * READ OPERATIONS 
 */
bool 		StreamStringBuffer::Resync(TimeoutType         msecTimeout){
	// nothing to do.
	return true;
}    

/**
 * position is set relative to start of buffer
 */
bool        StreamStringBuffer::Seek(uint32 position){
	if (position >= Size()) {
		return false;
	}
	bufferPtr = BufferReference() + position;
	amountLeft = maxAmount - position; 
	return true;
}

/**
 * position is set relative to start of buffer
 */
bool        StreamStringBuffer::RelativeSeek(int32 delta){
	if (delta >= 0){
		//cannot seek beyond fillLeft
		if ((uint32)delta > (amountLeft-fillLeft)){
			/// maybe saturate at the end?
			return false;
		}
	} else 
		// cannot seek below 0
		if ((amountLeft - delta) > maxAmount){
			/// maybe saturate at the beginning?
			return false;
		}
	amountLeft -= delta;
	bufferPtr += delta;
	return true;
}    


/** Destructor */
StreamString::~StreamString() {
}



/** 
    Reads data into buffer. 
    As much as size byte are read, 
    actual read size is returned in size. (unless complete = True)
    msecTimeout is how much the operation should last - no more - if not any (all) data read then return false  
    timeout behaviour depends on class characteristics and sync mode.
*/
bool StreamString::Read(
                        char*               buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout,
                        bool                complete){
	this->buffer.Read(buffer,size);
	return true;
}

/** 
    Write data from a buffer to the stream. 
    As much as size byte are written, 
    actual written size is returned in size. 
    msecTimeout is how much the operation should last.
    timeout behaviour depends on class characteristics and sync mode. 
*/
bool StreamString::Write(
                        const char*         buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout,
                        bool                complete){
	this->buffer.Write(buffer,size);
	this->buffer.Terminate();
	return true;
	
}

/** whether it can be written into */
bool StreamString::CanWrite()const { 
	return true; 
};

/** whether it can be  read */
bool StreamString::CanRead()const { 
	return true; 
};

/** The size of the stream */
int64 StreamString::Size(){ 
	return buffer.Size(); 
}

/** Moves within the file to an absolute location */
bool StreamString::Seek(int64 pos){
	if (pos > buffer.Size()) {
		REPORT_ERROR(ParametersError,"pos out of range")
		return false;
	}
	
	return buffer.Seek((uint32)pos);
}

/** Moves within the file relative to current location */
bool StreamString::RelativeSeek(int32 deltaPos){
	return buffer.RelativeSeek(deltaPos);
}

/** Returns current position */
int64 StreamString::Position() { 
	return buffer.Position(); 
}


/** Clip the string size to a specified point
 @param newStringSize The size of the buffer.
 @return True if successful. False otherwise.
 */
bool StreamString::SetSize(int64 size){
    buffer.SetBufferAllocationSize(size+1, 0xFFFFFFC0);
    return True;
}

/** can you move the pointer */
bool StreamString::CanSeek() const {
	return true; 
};




