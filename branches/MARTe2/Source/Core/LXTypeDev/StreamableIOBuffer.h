#if !defined STREAMABLE_IOBUFFER
#define STREAMABLE_IOBUFFER

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "Streamable.h"
#include "IOBuffer.h"
/**
 * @file StreamableIOBuffer.h
 * @brief Implementation of the IOBuffer for streamable types.
 * 
 * This class derives from IOBuffer and implements Refill, Resync and Flush functions specifically 
 * for streamable types.*/ 




class Streamable;

/** @brief StreamableIOBuffer class. */
class StreamableIOBuffer:public IOBuffer{
private:
    /** the stream related to the IOBuffer. */
    Streamable *stream;
       
public: // read buffer private methods

    /** @brief Default constructor */
    StreamableIOBuffer(Streamable *s){
        stream=s;
    }

    /**  
      * @brief Refill readBuffer assuming that the read position is now at the end of buffer.
      * @param msecTimeout is the timeout.
      * @return false in case of errors.
    */
    virtual bool 		Refill(TimeoutType      msecTimeout     = TTDefault);
    
    /**
        @brief Sets the readBufferFillAmount to 0 and adjusts the seek position.
        @param msecTimeout is the timeout.
        @return false in case of errors. 
    */
    virtual bool 		Resync(TimeoutType      msecTimeout     = TTDefault);    

    /**
     * @brief Flushes the buffer writing on the stream.
     * @param msecTimeout is the timeout. 
     * @return false in case of errors. */
    virtual bool 		Flush(TimeoutType     	msecTimeout     = TTDefault);
    
    /**
     * position is set relative to start of buffer
     */
/*    virtual bool        Seek(uint32 			position);
    
    ///
    virtual bool        RelativeSeek(int32 		delta);
  */  
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
