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
    Replaces CStream and BufferedStream of BL1
    Inherits from pure virtual StreamInterface and does not resolve all pure virtual functions
   
    Uses the following StreamInterface methods to operate:
        CanRead  Read
        CanWrite Write
        CanSeek  Seek 
    But only allows using GetC and PutC as it masks the whole StreamInterface
    To use the whole StreamInterface one needs to implement safe versions of 
    all functions. This is left for for further class derivations. 

    Note also that this class supports partially or fully disabled buffering.
    Just set the buffer size to 0 and the buffer to NULL.

    It is a buffering mechanism for character streams
    It operates in 6 modes (+variants) depending on the Stream capabilities 
       CanSeek CanRead CanWrite
    and user Buffering choices

    1) Separate Buffering Output Mode
        CanWrite !CanSeek
        Used for devices where in and out streams are separate (for instance console )
        writeBuffer.Buffer()!=NULL && operatingMode.MutexBuffering() = False   
        !Can be combined with Separate Buffering Input Mode
    1a) writeBuffer.Buffer()== NULL just call directly StreamInterface::Write  

    2) Separate buffering Input Mode
        CanRead  !CanSeek
        Used for devices where in and out streams are separate (for instance console )
        readBuffer.Buffer()!=NULL  &&  operatingMode.mutexBuffering = False   
        !Can be combined with Separate Buffering Output Mode
    2a) readBuffer.Buffer()== NULL just call directly StreamInterface::Read  

    3) Dual separate buffers Input and Output Mode
        CanRead CanWrite !CanSeek
        Mode 1/1a and 2/2a combined

    4) Joint buffering Mode
        CanRead CanWrite CanSeek
        readBuffer.Buffer()!=NULL  && writeBuffer.Buffer()!=NULL && 
        operatingMode.mutexBuffering() = True   
        operatingMode.mutexWriteBufferActive and
        operatingMode.mutexReadBufferActive  determines whether the read 
        or write buffering is active
        Get and Put toggle the two flags
        everytime the flags are changed a proper Flush operation is 
        triggered to clean the buffers
    4a-b) one of readBuffer or writeBuffer is NULL
        same toggling of flags and flushing

    5) Joint buffering Read Only Mode
        CanRead !CanWrite CanSeek
        readBuffer.Buffer()!=NULL  && writeBuffer.Buffer()==NULL && 
        operatingMode.mutexBuffering() = false   
        Operates identically to mode 2 but cannot be active together
        with mode 6 
    5a) same as 2a

    6) Joint buffering Write Only Mode
        !CanRead CanWrite CanSeek
        readBuffer.Buffer()==NULL  && writeBuffer.Buffer()!=NULL && 
        operatingMode.mutexBuffering() = false   
        Operates identically to mode 1 but cannot be active together
        with mode 5
    6a) same as 1a 

*/
#if !defined BASIC_STREAM_BUFFER
#define BASIC_STREAM_BUFFER

#include "StreamInterface.h"
#include "CharBuffer.h"

class BasicStreamBuffer: public StreamInterface {
 
    /**
       Defines the operation mode and statsu of a basic stream
       one only can be set of the first 4.
    */
    struct OperatingMode{


        /** writeBuffer is the active one.
        */
        bool mutexReadBufferActive:1;

        /** writeBuffer is the active one.
        */
        bool mutexWriteBufferActive:1;

        /** append 0 
            always only used for SingleBuffering
        */
        bool stringMode:1;

        /**  
             mutually exclusive buffering 
             active when CanSeek is set and both CanRead and CanWrite
             only one buffer can be used at a time
             Flush is executed in between changes
        */
        bool MutexBuffering(){
            return (mutexReadBufferActive || mutexWriteBufferActive);
        }
    };

    
    /// set automatically on initialisation by calling of the Canxxx functions 
    OperatingMode           operatingMode;

    uint32 writeAccessPosition;
    uint32 readAccessPosition;    
    
protected: // read buffer and statuses

    /** starts empty 
        used for separate input operations or dual operations
        to be sized up by final class appropriately 
    */
    CharBuffer              readBuffer;
    
    /**
        for read mode is from where to read data from readBuffer
    */
    uint32                  readBufferAccessPosition;

    /**
        for read mode is how much data was filled in the buffer
    */
    uint32                  readBufferFillAmount;

protected: // read buffer protected methods
    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
        no check will be performed here on that
    */
    inline bool RefillReadBuffer(){

        // load next batch of data
        readBufferAccessPosition = 0;
        readBufferFillAmount = readBuffer.BufferAllocatedSize();
        return UnBufferedRead(readBuffer.BufferReference(),readBufferFillAmount);  
    }

    /// copies to buffer size bytes from the end of readBuffer
    inline void BufferRead(char *buffer, uint32 &size){

        uint32 spaceLeft = readBufferFillAmount - readBufferAccessPosition;

        // clip to available space
        if (size > spaceLeft) size = spaceLeft;
  	
  	    // fill the buffer with the remainder 
  	    MemoryCopy(buffer, readBuffer.Buffer()+readBufferAccessPosition,size);
        readBufferAccessPosition+=size;

    }    

    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    inline bool ResyncReadBuffer(){
        if (readBufferFillAmount == 0) return true;
        // adjust seek position
        // in read mode the actual stream 
        // position is to the character after the buffer end
        if (!UnBufferedSeek (UnBufferedPosition()-readBufferFillAmount+readBufferAccessPosition)) return false;
        readBufferFillAmount = 0;
        readBufferAccessPosition = 0;
    } 

protected: // write buffer and statuses

    /** starts empty 
        used exclusively for separate output operations 
        to be sized up by final class appropriately 
    */
    CharBuffer              writeBuffer;

    /**
        for write mode is from where to write data
        or how much data was added
    */
    uint32                  writeBufferAccessPosition;

protected: // write buffer methods
    
    /**  
        empty writeBuffer
        only called internally when no more space available 
    */
    inline bool FlushWriteBuffer(TimeoutType         msecTimeout     = TTDefault){
        uint32 writeSize = writeBufferAccessPosition;
        if (!UnBufferedWrite(readBuffer.Buffer(),writeSize,msecTimeout,true)) return False;

        writeBufferAccessPosition = 0;
        
        return True;  
    }

    /// copies buffer of size size at the end of writeBuffer
    inline void BufferWrite(const char *buffer, uint32 &size){

        // recalculate how much we can write
        uint32 spaceLeft = writeBuffer.BufferAllocatedSize() - writeBufferAccessPosition;

        // clip to spaceLeft
        if (size > spaceLeft) size = spaceLeft;
  	
  	    // fill the buffer with the remainder 
  	    MemoryCopy(writeBuffer.BufferReference()+writeBufferAccessPosition,buffer,size);
        writeBufferAccessPosition+=size;
    }

protected: // mode methods
    
    /** 
        sets the readBufferFillAmount to 0
        adjust the seek position
        sets the mutexWriteBufferActive
        does not check for mutexBuffering to be active
    */
    inline bool SwitchToWriteMode(){
        if (!ResyncReadBuffer()) return false;
 
        operatingMode.mutexWriteBufferActive = true;
        operatingMode.mutexReadBufferActive = false;
    }
    
    /** 
        Flushes output buffer
        resets mutexWriteBufferActive
        does not refill the buffer nor check the mutexBuffering is active
    */
    bool SwitchToReadMode(){
        // adjust seek position
        if (!FlushWriteBuffer()) return false;
        operatingMode.mutexWriteBufferActive = false;
        operatingMode.mutexReadBufferActive = true; 
    }

public:
    /**
        sets appropriate buffer sizes and adjusts operatingMode
    */
    bool SetBufferSize(uint32 readBufferSize=0, uint32 writeBufferSize=0){
        if (!CanRead())  readBufferSize = 0;   
        if (!CanWrite()) writeBufferSize = 0;   

        // dump any data in the write Queue
        Flush();

        // adjust seek position if needed
        ResyncReadBuffer();

        // assume separate buffers
        operatingMode.mutexReadBufferActive = false;
        operatingMode.mutexWriteBufferActive = false;

        // possibly mutex buffering mode ?
        if (CanSeek() && CanWrite() && CanRead() && 
            (readBufferSize>0) && (writeBufferSize>0)){
            operatingMode.mutexWriteBufferActive = true;
        }
        
        // adjust readBufferSize
        readBuffer.SetBufferSize(readBufferSize);

        // adjust writeBufferSize
        writeBuffer.SetBufferSize(writeBufferSize);
    
        return true;    
    } 
    
    /// default constructor
    BasicStreamBuffer(uint32 readBufferSize=0, uint32 writeBufferSize=0){
        readBufferAccessPosition    = 0;
        writeBufferAccessPosition   = 0;
        readBufferFillAmount        = 0;
        readAccessPosition          = 0;
        writeAccessPosition         = 0;
        SetBufferSize(readBufferSize,writeBufferSize);
    }

    /// default destructor
    virtual ~BasicStreamBuffer(){
    }


public:  // special methods for buffering
    
    /** 
         saves any pending write operations 
    */
    bool Flush(TimeoutType         msecTimeout     = TTDefault){
        // no data
        if (writeAccessPosition == 0) return true;
 
        return FlushWriteBuffer(msecTimeout);
    }


    /// simply write to buffer if space exist and if operatingMode allows
    inline bool         PutC(char c)
    {
        // if in mutex mode switch to write mode
        if (operatingMode.mutexReadBufferActive) {
           if (!SwitchToWriteMode()) return false;
        }
        
        // if write buffer exists then we have write buffer capabilities
        if (writeBuffer.Buffer() != NULL){
            // check if buffer needs updating
            if (writeBufferAccessPosition >= writeBuffer.BufferAllocatedSize()){ 
                // on success the access position is set to 0
                if (!FlushWriteBuffer()) return false;
            }
            // write data
            writeBuffer.BufferReference()[writeAccessPosition++] = c;
            
            return True;
        }


        uint32 size = 1;
        return (UnBufferedWrite(&c, size) && (size == 1));
    }    

    /// simply read from buffer 
    inline bool         GetC(char &c) {

        // if in mutex mode switch to write mode
        if (operatingMode.mutexWriteBufferActive) {
           if (!SwitchToReadMode()) return false;
        }

        // if read buffer exists then we are either in joint buffer or separated read mode enabled
        if (readBuffer.Buffer() != NULL){

            // check if buffer needs updating and or saving            
            if (readBufferAccessPosition >= readBufferFillAmount) {
                if (!RefillReadBuffer()) return false;
            }

            c = readBuffer.BufferReference()[readAccessPosition++];

            return True;
        }

        uint32 size = 1;
        return (UnBufferedRead(&c, size) && (size == 1));
    }    


public:  // replaced StreamInterface methods

    // PURE STREAMING

    /** Reads data into buffer. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool         Read(
                            char *              buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeRead    = false)
    {
        // check for mutually exclusive buffering and 
        // whether one needs to switch to ReadMode
        if (operatingMode.mutexWriteBufferActive) {
           if (!SwitchToReadMode()) return false;
        }
 
        if (readBuffer.Buffer()!=NULL){ 

            // read from buffer first
            uint32 toRead = size;
        	
        	BufferRead(buffer, size);
        	
        	if (size != toRead ){
        		
        		toRead -= size;
        		
        		if ((toRead*4) < readBuffer.BufferAllocatedSize()){
        			if (!RefillReadBuffer()) return false;
        			
        			BufferRead(buffer+size, toRead);
        			size+= toRead;
        			
        			// should have completed
        			return true;
        			
        		} else {
                    // if needed read directly from stream
                    if (!UnBufferedRead(buffer+size,toRead,msecTimeout)) return false;
                    size += toRead;
        			
        		}
        	}
       }
        	
       // if needed read directly from stream
       return UnBufferedRead(buffer,size,msecTimeout);
    }

    /** Write data from a buffer to the stream. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool         Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeWrite   = false)
    {

        // check for mutually exclusive buffering and 
        // whether one needs to switch to WriteMode
        if (operatingMode.mutexReadBufferActive) {
           if (!SwitchToWriteMode()) return false;
        }
        
        // buffering active?
        if (writeBuffer.Buffer() != NULL){
        	// separate input and output size
        	uint32 toWrite = size;
        	// check available buffer size versus write size 
            // if size is comparable to buffer size there 
            // is no reason to use the buffering mechanism
            if (writeBuffer.BufferAllocatedSize() > (4 *size)){
            	// try writing the buffer
            	BufferWrite(buffer, size);
            	
            	// all done! space available! 
            	if (size == toWrite) return true;
            	
            	// make space
            	if (!Flush()) return false;

            	toWrite -= size;
            	uint32 leftToWrite = toWrite;
            	
            	// try writing the buffer
            	BufferWrite(buffer+size, leftToWrite);

            	size+= leftToWrite;
            	
            	// should have been able to fill in it!!!
            	if (leftToWrite != toWrite) return false;
            	return true;               
            } else {
            	// write the buffer so far
            	if (!Flush()) return false;
            }
            
        }
        return UnBufferedWrite(buffer,size,msecTimeout);

    }

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size()    { 
    	Flush();
    	return UnBufferedSize(); 
    }

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos)
    {
        Flush();
    	return UnBufferedSeek(pos);
    }

    /** Returns current position */
    virtual int64       Position() {
    	
    	
    	return UnBufferedPosition(); 
    }

    /** Clip the stream size to a specified point */
    virtual bool        SetSize(int64 size)
    {
        return UnBufferedSetSize(size);
    }

    // Extended Attributes or Multiple Streams INTERFACE

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n)
    {
        Flush();
    	return UnBufferedSwitch(n);
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name)
    {
        Flush();
        return UnBufferedSwitch(name);
    }

    /**  remove an existing stream . */
    virtual bool        RemoveStream(const char *name)
    {
    	Flush();
        return UnBufferedRemoveStream(name);
    }



};







#endif
