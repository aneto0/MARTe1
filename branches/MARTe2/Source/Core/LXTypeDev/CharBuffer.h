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
#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include "GeneralDefinitions.h"

/**
    A basic implementation of a buffer of character.
    A replacement for dealing directly with mallocs and reallocs 
    Access as char * both read-only and read-write
    Supports up to 4G of ram 
*/
class CharBuffer {
private:
	
	struct {
		/** true if memory was allocated by this class methods 
		 *  false means that it is a reference to a buffer*/
		bool 		allocated:1;		
		
		/** true if it is read-only memory */
		bool 		readOnly:1;		
				
	} bufferMode;
	
    /** the size of the allocated memory block  */
    uint32 		bufferSize;

    /**  the memory buffer  */
    char * 		buffer;
	   
private:

    /** 
     * deallocates memory if appropriate
     * sets all members to default  
     */
    virtual void Clean();

public:

    /** Creates a buffer of a given size */
    CharBuffer() {
    	this->bufferSize 		= 0;
        this->buffer 			= NULL;
        bufferMode.readOnly 	= true;
        bufferMode.allocated 	= false;
    }

    /** Destructor */
    virtual ~CharBuffer();
    
    /**
        allocate or reallocate memory to the desired size
        content is preserved by copy, if contiguus memory is not available, as long as it fits the newsize
        allocationGranularityMask defines how many bits to consider 
        for the buffer size. round up the others
    */
    virtual void SetBufferAllocationSize(
    		uint32 			desiredSize,
            uint32 			allocationGranularityMask 		= 0xFFFFFFFF);
    
    /**
        allocate or reallocate memory to the desired size
        content is preserved by copy, if contiguus memory is not available, as long as it fits the newsize
        allocationGranularityMask defines how many bits to consider 
        for the buffer size. round up the others
    */
    virtual void SetBufferReference(
    		char *			buffer, 
    		uint32 			bufferSize
    );
    
    /**
        allocate or reallocate memory to the desired size
        content is preserved by copy, if contiguus memory is not available, as long as it fits the newsize
        allocationGranularityMask defines how many bits to consider 
        for the buffer size. round up the others
    */
    virtual void SetBufferReference(
    		const char *	buffer, 
    		uint32 			bufferSize
    );
    
public:
    
    /** Read Only access to the internal buffer
     @return The pointer to the buffer
     */
    inline const char *Buffer() const {
        return buffer;
    }

    /** Read Write access top the internal buffer
       @return The pointer to the buffer
     */
    inline char *BufferReference() const {
    	if (bufferMode.readOnly) return NULL;
        return buffer;
    }

    /// how much memory used by buffer
    inline uint32 BufferSize() const{
        return bufferSize;
    }

};

#endif

