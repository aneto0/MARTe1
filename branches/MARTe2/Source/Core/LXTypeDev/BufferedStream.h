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


#if !defined BUFFERED_STREAM
#define BUFFERED_STREAM

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "IOBuffer.h"


/** 
 * common point of implementtion between Streamable descendent 
 * and MemoryMappedStreams
 * provide common abstract buffering scheme
 */
class BufferedStream: public StreamInterface {
       
protected: // methods to be implemented by deriving classes
    ///
    virtual IOBuffer *GetInputBuffer() = 0;

    ///
    virtual IOBuffer *GetOutputBuffer() = 0;

protected: // mode switch methods

    /// default constructor
    BufferedStream(){
    }

    /// default destructor
    virtual ~BufferedStream();
      
public:  // auxiliary functions based on buffering
    
    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters and those that are also terminators at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        skipCharacters=NULL is equivalent to skipCharacters = terminator
        {BUFFERED}    */
    virtual bool        GetToken(
                            char *              outputBuffer,
                            const char *        terminator,
                            uint32              outputBufferSize,
                            char *              saveTerminator,
                            const char *        skipCharacters);

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
    virtual bool        GetToken(
    		                BufferedStream &    output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL);

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                            uint32              count,
                            const char *        terminator);

      
public: //  Methods to convert and print numbers and other objects 
    
    /**
     * Very powerful function to handle data conversion into a stream of chars
    */
    virtual bool Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor);

    /** 
         pars is a vector terminated by voidAnyType value
         format follows the TypeDescriptor::InitialiseFromString
         prints all data pointed to by pars
    */
    virtual bool PrintFormatted(const char *format, const AnyType pars[]);

};


#endif
