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
    
    // adjust seek position
    // in read mode the actual stream 
    // position is to the character after the buffer end
    if (!stream.UnBufferedSeek (stream.UnBufferedPosition()-amountLeft)) {
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
    if (stream.UnBufferedRead(BufferReference(),maxAmount)){
    	amountLeft = maxAmount;
    	return true;
    }  

    Empty();
	return false;
    	
}

/**  
    empty writeBuffer
    only called internally when no more space available 
*/
bool StreamableWriteBuffer::Flush(TimeoutType         msecTimeout  ){
	// no buffering!
	if (Buffer()== NULL) return true;
	
	// how much was written?
    uint32 writeSize = maxAmount - amountLeft;
    
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


