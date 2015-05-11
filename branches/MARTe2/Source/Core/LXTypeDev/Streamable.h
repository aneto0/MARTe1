#if !defined STREAMABLE
#define STREAMABLE

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"
#include "AnyType.h"
#include "FormatDescriptor.h"
#include "BufferedStream.h"

class Streamable;

/// Read buffer Mechanism for Streamable
class StreamableReadBuffer:public IOBuffer,protected CharBuffer{
private:
	///
	Streamable &stream;
       
public: // read buffer private methods

    ///
    StreamableReadBuffer(Streamable &s):stream(s){
    	bufferPtr = BufferReference();
//    	this->stream = stream;
    }

    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    virtual bool 		Refill(TimeoutType         msecTimeout     = TTDefault);
    
    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault);    

    ///
	virtual bool 		Flush(TimeoutType         msecTimeout     = TTDefault){ return false; }
    
    /**
     * position is set relative to start of buffer
     */
    virtual bool        Seek(uint32 position){
    	if (position >= maxAmount) return false; 
		amountLeft = maxAmount - position;
		bufferPtr = BufferReference() + position;
		return true;
    }
    
    ///
    virtual bool        RelativeSeek(int32 delta){
    	if (delta > 0){
    		if ((uint32)delta >= amountLeft) return false;
    	}
    	if (delta < 0){
    		if ((amountLeft-delta) > maxAmount) return false;
    	}
    	amountLeft += delta;
    	bufferPtr += delta;
		return true;
    }
    
	///
	bool SetBufferSize(uint32 size){
		SetBufferAllocationSize(size);
    	bufferPtr = BufferReference();
    	return true;
	}
   
	
};


/// Read buffer Mechanism for Streamable
class StreamableWriteBuffer:public IOBuffer,protected CharBuffer{
private:
	///
	Streamable &stream;

public:

    ///
    StreamableWriteBuffer(Streamable &s):stream(s){
    	bufferPtr = BufferReference();
    }
	
	/**  
	    empty writeBuffer
	    only called internally when no more space available 
	*/
	virtual bool 		Flush(TimeoutType         msecTimeout     = TTDefault);

    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    virtual bool 		Refill(TimeoutType         msecTimeout     = TTDefault){ return false;} 
    
    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault){ return false;}    

    /**
     * position is set relative to start of buffer
     */
    virtual bool        Seek(uint32 position){
    	if (position >= maxAmount) return false; 
		amountLeft = maxAmount - position;
		bufferPtr = BufferReference() + position;
		return true;
    }
    
    ///
    virtual bool        RelativeSeek(int32 delta){
    	if (delta > 0){
    		if ((uint32)delta >= amountLeft) return false;
    	}
    	if (delta < 0){
    		if ((amountLeft-delta) > maxAmount) return false;
    	}
    	amountLeft += delta;
    	bufferPtr += delta;
		return true;
    }
    
	///
	bool SetBufferSize(uint32 size){
		SetBufferAllocationSize(size);
    	bufferPtr = BufferReference();

    	return true;
	}
	
};

/**
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
        operatingMode.mutexWriteMode and
        operatingMode.mutexReadMode  determines whether the read 
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
class Streamable: public BufferedStream {
 
private: // read and write buffers
friend class StreamableReadBuffer; 
friend class StreamableWriteBuffer; 
       
    /** starts empty 
        used for separate input operations or dual operations
        to be sized up by final class appropriately 
    */
    StreamableReadBuffer	readBuffer;
    

    /** starts empty 
        used exclusively for separate output operations 
        to be sized up by final class appropriately 
    */
    StreamableWriteBuffer   writeBuffer;
    
protected: // methods to be implemented by deriving classes
    ///
    virtual IOBuffer &GetInputBuffer(){
    	return readBuffer;
    }

    ///
    virtual IOBuffer &GetOutputBuffer(){
    	return writeBuffer;
    }
    
   
protected:
    /// default constructor
    Streamable() : readBuffer(*this),writeBuffer(*this)
    {
       
    }

    /// default destructor
    virtual ~Streamable();
    
    /**
        sets appropriate buffer sizes and adjusts operatingModes
        must be called by descendant
        can be overridden but is not meant to - just accessed via VT
    */
    virtual bool SetBufferSize(uint32 readBufferSize=0, uint32 writeBufferSize=0);
    

};


#endif
