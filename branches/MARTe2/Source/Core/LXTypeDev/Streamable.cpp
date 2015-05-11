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
bool StreamableReadBuffer::Resync(TimeoutType         msecTimeout){
	// empty!
    if (maxAmount == 0) {
    	return true;
    }
    
    // distance to end 
    uint32 deltaToEnd =  amountLeft - fillLeft;
    
    // adjust seek position
    // in read mode the actual stream 
    // position is to the character after the buffer end
    if (!stream.UnBufferedSeek (stream.UnBufferedPosition()-deltaToEnd)) {
    	return false;
    }
                                                                          
    // mark it as empty
    Empty();
    return true;
} 

/**  
    refill readBuffer
    assumes that the read position is now at the end of buffer
*/
bool StreamableReadBuffer::Refill(TimeoutType         msecTimeout){
	// can we write on it?
	if (BufferReference() == NULL) {
		return false;
	}
	
    // load next batch of data
    bufferPtr = BufferReference();
    maxAmount  = BufferSize();
	amountLeft = maxAmount;
	// just use this as a temp variable
    fillLeft   = maxAmount; 
    if (stream.UnBufferedRead(BufferReference(),fillLeft)){
    	fillLeft   = maxAmount - fillLeft; 
    	return true;
    }  

    Empty();
	return false;
    	
}

bool StreamableReadBuffer::Seek(uint32 position){
	if (position >= (maxAmount-fillLeft)) return false; 
	amountLeft = maxAmount - position;
	bufferPtr = BufferReference() + position;
	return true;
}

///
bool StreamableReadBuffer::RelativeSeek(int32 delta){
	if (delta > 0){
		if ((uint32)delta >= (amountLeft-fillLeft)) return false;
	}
	if (delta < 0){
		if ((amountLeft-delta) > maxAmount) return false;
	}
	amountLeft += delta;
	bufferPtr += delta;
	return true;
}


/**  
    empty writeBuffer
    only called internally when no more space available 
*/
bool StreamableWriteBuffer::Flush(TimeoutType         msecTimeout  ){
	// no buffering!
	if (Buffer()== NULL) return true;
	
	// how much was written?
    uint32 writeSize = maxAmount - fillLeft;
    
    // write
    if (!stream.UnBufferedWrite(Buffer(),writeSize,msecTimeout,true)) {
    	return False;
    }

    Empty(); 
    maxAmount = BufferSize();
    return True;  
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
    readBuffer.SetBufferSize(readBufferSize);

    // adjust writeBufferSize
    writeBuffer.SetBufferSize(writeBufferSize);
    
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

    // check whether we have a buffer
    IOBuffer &ib = GetInputBuffer();
    if (ib.BufferPtr()!=NULL){ 

        // read from buffer first
        uint32 toRead = size;
    	
        // try once 
    	ib.Read(buffer, size);
    	
    	if (size == toRead ){
    		return true;
    	} else {  // partial only so continue
    		
    		// adjust toRead
    		toRead -= size;
    		
    		// decide whether to use the buffer again or just to read directly
    		if ((toRead*4) < ib.MaxAmount()){
    			if (!ib.Refill()) return false;
    			
    			ib.Read(buffer+size, toRead);
    			size += toRead;
    			
    			// should have completed
    			// as our buffer is at least 4x the need
    			return true;
    			
    		} else {
                // if needed read directly from stream
                if (!UnBufferedRead(buffer+size,toRead,msecTimeout)) return false;
                size += toRead;
    			return true;
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
    
    IOBuffer &ob = GetOutputBuffer();
    // buffering active?
    if (ob.BufferPtr() != NULL){
    	// separate input and output size
    	
    	uint32 toWrite = size;
    	// check available buffer size versus write size 
        // if size is comparable to buffer size there 
        // is no reason to use the buffering mechanism
        if (ob.MaxAmount() > (4 *size)){
        	
        	// try writing the buffer
        	ob.Write(buffer, size);
        	
        	// all done! space available! 
        	if (size == toWrite) return true;
        	
        	// make space
        	if (!ob.Flush()) return false;

        	toWrite -= size;
        	uint32 leftToWrite = toWrite;
        	
        	// try writing the buffer
        	ob.Write(buffer+size, leftToWrite);

        	size+= leftToWrite;
        	
        	// should have been able to fill in it!!!
        	if (leftToWrite != toWrite) return false;
        	return true;               
        } else {
        	// write the buffer so far
        	if (!ob.Flush()) return false;
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
    	GetOutputBuffer().Flush();
    } else {
        IOBuffer &ib = GetInputBuffer();
    	// if read buffer has some data, check whether seek can be within buffer
    	if (ib.MaxAmount() > 0){
    		int64 currentStreamPosition = UnBufferedPosition();
    		int64 bufferStartPosition = currentStreamPosition - ib.MaxAmount();
    		
    		// if within range just update readBufferAccessPosition
    		if ((pos >= bufferStartPosition) &&
    	        (pos < currentStreamPosition)){
    			ib.Seek(pos - bufferStartPosition);
    			
    			return true;
    		} else { // otherwise mark read buffer empty and proceed with normal seek
    			ib.Empty();
    			// continues at the end of the function
    		}
    	}       	
    }
	
	return UnBufferedSeek(pos);
}

/** Moves within the file relative to current location */
bool  Streamable::RelativeSeek(int32 deltaPos){
    if (!operatingModes.canSeek) return false;
	if (deltaPos == 0) return true;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	GetOutputBuffer().Flush();
    } else {
    	if (GetInputBuffer().RelativeSeek(deltaPos)){
    		return true;
    	}
		// adjust seek poistion to account for buffer usage
		deltaPos -= GetInputBuffer().AmountLeft();
		
		// empty buffer
		GetInputBuffer().Empty();
    	
    }
	
	return UnBufferedSeek( UnBufferedPosition() + deltaPos);
}    

/** Returns current position */
int64 Streamable::Position() {
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	return UnBufferedPosition() + GetOutputBuffer().MaxAmount() - GetOutputBuffer().AmountLeft();
    } else {
    	return UnBufferedPosition() - GetInputBuffer().AmountLeft();
    }
}

/** Clip the stream size to a specified point */
bool Streamable::SetSize(int64 size)
{
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	GetOutputBuffer().Flush();
    } else { // simply empty read buffer
    	GetInputBuffer().Empty();
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



