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
 * @file StreamString.h
 * @brief Basic implementation of strings. 
 *
 *  A basic implementation of a string.
 *  A replacement for dealing directly with mallocs
 *  StreamString simplified no confusing position 
 *  and derived from CharBuffer. 
 *  The string model is simple, It can do only:
 *  Construct as copy of char * or other BasicString
 *  Assign    as copy of char * or other BasicString
 *  Concatenate       a  char * or other BasicString
 *  Truncate to a given size
 *  Compare with      a  char *    
 *  Access as char * both read-only and read-write
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



/** @brief StreamString class. */
class StreamString: public BufferedStream {

private:    

	/** memory allocated buffer. */
	StreamStringIOBuffer 	buffer;	
   
protected: // methods to be implemented by deriving classes
    /** @brief Implemented because pure virtual. */
    virtual IOBuffer *GetInputBuffer();

    /** @brief Implemented because pure virtual. */
    virtual IOBuffer *GetOutputBuffer();

public: // usable constructors

    /** @brief Creates an empty string */
    StreamString(){}
    
    /** @brief Destructor */
    virtual ~StreamString() ;
    
public:
    /** 
     * @brief Reads data into buffer.
     * @param buffer is the output buffer.
     * @param size is the number of bytes (char) to copy.
     * @param msecTimeout is the timeout.
     * @param complete is a flag which specifies if the read operation is completed.
     * @return false in case of errors.   
     *
     *  As much as size byte are read, 
     *  actual read size is returned in size. (unless complete = True)
     *  msecTimeout is how much the operation should last - no more - if not any (all) data read then return false  
     *  timeout behaviour depends on class characteristics and sync mode.
    */
    virtual bool        Read(
                            char*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false);

    /**
     * @brief Write from a buffer to the string.
     * @param buffer is the input buffer.
     * @param size is the number of bytes (char) to copy.
     * @param msecTimeout is the timeout.
     * @param complete is a flag which specifies if the write operation is completed.
     * @return false in case of errors.
     *
     *  As much as size byte are written, 
     *  actual written size is returned in size. 
     *  msecTimeout is how much the operation should last.
     *  timeout behaviour depends on class characteristics and sync mode. 
    */
    virtual bool        Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false);
    
    /**
      * @brief Specifies if the string can be written.
      * @return true if write operations are allowed.
      */
    virtual bool        CanWrite() const ;

    /**
     * @brief Specifies if the string can be read.
     * @return true if read operations are allowed. */
    virtual bool        CanRead() const ;
    
    /**
     * @brief The size of the string.
     * @return the current string size. */
    virtual int64       Size() ;

    /**
     * @brief Moves within the string to an absolute location.
     * @param pos is the desired cursor position.
     * @return false in case of cursor out of ranges or other errors. */
    virtual bool        Seek(int64 pos);
    
    /** 
     * @brief Moves within the string relative to current location.
     * @param deltaPos is the desired gap from the current position.
     * @return false in case of cursor out of ranges or other errors. */
    virtual bool        RelativeSeek(int32 deltaPos);
    
    /** 
     * @brief Returns current position.
     * @return the current position. */
    virtual int64       Position() ;

    /**
     * @brief Clip the string size to a specified point
     * @param size The desired size of the buffer.
     * @return True if successful. False otherwise.
     */
    virtual bool        SetSize(int64 size);

    /** 
     * @brief Specifies if you can move the cursor.
     * @return true if you can move the cursor in the string. */
    virtual bool        CanSeek() const ;

    // MULTISTREAM INTERFACE
    
    /**
     * @brief Returns how many streams are available.
     * @return the number ov avaiable streams. */
    virtual uint32      NOfStreams();

    /**
     * @brief Select the stream to read from. Switching may reset the stream to the start.
     * @param n is the number of the desired stream.
     * @return false in case of errors.  */
    virtual bool        Switch(uint32 n);

    /**
     * @brief Select the stream to read from. Switching may reset the stream to the start.
     * @param name is the name of the desired stream.
     * @return false in case of errors. */
    virtual bool        Switch(const char *name);

    /** 
     * @brief Returns the number of the selected stream.
     * @return the number of the selected stream.
     */
    virtual uint32      SelectedStream();

    /**
     * @brief The name of a stream.
     * @param n is the number of the desired stream.
     * @param name is the output buffer which will contains the stream name.
     * @param nameSize is the size of the buffer.
     * @return false in case of errors.
     */
    virtual bool        StreamName(uint32 n,char *name,int nameSize) const ;

    /**
     * @brief Add a new stream to write to.
     * @param name is the name to be assigned to the new stream.
     * @return false in case of errors. */
    virtual bool        AddStream(const char *name);

    /**  
     * @brief Remove an existing stream.
     * @param name is the name of the stream to remove. */
    virtual bool        RemoveStream(const char *name);
    
public: // DIRECT ACCESS FUNCTIONS
      

    /**
     * @brief Read Only access to the internal buffer.
     * @return The pointer to the buffer.
     */
    inline const char *Buffer() const {
        return buffer.Buffer();
    }

    /**
     * @brief Read Write access top the internal buffer.
     * @return The pointer to the buffer
     */
    inline char *BufferReference() const {
        return buffer.BufferReference();
    }


    /**
     * @brief Returns a pointer to the tail of the buffer.
     * @param  ix the offset from the end of buffer. valid ranges is 0 to Size()-1.
     * @return pointer to the tail of the buffer.
     */
    inline const char *Tail(int32 ix) const {
    	if (ix > 0) 				return 0;
    	if ((ix - buffer.UsedSize() -1)< 0) 	return 0;
    	return buffer.BufferReference() + buffer.UsedSize() - ix - 1;
    }

public: // DIRECT MANIPULATION FUNCTIONS

    /**
     * @brief Copy a character into the StreamString buffer.
     * @param  c the character to be copied.
     * @return True if successful. False otherwise.
     */
    bool Copy(char c, bool append=false) {
        if (append){
        	buffer.Seek(buffer.UsedSize());
    	} else {
    		buffer.Empty();
    	} 
    	bool ret = buffer.PutC(c);
    	buffer.Terminate();
    	return ret;
    }

    /**
     * @brief Copy a string into the StreamString buffer.
     * @param  s The pointer to the string to be copied
     * @return True if successful. False otherwise.
     */
    bool Copy(const char *s, bool append=false) {
        if (s == NULL){
            return false;
        }
        uint32 size = StringHelper::Length(s);

        if (append){
        	buffer.Seek(buffer.UsedSize());
    	} else {
    		buffer.Empty();
    	} 
    	buffer.Write(s,size);
    	buffer.Terminate();
    	return true;
    }

    /**
     * @brief Copy a StreamString into a StreamString.
     * @param  s The StreamString to be copied.
     * @return True if successful. False otherwise.
     */
    bool Copy(const StreamString &s, bool append=false) {
        uint32 size = s.buffer.UsedSize();

        if (append){
        	buffer.Seek(buffer.UsedSize());
    	} else {
    		buffer.Empty();
    	} 

        buffer.Write(s.Buffer(),size);
    	buffer.Terminate();
    	return true;
    }
    
    /**
     * @brief Sets StreamString to be a copy of the input parameter.
     * @param c The character to copy.
     * @return True if successful. False otherwise.
     */
    inline bool operator=(char c) {
    	return Copy(c);
    }

    /**
     * @brief Sets StreamString to be a copy of the input parameter.
     * @param s The string to copy.
     * @return True if successful. False otherwise.
     */
    inline bool operator=(const char *s) {
        return Copy(s);
    }

    /**
     * @brief Sets StreamString to be a copy of the input parameter.
     * @param s The StreamString to copy.
     * @return True if successful. False otherwise.
     */
    inline bool operator=(const StreamString &s) {
        return Copy(s);
    }

    /**
     * @brief Concatenate the character to the string contained in the buffer.
     * @param  c The character to concatenate.
     * @return True if successful. False otherwise.
     */
    inline bool operator+=(const char c) {
    	return Copy(c,true);
    }

    /**
     * @brief Concatenate the string to the string contained in the buffer.
     * @param s The string to concatenate.
     * @return True if successful. False otherwise.
     */
    inline bool operator+=(const char *s) {
    	return Copy(s,true);
    }

    /**
     * @brief Concatenate the StreamString to the string contained in the buffer.
     * @param  s The StreamString to concatenate.
     * @return True if successful. False otherwise.
     */
    inline bool operator+=(StreamString &s) {
    	return Copy(s,true);
    }

    /**
     * @brief  Compare the buffer content with the input content.
     * @param s The buffer to be compared with.
     * @return True if the two buffers are the same. False otherwise.
     */
    inline bool operator==(StreamString &s) const {
        if (buffer.UsedSize() != s.buffer.UsedSize()){
            return false;
        }
        if (StringHelper::Compare(Buffer(), s.Buffer()) != 0){
            return false;
        }
        return true;
    }

    /**
     * @brief Compare the buffer content with the input content.
     * @param s The buffer to be compared with.
     * @return True if the two buffers are the same. False otherwise.
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

    /**
     * @brief Allows access to character within the buffer.
     * @param  pos The position in the buffer to be accessed.
     * @return 0 if the position is outside the buffer limits. The character at position pos otherwise.
     */
    inline char operator[](uint32 pos) {
        if (pos >= buffer.UsedSize()){
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
        if (x.UsedSize() == 0)
            return False;
        for (uint32 i = 0; i < (size - x.UsedSize() + 1); i++)
            if (memcmp(&buffer[i], x.Buffer(), x.UsedSize()) == 0)
                return True;
        return False;
    }
#endif
};

#endif

