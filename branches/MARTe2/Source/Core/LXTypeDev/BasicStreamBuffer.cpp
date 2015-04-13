#include "BasicStreamBuffer.h"


bool BSB_SetBufferSize(BasicStreamBuffer & bsb,uint32 readBufferSize, uint32 writeBufferSize){
        if (!bsb.CanRead())  bsb.readBufferSize = 0;   
        if (!bsb.CanWrite()) bsb.writeBufferSize = 0;   

        // dump any data in the write Queue
        bsb.Flush();
        
        // adjust readBufferSize
        bsb.readBuffer.SetBufferSize(bsb.readBufferSize);

        // adjust writeBufferSize
        bsb.writeBuffer.SetBufferSize(bsb.writeBufferSize);
        
    } 

/// copies to buffer size bytes from the end of readBuffer
void BasicStreamBuffer::BufferRead(char *buffer, uint32 &size){

    uint32 spaceLeft = readBufferFillAmount - readBufferAccessPosition;
    
    // clip to available space
    if (size > spaceLeft) size = spaceLeft;
	
	    // fill the buffer with the remainder 
	    if (size > 0) MemoryCopy(buffer, readBuffer.Buffer()+readBufferAccessPosition,size);
    readBufferAccessPosition+=size;

}


bool BasicStreamBuffer::Read(
                        char *              buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout     = TTDefault,
                        bool                completeRead    = false){
    // check for mutually exclusive buffering and 
    // whether one needs to switch to ReadMode
    if (operatingModes.mutexWriteMode) {
       if (!SwitchToReadMode()) return false;
    }

    if (readBuffer.Buffer()!=NULL){ 

        // read from buffer first
        uint32 toRead = size;
    	
    	BufferRead(buffer, size);
    	
    	if (size != toRead ){
    		
    		toRead -= size;
    		
    		if ((toRead*4) < readBuffer.BufferAllocatedSize()){
    			if (!RefillReadBuffer()) return false;
    			
    			BufferRead(buffer+size, toRead);
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
    

/// copies buffer of size size at the end of writeBuffer
void BasicStreamBuffer::BufferWrite(const char *buffer, uint32 &size){

	// recalculate how much we can write
	uint32 spaceLeft = writeBuffer.BufferAllocatedSize() - writeBufferAccessPosition;

	// clip to spaceLeft
	if (size > spaceLeft) size = spaceLeft;

	// fill the buffer with the remainder 
	MemoryCopy(writeBuffer.BufferReference()+writeBufferAccessPosition,buffer,size);
	writeBufferAccessPosition+=size;
}

    
/** Write data from a buffer to the stream. As much as size byte are written, actual size
    is returned in size. msecTimeout is how much the operation should last.
    timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
    when noWait is used .... */
bool BasicStreamBuffer::Write(
                        const void*         buffer,
                        uint32 &            size,
                        TimeoutType         msecTimeout     = TTDefault,
                        bool                completeWrite   = false)
{

    // check for mutually exclusive buffering and 
    // whether one needs to switch to WriteMode
    if (operatingModes.mutexReadMode) {
       if (!SwitchToWriteMode()) return false;
    }
    
    // buffering active?
    if (writeBuffer.Buffer() != NULL){
    	// separate input and output size
    	uint32 toWrite = size;
    	// check available buffer size versus write size 
        // if size is comparable to buffer size there 
        // is no reason to use the buffering mechanism
        if (writeBuffer.BufferAllocatedSize() > (4 *size)){
        	// try writing the buffer
        	BufferWrite(buffer, size);
        	
        	// all done! space available! 
        	if (size == toWrite) return true;
        	
        	// make space
        	if (!Flush()) return false;

        	toWrite -= size;
        	uint32 leftToWrite = toWrite;
        	
        	// try writing the buffer
        	BufferWrite(buffer+size, leftToWrite);

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

/** Moves within the file to an absolute location */
bool BasicStreamBuffer::Seek(int64 pos)
{
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	FlushWriteBuffer();
    } else {
    	// if read buffer has some data, check whether seek can be within buffer
    	if (readBufferFillAmount > 0){
    		int64 currentStreamPosition = UnBufferedPosition();
    		int64 bufferStartPosition = currentStreamPosition - readBufferFillAmount;
    		
    		// if within range just update readBufferAccessPosition
    		if ((pos >= bufferStartPosition) &&
    	        (pos < currentStreamPosition)){
    			readBufferAccessPosition = pos - bufferStartPosition;
    			return true;
    		} else { // otherwise mark read buffer empty and proceed with normal seek
                readBufferFillAmount = 0;
                readBufferAccessPosition = 0;
    		}
    	}       	
    }
	
	return UnBufferedSeek(pos);
}

/** Moves within the file relative to current location */
bool  BasicStreamBuffer::RelativeSeek(int32 deltaPos){
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	FlushWriteBuffer();
    } else {
    	// if read buffer has some data, check whether seek can be within buffer
    	if (readBufferFillAmount > 0){
    		
    		if (((readBufferAccessPosition + deltaPos) < readBufferFillAmount) && (deltaPos>=0))
    		{
    			readBufferAccessPosition += deltaPos;
    			return true;
    		}
    		if (((readBufferAccessPosition + deltaPos) > 0)&& (deltaPos<0)){
    			readBufferAccessPosition += deltaPos;
    			return true;
    		}
   		
    	}       	
    }
	
	return UnBufferedSeek( UnBufferedPosition() + deltaPos);
}    

/** Returns current position */
int64 BasicStreamBuffer::Position() {
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	return UnBufferedPosition() + writeBufferAccessPosition;
    } else {
    	return UnBufferedPosition() + readBufferAccessPosition - readBufferFillAmount;
    }
}

/** Clip the stream size to a specified point */
bool BasicStreamBuffer::SetSize(int64 size)
{
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	FlushWriteBuffer();
    } else { // simply empty read buffer
        readBufferFillAmount = 0;
        readBufferAccessPosition = 0;
    }
    return UnBufferedSetSize(size);
}


/** extract a token from the stream into a string data until a terminator or 0 is found.
    Skips all skip characters and those that are also terminators at the beginning
    returns true if some data was read before any error or file termination. False only on error and no data available
    The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
    skipCharacters=NULL is equivalent to skipCharacters = terminator
    {BUFFERED}    */
bool BasicStreamBuffer::GetToken(
                            char *              outputBuffer,
                            const char *        terminator,
                            uint32              outputBufferSize,
                            char *              saveTerminator,
                            const char *        skipCharacters){
    // need space for trailing 0
	outputBufferSize--;

    if (skipCharacters == NULL) skip = terminator;

    uint32 tokenSize=0;
    while(1){
        char c;
        if (GetC(c)==False){

            // 0 terminated string
        	outputBuffer[tokenSize] = 0;

            if (saveTerminator!=NULL) *saveTerminator = 0;

            //
            if (tokenSize == 0) return False;
            else                return True;
        }

        bool isTerminator = (strchr(terminator,c)!=NULL);
        bool isSkip       = (strchr(skip      ,c)!=NULL);
        if (isTerminator || ( c==0 )){

            // exit only if some data was read, otw just skip separator block
            if ((tokenSize != 0) || (!isSkip)){
                // 0 terminated string
            	outputBuffer[tokenSize] = 0;

                if (saveTerminator!=NULL) *saveTerminator = c;

                return True;
            }
        } else
        if (!isSkip && !isTerminator){

        	outputBuffer[tokenSize++] = c;
            if (tokenSize >= outputBufferSize){
                // 0 terminated string
                buffer[tokenSize] = 0;

                if (saveTerminator!=NULL) *saveTerminator = c;

                return True;
            }
        }
    }

    return True;
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
virtual bool BasicStreamBuffer::GetToken(
		                    BasicStreamBuffer &  output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL){

    if (skip == NULL) skipCharacters = terminator;

    uint32 tokenSize=0;
    while(1){
        char c;
        if (GetC(c)==False){

            if (saveTerminator != NULL) *saveTerminator = 0;

            //
            if (tokenSize==0) return False;
            else              return True;
        }

        bool isTerminator = (strchr(terminator,c)!=NULL);
        bool isSkip       = (strchr(skipCharacters ,c)!=NULL);
        if (isTerminator || ( c==0 )){
            // exit only if some data was read, otw just skip separator block
            if ((tokenSize != 0) || (!isSkip)){

                if (saveTerminator != NULL) *saveTerminator = c;

                return True;
            }
        } else
        if (!isSkip && !isTerminator){
            output.PutC(c);
            tokenSize++;
        }
    }

    return True;
}

/** to skip a series of tokens delimited by terminators or 0
    {BUFFERED}    */
bool BasicStreamBuffer::SkipTokens(
                            uint32              count,
                            const char *        terminator){

    uint32 tokenSize=0;
    while(count>0){
        char c;
        if (GetC(c)==False){

            if (tokenSize==0) return False;
            else              return (count == 0);
        } else
        //
        if ((strchr(terminator,c)!=NULL)||(c==0)){
            // exit only if some data was read, otw just skip separator block
            if (tokenSize!=0) {
                tokenSize = 0;
                count--;
            }
        } else {
            tokenSize++;
        }
    }

    return True;
}

bool CPrintf(CStream *cs,const char *format,...){
    if (format==NULL) return False;

    va_list argList;
    va_start(argList,format);
    bool ret = VCPrintf(cs,format,argList);
    va_end(argList);

    return ret;
};


static const int bl2_vsprintfDummySize  = 1;

static void bl2_vsprintfReadFN(CStream *p){
    p->bufferPtr = (char *)p->context;
    p->sizeLeft = bl2_vsprintfDummySize;
}

int bl2_vsprintf(char *buffer,const char *format,va_list argList){

    char temp[bl2_vsprintfDummySize];
    int max = 2000000000;

    CStream cs;
    cs.context   = &temp[0];
    cs.bufferPtr = buffer;
    cs.sizeLeft  = max;
    cs.NewBuffer = bl2_vsprintfReadFN;

    VCPrintf(&cs,format,argList);
    *cs.bufferPtr++ = 0;

    return  max - cs.sizeLeft;

}

int bl2_sprintf(char *buffer,const char *format,...){
    if (format==NULL) return 0;

    va_list argList;
    va_start(argList,format);
    bool ret = bl2_vsprintf(buffer,format,argList);
    va_end(argList);

    return ret;
};

int bl2_vsnprintf(char *buffer,size_t size,const char *format,va_list argList){

    char temp[bl2_vsprintfDummySize];

    CStream cs;
    cs.context   = &temp[0];
    cs.bufferPtr = buffer;
    cs.sizeLeft  = size - 1;
    cs.NewBuffer = bl2_vsprintfReadFN;

    VCPrintf(&cs,format,argList);

    int amount = cs.bufferPtr - buffer;

    if ((amount >= 0) && (amount < size)){
        *cs.bufferPtr++ = 0;
        return amount + 1;
    }

    buffer[size-1] = 0;
    return size;

}

int bl2_snprintf(char *buffer,size_t size,const char *format,...){
    if (format==NULL) return 0;

    va_list argList;
    va_start(argList,format);
    bool ret = bl2_vsnprintf(buffer,size,format,argList);
    va_end(argList);

    return ret;
};




