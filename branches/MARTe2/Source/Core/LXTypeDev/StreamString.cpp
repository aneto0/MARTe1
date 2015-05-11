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
#include "GeneralDefinitions.h"
#include "StreamHelper.h"

///
StreamStringBuffer::StreamStringBuffer(StreamString &s):string(s){
	bufferPtr = BufferReference();
	maxAmount = BufferSize();
	if (maxAmount > 0){
		// remove one for the terminator 0
		maxAmount--;
	} 
	amountLeft = maxAmount;	
	fillLeft   = maxAmount;
}

~StreamStringBuffer::StreamStringBuffer(){

}

/** 
 *  desiredSize is the desired buffer size
 *  strings will fit upto desiredSize-1 
 *  sets the size of the buffer to be desiredSize or greater up next granularity
 */
void StreamStringBuffer::SetBufferAllocationSize(
		uint32 			desiredSize,
		uint32 			allocationGranularityMask 		= 0xFFFFFFFF){

	// save position    	
	uint32 position   = Position();
	uint32 stringSize = Size();
	
	// truncating
	if ((desiredSize-1) < stringSize){
		BufferReference()[desiredSize-1] = 0;
		stringSize = desiredSize-1;
	}
	// saturate index 
	if (position > stringSize) position = stringSize;  

	// reallocate buffer
	if (!CharBuffer::SetBufferAllocationSize(desiredSize,allocationGranularityMask)) {
		return false;
	}

	// update max size
	maxAmount  = BufferSize();
	if (maxAmount > 0){
		// remove one for the terminator 0
		maxAmount--;
	} 

	// update bufferPtr and amountLeft
	// stringSize is not changing here
	bufferPtr  = BufferReference() + position;
	amountLeft = maxAmount - position;
	fillLeft = maxAmount - stringSize;

	return true;

}

// read buffer private methods
/// if buffer is full this is called 
bool 		StreamStringBuffer::Flush(){

	// reallocate buffer
	// uses safe version of the function
	// implemented in this class
	if (!SetBufferAllocationSize(BufferSize()+1)) {
		return false;
	}

	return true;
}

/** 
 * loads more data into buffer and sets amountLeft and bufferEnd
 * READ OPERATIONS 
 * */
bool 		StreamStringBuffer::Refill(){
		// nothing to do.
		return true;
}

/**
        sets amountLeft to 0
        adjust the seek position of the stream to reflect the bytes read from the buffer
 * READ OPERATIONS 
 */
bool 		StreamStringBuffer::Resync(){
	// nothing to do.
	return true;
}    

/**
 * position is set relative to start of buffer
 */
bool        StreamStringBuffer::Seek(uint32 position){
	if (position >= Size()) {
		return false;
	}
	bufferPtr = BufferReference() + position;
	amountLeft = maxAmount - position; 
	return true;
}

/**
 * position is set relative to start of buffer
 */
bool        StreamStringBuffer::RelativeSeek(int32 delta){
	if (delta >= 0){
		//cannot seek beyond fillLeft
		if ((uint32)delta > (amountLeft-fillLeft)){
			/// maybe saturate at the end?
			return false;
		}
	} else 
		// cannot seek below 0
		if ((amountLeft - delta) > maxAmount){
			/// maybe saturate at the beginning?
			return false;
		}
	amountLeft -= delta;
	bufferPtr += delta;
	return true;
}    


/** Destructor */
~StreamString::StreamString() {
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
	return buffer.Read(buffer,size);
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
	bool ret = buffer.Write(buffer,size);
	Terminate();
	return ret;
	
}

/** whether it can be written into */
bool StreamString::CanWrite(){ 
	return true; 
};

/** whether it can be  read */
bool StreamString::CanRead(){ 
	return true; 
};

/** The size of the stream */
int64 StreamString::Size(){ 
	return buffer.Size(); 
}

/** Moves within the file to an absolute location */
bool StreamString::Seek(int64 pos){
	if (pos > buffer.Size()) {
		REPORT_ERROR(ParametersError,"pos out of range")
		return false;
	}
	
	return buffer.Seek((uint32)pos);
}

/** Moves within the file relative to current location */
bool StreamString::RelativeSeek(int32 deltaPos){
	return buffer.RelativeSeek((uint32)pos);
}

/** Returns current position */
int64 StreamString::Position() { 
	return buffer.Position(); 
}


/** Clip the string size to a specified point
 @param newStringSize The size of the buffer.
 @return True if successful. False otherwise.
 */
bool StreamString::SetSize(int64 size) = 0;
    buffer.SetStringBufferSize(size+1, 0xFFFFFFC0);
    return True;
}

/** can you move the pointer */
bool StreamString::CanSeek(){ return true; };


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



