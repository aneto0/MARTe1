/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id: ErrorManagement.h 3 2012-01-15 16:26:07Z aneto $
 *
**/


#if !defined BUFFERED_STREAM
#define BUFFERED_STREAM

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"


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
class BufferedStream: public StreamInterface {
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
       
protected: // methods to be implemented by deriving classes
    ///
    virtual IOBuffer &GetInputBuffer()= 0;

    ///
    virtual IOBuffer &GetOutputBuffer()= 0;

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
    virtual int64       UnBufferedSize()=0;

    /** Moves within the file to an absolute location */
    virtual bool        UnBufferedSeek(int64 pos)=0;

    /** Returns current position */
    virtual int64       UnBufferedPosition()=0;

    /** Clip the stream size to a specified point */
    virtual bool        UnBufferedSetSize(int64 size)=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        UnBufferedSwitch(uint32 n)=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        UnBufferedSwitch(const char *name)=0;
    
    virtual bool        UnBufferedRemoveStream(const char *name)=0;


   
protected:
    /// default constructor
    BufferedStream()
    {
    	operatingModes.canSeek      	= false;
    	operatingModes.mutexReadMode 	= false;
    	operatingModes.mutexWriteMode 	= false;
    	operatingModes.stringMode 		= false;
       
    }

    /// default destructor
    virtual ~BufferedStream();
      

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
        IOBuffer &ob = GetOutputBuffer();
        if (ob.BufferPtr() != NULL){
        	return ob.PutC(c);
        }
        	
        uint32 size = 1;
        return (UnBufferedWrite(&c, size) && (size == 1));
    }    

    /// simply read from buffer 
    inline bool         GetC(char &c) {

        // if in mutex mode switch to write mode
        if (operatingModes.mutexWriteMode) {
           if (!SwitchToReadMode()) return false;
        }

        IOBuffer &ib = GetInputBuffer();
        if (ib.BufferPtr() != NULL){
        	return ib.GetC(c);
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
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                completeWrite   = false);

    // NOTE: Implemented in .cpp but no need to have c- mangling functions as function will be normally acceessed via VT 


    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size();

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
    		                BufferedStream &    output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL);

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                            uint32              count,
                            const char *        terminator);

      
public: //  Methods to convert and print numbers and other objects 
    
    /**
     * Very powerful function to handle data conversion into a stream of chars
    */
    virtual bool Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor);

    /** 
         pars is a vector terminated by voidAnyType value
         format follows the TypeDescriptor::InitialiseFromString
         prints all data pointed to by pars
    */
    virtual bool PrintFormatted(const char *format, const AnyType pars[]);

};


#endif
