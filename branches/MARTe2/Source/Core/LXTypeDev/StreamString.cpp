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

#include "StreamString.h"
#include "ErrorManagement.h"
#include "StreamHelper.h"
#include "StreamWrapperIOBuffer.h"


/** Destructor */
StreamString::~StreamString() {
}

///
IOBuffer *StreamString::GetInputBuffer() {
	return &buffer;
}

///
IOBuffer *StreamString::GetOutputBuffer() {
	return &buffer;
}


/** 
    Reads data into buffer. 
    As much as size byte are read, 
    actual read size is returned in size. (unless complete = True)
    msecTimeout is how much the operation should last - no more - if not any (all) data read then return false  
    timeout behaviour depends on class characteristics and sync mode.
*/
bool StreamString::Read(
                        char*               buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout,
                        bool                complete){
	this->buffer.Read(buffer,size);
	return true;
}

/** 
    Write data from a buffer to the stream. 
    As much as size byte are written, 
    actual written size is returned in size. 
    msecTimeout is how much the operation should last.
    timeout behaviour depends on class characteristics and sync mode. 
*/
bool StreamString::Write(
                        const char*         buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout,
                        bool                complete){
	this->buffer.Write(buffer,size);
	
	this->buffer.Terminate();
	return true;
	
}

/** whether it can be written into */
bool StreamString::CanWrite()const { 
	return true; 
};

/** whether it can be  read */
bool StreamString::CanRead()const { 
	return true; 
};

/** The size of the stream */
int64 StreamString::Size(){ 
	return buffer.UsedSize(); 
}

/** Moves within the file to an absolute location */
bool StreamString::Seek(int64 pos){
	if (pos > buffer.UsedSize()) {
		REPORT_ERROR(ParametersError,"pos out of range")
		buffer.Seek(buffer.UsedSize());
		return false;
	}
	
	return buffer.Seek((uint32)pos);
}

/** Moves within the file relative to current location */
bool StreamString::RelativeSeek(int32 deltaPos){
	return buffer.RelativeSeek(deltaPos);
}

/** Returns current position */
int64 StreamString::Position() { 
	return buffer.Position(); 
}


/** Clip the string size to a specified point
 @param newStringSize The size of the buffer.
 @return True if successful. False otherwise.
 */
bool StreamString::SetSize(int64 size){
    buffer.SetBufferAllocationSize(size+1, 0xFFFFFFC0);
    return True;
}

/** can you move the pointer */
bool StreamString::CanSeek() const {
    return true; 
};


/**
 * @brief Copy a character into the StreamString buffer.
 * @param  c the character to be copied.
 * @return True if successful. False otherwise.
 */
bool StreamString::AppendOrSet(char c, bool append) {
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
bool StreamString::AppendOrSet(const char *s, bool append) {
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
bool StreamString::AppendOrSet(const StreamString &s, bool append) {
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

/** Checks if a char is in the string
 @param c The character to look for.
 @return >0 the first position if found. -1 otherwise.
 */
int32 StreamString::Locate(char c) const {
	// Stream::Size is not const!
    if (buffer.UsedSize() == 0){
        return -1;
    }
	const char *p = StringHelper::SearchChar(Buffer(), c);
	if (p == NULL) return -1;
	
	return (int32) (p -Buffer());
}

/** Checks if a string is contained in the string.
 @param x The string to look for.
 @return >0 the first position if found. -1 otherwise.
 */
int32 StreamString::Locate(const StreamString &x) const {
    if (x.buffer.UsedSize() == 0){
        return False;
    }
    if (x.buffer.UsedSize() > buffer.UsedSize()){
        return False;
    }
	const char *p = StringHelper::SearchString(Buffer(), x.Buffer());
	if (p == NULL) return -1;
	
	return (int32) (p -Buffer());
}



