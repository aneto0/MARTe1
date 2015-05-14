#if !defined BUFFEREDSTREAM_IOBUFFER
#define BUFFEREDSTREAM_IOBUFFER

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"


/// Read buffer Mechanism for Streamable
class BufferedStreamIOBuffer:public IOBuffer{
private:
	///
	StreamInterface *stream;
       
public: // read buffer private methods

    /// allocate from dynamic memory
    BufferedStreamIOBuffer(StreamInterface *s,uint32 size){
        stream=s;
        SetBufferHeapMemory(size);
    }

    /// allocate from any memory
    BufferedStreamIOBuffer(StreamInterface *s,char *buffer,uint32 size){
        stream=s;
        SetBufferReferencedMemory(buffer,size);
    }
    
    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    virtual bool 		Refill(TimeoutType      msecTimeout     = TTDefault);
    
    ///
    virtual bool 		Flush(TimeoutType     	msecTimeout     = TTDefault);

    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    virtual bool 		Resync(TimeoutType     	msecTimeout     = TTDefault);
    
};



#endif 