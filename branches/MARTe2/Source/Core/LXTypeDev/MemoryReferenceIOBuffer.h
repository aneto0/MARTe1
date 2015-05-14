#if !defined (MEMORYREFERENCE_IO_BUFFER)
#define MEMORYREFERENCE_IO_BUFFER

#include "GeneralDefinitions.h"
#include "Memory.h"
#include "IOBuffer.h"


/// Read buffer Mechanism for Streamable
class MemoryReferenceIOBuffer:public IOBuffer {
       
public: // 

	///
	MemoryReferenceIOBuffer()
	{
	}
	
	///
	virtual ~MemoryReferenceIOBuffer();
		

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

   
};



#endif