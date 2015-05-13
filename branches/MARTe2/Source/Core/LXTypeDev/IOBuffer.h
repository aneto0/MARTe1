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
#ifndef IO_BUFFER_H
#define IO_BUFFER_H

#include "CharBuffer.h"
#include "Memory.h"
#include "TimeoutType.h"

///
class IOBuffer
{
protected:	
    /**
		how many chars usable in buffer
        to be initialised by derived class
    */
    uint32              	maxAmount;
    /**
		how many chars left in buffer
        to be initialised by derived class
    */
    uint32               	amountLeft;
    
    /**
     * how many chars less than the full size of the buffer
     * distinct from amountLeft to allow duplex operation read and write
     */
    uint32                  fillLeft;

    /**
		pointer to the next character
        to be initialised by derived class
        never to be NULL! unless buffer is empty
    */
    char *					bufferPtr;

public:
    /** 
     * writes out buffer data and resets accessPosition
     * WRITE OPERATIONS
     * */
    virtual bool 		Flush(TimeoutType         msecTimeout     = TTDefault)=0;

    /** 
     * loads more data into buffer and sets amountLeft and bufferEnd
     * READ OPERATIONS 
     * */
    virtual bool 		Refill(TimeoutType         msecTimeout     = TTDefault)=0;
    
    /**
        sets amountLeft to 0
        adjust the seek position of the stream to reflect the bytes read from the buffer
     * READ OPERATIONS 
    */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault)=0;    

    /**
     * position is set relative to start of buffer
     */
//    virtual bool        Seek(uint32 position) = 0;

    /**
     * position is set relative to start of buffer
     */
//    virtual bool        RelativeSeek(int32 position) = 0;
public:
    ///
    IOBuffer(){
    	amountLeft = 0;
    	maxAmount = 0;
    	bufferPtr = NULL;
    	fillLeft = 0;
    }
    
    virtual ~IOBuffer(){
    }
    
	///
    inline uint32 		MaxAmount() const {
    	return maxAmount;
    }
    
	///
    inline uint32 		AmountLeft() const {
    	return amountLeft;
    }

    char *			    BufferPtr() const {
    	return bufferPtr;
    }
    
    /// seek position
    inline uint32 		Position() const{
    	return maxAmount - amountLeft;
    }

    /// how many characters in it
    inline uint32 		Size() const{
    	return maxAmount - fillLeft;
    }
    
    /** 
     * reads from buffer at current position
     * before calling check that bufferPtr is not NULL
     */ 
    inline bool         PutC(char &c) {

        // check if buffer needs updating and or saving            
        if (amountLeft <= 0) {
            if (!Flush()) return false;
        }

        *bufferPtr = c;
        
        bufferPtr++;
        amountLeft--;
        if (fillLeft > amountLeft){
        	fillLeft = amountLeft;
        }

        return True;    	
    }	
	
    /** copies buffer of size size at the end of writeBuffer
     * before calling check that bufferPtr is not NULL
     * can be overridden to allow resizeable buffers
	 */ 
    virtual void Write(const char *buffer, uint32 &size){
    	
    	// clip to spaceLeft
    	if (size > amountLeft) {
    		size = amountLeft;
    	}

    	// fill the buffer with the remainder 
    	if (size > 0){
    		MemoryCopy(bufferPtr,buffer,size);

    		amountLeft -=size;
        	bufferPtr += size; 
            if (fillLeft > amountLeft){
            	fillLeft = amountLeft;
            }
    	}    	
    }

    /** 
     * reads from buffer at current position
     * before calling make sure that bufferPtr is not NULL
    */ 
    inline bool         GetC(char &c) {

    	
        // check if buffer needs updating and or saving            
        if (amountLeft <= fillLeft) {
            if (!Refill()) return false;
        }
    	
        c = *bufferPtr;
        
        bufferPtr++;
        amountLeft--;

        return True;    	
    }	

    /** 
      * before calling make sure that bufferEnd is not NULL 
    	copies to buffer size bytes from the end of readBuffer
    */
    inline void 		Read(char *buffer, uint32 &size){
	    
    	uint32 maxSize = amountLeft-fillLeft;
	    // clip to available space
	    if (size > maxSize) {
	    	size = maxSize;
	    }
		    
		// fill the buffer with the remainder 
		if (size > 0) {
			MemoryCopy(buffer, bufferPtr,size);

			amountLeft -= size;
			bufferPtr += size;
		}
    }    
    /**
     * reset to empty read buffer
    */
    inline void 		Empty(){
		amountLeft = maxAmount;  // Seek 0 
		fillLeft = maxAmount;    // SetSize 0
    }
    
};
#endif 
