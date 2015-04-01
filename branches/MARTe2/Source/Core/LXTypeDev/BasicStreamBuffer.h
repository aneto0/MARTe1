#if !defined BASIC_STREAM_BUFFER
#define BASIC_STREAM_BUFFER

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
        operatingMode.mutexWriteBufferActive and
        operatingMode.mutexReadBufferActive  determines whether the read 
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
class BasicStreamBuffer: protected StreamInterface {
 
    /**
       Defines the operation mode and statsu of a basic stream
       one only can be set of the first 4.
    */
    struct OperatingModes{


        /** writeBuffer is the active one.
        */
        bool mutexReadBufferActive:1;

        /** writeBuffer is the active one.
        */
        bool mutexWriteBufferActive:1;

        /** append 0 
            always only used for SingleBuffering
        */
        bool stringMode:1;

        /**  
             mutually exclusive buffering 
             active when CanSeek is set and both CanRead and CanWrite
             only one buffer can be used at a time
             Flush is executed in between changes
        */
        bool MutexBuffering(){
            return (mutexReadBufferActive || mutexWriteBufferActive) 
        }
    };


    /**
        for read mode is from where to read data from readBuffer
    */
    uint32                  readBufferAccessPosition;

    /**
        for read mode is how much data was filled in the buffer
    */
    uint32                  readBufferFillAmount

    /**
        for write mode is from where to write data
        or how much data was added
    */
    uint32                  writeBufferAccessPosition;

protected:
    
    /// to be set by final class appropriately 
    OperatingMode           operatingModes;

    /** starts empty 
        used for separate input operations or dual operations
        to be sized up by final class appropriately 
    */
    CharBuffer              readBuffer;

    /** starts empty 
        used exclusively for separate output operations 
        to be sized up by final class appropriately 
    */
    CharBuffer              writeBuffer;


    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
        no check will be performed here on that
    */
    inline bool RefillReadBuffer(){

        // load next batch of data
        readBufferAccessPosition = 0;
        readBufferFillAmount = readBuffer.BufferAllocatedSize();
        return Read(readBuffer.BufferReference(),readBufferFillAmount);  
    }

    /**  
        empty writeBuffer
        only called internally when no more space available 
    */
    inline bool FlushWriteBuffer(){
        
        uint32 writeSize = writeBufferAccessPosition;
        if (!CompleteWrite(readBuffer.Buffer(),writeSize)) return False;

        writeBufferAccessPosition = 0;
        
        return True;  

    }
    
    /** 
        sets the readBufferFillAmount to 0
        adjust the seek position
        sets the mutexWriteBufferActive
        does not check for mutexBuffering to be active
    */
    inline bool SwitchToWriteMode(){
        // adjust seek position
        if (!Seek (Position()-readBufferFillAmount+readBufferAccessPosition)) return false;
 
        readBufferFillAmount = 0;
        readBufferAccessPosition = 0;
        mutexWriteBufferActive = true;
        mutexReadBufferActive = false;
    }

    /** 
        Flushes output buffer
        resets mutexWriteBufferActive
        does not refill the buffer nor check the mutexBuffering is active
    */
    bool SwitchToReadMode(){
        // adjust seek position
        if (!FlushWriteBuffer()) return false;
        mutexWriteBufferActive = false;
        mutexReadBufferActive = true; 
    }


public:
    /// default constructor
    BasicStreamBuffer(){
        readBufferAccessPosition    = 0;
        writeBufferAccessPosition   = 0;
        readBufferFillAmount        = 0;
        operatingModes = {false, false, false };
    }

    /// default destructor
    virtual ~BasicStreamBuffer(){
    }

    /// simply write to buffer if space exist and if operatingModes allows
    inline bool         PutC(char c)
    {
        // if in mutex mode switch to write mode
        if (mutexReadBufferActive) {
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
        return (Write(&c, size) && (size == 1));
    }    

    /// simply read from buffer 
    inline bool         GetC(char &c)

        // if in mutex mode switch to write mode
        if (mutexWriteBufferActive) {
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
        return (Read(&c, size) && (size == 1));
    }    

    /** 
         saves any pending write operations 
    */
    bool Flush(){
        // no data
        if (writeAccessPosition == 0) return true;
 
        return FlushWriteBuffer();
    }


};







#endif