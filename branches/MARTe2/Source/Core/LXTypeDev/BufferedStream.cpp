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

#include "BufferedStream.h"
#include "ErrorManagement.h"
#include "StringHelper.h"
#include "StreamHelper.h"


/// default destructor
BufferedStream::~BufferedStream(){
	Flush();
}


bool BufferedStream::Read(
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
    	
    	// partial only so continue
    	if (size != toRead ){
    		// adjust toRead
    		toRead -= size;
    		
    		// decide whether to use the buffer again or just to read directly
    		if ((toRead*4) < ib.MaxAmount()){
    			if (!ib.Refill()) return false;
    			
    			ib.Read(buffer+size, toRead);
    			size += toRead;
    			
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
bool BufferedStream::Write(
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
int64 BufferedStream::Size()    {
	// just commit all pending changes if any
	// so stream size will be updated     	
	Flush();
	// then call Size from unbuffered stream 
	return UnBufferedSize(); 
}


/** Moves within the file to an absolute location */
bool BufferedStream::Seek(int64 pos)
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
bool  BufferedStream::RelativeSeek(int32 deltaPos){
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
int64 BufferedStream::Position() {
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	return UnBufferedPosition() + GetOutputBuffer().MaxAmount() - GetOutputBuffer().AmountLeft();
    } else {
    	return UnBufferedPosition() - GetInputBuffer().AmountLeft();
    }
}

/** Clip the stream size to a specified point */
bool BufferedStream::SetSize(int64 size)
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
bool BufferedStream::Switch(uint32 n)
{
    Flush();
	return UnBufferedSwitch(n);
}

/** select the stream to read from. Switching may reset the stream to the start. */
bool BufferedStream::Switch(const char *name)
{
    Flush();
    return UnBufferedSwitch(name);
}

/**  remove an existing stream .
    current stream cannot be removed 
*/
bool BufferedStream::RemoveStream(const char *name)
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
bool BufferedStream::GetToken(
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
bool BufferedStream::GetToken(
		                    BufferedStream &  output,
                            const char *        terminator,
                            char *              saveTerminator,
                            const char *        skipCharacters){

	return GetTokenFromStream(*this, output,terminator,saveTerminator,skipCharacters);
}

/** to skip a series of tokens delimited by terminators or 0
    {BUFFERED}    */
bool BufferedStream::SkipTokens(
                            uint32              count,
                            const char *        terminator){

	return SkipTokensInStream(*this,count,terminator);
}

bool BufferedStream::Print(const AnyType& par,FormatDescriptor fd){

	return PrintToStream(*this,par,fd);
}

bool BufferedStream::PrintFormatted(const char *format, const AnyType pars[]){

	return PrintFormattedToStream(*this,format,pars);
}















