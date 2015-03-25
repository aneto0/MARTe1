#if !defined BASIC_STREAM_BUFFER
#define BASIC_STREAM_BUFFER

/**
    Replaces CStream and BufferedStream of BL1
    Inherits from pure virtual StreamInterface and does not resolve all pure virtual functions
   
    Needs the following implemented StreamInterface methods to function
        CanRead  if true Read
        CanWrite if true Write
        CanSeek  if true Seek 
    All the other StreamInterface methods need to be defined to instantiate.

    Only allows using GetC and PutC as it masks the whole StreamInterface
 
    It is a buffering mechanism for character streams
    It operates in 4 modes 
    1) Single buffer Output Mode
        Write method works Read does not
        Used for devices where in and out streams are separate (for instance console )
        Read method returns false
    2) Single buffer Input Mode
        Read method works Write does not
        Used for devices where in and out streams are separate (for instance console )
        Write method returns false
    5) Dual buffers Input and Output Mode
        Both Read and Write method works Seek does not 
        Used for devices where in and out streams are separate (for instance console )
        Write method returns false
    4) Single buffer IO Mode
        Read Write and Seek methods works and can be intermixed.
        Read consumes data and Write adds data


        There is an Position() index indicating where in the buffer we are raeding from 
        and where last Write() was made
        There is an AvailableToRead() method to see how many characters are available for read        
        before the ReadUpdate callBack function is used
        There is an AvailableToWrite() method to see how many characters can be written        
        before the WriteUpdate callBack function is used
        Used for devices where in and out streams are separate (for instance console )
*/
class BasicStreamBuffer: protected StreamInterface {
 
    /**
       Defines the operation mode and statsu of a basic stream
       one only can be set of the first 4.
    */
    struct OperatingModes{

        /**  double buffering with read supported
             stream CanRead  ~CanWrite
        */
        bool BufferingSeparatedRead:1;

        /**  double buffering with write supported
             stream CanWrite ~CanRead
        */
        bool BufferingSeparatedWrite:1;

        /**  single buffer 
             if True then the two above flags are meaningless
             stream CanSeek CanRead and CanWrite
        */
        bool SingleBuffering:1;

        /** buffer content has changed 
            only used for SingleBuffering
        */
        bool HasBeenWrittenTo:1;

        /** append 0 
            always only used for SingleBuffering
        */
        bool StringMode:1;
    };


    /**
        for read/mixed mode is from where to read/write data
    */
    uint32                  readAccessPosition;

    /**
        for read/mixed mode is how much data was filled in the buffer
    */
    uint32                  readBufferFillAmount

    /**
        for write mode is from where to write data
    */
    uint32                  writeAccessPosition;

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
        if buffer was modified also (mixed mode) then Flush data out first
        only called internally when no more space available 
    */
    bool FlushReadBuffer(){
 
        if ((operatingModes.HasBeenWrittenTo) && (readBufferFillAmount > 0)){
            if (!Seek(Position()-readBufferFillAmount)) return False;

            uint32 writeSize = readAccessPosition;
            if (!Write(readBuffer.Buffer(),writeSize)) return False;
            if (writeSize != readAccessPosition) return False;

            if (!Seek(Position()+readBufferFillAmount-readAccessPosition)) return False;
        }

        readAccessPosition = 0;
        readBufferFillAmount = readBuffer.BufferAllocatedSize();
        if (!Read(readBuffer.BufferReference(),readBufferFillAmount)) return False;
        
        return True;  
    }


    /**  
        empty writeBuffer
        only called internally when no more space available 
    */
    bool FlushWriteBuffer(){
        uint32 writeSize = writeAccessPosition;
        if (!Write(readBuffer.Buffer(),writeSize)) return False;
        if (writeSize != writeAccessPosition) return False;

        writeAccessPosition = 0;
        
        return True;  

    }

public:
    /// default constructor
    BasicStreamBuffer(){
        readAccessPosition    = 0;
        writeAccessPosition   = 0;
        readBufferFillAmount  = 0;
        operatingModes = {false, false, false, false, false };
    }

    /// default destructor
    virtual ~BasicStreamBuffer(){
    }

    /// simply write to buffer if space exist and if operatingModes allows
    inline bool         PutC(char c)
    {
        // single buffering mode ==> can Seek and can Read and can write
        if (operatingModes.SingleBuffering){

            // check if buffer needs updating and or saving            
            if (readAccessPosition >= readBufferFillAmount) FlushAndRefill();

            // do not try twice give up
            if (readAccessPosition >= readBufferFillAmount) return False;

            readBuffer.BufferReference()[readAccessPosition++] = c;

            // read buffer modified....
            operatingModes.HasBeenWrittenTo = True;

            return True;
        }

        // dual buffering mode with write enabled stream
        if (operatingModes.BufferingSeparatedWrite){
            // check if buffer needs updating
            if (writeAccessPosition >= writeBuffer.BufferAllocatedSize()) Flush();

            // no point to flush twice - give up.
            if (writeAccessPosition >= writeBuffer.BufferAllocatedSize()) return False;

            writeBuffer.BufferReference()[writeAccessPosition++] = c;
            
            return True;
	}

        uint32 size = 1;
        return (Write(&c, size) && (size == 1));

    }    

    /// simply write to buffer if space exist and if operatingModes allows
    inline bool         GetC(char &c)
        if ((operatingModes.SingleBuffering)        ||
            (operatingModes.BufferingSeparatedRead)){

            // check if buffer needs updating and or saving            
            if (readAccessPosition >= readBufferFillAmount) FlushAndRefill();

            // do not try twice give up
            if (readAccessPosition >= readBufferFillAmount) return False;

            c = readBuffer.BufferReference()[readAccessPosition++];

            return True;
	}

        uint32 size = 1;
        return (Read(&c, size) && (size == 1));
    }    

    /** 
         save any pending write operations 
         clear read buffer and adjust real Stream position to last read point
    */
    bool Flush(){
        if ((operatingModes.SingleBuffering)        ||
            (operatingModes.BufferingSeparatedRead)){

            if (operatingModes.HasBeenWrittenTo){
                if (!Seek(Position()-readBufferFillAmount)) return False;

                uint32 writeSize = readAccessPosition;
                if (!Write(readBuffer.Buffer(),writeSize)) return False;
                if (writeSize != readAccessPosition) return False;
            } else {
                if (!Seek(Position()-readBufferFillAmount+readAccessPosition)) return False;
            }
            readAccessPosition = 0;
            readBufferFillAmount = 0;
        }
        if (operatingModes.BufferingSeparatedWrite){
            uint32 writeSize = writeAccessPosition;
            if (!Write(readBuffer.Buffer(),writeSize)) return False;
            if (writeSize != writeAccessPosition) return False;

            writeAccessPosition = 0;
        }
        return True;       
    }

};







#endif