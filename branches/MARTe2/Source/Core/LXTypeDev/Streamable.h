#if !defined STREAMABLE
#define STREAMABLE

#include "TypeConversion.h"

extern "C"{

	bool StreamableSetBufferSize(
            Streamable & stream,  
            uint32 				readBufferSize, 
            uint32 				writeBufferSize);
}


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
class Streamable: public StreamInterface {
 
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

private:    
    /// set automatically on initialisation by calling of the Canxxx functions 
    OperatingMode           operatingModes;
    
    
protected: // read buffer and statuses

    /** starts empty 
        used for separate input operations or dual operations
        to be sized up by final class appropriately 
    */
    CharBuffer              readBuffer;
    
    /**
        for read mode is from where to read data from readBuffer
    */
    uint32                  readBufferAccessPosition;

    /**
        for read mode is how much data was filled in the buffer
    */
    uint32                  readBufferFillAmount;

protected: // mode switch methods
    
    /** 
        sets the readBufferFillAmount to 0
        adjust the seek position
        sets the mutexWriteMode
        does not check for mutexBuffering to be active
    */
    inline bool SwitchToWriteMode(){
        if (!ResyncReadBuffer()) return false;
        mutexWriteMode = true;
        mutexReadMode = false;
    }
    
    /** 
        Flushes output buffer
        resets mutexWriteMode
        does not refill the buffer nor check the mutexBuffering is active
    */
    inline bool SwitchToReadMode(){
        // adjust seek position
        if (!FlushWriteBuffer()) return false;
        mutexWriteMode = false;
        mutexReadMode = true; 
    }

protected: // read buffer protected methods

    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    inline bool ResyncReadBuffer(){
        if (readBufferFillAmount == 0) return true;
        // adjust seek position
        // in read mode the actual stream 
        // position is to the character after the buffer end
        if (!UnBufferedSeek (UnBufferedPosition()-readBufferFillAmount+readBufferAccessPosition)) return false;
        readBufferFillAmount = 0;
        readBufferAccessPosition = 0;
        return true;
    } 
    
    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    inline bool RefillReadBuffer(){
    	if (readBuffer.Buffer() == NULL) return false;
        // load next batch of data
        readBufferAccessPosition = 0;
        readBufferFillAmount = readBuffer.BufferAllocatedSize();
        return UnBufferedRead(readBuffer.BufferReference(),readBufferFillAmount);  
    }

private:    
    /// copies to buffer size bytes from the end of readBuffer
    void BufferRead(char *buffer, uint32 &size);
    // NOTE used privately by Read 


protected: // write buffer and statuses

    /** starts empty 
        used exclusively for separate output operations 
        to be sized up by final class appropriately 
    */
    CharBuffer              writeBuffer;

    /**
        for write mode is from where to write data
        or how much data was added
    */
    uint32                  writeBufferAccessPosition;

protected: // write buffer methods
    
    /**  
        empty writeBuffer
        only called internally when no more space available 
    */
    inline bool FlushWriteBuffer(TimeoutType         msecTimeout     = TTDefault){
    	// no buffering!
    	if (writeBuffer.Buffer()== NULL) return true;
    	// how much was written?
        uint32 writeSize = writeBufferAccessPosition;
        // write
        if (!UnBufferedWrite(writeBuffer.Buffer(),writeSize,msecTimeout,true)) return False;

        writeBufferAccessPosition = 0;
        
        return True;  
    }
private:
    /// copies buffer of size size at the end of writeBuffer
    void BufferWrite(const char *buffer, uint32 &size);
    // NOTE used privately by Read 

public:

    //
    friend bool StreamableSetBufferSize(
	            Streamable & 		stream,  
	            uint32 				readBufferSize, 
	            uint32 				writeBufferSize);

    /**
        sets appropriate buffer sizes and adjusts operatingModes
    */
    bool SetBufferSize(uint32 readBufferSize=0, uint32 writeBufferSize=0){
    	return StreamableSetBufferSize(*this, readBufferSize, writeBufferSize);
    }   
    
    /// default constructor
    Streamable(
    			uint32 readBufferSize=0, 
    			uint32 writeBufferSize=0)
    {
        readBufferAccessPosition    = 0;
        writeBufferAccessPosition   = 0;
        readBufferFillAmount        = 0;
    	operatingModes.canSeek      = false;
    	operatingModes.mutexReadMode = false;
    	operatingModes.mutexWriteMode = false;
    	operatingModes.stringMode = false;
        operatingModes.canSeek = bsb.CanSeek(); 
    	
        // mutex mode is enabled if CanSeek and both can Read and Write
    	// in that case the stream is single and bidirectional
        if (CanSeek() && CanWrite() && CanRead()) {
        	bsb.operatingModes.mutexWriteMode = true;
        }    	
        
        SetBufferSize(readBufferSize,writeBufferSize);
       
    }

    /// default destructor
    virtual ~Streamable(){
    	Flush();
    }


public:  // special methods for buffering
    
    /** 
         on dual separate buffering (CanSeek=False) just Flush output 
     
         on joint buffering (CanSeek= True) depending on read/write mode 
         either Resync or Flush
    */
    inline bool Flush(TimeoutType         msecTimeout     = TTDefault){
    	// mutexReadMode --> can seek so makes sense to resync
    	if (operatingModes.mutexReadMode){
    		return ResyncReadBuffer();
    	} 
         	
   		return FlushWriteBuffer();
    }


    /// simply write to buffer if space exist and if operatingModes allows
    inline bool         PutC(char c)
    {
        // if in mutex mode switch to write mode
        if (operatingModes.mutexReadMode) {
           if (!SwitchToWriteMode()) return false;
        }
        
        // if write buffer exists then we have write buffer capabilities
        if (writeBuffer.Buffer() != NULL){
            // check if buffer needs updating
            if (writeBufferAccessPosition >= writeBuffer.BufferAllocatedSize()){ 
                // on success the access position is set to 0
                if (!FlushWriteBuffer()) return false;
            }
            // write data
            writeBuffer.BufferReference()[writeAccessPosition++] = c;
            
            return True;
        }

        uint32 size = 1;
        return (UnBufferedWrite(&c, size) && (size == 1));
    }    

    /// simply read from buffer 
    inline bool         GetC(char &c)

        // if in mutex mode switch to write mode
        if (operatingModes.mutexWriteMode) {
           if (!SwitchToReadMode()) return false;
        }

        // if read buffer exists then we are either in joint buffer or separated read mode enabled
        if (readBuffer.Buffer() != NULL){

            // check if buffer needs updating and or saving            
            if (readBufferAccessPosition >= readBufferFillAmount) {
                if (!RefillReadBuffer()) return false;
            }

            c = readBuffer.BufferReference()[readAccessPosition++];

            return True;
        }

        uint32 size = 1;
        return (UnBufferedRead(&c, size) && (size == 1));
    }    


public:  // replaced StreamInterface methods

    // PURE STREAMING

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
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeWrite   = false);

    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 


    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size()    {
    	// just commit all pending changes if any
    	// so stream size will be updated     	
    	Flush();
    	// then call Size from unbuffered stream 
    	return UnBufferedSize(); 
    }

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
    virtual bool        Switch(uint32 n)
    {
        Flush();
    	return UnBufferedSwitch(n);
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name)
    {
        Flush();
        return UnBufferedSwitch(name);
    }

    /**  remove an existing stream .
        current stream cannot be removed 
    */
    virtual bool        RemoveStream(const char *name)
    {
    	Flush();
        return UnBufferedRemoveStream(name);
    }

public:  // auxiliary functions based on buffering
    
    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters and those that are also terminators at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        skipCharacters=NULL is equivalent to skipCharacters = terminator
        {BUFFERED}    */
    virtual bool        GetToken(
                            char *              outputBuffer,
                            const char *        terminator,
                            uint32              outputBufferSize,
                            char *              saveTerminator,
                            const char *        skipCharacters);

    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters and those that are also terminators at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        skipCharacters=NULL is equivalent to skipCharacters = terminator
        {BUFFERED}
        A character can be found in the terminator or in the skipCharacters list  in both or in none
        0) none                 the character is copied
        1) terminator           the character is not copied the string is terminated
        2) skip                 the character is not copied
        3) skip + terminator    the character is not copied, the string is terminated if not empty
    */
    virtual bool        GetToken(
    		                Streamable &  output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL);

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                            uint32              count,
                            const char *        terminator);

    
    /** Extract a line */
    inline bool 		GetLine(
    						char *				outputBuffer,
    						uint32 				outputBufferSize,
    						bool 				skipTerminators=True){
        const char *skipCharacters = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skipCharacters = "\r";
#else
        if (!skipTerminators) skipCharacters = "";
#endif
        return GetToken(outputBuffer,"\n",outputBufferSize,NULL,skipCharacters);
    }

    /** @param skipTerminators will skip an empty line or any part of a line termination */
    inline bool 		GetLine(
    						Streamable &	output,
    						bool 				skipTerminators=True){
        const char *skipCharacters = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skipCharacters = "\r";
#else
        if (!skipTerminators) skipCharacters = "";
#endif
        return GetToken(output,"\n",NULL,skipCharacters);
    }

    /**
     * Very powerful function to handle data conversion into a stream of chars
    */
    virtual bool Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor);

    /** 
         pars is a vector terminated by voidAnyType value
         format follows the TypeDescriptor::InitialiseFromString
         prints all data pointed to by pars
    */
    inline bool PrintFormatted(const char *format, const AnyType pars[]){
    	// indicates active parameter
    	int parsIndex = 0;
    	// checks silly parameter
    	if (format == NULL) return false;
    	
    	// loops through parameters
    	while(1){
    		// scans for % and in the meantime prints what it encounters
    		while ((*format !=0) && (*format != '%')) {
    			if (!PutC(*format)) return false;
    			format++;
    		}
    		// end of format
    		if (*format == 0) return true;
    		
    		// if not end then %
    		// keep on parsing format to build a FormatDescriptor
    		FormatDescriptor fd;
    		if (!fd.InitialiseFromString(format)) return false;
    		
    		// if void simply skip and continue
    		if (!pars[parIndex].IsVoid()){
    		    // use it to process parameters
    		    if (!Print(pars[parsIndex++], fd) return false
    		}
    	}
        // never comes here!
    	return false;
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1){
    	AnyType pars[2] = { par1,voidAnyType};
    	return PrintFormatted(format, const AnyType[] pars);
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1, const AnyType& par2){
    	AnyType pars[3] = { par1,par2,voidAnyType}; 
    	return PrintFormatted(format, const AnyType[] pars);
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1, const AnyType& par2, const AnyType& par3){
    	AnyType pars[4] = { par1,par2,par3,voidAnyType}; 
    	return PrintFormatted(format, const AnyType[] pars);
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1, const AnyType& par2, const AnyType& par3, const AnyType& par4){
    	AnyType pars[5] = { par1,par2,par3,par4,voidAnyType}; 
    	return PrintFormatted(format, const AnyType[] pars);
    }

};







#endif