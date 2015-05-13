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



/** 
 * @file
 * Basic implementation of strings 
 */
#ifndef STREAM_STRING_H
#define STREAM_STRING_H

#include "GeneralDefinitions.h"
#include "Memory.h"
#include "CharBuffer.h"
#include "StreamStringIOBuffer.h"
#include "BufferedStream.h"
#include "TimeoutType.h"
#include "StringHelper.h"




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
class StreamString: public BufferedStream {

private:    

	///
	StreamStringIOBuffer 	buffer;	
   
protected: // methods to be implemented by deriving classes
    ///
    virtual IOBuffer *GetInputBuffer();

    ///
    virtual IOBuffer *GetOutputBuffer();

public: // usable constructors

    /** Creates an empty string */
    StreamString():buffer(*this){}
    
    /** Destructor */
    virtual ~StreamString() ;
    
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
    virtual bool        CanWrite() const ;

    /** whether it can be  read */
    virtual bool        CanRead() const ;
    
    /** The size of the stream */
    virtual int64       Size() ;

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos);
    
    /** Moves within the file relative to current location */
    virtual bool        RelativeSeek(int32 deltaPos);
    
    /** Returns current position */
    virtual int64       Position() ;

    /** Clip the string size to a specified point
     @param newStringSize The size of the buffer.
     @return True if successful. False otherwise.
     */
    virtual bool        SetSize(int64 size);

    /** can you move the pointer */
    virtual bool        CanSeek() const ;

    // MULTISTREAM INTERFACE
    
    /** how many streams are available */
    virtual uint32      NOfStreams();

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n);

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name);

    /** how many streams are available */
    virtual uint32      SelectedStream();

    /** the name of the stream we are using */
    virtual bool        StreamName(uint32 n,char *name,int nameSize) const ;

    /**  add a new stream to write to. */
    virtual bool        AddStream(const char *name);

    /**  remove an existing stream . */
    virtual bool        RemoveStream(const char *name);
    
public: // DIRECT ACCESS FUNCTIONS
      

    /** Read Only access to the internal buffer
     @return The pointer to the buffer
     */
    inline const char *Buffer() const {
        return buffer.Buffer();
    }

    /** Read Write access top the internal buffer
     @return The pointer to the buffer
     */
    inline char *BufferReference() const {
        return buffer.BufferReference();
    }


    /** Returns a pointer to the tail of the buffer.
     @param  ix the offset from the end of buffer. valid ranges is 0 to Size()-1
     @return pointer to the tail of the buffer
     */
    inline const char *Tail(int32 ix) const {
    	if (ix > 0) 				return 0;
    	if ((ix - buffer.Size() -1)< 0) 	return 0;
    	return buffer.BufferReference() + buffer.Size() - ix - 1;
    }

public: // DIRECT MANIPULATION FUNCTIONS

    /** Copy a character into the StreamString buffer.
     @param  c the character to be copied
     @return True if successful. False otherwise.
     */
    bool Copy(char c, bool append=false) {
        if (append){
        	buffer.Seek(buffer.Size());
    	} else {
    		buffer.Empty();
    	} 
    	bool ret = buffer.PutC(c);
    	buffer.Terminate();
    	return ret;
    }

    /** Copy a string into the StreamString buffer.
     @param  s The pointer to the string to be copied
     @return True if successful. False otherwise.
     */
    bool Copy(const char *s, bool append=false) {
        if (s == NULL){
            return false;
        }
        uint32 size = StringHelper::Length(s);

        if (append){
        	buffer.Seek(buffer.Size());
    	} else {
    		buffer.Empty();
    	} 
    	buffer.Write(s,size);
    	buffer.Terminate();
    	return true;
    }

    /** Copy a StreamString into a StreamString.
     @param  s The StreamString to be copied
     @return True if successful. False otherwise.
     */
    bool Copy(const StreamString &s, bool append=false) {
        uint32 size = s.buffer.Size();

        if (append){
        	buffer.Seek(buffer.Size());
    	} else {
    		buffer.Empty();
    	} 

        buffer.Write(s.Buffer(),size);
    	buffer.Terminate();
    	return true;
    }
    
    /** Sets StreamString to be a copy of the input parameter.
     @param c The character to copy
     @return True if successful. False otherwise.
     */
    inline bool operator=(char c) {
    	return Copy(c);
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
    	return Copy(c,true);
    }

    /** Concatenate the string to the string contained in the buffer
     @param  s The string to concatenate
     @return True if successful. False otherwise.
     */
    inline bool operator+=(const char *s) {
    	return Copy(s,true);
    }

    /** Concatenate the StreamString to the string contained in the buffer
     @param  s The StreamString to concatenate
     @return True if successful. False otherwise.
     */
    inline bool operator+=(StreamString &s) {
    	return Copy(s,true);
    }

    /** Compare the buffer content with the input content
     @param s The buffer to be compared with
     @return True if the two buffers are the same. False otherwise.
     */
    inline bool operator==(StreamString &s) const {
        if (buffer.Size() != s.buffer.Size()){
            return false;
        }
        if (StringHelper::Compare(Buffer(), s.Buffer()) != 0){
            return false;
        }
        return true;
    }

    /** Compare the buffer content with the input content
     @param s The buffer to be compared with
     @return True if the two buffers are the same. False otherwise.
     */
    inline bool operator==(const char *s) const {
        if (s == NULL){
            return false;
        }
        if (StringHelper::Compare(Buffer(), s) != 0){
            return false;
        }
        return true;
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
        if (pos >= buffer.Size()){
            return 0; 
        }
        return buffer.BufferReference()[pos];
    }
#if 0
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
#endif
};

#endif

