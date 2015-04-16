#include "Streamable.h"


bool StreamableSetBufferSize(Streamable & bsb,uint32 readBufferSize, uint32 writeBufferSize){
        if (!stream.CanRead())  stream.readBufferSize = 0;   
        if (!stream.CanWrite()) stream.writeBufferSize = 0;   

        // dump any data in the write Queue
        stream.Flush();
        
        // adjust readBufferSize
        stream.readBuffer.SetBufferSize(stream.readBufferSize);

        // adjust writeBufferSize
        stream.writeBuffer.SetBufferSize(stream.writeBufferSize);
        
    } 

/// copies to buffer size bytes from the end of readBuffer
void Streamable::BufferRead(char *buffer, uint32 &size){

    uint32 spaceLeft = readBufferFillAmount - readBufferAccessPosition;
    
    // clip to available space
    if (size > spaceLeft) size = spaceLeft;
	
	    // fill the buffer with the remainder 
	if (size > 0) MemoryCopy(buffer, readBuffer.Buffer()+readBufferAccessPosition,size);
    readBufferAccessPosition+=size;

}


bool Streamable::Read(
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
void Streamable::BufferWrite(const char *buffer, uint32 &size){

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
bool Streamable::Write(
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
bool Streamable::Seek(int64 pos)
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
bool  Streamable::RelativeSeek(int32 deltaPos){
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
int64 Streamable::Position() {
    if (!operatingModes.canSeek) return false;
    
    // if write mode on then just flush out data
    if (operatingModes.mutexWriteMode){
    	return UnBufferedPosition() + writeBufferAccessPosition;
    } else {
    	return UnBufferedPosition() + readBufferAccessPosition - readBufferFillAmount;
    }
}

/** Clip the stream size to a specified point */
bool Streamable::SetSize(int64 size)
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
bool Streamable::GetToken(
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
virtual bool Streamable::GetToken(
		                    Streamable &  output,
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
bool Streamable::SkipTokens(
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


bool PrintAnyInteger()


bool Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor){
	
	/// empty - an error?
	if (par.dataPointer == NULL) return false;
	
	if (par.isStructuredData ){
		Errormanagement::ReportError(UnsupportedError, "Streamable::Print StructuredData not supported");
		return false;
	}
	
	bool isSigned = false;
	switch (par.dataDescriptor.type){
	 
	case SignedInteger: 
		isSigned = true 
	case UnsignedInteger:
	{
		int64 number = ExpandToInt64(par.dataPointer, par.dataDescriptor.size);
		PrintInt64(number);
	}break;
		
		
	        /** An integer   pointer = uintxx * 
	            size is in bytes */
	        UnsignedInteger       = 1,
		
	
	}
	
	
}















#if 0

bool CPrintInt32(CStream *cs,int32 n,uint32 desiredSize,char desiredPadding,char mode){
    if(n == (int32)0x80000000){
        if((mode == 'i') || (mode == 'd')){

            CPut(cs,'-');
            uint32 ccount = 11;
            if(desiredPadding != 0)  while (desiredSize > ccount) { CPut(cs,desiredPadding); desiredSize--;}
            else                     while (desiredSize > ccount) { CPut(cs,' ');            desiredSize--;}
            CPut(cs,'2');
            CPut(cs,'1');
            CPut(cs,'4');
            CPut(cs,'7');
            CPut(cs,'4');
            CPut(cs,'8');
            CPut(cs,'3');
            CPut(cs,'6');
            CPut(cs,'4');
            CPut(cs,'8');
        }else if ((mode == 'x') || (mode == 'X') || (mode == 'p')){
            uint32 ccount = 8;
            if(desiredPadding != 0)  while (desiredSize > ccount) { CPut(cs,desiredPadding); desiredSize--;}
            else                     while (desiredSize > ccount) { CPut(cs,' ');            desiredSize--;}
            CPut(cs,'8');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
        } else if (mode == 'o'){
            uint32 ccount = 11;
            if(desiredPadding != 0)  while (desiredSize > ccount) { CPut(cs,desiredPadding); desiredSize--;}
            else                     while (desiredSize > ccount) { CPut(cs,' ');            desiredSize--;}
            CPut(cs,'2');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
            CPut(cs,'0');
        }
        return True;
    }

    if ((mode == 'i') || (mode == 'd')){
        int divid = 1000000000;
        // # of characters
        uint32 ccount=10;

        int na = n;

        if (n < 0) {
            na = -n;
            ccount++;
        }
        if (n != 0){
            while(divid > na){
                divid/=10;
                ccount--;
            }
        } else {
            ccount=1;
        }
    //printf("{%i %i}",divid,ccount);
        if ((desiredPadding == '0') && (n<0)) CPut(cs,'-');
        if (desiredPadding != 0)  while (desiredSize > ccount) { CPut(cs,desiredPadding); desiredSize--;}
        else                     while (desiredSize > ccount) { CPut(cs,' ');            desiredSize--;}
        if ((desiredPadding != '0') && (n<0)) CPut(cs,'-');
        if (n==0) CPut(cs,'0');
        else{
            while(divid>0){
                int n = na/divid;
                na = na % divid;
                CPut(cs,n+'0');
                divid/=10;
            }
        }
    } 
    else if (mode == 'u'){
        int divid = 1000000000;
        // # of characters
        uint32 ccount=10;
        uint32 na = n;
        if (n != 0){
            while(divid > na){
                divid/=10;
                ccount--;
            }
        } else {
            ccount=1;
        }
        if(desiredPadding != 0){
            while (desiredSize > ccount){
                CPut(cs,desiredPadding); 
                desiredSize--;
            }
        }
        else{
            while(desiredSize > ccount){
                CPut(cs,' ');            
                desiredSize--;
            }
        }
        if(n==0){ 
            CPut(cs,'0');
        }
        else{
            while(divid>0){
                int n = na/divid;
                na = na % divid;
                CPut(cs,n+'0');
                divid/=10;
            }
        }
    }
    else if ((mode == 'x') || (mode == 'X') || (mode == 'p')){
        uint32 ccount=  8;
        int shift = 28;
        uint32 an = n;
        if (an!=0) {
            while ((an>>shift)==0) {
                shift-=4;
                ccount--;
            }
        } else {
            shift = 0;
            ccount = 1;
        }

        if (desiredPadding != 0) while (desiredSize > ccount) { CPut(cs,desiredPadding); desiredSize--;}
        else                     while (desiredSize > ccount) { CPut(cs,' ');            desiredSize--;}

        while(shift>=0) {
            uint32 nib = (an>>shift)& 0xF;
            if (nib<10) CPut(cs,nib+'0');
            else
            if (mode == 'X') CPut(cs,nib+'A'- 10);
            else        CPut(cs,nib+'a'- 10);
            shift -=4;
        }
    } else
    if (mode == 'o'){
        uint32 ccount= 11;
        int shift = 30;
        uint32 an = n;
        if (an!=0) while((an>>shift)==0) {shift-=3;ccount--;}
        else { shift = 0; ccount = 1;}

        if (desiredPadding != 0) while (desiredSize > ccount) { CPut(cs,desiredPadding); desiredSize--;}
        else                     while (desiredSize > ccount) { CPut(cs,' ');            desiredSize--;}

        while(shift>=0) {
            uint32 nib = (an>>shift)& 0x7;
            CPut(cs,nib+'0');
            shift -=3;
        }
    }
    return True;
}

bool CPrintInt64(CStream *cs,int64 n,uint32 desiredSize,char desiredPadding,char mode){
    char buffer[32];
    uint32 ix = sizeof(buffer)-1;
    buffer[ix] = 0;

    if ((mode == 'i') || (mode == 'd')){
        bool sign = (n < 0);
        while ((n != 0) && (ix > 0)){
            ix--;
            #ifndef _RTAI
                int nr = n % 10;
            #else
                int64 temp = n;
                int nr = do_div(temp, 10);
            #endif
            buffer[ix] = '0' + abs(nr);
            #ifndef _RTAI
                n = n / 10;
            #else
                do_div(n, 10);
            #endif
        }
        if (ix == (sizeof(buffer)-1)){
            ix--;
            buffer[ix] = '0';
        }

        uint32 ccount=sizeof(buffer) - 1 - ix;

        if ((desiredPadding == '0') && sign) CPut(cs,'-');
        if (desiredPadding != 0){
            while (desiredSize > ccount) {
                CPut(cs,desiredPadding);
                desiredSize--;
            }
        } else {
            while (desiredSize > ccount) {
                CPut(cs,' ');
                desiredSize--;
            }
        }
        if ((desiredPadding != '0') && sign) CPut(cs,'-');
        CWrite(cs,buffer+ix,ccount);
    } else
    if ((mode == 'x') || (mode == 'X') || (mode == 'p')){
        while ((n != 0) && (ix > (sizeof(buffer) - 17))){
            ix--;
            int nr = n & 0xF;
            if (nr >= 10){
                buffer[ix] = 'A' + abs(nr) - 10;
            } else {
                buffer[ix] = '0' + abs(nr);
            }
            n = n >> 4;
        }
        if (ix == (sizeof(buffer)-1)){
            ix--;
            buffer[ix] = '0';
        }

        uint32 ccount=sizeof(buffer) - 1 - ix;
        if (desiredPadding == 0) desiredPadding = ' ';
        while (desiredSize > ccount) {
            CPut(cs,desiredPadding);
            desiredSize--;
        }

        CWrite(cs,buffer+ix,ccount);

    } else
    if (mode == 'o'){
        while ((n != 0) && (ix > (sizeof(buffer) - 22))){
            int nr = n & 0x7;
            ix--;
            buffer[ix] = '0' + abs(nr);
            n = n >> 3;
        }
        if (ix == (sizeof(buffer)-1)){
            ix--;
            buffer[ix] = '0';
        }

        uint32 ccount=sizeof(buffer) - 1 - ix;
        if (desiredPadding == 0) desiredPadding = ' ';
        while (desiredSize > ccount) {
            CPut(cs,desiredPadding);
            desiredSize--;
        }

        CWrite(cs,buffer+ix,ccount);
    }
    return True;
}

static inline bool IsNaN(float &x){
     return (((*(long *)&(x) & 0x7f800000L)==0x7f800000L) &&  ((*(long *)&(x) & 0x007fffffL)!=0000000000L));
}

static inline bool IsInf(float &x){
    return  (((*(long *)&(x) & 0x7f800000L)==0x7f800000L) &&  ((*(long *)&(x) & 0x007fffffL)==0000000000L));
}

static const uint32 FPLmask0 = 0x7FF00000;
static const uint32 FPLmask1 = 0x000FFFFF;
static const uint32 FPLmask2 = 0xFFFFFFFF;

static inline bool IsNaN(double &x){
    uint64 &data = (uint64 &)x;
    uint64 mask = FPLmask0;
    uint64 temp = data;
    temp >>= 32;
    temp &= mask;
    if (temp != FPLmask0) return False;
    mask = FPLmask1;
    mask <<= 32;
    mask |= FPLmask2;
    temp = data & mask;
    if (temp == 0) return False;
    return True;
}

static inline bool IsInf(double &x){
    uint64 &data = (uint64 &)x;
    uint64 mask = FPLmask0;
    uint64 temp = data;
    temp >>= 32;
    temp &= mask;
    if (temp != FPLmask0) return False;
    mask = FPLmask1;
    mask <<= 32;
    mask |= FPLmask2;
    temp = data & mask;
    if (temp == 0) return True;
    return False;
}


bool CPrintDouble(CStream *cs,double ff,int desiredSize,int desiredSubSize,char desiredPadding,char mode){
    if (IsNaN(ff)){
        CPrintString(cs,"NaN");
        for(int i=0;i<(desiredSize-3);i++) CPut(cs,' ');
        return True;
    }
    if (IsInf(ff)){
        if (ff<0) CPut(cs,'-');
        else      CPut(cs,'+');
        CPrintString(cs,"Inf");
        for(int i=0;i<(desiredSize-4);i++) CPut(cs,' ');
        return True;
    }

    int ccount=0;

    double fa = ff;
    if (ff<0) { fa = -ff; ccount++;}

    int exponent = 0;
    if (fa > 0.0){
        while (fa>=10.0) { fa*=0.1; exponent++;}
        while (fa<1)     { fa*=10; exponent--;}
    } else {
        exponent = 0;
    }

    switch(mode){
        case 'e':{
            ccount += 7; // x.  e-005
            ccount += desiredSubSize;

            // for rounding add 0.5 * 10**(exponent-desiredSubSize)
            double roundingLog = -2.30258509299404568401799*desiredSubSize-0.6931471805599453094172321;
            double rounding = exp(roundingLog);
            fa += rounding;

            // 9.9999 =>10.00
            if (fa>=10.0){
                exponent++;
                fa*=0.1;
            }
        }break;

        case 'f':
        default:{
            if (exponent<=0){
                ccount += 2; // 0.
                ccount += desiredSubSize;
            } else {
                ccount += 2; // 0.
                ccount += desiredSubSize;
                ccount += exponent; // 1000.
            }
            // for rounding add 0.5 * 10**(exponent-desiredSubSize)
            double roundingLog = -2.30258509299404568401799*(desiredSubSize+exponent)-0.6931471805599453094172321;
            double rounding = exp(roundingLog);
            fa += rounding;

            // 9.9999 =>10.00
            if (fa>=10.0){
                exponent++;
                fa*=0.1;
            }

        }
    }

    if(desiredPadding != 0){
        if ((desiredPadding==' ')&&(ccount>=desiredSize)) CPut(cs,desiredPadding);
        while (desiredSize-- > ccount) CPut(cs,desiredPadding);
    }
    else                     while (desiredSize-- > ccount) CPut(cs,' ');
    if (ff<0) CPut(cs,'-');

    switch(mode){
        case 'e':{
            int fi = (int)fa;
            CPut(cs,fi+'0');
            CPut(cs,'.');
            fa-=fi;
            while(desiredSubSize>0){
                if (desiredSubSize >4){
                    fa*=1e4;
                    fi = (int)fa;
                    CPrintInt32(cs,fi,4,'0');
                    fa-=fi;
                    desiredSubSize-=4;
                } else {
                    int sav = desiredSubSize;
                    while(desiredSubSize>0){ fa*=10;desiredSubSize--;}
                    fi = (int)fa;
                    fa-=fi;
                    CPrintInt32(cs,fi,sav,'0');
                }
            }
            CPut(cs,'e');
            if (exponent<0){
                CPut(cs,'-');
                CPrintInt32(cs,-exponent,3,'0');
            } else {
                CPut(cs,'+');
                CPrintInt32(cs,exponent,3,'0');
            }
        } break;

        case 'f':
        default:{
            int fi;
            while(exponent>4){
                fa*=1e4;
                fi = (int)fa;
                CPrintInt32(cs,fi,4,'0');
                fa-=fi;
                exponent-=4;
            }
            if (exponent>=0){
                int sav = exponent;
                while(exponent>0) {fa*=10;exponent--;}
                fi = (int)fa;
                CPrintInt32(cs,fi,sav,'0');
                fa-=fi;
            }
            if (exponent < 0){
                 CPut(cs,'0');
                 exponent++;
                 fa*=0.1;
            }
            CPut(cs,'.');
            while((exponent<0)&&(desiredSubSize>0)){
                desiredSubSize--;
                exponent++;
                CPut(cs,'0');
            }
            while (desiredSubSize >4){ // >4 so that the next block is always executed
                fa*=1e4;
                fi = (int)fa;
                CPrintInt32(cs,fi,4,'0');
                fa-=fi;
                desiredSubSize-=4;
            }
            if (desiredSubSize>0){
                int sav = desiredSubSize;
                while(desiredSubSize>0) {fa*=10; desiredSubSize--;}
                fi = (int)fa;
                fa-=fi;
                CPrintInt32(cs,fi,sav,'0');
            }
        }
    }

    return True;
}

bool CPrintString(CStream *cs,const char *s,uint32 desiredSize,char desiredPadding, bool rightJustify){
    if (s==NULL) return False;
    uint32 ccount = strlen(s);

    if(rightJustify){
        if(desiredPadding != 0){
            if ((desiredPadding==' ') && (ccount>=desiredSize)) CPut(cs,desiredPadding);
            while (desiredSize-- > ccount) CPut(cs,desiredPadding);
        }
        else                     while (desiredSize-- > ccount) CPut(cs,' ');
    }

    for (uint32 i=0;i<ccount;i++) CPut(cs,s[i]);

    if(!rightJustify){
        if(desiredPadding != 0){
            if ((desiredPadding==' ')&& (ccount>=desiredSize)) CPut(cs,desiredPadding);
            while (desiredSize-- > ccount) CPut(cs,desiredPadding);
        }
        else                     while (desiredSize-- > ccount) CPut(cs,' ');
    }
    
    return True;
}


bool VCPrintf(CStream *cs,const char *format,va_list argList){    
    if (format==NULL) return False;

    char desiredPadding = 0;
    int desiredSize = 0;
    int subFieldSize = 0;
    int status = 0;  // 1 = after %
    int subFieldStatus = 0;  // after .
    int longMode = 0;
    bool rightJustify = True;

    // to store the option string
    const int optionBufferSize = 22;
    char optionBuffer[optionBufferSize+2];
    char *optionBufferPtr = optionBuffer;

    char c;
    while((c = *format)!=0){
        if (status == 0){
            if (c == '%'){
                status = 1;
                desiredPadding = 0;
                desiredSize = 0;
                subFieldSize = 0;
                subFieldStatus = 0;
                longMode = 0;
                optionBufferPtr = optionBuffer;
                *optionBufferPtr++ = '%';
            } else CPut(cs,c);
        } else
        if (status == 1){
            switch(c){
                case '%':{
                    status = 0;
                    CPut(cs,c);
                } break;
/*
                case ' ':{
                    if ((desiredPadding == 0)&&(desiredSize == 0)&&((int)(optionBufferPtr - optionBuffer)<optionBufferSize)){
                        *optionBufferPtr++ = c;
                        desiredPadding = ' ';
                    } else {
                        CPut(cs,c);
                        status = 0;
                    }
                } break;
*/
                case '.':{
                    if ((subFieldStatus == 0)&&((int)(optionBufferPtr - optionBuffer)<optionBufferSize)){
                        subFieldStatus = 1;
                        *optionBufferPtr++ = c;
                    } else {
                        CPut(cs,c);
                        status = 0;
                    }
                } break;
                case '0':
                    if ((desiredSize==0)&&((int)(optionBufferPtr - optionBuffer)<optionBufferSize)){
                        desiredPadding = '0';
                        *optionBufferPtr++ = c;
                        break;
                    }
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':{
                    if ((int)(optionBufferPtr - optionBuffer)<optionBufferSize){
                        if (subFieldStatus == 1){
                            subFieldSize = subFieldSize * 10 + c -'0';
                        } else {
                            desiredSize = desiredSize * 10 + c -'0';
                        }
                        *optionBufferPtr++ = c;
                    } else {
                        CPut(cs,c);
                        status = 0;
                    }
                } break;
                case 'o':
                case 'd':
                case 'u':
                case 'i':
                case 'X':
                case 'p':
                case 'x':{
                    if (longMode == 2){
                        CPrintInt64(cs,va_arg(argList,int64),desiredSize,desiredPadding,c);
                    } else {
                        CPrintInt32(cs,va_arg(argList,int32),desiredSize,desiredPadding,c);
                    }
                    status = 0;
                } break;
                case 'f':{
                    if (subFieldSize == 0) subFieldSize = 6;
                    CPrintDouble(cs,va_arg(argList,double),desiredSize,subFieldSize,desiredPadding,'f');
                    status = 0;
                } break;
                case 'e':{
                    if (subFieldSize == 0) subFieldSize = 6;
                    CPrintDouble(cs,va_arg(argList,double),desiredSize,subFieldSize,desiredPadding,'e');
                    status = 0;
                } break;
                case 'l':{
                    if(longMode < 2)
                        longMode++;
                    else
                        longMode = 0;
                } break;
                case 'L':{
                    if (!longMode) longMode = 2;
                    else {
                        CPut(cs,c);
                        status = 0;
                    }
                } break;
                case '-':{
                    if (rightJustify){
                        rightJustify = False;
                    }
                    else if(desiredPadding == 0){
                        desiredPadding = c;
                    }
                    else {
                        CPut(cs,'%');
                        CPut(cs,desiredPadding);
                        CPut(cs,'-');
                        CPut(cs,'-');
                        status = 0;
                    }
                } break;
                case 's':{                    
                    CPrintString(cs,va_arg(argList,char *),desiredSize,desiredPadding, rightJustify);
                    status = 0;
                } break;
                case 'c':{
                    char cc = va_arg(argList,int);
                    CPut(cs,cc);
                    status = 0;
                } break;
                default:{
                    if ((rightJustify)&&(desiredPadding == 0)&&(desiredSize == 0)&&((int)(optionBufferPtr - optionBuffer)<optionBufferSize)){
                        *optionBufferPtr++ = c;
                        desiredPadding = c;
                    } else {
                        CPut(cs,c);
                        status = 0;
                    }
                } break;
/*
                default:{
                    CPut(cs,c);
                    status = 0;
                } break;
*/
            }
        }
        format++;
    }

    return True;
}


#endif