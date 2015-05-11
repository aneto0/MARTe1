/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/

class StreamString;

/// Read buffer Mechanism for Streamable
class SSReadBuffer:public InputBuffer{
private:
	///
	StreamString &string;
       
public: // read buffer private methods

    ///
	SSReadBuffer(StreamString &s):string(s){
    	bufferPtr = s.Buffer();
//    	this->stream = stream;
    }

    /**  
        refill readBuffer
        assumes that the read position is now at the end of buffer
    */
    virtual bool 		Refill(TimeoutType         msecTimeout     = TTDefault){return false;}
    
    /**
        sets the readBufferFillAmount to 0
        adjust the seek position
    */
    virtual bool 		Resync(TimeoutType         msecTimeout     = TTDefault){return false;}    
  
};


/// Read buffer Mechanism for Streamable
class SSWriteBuffer:public OutputBuffer{
private:
	///
	StreamString &string;

public:

    ///
	SSWriteBuffer(Streamable &s):string(s){
    	bufferPtr = s.BufferReference();
    }
	
	/**  
	    empty writeBuffer
	    only called internally when no more space available 
	*/
	virtual bool Flush(TimeoutType         msecTimeout     = TTDefault){return false;}

    	return true;
	}
};


/** 
 * @file
 * Basic implementation of strings 
 */
#ifndef STREAM_STRING_H
#define STREAM_STRING_H

#include "GeneralDefinitions.h"
#include "Memory.h"
#include "CharBuffer.h"
#include "StreamInterface.h"

/**
    A basic implementation of a string.
    A replacement for dealing directly with mallocs
    StreamString simplified no confusing position 
           and derived from CharBuffer 
    The string model is simple, It can do only:
    Construct as copy of char * or other BasicString
    Assign    as copy of char * or other BasicString
    Concatenate       a  char * or other BasicString
    Truncate to a given size
    Compare with      a  char *    
    Access as char * both read-only and read-write
*/
class StreamString: protected IOBuffer,public BufferedStream {

private:    

    /** the size of the used memory block -1 (It excludes the 0)  */
///    uint32 size; replaced by maxAmount
    
    /** */ 
    int64 position;

protected: // methods to be implemented by deriving classes
    ///
    virtual InputBuffer &GetInputBuffer(){
    	return *this;
    }

    ///
    virtual OutputBuffer &GetOputBuffer(){
    	return *this;
    }
private:
    /** sets the size of the buffer so that to fit a fitStringSize
        accounts for string terminator
    */
    inline void SetStringBufferSize(uint32 fitStringSize){
        uint32 desiredBufferSize = fitStringSize+1;
        // 32 bytes granularity
        CharBuffer::SetBufferSize(desiredBufferSize, 0xFFFFFFE0);
    }

    /** used for constructors */
    void InitMembers() {
        size = 0;
        position = 0;
        SetStringBufferSize(0);
    }

    /** used for destructors */
    void FinishMembers() {
        size = 0;
    }

public: // usable constructors

    /** Creates an empty string */
    inline BasicString() {
        InitMembers();
    }

    /** Creates a StreamString as a copy of a StreamString.
     @param x The StreamString to use for initialisation
     */
    inline BasicString(const StreamString &x) {
        InitMembers();
        *this = x;
    }

    /** Creates a StreamString as a copy of string
     @param x The pointer to the string to use for initialisation
     */
    inline BasicString(const char *x) {
        InitMembers();
        *this = x;
    }

    /** Destructor */
    virtual ~BasicString() {
        FinishMembers();
    }

public:
    /** 
        Reads data into buffer. 
        As much as size byte are read, 
        actual read size is returned in size. (unless complete = True)
        msecTimeout is how much the operation should last - no more - if not any (all) data read then return false  
        timeout behaviour depends on class characteristics and sync mode.
    */
    virtual bool        Read(
                            char*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false);

    /** 
        Write data from a buffer to the stream. 
        As much as size byte are written, 
        actual written size is returned in size. 
        msecTimeout is how much the operation should last.
        timeout behaviour depends on class characteristics and sync mode. 
    */
    virtual bool        Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false);
    
    /** whether it can be written into */
    virtual bool        CanWrite(){ return true; };

    /** whether it can be  read */
    virtual bool        CanRead(){ return true; };
    
    /** The size of the stream */
    virtual int64       Size(){ return size; }

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos){
    	if ((pos < size) && (pos >=0)){
    		position = pos;
    		return true;
    	} else {
    		REPORT_ERROR(ParametersError,"pos out of range")
    		return false;
    	}
    }
    
    /** Moves within the file relative to current location */
    virtual bool        RelativeSeek(int32 deltaPos){
    	return Seek(position+deltaPos);
    }
    
    /** Returns current position */
    virtual int64       Position() { return position; }

    /** Clip the string size to a specified point
     @param newStringSize The size of the buffer.
     @return True if successful. False otherwise.
     */
    virtual bool        SetSize(int64 size) = 0;
        SetStringBufferSize(size);
        uint32 maxSize = BufferAllocatedSize() - 1;
        if (size > maxSize) size = maxSize;
        buffer[size] = 0;
        
        if (position > size) position = size;
        this->size = size;
        
        return True;
    }

    /** can you move the pointer */
    virtual bool        CanSeek(){ return true; };

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
    		                StreamInterface &  	output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL);

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                            uint32              count,
                            const char *        terminator); 
    
    /**
     * Very powerful function to handle data conversion into a stream of chars
    */
    virtual bool 		Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor);

    /** 
         pars is a vector terminated by voidAnyType value
         format follows the TypeDescriptor::InitialiseFromString
         prints all data pointed to by pars
    */
    virtual bool 		PrintFormatted(const char *format, const AnyType pars[]);
    
    
public: // DIRECT ACCESS FUNCTIONS
      

    /** Read Only access to the internal buffer
     @return The pointer to the buffer
     */
    inline const char *Buffer() const {
        return buffer;
    }

    /** Read Write access top the internal buffer
     @return The pointer to the buffer
     */
    inline char *BufferReference() const {
        return buffer;
    }


    /** Returns a pointer to the tail of the buffer.
     @param  ix the offset from the end of buffer
     @return pointer to the tail of the buffer
     */
    inline const char *Tail(int32 ix) const {
        return buffer + size - ix - 1;
    }

public: // DIRECT MANIPULATION FUNCTIONS

    /** Copy a character into the StreamString buffer.
     @param  c the character to be copied
     @return True if successful. False otherwise.
     */
    bool Copy(char c) {
        uint32 wsize = 1;
        size = 0;
        bool ret = BSWrite(*this, &c, 0, wsize);
        return ret;
    }

    /** Copy a string into the StreamString buffer.
     @param  s The pointer to the string to be copied
     @return True if successful. False otherwise.
     */
    bool Copy(const char *s) {
        if (s == NULL)
            return False;
        uint32 wsize = strlen(s);
        size = 0;
        bool ret = BSWrite(*this, s, 0, wsize);
        return ret;
    }

    /** Copy a StreamString into a StreamString.
     @param  s The StreamString to be copied
     @return True if successful. False otherwise.
     */
    bool Copy(const BasicString &s) {
        uint32 wsize = s.size;
        size = 0;
        bool ret = BSWrite(*this, s.Buffer(), 0, wsize);
        return ret;
    }
    
    /** Sets StreamString to be a copy of the input parameter.
     @param c The character to copy
     @return True if successful. False otherwise.
     */
    inline bool operator=(char c) {
    	size = 1;
    	position = 0;
    	buffer[0] = c;
    	buffer[1] = 0;
        return ;
    }

    /** Sets StreamString to be a copy of the input parameter.
     @param s The string to copy
     @return True if successful. False otherwise.
     */
    inline bool operator=(const char *s) {
        return Copy(s);
    }

    /** Sets StreamString to be a copy of the input parameter.
     @param s The StreamString to copy
     @return True if successful. False otherwise.
     */
    inline bool operator=(const StreamString &s) {
        return Copy(s);
    }

    /** Concatenate the character to the string contained in the buffer
     @param  c The character to concatenate
     @return True if successful. False otherwise.
     */
    inline bool operator+=(const char c) {
        uint32 wsize = 1;
        char temp = c;
        return BSWrite(*this, &temp, size, wsize);
    }

    /** Concatenate the string to the string contained in the buffer
     @param  s The string to concatenate
     @return True if successful. False otherwise.
     */
    inline bool operator+=(const char *s) {
        if (s == NULL)
            return False;
        uint32 wsize = strlen(s);
        return BSWrite(*this, s, size, wsize);
    }

    /** Concatenate the StreamString to the string contained in the buffer
     @param  s The StreamString to concatenate
     @return True if successful. False otherwise.
     */
    inline bool operator+=(StreamString &s) {
        uint32 wsize = s.Size();
        return BSWrite(*this, s.Buffer(), size, wsize);
    }

    /** Compare the buffer content with the input content
     @param s The buffer to be compared with
     @return True if the two buffers are the same. False otherwise.
     */
    inline bool operator==(StreamString &s) const {
        if (size != s.size)
            return False;
        if (strcmp(buffer, s.buffer) != 0)
            return False;
        return True;
    }

    /** Compare the buffer content with the input content
     @param s The buffer to be compared with
     @return True if the two buffers are the same. False otherwise.
     */
    inline bool operator==(const char *s) const {
        if (s == NULL)
            return False;
        if (strcmp(buffer, s) != 0)
            return False;
        return True;
    }

    inline bool operator!=(StreamString &s) const {
        return !((*this) == s);
    }

    inline bool operator!=(const char *s) const {
        return !((*this) == s);
    }

    /** Allows access to character within the buffer
     @param  pos The position in the buffer to be accessed.
     @return 0 if the position is outside the buffer limits. The character at position pos otherwise.
     */
    inline char operator[](uint32 pos) {
        if (pos >= size)
            return 0; // was -1 ?? Anton ??
        return buffer[pos];
    }

    /** Checks if a char is in the string
     @param c The character to look for.
     @return True if found. False otherwise.
     */
    inline bool In(char c) const {
        for (uint32 i = 0; i < size; i++)
            if (buffer[i] == c)
                return True;
        return False;
    }

    /** Checks if a string is contained in the string.
     @param x The string to look for.
     @return True if the string is found. False otherwise.
     */
    inline bool In(StreamString &x) const {
        if (x.Size() == 0)
            return False;
        for (uint32 i = 0; i < (size - x.Size() + 1); i++)
            if (memcmp(&buffer[i], x.Buffer(), x.Size()) == 0)
                return True;
        return False;
    }

};

#endif

