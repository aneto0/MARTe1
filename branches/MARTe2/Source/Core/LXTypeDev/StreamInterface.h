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
 * $Id: StreamInterface.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

/** 
 * @file 
 * The base interface for the streams
 */
#ifndef STREAM_INTERFACE_H
#define STREAM_INTERFACE_H

#include "TimeoutType.h"
#include "AnyType.h"
#include "FormatDescriptor.h"


/** A more abstract version of Streamable. It is used to allow referring to streams at lower levels */
class StreamInterface{

public:
    /** the destructor */
    virtual             ~StreamInterface(){};

public: 

    // PURE STREAMING (UNBUFFERED)

    /** 
        Reads data into buffer. 
        As much as size byte are read, 
        actual read size is returned in size. (unless complete = True)
        msecTimeout is how much the operation should last - no more - if not any (all) data read then return false  
        timeout behaviour depends on class characteristics and sync mode.
    */
    virtual bool        Read(
                            char*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false)=0;

    /** 
        Write data from a buffer to the stream. 
        As much as size byte are written, 
        actual written size is returned in size. 
        msecTimeout is how much the operation should last.
        timeout behaviour depends on class characteristics and sync mode. 
    */
    virtual bool        Write(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false) = 0;

    /** whether it can be written into */
    virtual bool        CanWrite()const =0;

    /** whether it can be  read */
    virtual bool        CanRead()const =0;
    
    
    /// simply write to buffer if space exist and if operatingModes allows
    inline bool         PutC(char c){
    	uint32 size = 1;
    	return Write(&c,size);
    }

    /// simply read from buffer 
    inline bool         GetC(char &c) {
    	uint32 size = 1;
    	return Read(&c,size);
    	
    }

    // SYNCHRONISATION INTERFACE

    /** 
       whether it can wait to complete operation 
       or in absence of data or buffer space the 
       operation terminates immediately       
       msecTimeout is used to limit the blocking time 
       when blocking is active. It is not used otherwise - unless
       complete is set in which case the operation is tried multiple 
       times each after fixed interval (set as timeout/10 or 1ms min) 
       This implies that class implementations of Read and Write 
       might have to involve a select call or something similar
       for classes light String blocking has no meaning.        
    */
    virtual bool        CanBlock(){ return false; };
    
    /** activates blocking mode */
    virtual bool        SetBlocking(bool flag){ return false; };

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size()  = 0;

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos) = 0;
    
    /** Moves within the file relative to current location */
    virtual bool        RelativeSeek(int32 deltaPos)=0;
    
    /** Returns current position */
    virtual int64       Position()  = 0 ;

    /** Clip the stream size to a specified point */
    virtual bool        SetSize(int64 size) = 0;

    /** can you move the pointer */
    virtual bool        CanSeek()const =0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    virtual uint32      NOfStreams() =0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n)=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name)=0;

    /** how many streams are available */
    virtual uint32      SelectedStream()=0;

    /** the name of the stream we are using */
    virtual bool        StreamName(uint32 n,char *name,int nameSize)const =0;

    /**  add a new stream to write to. */
    virtual bool        AddStream(const char *name)=0;

    /**  remove an existing stream . */
    virtual bool        RemoveStream(const char *name)=0;

    // Utility functions interface

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
                            const char *        skipCharacters)=0;

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
    		                StreamInterface &  	output,
                            const char *        terminator,
                            char *              saveTerminator=NULL,
                            const char *        skipCharacters=NULL)=0;

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                            uint32              count,
                            const char *        terminator)=0;
  
    /** Extract a line */
    inline bool 		GetLine(
    						char *				outputBuffer,
    						uint32 				outputBufferSize,
    						bool 				skipTerminators=True){
        const char *skipCharacters = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skipCharacters = "\r";
#else
        if (!skipTerminators) skipCharacters = "";
#endif
        return GetToken(outputBuffer,"\n",outputBufferSize,NULL,skipCharacters);
    }

    /** @param skipTerminators will skip an empty line or any part of a line termination */
    inline bool 		GetLine(
    						StreamInterface &  	output,
    						bool 				skipTerminators=True){
        const char *skipCharacters = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skipCharacters = "\r";
#else
        if (!skipTerminators) skipCharacters = "";
#endif
        return GetToken(output,"\n",NULL,skipCharacters);
    }    
    
    /**
     * Very powerful function to handle data conversion into a stream of chars
    */
    virtual bool 		Print(const AnyType& par,FormatDescriptor fd=standardFormatDescriptor)=0;

    /** 
         pars is a vector terminated by voidAnyType value
         format follows the TypeDescriptor::InitialiseFromString
         prints all data pointed to by pars
    */
    virtual bool 		PrintFormatted(const char *format, const AnyType pars[])=0;
    
    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1){
    	AnyType pars[2] = { par1,voidAnyType};
    	return PrintFormatted(format, pars);
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1, const AnyType& par2){
    	AnyType pars[3] = { par1,par2,voidAnyType}; 
    	return PrintFormatted(format, pars);
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1, const AnyType& par2, const AnyType& par3){
    	AnyType pars[4] = { par1,par2,par3,voidAnyType}; 
    	return PrintFormatted(format, pars);
    }

    /** 
    */
    inline bool PrintFormatted(const char *format, const AnyType& par1, const AnyType& par2, const AnyType& par3, const AnyType& par4){
    	AnyType pars[5] = { par1,par2,par3,par4,voidAnyType}; 
    	return PrintFormatted(format, pars);
    }
    
};

#endif
