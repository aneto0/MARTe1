#if !defined BASIC_STREAM
#define BASIC_STREAM

class BasicStream;

/// to implement 
typedef void(*BasicStreamNewBufferFN)(BasicStream &p);


/**
    Replaces CStream and BufferedStream of BL1
    It is a buffering mechanism for character streams
    It operates in 3 modes 
    1) Output Mode
        Only Write method works
        Used for devices where in and out streams are separate (for instance console )
        Read method returns false
    2) Input Mode
        Only Read method works
        Used for devices where in and out streams are separate (for instance console )
        Write method returns false
// NO    3) IO Mode
        Both Read and Write method works and can be intermixed.
        Read consumes data and Write adds data
        There is an Position() index indicating where in the buffer we are raeding from 
        and where last Write() was made
        There is an AvailableToRead() method to see how many characters are available for read        
        before the ReadUpdate callBack function is used
        There is an AvailableToWrite() method to see how many characters can be written        
        before the WriteUpdate callBack function is used
        Used for devices where in and out streams are separate (for instance console )
*/
class BasicStream: public CharBuffer {
 
    /**
       Defines the operation mode and statsu of a basic stream
    */
    struct OperatingModes{

        /// 
        bool CanReadFrom:1;

        /// 
        bool CanWriteTo:1;

        /// 
        bool HasBeenWrittenTo:1;
    };


// STREAMING

    OperatingMode           operatingModes;

    /**
        for read mode is from where to read data
        for write mode is from where to write data
    */
    uint32                  accessPosition;

    /// to update buffer 
    BasicStreamNewBufferFN  newBuffer;

public:

    /// forces a buffer update
    inline void Flush(){
        newBuffer(*this);
    }

    /// simply write to buffer if space exist and if operatingModes allows
    inline void Put(char c){
        if (operatingModes.CanWriteTo){
            /// check if buffer needs updating
            if (accessPosition >= BufferAllocatedSize()) Flush();

            // do we have space?
            if (accessPosition < BufferAllocatedSize()) {
                buffer[accessPosition++] = c;
                operatingModes.HasBeenWrittenTo = true;
            }
	}
    }    

    /// simply write to buffer if space exist and if operatingModes allows
    inline char Get(){
        if (operatingModes.CanReadFrom){
            /// check if buffer needs updating
            if (accessPosition >= BufferAllocatedSize()) Flush();

            // do we have space?
            if (accessPosition < BufferAllocatedSize()) {
                return buffer[accessPosition++];
            }
	}
    }    
};

const BasicStream::OperatingModes ReadMode = {true ,false,false};
const BasicStream::OperatingModes WriteMode= {false,true ,false};
const BasicStream::OperatingModes DualMode = {true ,true ,false};






#endif