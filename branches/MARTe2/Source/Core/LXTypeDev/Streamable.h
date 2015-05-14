#if !defined STREAMABLE
#define STREAMABLE

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"
#include "AnyType.h"
#include "FormatDescriptor.h"
#include "BufferedStream.h"
#include "StreamableIOBuffer.h"
/**
 * @file Streamable.h
 * @brief Implementation of streamable type functions.
 * 
 * Implementation of streamable type functions for read, write and seek operations in buffered and unbuffered modes for streamable types
 * i.e files. Most of the functions depends from the virtual UnBuffered functions which must be implemented in the derived classes
 * because could be differents for different streamable types. 
 *
 * Streamable uses two IOBuffer in buffered modes, one for reading and one for writing operations and they are used togheter if the stream is defined as
 * readable, writable and seekable.*/ 

#if 0
class Streamable;

/// Read buffer Mechanism for Streamable
class StreamableReadBuffer:public IOBuffer,protected CharBuffer{
private:
	///
	Streamable *stream;
       
public: // read buffer private methods

    ///
    StreamableReadBuffer(Streamable *s){
	stream=s;
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
	Streamable *stream;

public:

    ///
    StreamableWriteBuffer(Streamable *s){
	stream=s;
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
#endif 


/**
  * @brief Streamable class.

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
    /** set automatically on initialisation by calling of the Canxxx functions */
    OperatingModes           operatingModes;
    
private: // read and write buffers
friend class StreamableIOBuffer; 
//friend class StreamableWriteBuffer; 
       
    /** starts empty, 
        used for separate input operations or dual operations
        to be sized up by final class appropriately 
    */
	StreamableIOBuffer		readBuffer;
    

    /** starts empty, 
        used exclusively for separate output operations 
        to be sized up by final class appropriately 
    */
	StreamableIOBuffer   	writeBuffer;
    
protected: // methods to be implemented by deriving classes

    /**
     * switches to input mode and returns input buffer
     * used in read operations 
    */
    virtual IOBuffer *		GetInputBuffer();
    
    /** 
     * switches to output mode and returns output buffer
     * used in write operations 
     */ 
    virtual IOBuffer *		GetOutputBuffer();
    
protected: // methods to be implemented by deriving classes
    
    /** 
     * @brief Reads data into buffer.
     * @param buffer is the buffer where stream datas must be written
     * @param size is the number of bytes to read.
     * @param msecTimeout is the timeout.
     * @param complete is a flag which specifies if the read operation is completed.
     *
     *  As much as size byte are read, 
     *  actual read size is returned in size. 
     *  msecTimeout is how much the operation should last - no more.
     *  Timeout behaviour depends on class characteristics and sync mode. 
     *  I.E. sockets with blocking activated wait forever when noWait is used .... 
    */
    virtual bool        UnBufferedRead(
                            char*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false)=0;

  
    /** 
     * @brief Write data from a buffer to the stream.
     * @param buffer is the buffer which contains datas to write on the stream.
     * @param size is the number of bytes to write.
     * @param msecTimeout is the timeout.
     * @param complete is a flag which specifies if the read operation is completed.
     *
     *  This function is not implemented here but depends from the derived classes.
     *  As much as size byte are written, 
     *  actual written size is returned in size. 
     *  msecTimeout is how much the operation should last.
     *  Timeout behaviour depends on class characteristics and sync mode. 
     *  I.E. sockets with blocking activated wait forever when noWait is used .... 
    */
    virtual bool        UnBufferedWrite(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false)=0;

    // RANDOM ACCESS INTERFACE

    /** @brief The size of the stream.
      * 
      * Not implemented here. */
    virtual int64       UnBufferedSize()const =0;

    /** @brief Moves within the file to an absolute location.
     *
     * Not implemented here. */
    virtual bool        UnBufferedSeek(int64 pos)=0;

    /** @brief Returns current position.
     *
     * Not implemented here. */
    virtual int64       UnBufferedPosition()const =0;

    /** @brief Clip the stream size to a specified point.
      * 
      * Not implemented here. */
    virtual bool        UnBufferedSetSize(int64 size)=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** @brief Select the stream to read from. Switching may reset the stream to the start.
      * 
      * Not implemented here. */
    virtual bool        UnBufferedSwitch(uint32 n)=0;

    /** @brief Select the stream to read from. Switching may reset the stream to the start.
      * 
      * Not implemented here. */
    virtual bool        UnBufferedSwitch(const char *name)=0;
    
    /** @brief Remove a stream.
      * 
      * Not implemented here. */
    virtual bool        UnBufferedRemoveStream(const char *name)=0;
 
    
private: // mode switch methods
    
    /** 
     * @brief Switch to the write mode.
     * @return false if the resyncronization goes wrong.
     *
     *  Sets the readBufferFillAmount to 0.
     *  Adjust the seek position
     *  Sets the mutexWriteMode.
     *  Does not check for mutexBuffering to be active
    */
    inline bool SwitchToWriteMode(){
        if (!GetInputBuffer()->Resync()) return false;
        operatingModes.mutexWriteMode = true;
        operatingModes.mutexReadMode = false;
        return true;
    }
    
    /** 
     * @brief Switch to the read mode.
     * @return false if the flush goes wrong.
     *
     *  Flushes write buffer.
     *  Resets mutexWriteMode.
     *  Does not refill the buffer nor check the mutexBuffering is active.
    */
    inline bool SwitchToReadMode(){
        // adjust seek position
        if (!GetOutputBuffer()->Flush()) return false;
        operatingModes.mutexWriteMode = false;
        operatingModes.mutexReadMode = true; 
        return true;
    }
   
protected:
    /** @brief Default constructor. */
    Streamable() : readBuffer(this),writeBuffer(this)
    {
    	operatingModes.canSeek      	= false;
    	operatingModes.mutexReadMode 	= false;
    	operatingModes.mutexWriteMode 	= false;
    	operatingModes.stringMode 		= false;
    }

    /** @brief Default destructor. */
    virtual ~Streamable();
    
    /**
     * @brief Sets the buffers size, impose the buffered modality.
     * @param readBufferSize is the desired size for the read buffer.
     * @param readBufferSize is the desired size for the write buffer.
     * @return true if the memory is allocated correctly.
     *  
     * Sets appropriate buffer sizes and adjusts operatingModes depending on CanRead, CanWrite and CanSeek functions.
     * Must be called by descendant.
     * Can be overridden but is not meant to - just accessed via VT.
    */
    virtual bool SetBufferSize(uint32 readBufferSize=0, uint32 writeBufferSize=0);

public:  // special inline methods for buffering
    
    /** 
     * @brief Resyncronization and flush of the buffers.
     * @param msecTimeout is the timeout.
     * @return true if the Resync and Flush operations for buffers goes fine.
     *
     * On dual separate buffering (CanSeek=False) just Flush output.
     * On joint buffering (CanSeek= True) depending on read/write mode 
     * either Resync or Flush. */
    inline bool FlushAndResync(TimeoutType         msecTimeout     = TTDefault){
    	// if there is something in the buffer, and canSeek it means we can and need to resync
    	// if the buffers are separated (!canseek) than resync cannot be done
    	if (readBuffer.UsedSize() && operatingModes.canSeek){
    		return readBuffer.Resync();
    	} 
        // some data in writeBuffer
    	// we can flush in all cases then
    	if (writeBuffer.UsedSize() ){
       		return writeBuffer.Flush();    		
    	}
    	return true;
    }

    /**
     * @brief Simply write a character to the stream if space exist and if operatingModes allows.
     * @param c is the character to be written on the stream.
     * @return false in case of errors.
     */  
    inline bool         PutC(char c)
    {   	
    	if (operatingModes.mutexReadMode) {
    	     if (!SwitchToWriteMode()) return false;
    	}
    	
        if (writeBuffer.BufferSize() > 0){
        	return writeBuffer.PutC(c);
        }
        
        uint32 size = 1;
        return UnBufferedWrite(&c,size);
    }    

    /**
     * @brief Simply read a character from stream.
     * @param c is the character by reference in return.
     * @return false in case of errors.
     */
    inline bool         GetC(char &c) {

    	if (operatingModes.mutexWriteMode) {
    	     if (!SwitchToReadMode()) return false;
    	}

        if (readBuffer.BufferSize() > 0){
        	return readBuffer.GetC(c);
        }
        
        uint32 size = 1;
        return UnBufferedRead(&c,size);
    }    

public:
    // PURE STREAMING  built upon UnBuffered version 

    /** @brief Reads data from stream into buffer.
      * @param buffer is the output memory where datas must be written.
      * @param size is the number of bytes to read from stream.
      * @param msecTimeout is the timeout.
      * @param completeRead is a flag which specified is the read operation is completed.
      * As much as size byte are written, actual size
      * is returned in size. msecTimeout is how much the operation should last.
      * Timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
      * when noWait is used .... */
    virtual bool         Read(
                            char *              buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeRead    = false);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 
    

    /**
     * @brief Write data from a buffer to the stream.
     * @param buffer contains the datas to write on the stream.
     * @param size is the desired number of bytes to write.
     * @param msecTimeout is the timeout.
     * @param completeWrite is a flac which specified is the write operations is completed.
     * 
     * As much as size byte are written, actual size
     * is returned in size. msecTimeout is how much the operation should last.
     * Timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
     *  when noWait is used .... */
    virtual bool         Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeWrite   = false);

    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 


    // RANDOM ACCESS INTERFACE

    /** @brief The size of the stream.
      * 
      * Depends on UnBufferedSize and should return the current size of the stream. */
    virtual int64       Size() ;

    /** @brief Moves within the file to an absolute location.
      * @param pos is the absolute desired position in the stream.
      *
      * Depends in UnBufferedSeek and should move the cursor to the desired position in the stream/file.
      */
    virtual bool        Seek(int64 pos);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 

    /** @brief Moves within the file relative to current location.
      * @param deltaPos is the desired relative position from the current.
      * 
      * Depends on UnBufferedSeek and should move the cursor to current+deltaPos in the stream.*/
    virtual bool        RelativeSeek(int32 deltaPos);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 
    
    /** @brief Returns current position.
      * @return the current position in the stream.
      *
      * Depends on UnBufferedPosition. */
    virtual int64       Position() ;
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 

    /** @brief Clip the stream size to a specified point.
      * @param size is the new desired size for the stream.
      *
      * Depends on UnBufferedSetSize. */
    virtual bool        SetSize(int64 size);
    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 

    // MULTISTREAM INTERFACE

    /** @brief Select the stream to read from. Switching may reset the stream to the start.ù
      * @param n is the number of the desired stream.
      * 
      * Depends on UnBufferedSwitch. */
    virtual bool        Switch(uint32 n);

    /** @brief Select the stream to read from. Switching may reset the stream to the start.
      * @param name is the name of the desired stream.
      * 
      * Depends on UnBufferedSwitch. */
    virtual bool        Switch(const char *name);

    /** @brief Remove an existing stream.
      * @param name is the name of the stream which should be removed.
      *
      * Depends on UnBufferedRemoveStream.
    */
    virtual bool        RemoveStream(const char *name);

};


#endif
