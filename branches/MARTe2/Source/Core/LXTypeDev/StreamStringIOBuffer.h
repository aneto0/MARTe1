#if !defined (STREAMSTRING_IO_BUFFER)
#define STREAMSTRING_IO_BUFFER

#include "GeneralDefinitions.h"
#include "Memory.h"
#include "CharBuffer.h"
#include "IOBuffer.h"

class StreamString;

/// Read buffer Mechanism for Streamable
class StreamStringIOBuffer:public IOBuffer,public CharBuffer {
       
public: // 

	///
	StreamStringIOBuffer(StreamString &s)//:string(s)
	{
		bufferPtr = BufferReference();
		maxAmount = BufferSize();
		if (maxAmount > 0){
			// remove one for the terminator 0
			maxAmount--;
		} 
		amountLeft = maxAmount;	
		fillLeft   = maxAmount;
	}
	
	
	///
	virtual ~StreamStringIOBuffer();
	
	/** sets the size of the buffer to be desiredSize or greater up next granularity
	 *  truncates stringSize to desiredSize-1 
	*/
    virtual bool  SetBufferAllocationSize(
                uint32 			desiredSize,
                uint32 			allocationGranularityMask 		= 0xFFFFFFC0);
	
public: // read buffer private methods
    /// if buffer is full this is called 
    virtual bool 		Flush(TimeoutType         msecTimeout     = TTDefault);

    /** 
     * loads more data into buffer and sets amountLeft and bufferEnd
     * READ OPERATIONS 
     * */
    virtual bool 		Refill(TimeoutType         msecTimeout     = TTDefault);
    
    /**
        sets amountLeft to 0
        adjust the seek position of the stream to reflect the bytes read from the buffer
     * READ OPERATIONS 
    */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault);

    /**
     * position is set relative to start of buffer
     */
    virtual bool        Seek(uint32 position);

    /**
     * position is set relative to start of buffer
     */
    virtual bool        RelativeSeek(int32 delta);    
    
    /// add trailing 0
    void Terminate(){
    	if (BufferReference() != NULL)
    		BufferReference()[Size()]= 0;
    }
    
    /** copies buffer of size size at the end of writeBuffer
     * before calling check that bufferPtr is not NULL
	 */ 
    virtual void Write(const char *buffer, uint32 &size);    
   
};



#endif