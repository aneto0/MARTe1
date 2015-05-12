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
    virtual bool 		Refill(TimeoutType      msecTimeout     = TTDefault);
    
    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    virtual bool 		Resync(TimeoutType      msecTimeout     = TTDefault);    

    ///
	virtual bool 		Flush(TimeoutType     	msecTimeout     = TTDefault){ return false; }
    
    /**
     * position is set relative to start of buffer
     */
    virtual bool        Seek(uint32 			position);
    
    ///
    virtual bool        RelativeSeek(int32 		delta);
    
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
protected:    
    /**
       Defines the operation mode and statsu of a basic stream
       one only can be set of the first 4.
    */
    struct OperatingModes{

    	/** cache of canSeek() used in all the buffering functions 
    	for accelleration sake
    	*/
    	bool canSeek:1;

        /** writeBuffer is the active one.
        */
        bool mutexReadMode:1;

        /** writeBuffer is the active one.
        */
        bool mutexWriteMode:1;
    	
        /** append 0 
            always only used for SingleBuffering
        */
        bool stringMode:1;

    };
    /// set automatically on initialisation by calling of the Canxxx functions 
    OperatingModes           operatingModes;
    
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
    
    /** 
        Reads data into buffer. 
        As much as size byte are read, 
        actual read size is returned in size. 
        msecTimeout is how much the operation should last - no more
        timeout behaviour depends on class characteristics and sync mode. 
        I.E. sockets with blocking activated wait forever when noWait is used .... 
    */
    virtual bool        UnBufferedRead(
                            char*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false)=0;

    /** 
        Write data from a buffer to the stream. 
        As much as size byte are written, 
        actual written size is returned in size. 
        msecTimeout is how much the operation should last.
        timeout behaviour depends on class characteristics and sync mode. 
        I.E. sockets with blocking activated wait forever when noWait is used .... 
    */
    virtual bool        UnBufferedWrite(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false)=0;

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       UnBufferedSize()const =0;

    /** Moves within the file to an absolute location */
    virtual bool        UnBufferedSeek(int64 pos)=0;

    /** Returns current position */
    virtual int64       UnBufferedPosition()const =0;

    /** Clip the stream size to a specified point */
    virtual bool        UnBufferedSetSize(int64 size)=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        UnBufferedSwitch(uint32 n)=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        UnBufferedSwitch(const char *name)=0;
    
    virtual bool        UnBufferedRemoveStream(const char *name)=0;


   
protected: // methods to be implemented by deriving classes
    ///
    virtual IOBuffer &GetInputBuffer() {
    	return readBuffer;
    }

    ///
    virtual IOBuffer &GetOutputBuffer() {
    	return writeBuffer;
    }
    
private: // mode switch methods
    
    /** 
        sets the readBufferFillAmount to 0
        adjust the seek position
        sets the mutexWriteMode
        does not check for mutexBuffering to be active
    */
    inline bool SwitchToWriteMode(){
        if (!GetInputBuffer().Resync()) return false;
        operatingModes.mutexWriteMode = true;
        operatingModes.mutexReadMode = false;
        return true;
    }
    
    /** 
        Flushes output buffer
        resets mutexWriteMode
        does not refill the buffer nor check the mutexBuffering is active
    */
    inline bool SwitchToReadMode(){
        // adjust seek position
        if (!GetOutputBuffer().Flush()) return false;
        operatingModes.mutexWriteMode = false;
        operatingModes.mutexReadMode = true; 
        return true;
    }
   
protected:
    /// default constructor
    Streamable() : readBuffer(*this),writeBuffer(*this)
    {
    	operatingModes.canSeek      	= false;
    	operatingModes.mutexReadMode 	= false;
    	operatingModes.mutexWriteMode 	= false;
    	operatingModes.stringMode 		= false;
    }

    /// default destructor
    virtual ~Streamable();
    
    /**
        sets appropriate buffer sizes and adjusts operatingModes
        must be called by descendant
        can be overridden but is not meant to - just accessed via VT
    */
    virtual bool SetBufferSize(uint32 readBufferSize=0, uint32 writeBufferSize=0);

public:  // special inline methods for buffering
    
    /** 
         on dual separate buffering (CanSeek=False) just Flush output 
     
         on joint buffering (CanSeek= True) depending on read/write mode 
         either Resync or Flush
    */
    inline bool Flush(TimeoutType         msecTimeout     = TTDefault){
    	// mutexReadMode --> can seek so makes sense to resync
    	if (operatingModes.mutexReadMode){
    		return GetInputBuffer().Resync();
    	} 
         	
   		return GetOutputBuffer().Flush();
    }

    /**
     *  simply write to buffer if space exist and if operatingModes allows
     */  
    inline bool         PutC(char c)
    {
        // if in mutex mode switch to write mode
        if (operatingModes.mutexReadMode) {
           if (!SwitchToWriteMode()) return false;
        }
        
        IOBuffer& outputBuffer = GetOutputBuffer();
        if (outputBuffer.BufferPtr()!= NULL){
        	return outputBuffer.PutC(c);
        }
        
        uint32 size = 1;
        return UnBufferedWrite(&c,size);
    }    

    /// simply read from buffer 
    inline bool         GetC(char &c) {

        // if in mutex mode switch to write mode
        if (operatingModes.mutexWriteMode) {
           if (!SwitchToReadMode()) return false;
        }

        IOBuffer& inputBuffer = GetInputBuffer();
        if (inputBuffer.BufferPtr()!= NULL){
        	return inputBuffer.GetC(c);
        }
        
        uint32 size = 1;
        return UnBufferedRead(&c,size);
    }    

public:
    // PURE STREAMING  built upon UnBuffered version 

    /** Reads data into buffer. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool         Read(
                            char *              buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeRead    = false);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 
    

    /** Write data from a buffer to the stream. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool         Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeWrite   = false);

    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 


    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size() ;

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 

    /** Moves within the file relative to current location */
    virtual bool        RelativeSeek(int32 deltaPos);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 
    
    /** Returns current position */
    virtual int64       Position() ;
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 

    /** Clip the stream size to a specified point */
    virtual bool        SetSize(int64 size);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 

    // Extended Attributes or Multiple Streams INTERFACE

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n);

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name);

    /**  remove an existing stream .
        current stream cannot be removed 
    */
    virtual bool        RemoveStream(const char *name);

};


#endif
