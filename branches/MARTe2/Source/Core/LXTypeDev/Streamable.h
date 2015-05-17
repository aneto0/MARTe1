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


#if !defined STREAMABLE
#define STREAMABLE

//#include "TypeConversion.h"
#include "TimeoutType.h"
#include "StreamInterface.h"
#include "AnyType.h"

/** 
 * common point of implementtion between Streamable descendent 
 * and MemoryMappedStreams
 * provide common abstract buffering scheme
 */
class Streamable: public StreamInterface {
       
protected: // mode switch methods

    /// default constructor
    Streamable(){
    }

    /// default destructor
    virtual ~Streamable();

    
    // MULTISTREAM INTERFACE
    // DEFAULT IMPLEMENTATION = NO MULTISTREAM
    
    /**
     * @brief Returns how many streams are available.
     * @return the number ov avaiable streams. */
    virtual uint32      NOfStreams();

    /**
     * @brief Select the stream to read from. Switching may reset the stream to the start.
     * @param n is the number of the desired stream.
     * @return false in case of errors.  */
    virtual bool        Switch(uint32 n);

    /**
     * @brief Select the stream to read from. Switching may reset the stream to the start.
     * @param name is the name of the desired stream.
     * @return false in case of errors. */
    virtual bool        Switch(const char *name);

    /** 
     * @brief Returns the number of the selected stream.
     * @return the number of the selected stream.
     */
    virtual uint32      SelectedStream();

    /**
     * @brief The name of a stream.
     * @param n is the number of the desired stream.
     * @param name is the output buffer which will contains the stream name.
     * @param nameSize is the size of the buffer.
     * @return false in case of errors.
     */
    virtual bool        StreamName(uint32 n,char *name,int nameSize) const ;

    /**
     * @brief Add a new stream to write to.
     * @param name is the name to be assigned to the new stream.
     * @return false in case of errors. */
    virtual bool        AddStream(const char *name);

    /**  
     * @brief Remove an existing stream.
     * @param name is the name of the stream to remove. */
    virtual bool        RemoveStream(const char *name);
    
    
public:  // auxiliary functions based on buffering

    /** */
    operator AnyType(){
    	AnyType at;
    	at.dataPointer = (void *)this;
    	at.bitAddress  = 0;
    	at.dataDescriptor.type = TypeDescriptor::StreamInterface;
    	at.dataDescriptor.isConstant = false;
    	
    	return at;
    }   
    
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
    						StreamInterface &   output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL);
   
    
    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                            uint32              count,
                            const char *        terminator);

      
    //  Methods to convert and print numbers and other objects 
    
    /**
     * Very powerful function to handle data conversion into a stream of chars
    */
    virtual bool 		Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor);

    /** 
         pars is a vector terminated by voidAnyType value
         format follows the TypeDescriptor::InitialiseFromString
         prints all data pointed to by pars
    */
    virtual bool 		PrintFormatted(const char *format, const AnyType pars[]);

    /**
     * copies a const char* into this stream from current position
    */
    virtual bool 		Copy(const char *buffer);

    /**
     * copies from stream current Position to end
    */
    virtual bool 		Copy(StreamInterface &stream); 
    
    
};


#endif
