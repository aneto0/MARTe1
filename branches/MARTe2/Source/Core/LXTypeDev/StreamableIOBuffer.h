#if !defined STREAMABLE_IOBUFFER
#define STREAMABLE_IOBUFFER

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "Streamable.h"
#include "IOBuffer.h"

class Streamable;

/// Read buffer Mechanism for Streamable
class StreamableIOBuffer:public IOBuffer,protected CharBuffer{
private:
    ///
    Streamable *stream;
       
public: // read buffer private methods

    ///
    StreamableIOBuffer(Streamable *s){
        stream=s;
        bufferPtr = BufferReference();        
    }

    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    virtual bool 		Refill(TimeoutType      msecTimeout     = TTDefault);
    
    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    virtual bool 		Resync(TimeoutType      msecTimeout     = TTDefault);    

    ///
    virtual bool 		Flush(TimeoutType     	msecTimeout     = TTDefault);
    
    /**
     * position is set relative to start of buffer
     */
    virtual bool        Seek(uint32 			position);
    
    ///
    virtual bool        RelativeSeek(int32 		delta);
    
    /**
    */ 
    bool SetBufferSize(uint32 size){
        SetBufferAllocationSize(size);
        bufferPtr = BufferReference();
        maxAmount = BufferSize();
	Empty();
    	return true;
    }
};



#endif
