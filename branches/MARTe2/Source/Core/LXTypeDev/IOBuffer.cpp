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

/** 
 * @file
 * Implementation of a buffer with allocation and reallocation and access functions
 */

#include "IOBuffer.h"


/**
 * position is set relative to start of buffer
 */
bool IOBuffer::Seek(uint32 position){
    if (position > UsedSize()) return false; 
    amountLeft = MaxUsableAmount() - position;
    bufferPtr = ( char *)Buffer() + position;
    return true;
}
    
/**
 * position is set relative to current position
 */
bool IOBuffer::RelativeSeek(int32 delta){
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

bool IOBuffer::SetUsedSize(uint32 size){
	if (size > maxUsableAmount){
		size = maxUsableAmount;
	}
	
	fillLeft = maxUsableAmount - size;
	
	return true;
}


IOBuffer::~IOBuffer(){
}


/**
    allocate or reallocate memory to the desired size
    content is preserved by copy, if contiguus memory is not available, as long as it fits the newsize
    allocationGranularityMask defines how many bits to consider 
    for the buffer size. round up the others
*/
bool IOBuffer::SetBufferHeapMemory(
		uint32 			desiredSize,
        uint32 			allocationGranularityMask,
        uint32          reservedSpaceAtEnd      
){
	// save position    	
	uint32 position = Position();
	uint32 usedSize = UsedSize();

	// truncating
	if ((desiredSize-reservedSpaceAtEnd) < usedSize){
		usedSize = desiredSize-reservedSpaceAtEnd;
	}
	// saturate index 
	if (position > usedSize) position = usedSize;  
	
	bool ret = CharBuffer::SetBufferAllocationSize(desiredSize,allocationGranularityMask);

	bufferPtr = ( char *)Buffer();

   	 maxUsableAmount = BufferSize();
	if (maxUsableAmount <= reservedSpaceAtEnd){
		maxUsableAmount = 0;
	} else {
        maxUsableAmount = BufferSize() - reservedSpaceAtEnd;
	}
	amountLeft = maxUsableAmount - position;
	fillLeft   = maxUsableAmount - usedSize;

	bufferPtr += position;
    return ret;
}       

/**
 * wipes all content and replaces the used buffer
*/
bool IOBuffer::SetBufferReferencedMemory(
		char *			buffer, 
		uint32 			bufferSize,
        uint32          reservedSpaceAtEnd           
){
	CharBuffer::SetBufferReference(buffer,bufferSize);
    bufferPtr 		= ( char *)Buffer();
    maxUsableAmount = BufferSize() - reservedSpaceAtEnd;
    Empty();
    return true;    	
}

/**
 * wipes all content and replaces the used buffer
*/
bool IOBuffer::SetBufferReadOnlyReferencedMemory(
		const char *	buffer, 
		uint32 			bufferSize,
        uint32          reservedSpaceAtEnd          
){
	CharBuffer::SetBufferReference(buffer,bufferSize);
    bufferPtr 		= ( char *)Buffer();
    maxUsableAmount = BufferSize() - reservedSpaceAtEnd;
    Empty();
    return true;    	
}

/** copies buffer of size size at the end of writeBuffer
 * before calling check that bufferPtr is not NULL
 * can be overridden to allow resizeable buffers
 */ 
void IOBuffer::Write(const char *buffer, uint32 &size){
	if (!CanWrite()) return;
	
	// clip to spaceLeft
	if (size > amountLeft) {
		size = amountLeft;
	}

	// fill the buffer with the remainder 
	if (size > 0){
		MemoryCopy(bufferPtr,buffer,size);

    	bufferPtr += size; 
		amountLeft -=size;
        if (fillLeft > amountLeft){
        	fillLeft = amountLeft;
        }
	}    	
}

