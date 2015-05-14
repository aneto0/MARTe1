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
#include "BufferedStreamIOBuffer.h"


/// default destructor
BufferedStream::~BufferedStream(){
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
	
	// retrieve stream mechanism
	IOBuffer *inputIOBuffer = GetInputBuffer(); 
	if (inputIOBuffer == NULL){
		char stackBuffer[64];		
		BufferedStreamIOBuffer inputIOBuffer (this,stackBuffer,sizeof (stackBuffer));
		
		bool ret = GetTokenFromStream(inputIOBuffer, outputBuffer,terminator,outputBufferSize,saveTerminator,skipCharacters);
		
		return ret;
	}
	
	return GetTokenFromStream(*inputIOBuffer, outputBuffer,terminator,outputBufferSize,saveTerminator,skipCharacters);
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
							StreamInterface &   output,
                            const char *        terminator,
                            char *              saveTerminator,
                            const char *        skipCharacters){

	// retrieve stream mechanism
	IOBuffer *inputIOBuffer   = GetInputBuffer();
	IOBuffer *outputIOBuffer  = output.GetOutputBuffer();

	bool ret = false; 
	
	if (inputIOBuffer == NULL){
		char stackBuffer[64];		
		BufferedStreamIOBuffer inputIOBufferS (this,stackBuffer,sizeof (stackBuffer));		
		
		IOBuffer *outputIOBuffer  = GetOutputBuffer();		
		if (outputIOBuffer == NULL){
			char stackBuffer[64];		
			BufferedStreamIOBuffer outputIOBufferS (this,stackBuffer,sizeof (stackBuffer));		

			ret = GetTokenFromStream(inputIOBufferS, outputIOBufferS,terminator,saveTerminator,skipCharacters);
			
		} else { 
			inputIOBuffer = &inputIOBufferS; 
		
			ret = GetTokenFromStream(*inputIOBuffer, *outputIOBuffer,terminator,saveTerminator,skipCharacters);
		
		}
	} else {
		
		if (outputIOBuffer == NULL){
			char stackBuffer[64];		
			BufferedStreamIOBuffer outputIOBufferS (this,stackBuffer,sizeof (stackBuffer));		

			outputIOBuffer = &outputIOBufferS; 

			ret = GetTokenFromStream(*inputIOBuffer, *outputIOBuffer,terminator,saveTerminator,skipCharacters);
			
		} else { 
		
			ret = GetTokenFromStream(*inputIOBuffer, *outputIOBuffer,terminator,saveTerminator,skipCharacters);
		
		}		
	}

	return ret;
}
	

/** to skip a series of tokens delimited by terminators or 0
    {BUFFERED}    */
bool BufferedStream::SkipTokens(
                            uint32              count,
                            const char *        terminator){

	// retrieve stream mechanism
	IOBuffer *inputBuffer = GetInputBuffer(); 
	if (inputBuffer == NULL){
		char stackBuffer[64];		
		BufferedStreamIOBuffer inputBuffer (this,stackBuffer,sizeof (stackBuffer));
		
		return SkipTokensInStream(inputBuffer,count,terminator);
	}
	
	return SkipTokensInStream(*inputBuffer,count,terminator);
}

bool BufferedStream::Print(const AnyType& par,FormatDescriptor fd){

	// retrieve stream mechanism
	IOBuffer *inputBuffer = GetInputBuffer(); 
	if (inputBuffer == NULL){
		char stackBuffer[64];		
		BufferedStreamIOBuffer inputBuffer (this,stackBuffer,sizeof (stackBuffer));

		return PrintToStream(inputBuffer,par,fd);
	}
		
	return PrintToStream(*inputBuffer,par,fd);
}

bool BufferedStream::PrintFormatted(const char *format, const AnyType pars[]){

	// retrieve stream mechanism
	IOBuffer *inputBuffer = GetInputBuffer(); 
	if (inputBuffer == NULL){
		char stackBuffer[64];
		BufferedStreamIOBuffer inputBuffer (this,stackBuffer,sizeof (stackBuffer));
		
		return PrintFormattedToStream(inputBuffer,format,pars);
	}
	return PrintFormattedToStream(*inputBuffer,format,pars);
}















