#if !defined STREAMWRAPPER_IOBUFFER
#define STREAMWRAPPER_IOBUFFER

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"


/// Read buffer Mechanism for Streamable
class StreamWrapperIOBuffer:public IOBuffer{
private:
	///
	StreamInterface *stream;
       
public: // read buffer private methods

    /// allocate from dynamic memory
    StreamWrapperIOBuffer(StreamInterface *s,uint32 size){
        stream=s;
        SetBufferHeapMemory(size);
    }

    /// allocate from any memory
    StreamWrapperIOBuffer(StreamInterface *s,char *buffer,uint32 size){
        stream=s;
        SetBufferReferencedMemory(buffer,size);
    }
    
    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    virtual bool 		NoMoreDataToRead( TimeoutType         msecTimeout     = TTDefault);
    
    ///
    virtual bool 		NoMoreSpaceToWrite(
                uint32              neededSize      = 1,
                TimeoutType         msecTimeout     = TTDefault);

    /**
        @brief Sets the readBufferFillAmount to 0 and adjusts the seek position.
        @param msecTimeout is the timeout.
        @return false in case of errors. 
    */
    virtual bool 		Resync(TimeoutType      msecTimeout     = TTDefault);    
    
};



#endif 