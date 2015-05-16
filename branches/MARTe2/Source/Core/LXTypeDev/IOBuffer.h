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
class IOBuffer: protected CharBuffer
{
private:	
    /**
		how many chars usable in buffer
        to be initialised by derived class
        may be = bufferSize or bufferSize - 1 
    */
    uint32              	maxUsableAmount;
   
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
     *  minimum level of undo guaranteed.
     *  guarantees success of UnGet() and UnPut() operations always if greater than 1
     *  handling requires proper implementation of NoMoreSpaceToWrite and NoMoreDataToRead
     */
    uint32 					undoLevel;
    
    /**
		pointer to the next character
        to be initialised by derived class
        never to be NULL! unless buffer is empty
    */
    char *					bufferPtr;

public:
    
    /** 
     * @brief If buffer is full this is called to allocate new memory.
     * @param msecTimeout is the timeout. 
     * @return false in case of errors.     
     * deals with the case when we do not have enough space to write to 
     * it might write out buffered data and resets accessPosition
     * or it might extend the buffer size
     * or it might allocate some space
     * or it might fail
     * WRITE OPERATIONS
     * */
    virtual bool 		NoMoreSpaceToWrite(
                uint32              neededSize      = 1,
                TimeoutType         msecTimeout     = TTDefault);

    /** 
     * @brief Loads more data into buffer and sets amountLeft and bufferEnd.
     * @param msecTimeout is the timeout.
     * @return false in case of errors.  
     * deals with the case when we do not have any more data to read 
     * it might reset accessPosition and fill the buffer with more data
     * or it might fail
     * READ OPERATIONS 
     * default return false implementation
     * */
    virtual bool 		NoMoreDataToRead(TimeoutType         msecTimeout     = TTDefault);
    
    /**
     *sets amountLeft to 0
     * adjust the seek position of the stream to reflect the bytes read from the buffer
     * READ OPERATIONS 
     * default return false implementation
    */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault);   
    
    /**
     * perform a termination operation for a group of PutC and/or Write
     * used for instance by string to add terminator 0
     * default empty implementation
     */
    virtual void 		Terminate();
    
    /**
     * position is set relative to start of buffer
     */
    virtual bool 		Seek(uint32 position);
    
    /**
     * position is set relative to current position
     */
    virtual bool 		RelativeSeek(int32 delta);
    
    /**
     * forces the used size of the buffer  
     */
    virtual bool 		SetUsedSize(uint32 size);
    
public:
    
    ///
    IOBuffer(){
    	amountLeft 		= 0;
    	maxUsableAmount = 0;
    	bufferPtr 		= NULL;
    	fillLeft 		= 0;
    	undoLevel 		= 0;
    }
    
    ///
    virtual ~IOBuffer();
    

public:    
    
    /**
        allocate or reallocate memory to the desired size
        content is preserved by copy, if contiguus memory is not available, as long as it fits the newsize
        allocationGranularityMask defines how many bits to consider 
        for the buffer size. round up the others
    */
    virtual bool SetBufferHeapMemory(
            uint32 			desiredSize,
            uint32 			allocationGranularityMask 		= 0xFFFFFFFF,
            uint32          reservedSpaceAtEnd              = 0
    );    
    
    /**
     * wipes all content and replaces the used buffer
    */
    virtual bool SetBufferReferencedMemory(
    	    char *			buffer, 
    	    uint32 			bufferSize,
            uint32          reservedSpaceAtEnd              = 0
    );
    
    /**
     * wipes all content and replaces the used buffer
    */
    virtual bool SetBufferReadOnlyReferencedMemory(
    		const char *	buffer, 
    		uint32 			bufferSize,
            uint32          reservedSpaceAtEnd              = 0
    );
    
public:    
    
    /// The raw size of the buffer
    uint32 				BufferSize() 		const{
    	return CharBuffer::BufferSize();
    }
    
	/** amount of space of buffer that can be used
     	not reserved for other use
     */
    inline uint32 		MaxUsableAmount() 	const {
    	return maxUsableAmount;
    }
    
	/// how much can we write before filling buffer (considering any reserved space)
    inline uint32 		AmountLeft() 		const {
    	return amountLeft;
    }  

	/// how much more can we read before reaching end of used portion of buffer
    inline uint32 		UsedAmountLeft() 	const {
    	return amountLeft - fillLeft;
    }  
    
    /// seek position
    inline uint32 		Position() 			const {
    	return maxUsableAmount - amountLeft;
    }

    /// how many characters in it
    inline uint32 		UsedSize() 			const{
    	return maxUsableAmount - fillLeft;
    }

    ///
    inline const char *Buffer() 			const {
    	return CharBuffer::Buffer();    	
    }

    ///
    inline char *BufferReference() 			const {
    	return CharBuffer::BufferReference();    	
    }

public:    
    
    /** 
     * reads from buffer at current position
     * before calling check that bufferPtr is not NULL
     */ 
    inline bool         PutC(char c) {

        // check if buffer needs updating and or saving            
//        if (amountLeft <= 0) {
    	if (amountLeft <= undoLevel) {
//            if (!NoMoreSpaceToWrite()) return false;
    		NoMoreSpaceToWrite();
    		
            // check if we can continue or must abort
        	if (amountLeft <= 0) {
        		return false;
        	}
        }
        
        // check later so that to give a chance to allocate memory
        // if that is the policy of this buffering scheme
        if (!CanWrite()) return false;
       
        *bufferPtr = c;
        
        bufferPtr++;
        amountLeft--;
        if (fillLeft > amountLeft){
        	fillLeft = amountLeft;
        }

        return True;    	
    }	

    /** 
     * undo one write to the buffer 
     * if current position is usedSize
     * reduces usedSize
    */ 
    inline bool         UnPutC() {
    	// can I still do it?
    	if (Position() <= 0 ){
    		return false;
    	}
    	
    	if (amountLeft == fillLeft){
    		fillLeft++;
    	}
        bufferPtr--;
        amountLeft++;

        return True;    	
    }
    
    /** copies buffer of size size at the end of writeBuffer
     * before calling check that bufferPtr is not NULL
     * can be overridden to allow resizeable buffers
	 */ 
    virtual void 		Write(const char *buffer, uint32 &size);

    /** 
     * reads from buffer at current position
    */ 
    inline bool         GetC(char &c) {
    	
        // check if buffer needs updating and or saving            
//        if (UsedAmountLeft() <= 0) {
    	if (UsedAmountLeft() <= undoLevel) {
//            if (!NoMoreDataToRead()) return false;
    		NoMoreDataToRead();
    		
        	if (UsedAmountLeft() <= 0) {
        		return false;
        	}
        }
    	
        c = *bufferPtr;
        
        bufferPtr++;
        amountLeft--;

        return True;    	
    }	

    /** 
     * undo one reads from buffer at current position
    */ 
    inline bool         UnGetC() {
    	// can I still do it?
    	if (Position() <= 0 ){
    		return false;
    	}
    	
        bufferPtr--;
        amountLeft++;

        return True;    	
    }
    
    /** 
    	copies to buffer size bytes from the end of readBuffer
    */
    inline void 		Read(char *buffer, uint32 &size){
	    
    	uint32 maxSize = UsedAmountLeft();
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
     * reset to empty read/write buffer
    */
    inline void 		Empty(){
		amountLeft = maxUsableAmount;   // Seek 0 
		fillLeft   = maxUsableAmount;   // SetSize 0
        bufferPtr  = ( char *)Buffer(); // seek 0
    }
    
};
#endif 
