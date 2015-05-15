#if !defined (STREAMSTRING_IO_BUFFER)
#define STREAMSTRING_IO_BUFFER

#include "GeneralDefinitions.h"
#include "Memory.h"
#include "CharBuffer.h"
#include "IOBuffer.h"
/**
 * @file StreamStringIOBuffer.h
 * @brief Implementation of basic functions of the buffer used for StreamString class.
 *
 * These functions allows to allocate new space in the heap for the buffer, read and write operations.
 * This buffer is an attribute of StreamString class.
 */



class StreamString;

/** @brief StreamStringIOBuffer class. */
class StreamStringIOBuffer:public IOBuffer {
       
public: // 

	/** @brief Default constructor */
	StreamStringIOBuffer()
	{
/*
		bufferPtr = BufferReference();
		maxUsableAmount = BufferSize();
		if (maxUsableAmount > 0){
			// remove one for the terminator 0
			maxUsableAmount--;
		} 
		amountLeft = maxUsableAmount;	
		fillLeft   = maxUsableAmount;
*/		
	}
	
	/** @brief Default destructor. */
	virtual ~StreamStringIOBuffer();
	
    /** @brief Add the termination character. */
    void Terminate(){
    	if (BufferReference() != NULL)
    		BufferReference()[UsedSize()]= 0;
    }
	
	
	/**
         * @brief Sets the size of the buffer to be desiredSize or greater up next granularity. 
         *
         * Truncates stringSize to desiredSize-1.
         *
         * @param desiredSize is the desired size to allocate.
         * @param allocationGranularityMask specifies the minimum size to be allocated.
         * @return false in case of errors.
	*/
    virtual bool  		SetBufferAllocationSize(
                uint32 			desiredSize,
                uint32 			allocationGranularityMask 		= 0xFFFFFFC0);
	
public: // read buffer private methods
    /** 
     * @brief If buffer is full this is called to allocate new memory.
     * @param msecTimeout is the timeout. 
     * @return false in case of errors. */    
    virtual bool 		Flush(TimeoutType         msecTimeout     = TTDefault);

    /** 
     * @brief Loads more data into buffer and sets amountLeft and bufferEnd.
     * @param msecTimeout is the timeout.
     * @return false in case of errors.  
     */
    virtual bool 		Refill(TimeoutType         msecTimeout     = TTDefault);
    
    /**
     * @brief sets amountLeft to 0 and adjust the seek position of the stream to reflect the bytes read from the buffer.
     * @param msecTimeout is the timeout. 
     * @return false in case of errors. */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault);

    /**
     * position is set relative to start of buffer
     */
//    virtual bool        Seek(uint32 position);

    /**
     * position is set relative to start of buffer
     */
//    virtual bool        RelativeSeek(int32 delta);    
    
   
    /**
     * @brief Copies buffer the end of writeBuffer.
     * @param buffer contains datas to be written.
     * @param size is the desired number of bytes to copy.
     * 
     * Before calling be sure that bufferPtr is not NULL.
     */ 
    virtual void 		Write(const char *buffer, uint32 &size);    
   
};



#endif
