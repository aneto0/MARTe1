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

#include "Streamable.h"
#include "ErrorManagement.h"
#include "StringHelper.h"
#include "StreamHelper.h"


/**
    sets the readBufferFillAmount to 0
    adjust the seek position
*/
bool StreamableReadBuffer::ResyncReadBuffer(Streamable &stream){
    if (readBufferFillAmount == 0) return true;
    // adjust seek position
    // in read mode the actual stream 
    // position is to the character after the buffer end
    if (!stream.UnBufferedSeek (stream.UnBufferedPosition()-readBufferFillAmount+readBufferAccessPosition)) return false;
    readBufferFillAmount = 0;
    readBufferAccessPosition = 0;
    return true;
} 

/**  
    refill readBuffer
    assumes that the read position is now at the end of buffer
*/
bool StreamableReadBuffer::RefillReadBuffer(Streamable &stream){
	if (BufferReference() == NULL) return false;
    // load next batch of data
    readBufferAccessPosition = 0;
    readBufferFillAmount = BufferSize();
    return stream.UnBufferedRead(BufferReference(),readBufferFillAmount);  
}

/// copies to buffer size bytes from the end of readBuffer
void StreamableReadBuffer::BufferRead(char *buffer, uint32 &size){

    uint32 spaceLeft = readBufferFillAmount - readBufferAccessPosition;
    
    // clip to available space
    if (size > spaceLeft) size = spaceLeft;
	
	    // fill the buffer with the remainder 
	if (size > 0) MemoryCopy(buffer, Buffer()+readBufferAccessPosition,size);
    readBufferAccessPosition+=size;
}


/**  
    empty writeBuffer
    only called internally when no more space available 
*/
bool StreamableWriteBuffer::FlushWriteBuffer(Streamable &stream,TimeoutType         msecTimeout  ){
	// no buffering!
	if (Buffer()== NULL) return true;
	// how much was written?
    uint32 writeSize = writeBufferAccessPosition;
    // write
    if (!stream.UnBufferedWrite(Buffer(),writeSize,msecTimeout,true)) return False;

    writeBufferAccessPosition = 0;
    
    return True;  
}

/// copies buffer of size size at the end of writeBuffer
void StreamableWriteBuffer::BufferWrite(const char *buffer, uint32 &size){

	// recalculate how much we can write
	uint32 spaceLeft = BufferSize() - writeBufferAccessPosition;

	// clip to spaceLeft
	if (size > spaceLeft) size = spaceLeft;

	// fill the buffer with the remainder 
	MemoryCopy(BufferReference()+writeBufferAccessPosition,buffer,size);
	writeBufferAccessPosition+=size;
}



/// default destructor
Streamable::~Streamable(){
	Flush();
}


bool Streamable::SetBufferSize(uint32 readBufferSize, uint32 writeBufferSize){

    operatingModes.canSeek = CanSeek(); 
	
    // mutex mode is enabled if CanSeek and both can Read and Write
	// in that case the stream is single and bidirectional
    if (CanSeek() && CanWrite() && CanRead()) {
    	operatingModes.mutexWriteMode = true;
    }    	
    
    if (!CanRead())  readBufferSize = 0;   
    if (!CanWrite()) writeBufferSize = 0;   

    // dump any data in the write Queue
    if (!Flush()) return false;
    
    // adjust readBufferSize
    readBuffer.SetBufferAllocationSize(readBufferSize);

    // adjust writeBufferSize
    writeBuffer.SetBufferAllocationSize(writeBufferSize);
    
    return true;
} 


bool Streamable::Read(
                        char *              buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout,
                        bool                completeRead){
    // check for mutually exclusive buffering and 
    // whether one needs to switch to ReadMode
    if (operatingModes.mutexWriteMode) {
       if (!SwitchToReadMode()) return false;
    }

    if (readBuffer.Buffer()!=NULL){ 

        // read from buffer first
        uint32 toRead = size;
    	
    	readBuffer.BufferRead(buffer, size);
    	
    	if (size != toRead ){
    		
    		toRead -= size;
    		
    		if ((toRead*4) < readBuffer.BufferSize()){
    			if (!readBuffer.RefillReadBuffer(*this)) return false;
    			
    			readBuffer.BufferRead(buffer+size, toRead);
    			size+= toRead;
    			
    			// should have completed
    			return true;
    			
    		} else {
                // if needed read directly from stream
                if (!UnBufferedRead(buffer+size,toRead,msecTimeout)) return false;
                size += toRead;
    			
    		}
    	}
    }
    	
   // if needed read directly from stream
   return UnBufferedRead(buffer,size,msecTimeout);
}
  
    
/** Write data from a buffer to the stream. As much as size byte are written, actual size
    is returned in size. msecTimeout is how much the operation should last.
    timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
    when noWait is used .... */
bool Streamable::Write(
                        const char*         buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout ,
                        bool                completeWrite)
{

    // check for mutually exclusive buffering and 
    // whether one needs to switch to WriteMode
    if (operatingModes.mutexReadMode) {
       if (!SwitchToWriteMode()) return false;
    }
    
    // buffering active?
    if (writeBuffer.BufferReference() != NULL){
    	// separate input and output size
    	uint32 toWrite = size;
    	// check available buffer size versus write size 
        // if size is comparable to buffer size there 
        // is no reason to use the buffering mechanism
        if (writeBuffer.BufferSize() > (4 *size)){
        	
        	// try writing the buffer
        	writeBuffer.BufferWrite(buffer, size);
        	
        	// all done! space available! 
        	if (size == toWrite) return true;
        	
        	// make space
        	if (!Flush()) return false;

        	toWrite -= size;
        	uint32 leftToWrite = toWrite;
        	
        	// try writing the buffer
        	writeBuffer.BufferWrite(buffer+size, leftToWrite);

        	size+= leftToWrite;
        	
        	// should have been able to fill in it!!!
        	if (leftToWrite != toWrite) return false;
        	return true;               
        } else {
        	// write the buffer so far
        	if (!Flush()) return false;
        }
        
    }
    return UnBufferedWrite(buffer,size,msecTimeout);

} 

/** The size of the stream */
int64 Streamable::Size()    {
	// just commit all pending changes if any
	// so stream size will be updated     	
	Flush();
	// then call Size from unbuffered stream 
	return UnBufferedSize(); 
}


/** Moves within the file to an absolute location */
bool Streamable::Seek(int64 pos)
{
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	writeBuffer.FlushWriteBuffer(*this);
    } else {
    	// if read buffer has some data, check whether seek can be within buffer
    	if (readBuffer.readBufferFillAmount > 0){
    		int64 currentStreamPosition = UnBufferedPosition();
    		int64 bufferStartPosition = currentStreamPosition - readBuffer.readBufferFillAmount;
    		
    		// if within range just update readBufferAccessPosition
    		if ((pos >= bufferStartPosition) &&
    	        (pos < currentStreamPosition)){
    			readBuffer.readBufferAccessPosition = pos - bufferStartPosition;
    			return true;
    		} else { // otherwise mark read buffer empty and proceed with normal seek
    			readBuffer.readBufferFillAmount = 0;
                readBuffer.readBufferAccessPosition = 0;
    		}
    	}       	
    }
	
	return UnBufferedSeek(pos);
}

/** Moves within the file relative to current location */
bool  Streamable::RelativeSeek(int32 deltaPos){
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	writeBuffer.FlushWriteBuffer(*this);
    } else {
    	// if read buffer has some data, check whether seek can be within buffer
    	if (readBuffer.readBufferFillAmount > 0){
    		
    		if (((readBuffer.readBufferAccessPosition + deltaPos) < readBuffer.readBufferFillAmount) && (deltaPos>=0))
    		{
    			readBuffer.readBufferAccessPosition += deltaPos;
    			return true;
    		}
    		if (((readBuffer.readBufferAccessPosition + deltaPos) > 0)&& (deltaPos<0)){
    			readBuffer.readBufferAccessPosition += deltaPos;
    			return true;
    		}
   		
    	}       	
    }
	
	return UnBufferedSeek( UnBufferedPosition() + deltaPos);
}    

/** Returns current position */
int64 Streamable::Position() {
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	return UnBufferedPosition() + writeBuffer.writeBufferAccessPosition;
    } else {
    	return UnBufferedPosition() + readBuffer.readBufferAccessPosition - readBuffer.readBufferFillAmount;
    }
}

/** Clip the stream size to a specified point */
bool Streamable::SetSize(int64 size)
{
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	writeBuffer.FlushWriteBuffer(*this);
    } else { // simply empty read buffer
    	readBuffer.readBufferFillAmount = 0;
    	readBuffer.readBufferAccessPosition = 0;
    }
    return UnBufferedSetSize(size);
}


/** select the stream to read from. Switching may reset the stream to the start. */
bool Streamable::Switch(uint32 n)
{
    Flush();
	return UnBufferedSwitch(n);
}

/** select the stream to read from. Switching may reset the stream to the start. */
bool Streamable::Switch(const char *name)
{
    Flush();
    return UnBufferedSwitch(name);
}

/**  remove an existing stream .
    current stream cannot be removed 
*/
bool Streamable::RemoveStream(const char *name)
{
	Flush();
    return UnBufferedRemoveStream(name);
}


/** extract a token from the stream into a string data until a terminator or 0 is found.
    Skips all skip characters and those that are also terminators at the beginning
    returns true if some data was read before any error or file termination. False only on error and no data available
    The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
    skipCharacters=NULL is equivalent to skipCharacters = terminator
    {BUFFERED}    */
bool Streamable::GetToken(
                            char *              outputBuffer,
                            const char *        terminator,
                            uint32              outputBufferSize,
                            char *              saveTerminator,
                            const char *        skipCharacters){
	
	return GetTokenFromStream(*this, outputBuffer,terminator,outputBufferSize,saveTerminator,skipCharacters);
}


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
bool Streamable::GetToken(
		                    Streamable &  output,
                            const char *        terminator,
                            char *              saveTerminator,
                            const char *        skipCharacters){

	return GetTokenFromStream(*this, output,terminator,saveTerminator,skipCharacters);
}

/** to skip a series of tokens delimited by terminators or 0
    {BUFFERED}    */
bool Streamable::SkipTokens(
                            uint32              count,
                            const char *        terminator){

	return SkipTokensInStream(*this,count,terminator);
}

bool Streamable::Print(const AnyType& par,FormatDescriptor fd){

	return PrintToStream(*this,par,fd);
}

bool Streamable::PrintFormatted(const char *format, const AnyType pars[]){

	return PrintFormattedToStream(*this,format,pars);
}















