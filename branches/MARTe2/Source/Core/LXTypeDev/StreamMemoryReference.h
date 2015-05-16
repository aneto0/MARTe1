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
 * Basic implementation of strings 
 */
#ifndef STREAM_MEMORY_REFERENCE_H
#define STREAM_MEMORY_REFERENCE_H

#include "GeneralDefinitions.h"
#include "MemoryReferenceIOBuffer.h"
#include "BufferedStream.h"
#include "TimeoutType.h"

/**
    A basic implementation of a stream .
    allows reading / writing to a memory location
*/
class StreamMemoryReference: public BufferedStream {

private:    

	///
	MemoryReferenceIOBuffer 	buffer;	
   
protected: // methods to be implemented by deriving classes
    ///
    virtual IOBuffer *GetInputBuffer();

    ///
    virtual IOBuffer *GetOutputBuffer();

public: // usable constructors

    /** Binds this object to a memory area R/W mode
     *  assumes that the area of memory is empty and therefore the Stream::Size is 0
     * */
    StreamMemoryReference(char *buffer,uint32 bufferSize){
    	this->buffer.SetBufferReferencedMemory(buffer,bufferSize);
    }

    /** Binds this object to a memory area R only mode
     *  assumes that the area of memory is full and therefore the Stream::Size is bufferSize
     * */
    StreamMemoryReference(const char *buffer,uint32 bufferSize){
    	this->buffer.SetBufferReadOnlyReferencedMemory(buffer,bufferSize);
    	this->buffer.SetUsedSize(bufferSize);
    }
    
    /** Destructor */
    virtual ~StreamMemoryReference() ;
    
public:
    /** 
        Reads data into buffer. 
        As much as size byte are read, 
        actual read size is returned in size. (unless complete = True)
        msecTimeout is how much the operation should last - no more - if not any (all) data read then return false  
        timeout behaviour depends on class characteristics and sync mode.
    */
    virtual bool        Read(
                            char*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false);

    /** 
        Write data from a buffer to the stream. 
        As much as size byte are written, 
        actual written size is returned in size. 
        msecTimeout is how much the operation should last.
        timeout behaviour depends on class characteristics and sync mode. 
    */
    virtual bool        Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false);
    
    /** whether it can be written into */
    virtual bool        CanWrite() const ;

    /** whether it can be  read */
    virtual bool        CanRead() const ;
    
    /** The size of the stream */
    virtual int64       Size();

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos);
    
    /** Moves within the file relative to current location */
    virtual bool        RelativeSeek(int32 deltaPos);
    
    /** Returns current position */
    virtual int64       Position() ;

    /** Clip the string size to a specified point
     @param newStringSize The size of the buffer.
     @return True if successful. False otherwise.
     */
    virtual bool        SetSize(int64 size);

    /** can you move the pointer */
    virtual bool        CanSeek() const ;

public: // DIRECT ACCESS FUNCTIONS
      

    /** Read Only access to the internal buffer
     @return The pointer to the buffer
     */
    inline const char *Buffer() const {
        return buffer.Buffer();
    }

    /** Read Write access top the internal buffer
     @return The pointer to the buffer
     */
    inline char *BufferReference() const {
        return buffer.BufferReference();
    }


    /** Returns a pointer to the tail of the buffer.
     @param  ix the offset from the end of buffer. valid ranges is 0 to Size()-1
     @return pointer to the tail of the buffer
     */
    inline const char *Tail(int32 ix) const {
    	if (ix > 0) 				return 0;
    	if ((ix - buffer.UsedSize() -1)< 0) 	return 0;
    	return buffer.BufferReference() + buffer.UsedSize() - ix - 1;
    }


};

#endif

