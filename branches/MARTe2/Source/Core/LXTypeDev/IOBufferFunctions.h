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
 * $Id: CStream.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 * @file 
 * @brief Functions to print integer numbers on generic streams.
*/
#if !defined IOBUFFER_FUNCTIONS
#define IOBUFFER_FUNCTIONS

#include "ErrorManagement.h"
#include "FormatDescriptor.h"
#include "BitSetToInteger.h"
#include "IOBufferIntegerPrint.h"
#include "IOBufferFloatPrint.h"
#include "IOBuffer.h"
#include "StreamInterface.h"



/** extract a token from the stream into a string data until a terminator or 0 is found.
    Skips all skip characters and those that are also terminators at the beginning
    returns true if some data was read before any error or file termination. False only on error and no data available
    The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
    skipCharacters=NULL is equivalent to skipCharacters = terminator
    {BUFFERED}    */
static inline
bool GetTokenFromStream(
	                    	IOBuffer &          iobuff,
                            char *              outputBuffer,
                            const char *        terminator,
                            uint32              outputBufferSize,
                            char *              saveTerminator,
                            const char *        skipCharacters){
    // need space for trailing 0
	outputBufferSize--;

    if (skipCharacters == NULL) skipCharacters = terminator;

    uint32 tokenSize=0;
    while(1){
        char c;
        if (iobuff.GetC(c)==False){

            // 0 terminated string
        	outputBuffer[tokenSize] = 0;

            if (saveTerminator!=NULL) *saveTerminator = 0;

            //
            if (tokenSize == 0) return False;
            else                return True;
        }
        
        bool isTerminator = (StringHelper::SearchChar(terminator,c)!=NULL);
        bool isSkip       = (StringHelper::SearchChar(skipCharacters,c)!=NULL);
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
            	outputBuffer[tokenSize] = 0;

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
static inline
bool GetTokenFromStream(
		            		IOBuffer &          inputStream,
		            		IOBuffer &  		outputStream,
                            const char *        terminator,
                            char *              saveTerminator,
                            const char *        skipCharacters){

    if (skipCharacters == NULL) skipCharacters = terminator;

    uint32 tokenSize=0;
    while(1){
        char c;
        if (inputStream.GetC(c)==False){

            if (saveTerminator != NULL) *saveTerminator = 0;

            //
            if (tokenSize==0) return False;
            else              return True;
        }

        bool isTerminator = (StringHelper::SearchChar(terminator,c)!=NULL);
        bool isSkip       = (StringHelper::SearchChar(skipCharacters ,c)!=NULL);
        if (isTerminator || ( c==0 )){
            // exit only if some data was read, otw just skip separator block
            if ((tokenSize != 0) || (!isSkip)){

                if (saveTerminator != NULL) *saveTerminator = c;

                return True;
            }
        } else
        if (!isSkip && !isTerminator){
            outputStream.PutC(c);
            tokenSize++;
        }
    }

    return True;
}


/** to skip a series of tokens delimited by terminators or 0
    {BUFFERED}    */
static inline  
bool SkipTokensInStream(
							IOBuffer &          iobuff,
                            uint32              count,
                            const char *        terminator){

    uint32 tokenSize=0;
    while(count>0){
        char c;
        if (iobuff.GetC(c)==False){

            if (tokenSize==0) return False;
            else              return (count == 0);
        } else
        //
        if ((StringHelper::SearchChar(terminator,c)!=NULL)||(c==0)){
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

static inline
bool PrintString(
		IOBuffer &    			iobuff,
		const char *			string,
		FormatDescriptor 		fd)
{
	
	if (string == NULL) string = "NULL pointer";
	
	uint32 stringSize = (uint32)StringHelper::Length(string);
	uint32 paddingSize = 0;
	
	if (fd.size != 0){
		if (stringSize > fd.size) stringSize = fd.size; 
		
		if (fd.padded){
			if (stringSize < fd.size){
				paddingSize = fd.size - stringSize;  
			}			
		}
	}
	
	bool ret = true;
	uint32 i;
	if (!fd.leftAligned && (paddingSize > 0)){
		for (i=0;i < paddingSize;i++) ret = ret && iobuff.PutC(' ');		
	}
	
	ret = ret && iobuff.WriteAll(string,stringSize);
	
	if (fd.leftAligned && (paddingSize > 0)){
		for (i=0;i < paddingSize;i++) ret = ret && iobuff.PutC(' ');		
	}
	
	return ret;
}

static inline
bool PrintStream(
		IOBuffer &    			iobuff,
		StreamInterface *		stream,
		FormatDescriptor 		fd)
{
	if (stream == NULL) {
		return PrintString(iobuff,"!!NULL pointer!!",fd);
	}
	
	if (!stream->CanSeek()){
		return PrintString(iobuff,"!!stream !seek!!",fd);
	}
	
	// must be s
	int64 streamSizeL = (uint32)(stream->Size()-stream->Position());
	uint32 paddingSize = 0;
	
	if (fd.size != 0){
		if (streamSizeL > fd.size) streamSizeL = fd.size; 
		
		if (fd.padded){
			if (streamSizeL < fd.size){
				paddingSize = fd.size - streamSizeL;  
			}			
		}
	}
	// limit within 32 bit and further limit to 10000 chars
	if (streamSizeL > 10000){
		return PrintString(iobuff,"!! too big > 10000 characters!!",fd);
	}
	uint32 streamSize = (uint32)streamSizeL;
	
	bool ret = true;
	uint32 i;
	if (!fd.leftAligned && (paddingSize > 0)){
		for (i=0;i < paddingSize;i++) ret = ret && iobuff.PutC(' ');		
	}

	char c;
	while (streamSize > 0){
		if (!stream->GetC(c)){
			return false;
		}
		if (!iobuff.PutC(c)){
			return false;
		}
		streamSize--;
	}
		
	if (fd.leftAligned && (paddingSize > 0)){
		for (i=0;i < paddingSize;i++) ret = ret && iobuff.PutC(' ');		
	}
	
	return ret;
}


/**
 *  Prints any object of 
 */
static 
bool PrintToStream(
						IOBuffer &    			iobuff,
						const AnyType & 		par,
						FormatDescriptor 		fd)
{

	/// empty - an error?
	if (par.dataPointer == NULL) return false;

	if (par.dataDescriptor.isStructuredData ){
		ErrorManagement::ReportError(UnsupportedError, "Streamable::Print StructuredData not supported");
		return false;
	}
	
	switch (par.dataDescriptor.type){

	case TypeDescriptor::UnsignedInteger: 
	{
		if (par.bitAddress == 0){
			switch (par.dataDescriptor.size){
			case 8:{
				uint8 *data = (uint8 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			case 16:{
				uint16 *data = (uint16 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			case 32:{
				uint32 *data = (uint32 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			case 64:{
				uint64 *data = (uint64 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			}
		} 
		// use native standard integer
		unsigned int *number = (unsigned int *)par.dataPointer;
		// all the remaining cases here
		return BitSetToStream(iobuff,number,par.bitAddress,par.dataDescriptor.size,false,fd);
		
	} break;
	case TypeDescriptor::SignedInteger:
	{
		if (par.bitAddress == 0){
			switch (par.dataDescriptor.size){
			case 8:{
				int8 *data = (int8 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			case 16:{
				int16 *data = (int16 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			case 32:{
				int32 *data = (int32 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			case 64:{
				int64 *data = (int64 *)par.dataPointer;
				return IntegerToStream(iobuff,*data,fd);
			} break;
			}
		}
		// use native standard integer
		unsigned int *number = (unsigned int *)par.dataPointer;
		// all the remaining cases here
		return BitSetToStream(iobuff,number,par.bitAddress,par.dataDescriptor.size,true,fd);
		
	}break;
	case TypeDescriptor::Float:{
		switch (par.dataDescriptor.size){
		case 32:{
			float *data = (float *)par.dataPointer;
			return FloatToStream(iobuff,*data,fd);
		} break;
		case 64:{
			double *data = (double *)par.dataPointer;
			return FloatToStream(iobuff,*data,fd);
		} break;
		case 128:{
			REPORT_ERROR(UnsupportedError,"unsupported 128 bit float")			
			return false;
		} break;
		default:{
			REPORT_ERROR(ParametersError,"non standard float size")			
			return false;
		}
		} 
	}break;
	case TypeDescriptor::Pointer:{
		AnyType at(par);
		at.dataDescriptor.type = TypeDescriptor::UnsignedInteger;
		// the UnsignedInteger expects a pointer to the number
		at.dataPointer = (void *)&par.dataPointer;
		return PrintToStream(iobuff,at,fd);
	}	
	case TypeDescriptor::CCString:{
		if (fd.binaryNotation == Notation::HexNotation){
			AnyType at(par);
			at.dataDescriptor.type = TypeDescriptor::UnsignedInteger;
			//the UnsignedInteger expects a pointer to the number
			at.dataPointer = (void *)&par.dataPointer;
			return PrintToStream(iobuff,at,fd);
		} 
		const char *string = (const char *)par.dataPointer;
		return PrintString(iobuff,string,fd);	
	} break;
	case TypeDescriptor::StreamInterface:{
		StreamInterface * stream = (StreamInterface *)par.dataPointer;
		return PrintStream(iobuff,stream,fd);
		
	} break;
	
	default:{
	}
	}

	REPORT_ERROR(UnsupportedError,"unsupported format")			
	return false;
}

static
bool PrintFormattedToStream(
				IOBuffer &				iobuff,
				const char *			format, 
				const AnyType 			pars[])
{
	// indicates active parameter
	int parsIndex = 0;
	// checks silly parameter
	if (format == NULL) return false;
	
	// loops through parameters
	while(1){
		// scans for % and in the meantime prints what it encounters
		while ((*format !=0) && (*format != '%')) {
			if (!iobuff.PutC(*format)) return false;
			format++;
		}
                
		// end of format
		if (*format == 0) return true;

                // consume %
                format++;
		
		// if not end then %
		// keep on parsing format to build a FormatDescriptor
		FormatDescriptor fd;
		if (!fd.InitialiseFromString(format)) {
                    return false;
                }
		
		// if void simply skip and continue
		if (!pars[parsIndex].IsVoid()){
		    // use it to process parameters
		    if (!PrintToStream(iobuff,pars[parsIndex++], fd)) return false;
		}
	}
    // never comes here!
	return false;
}



#endif
