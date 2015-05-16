#if !defined BUFFEREDSTREAM_IOBUFFER
#define BUFFEREDSTREAM_IOBUFFER

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "BufferedStream.h"
#include "IOBuffer.h"
/**
 * @file BufferedStreamIOBuffer.h
 * @brief Implementation of the IOBuffer for streamable types.
 * 
 * This class derives from IOBuffer and implements Refill, Resync and Flush functions specifically 
 * for streamable types.*/ 

class BufferedStream;

/** @brief BufferedStreamIOBuffer class. */
class BufferedStreamIOBuffer:public IOBuffer{
private:
    /** the stream related to the IOBuffer. */
    BufferedStream *stream;
    
       
public: // read buffer private methods

    /** @brief Default constructor */
    BufferedStreamIOBuffer(BufferedStream *s){
        stream=s;
    }

    /**  
      * @brief Refill readBuffer assuming that the read position is now at the end of buffer.
      * @param msecTimeout is the timeout.
      * @return false in case of errors.
    */
    virtual bool 		NoMoreDataToRead( TimeoutType         msecTimeout     = TTDefault);
    
    /// for this type of buffer the NoMoreSpaceToWrite operates as a flush
    inline bool Refill(TimeoutType         msecTimeout     = TTDefault){
    	return NoMoreDataToRead(msecTimeout);
    }
    
    /**
     * @brief Flushes the buffer writing on the stream.
     * @param msecTimeout is the timeout. 
     * @return false in case of errors. */
    virtual bool 		NoMoreSpaceToWrite(
                uint32              neededSize      = 1,
                TimeoutType         msecTimeout     = TTDefault);
    
    /// for this type of buffer the NoMoreSpaceToWrite operates as a flush
    inline bool Flush(TimeoutType         msecTimeout     = TTDefault){
    	return NoMoreSpaceToWrite(0, msecTimeout);
    }

    /**
        @brief Sets the readBufferFillAmount to 0 and adjusts the seek position.
        @param msecTimeout is the timeout.
        @return false in case of errors. 
    */
    virtual bool 		Resync(TimeoutType      msecTimeout     = TTDefault);    
    
    /**
        @brief Allocate or reallocate memory to the desired size.
        @param size is the desired size for the buffer.
        @return false in case of errors. 
    */ 
    bool SetBufferSize(uint32 size){
    	return IOBuffer::SetBufferHeapMemory(size);
    }
    
};



#endif
