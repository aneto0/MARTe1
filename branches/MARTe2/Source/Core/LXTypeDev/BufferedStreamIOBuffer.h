#if !defined BUFFEREDSTREAM_IOBUFFER
#define BUFFEREDSTREAM_IOBUFFER

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"


/// Read buffer Mechanism for Streamable
class BufferedStreamIOBuffer:public IOBuffer,protected CharBuffer{
private:
	///
	StreamInterface *stream;
       
public: // read buffer private methods

    ///
    BufferedStreamIOBuffer(StreamInterface *s,uint32 size){
        stream=s;
        SetBufferAllocationSize(size);
        bufferPtr = BufferReference();
        Empty();
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