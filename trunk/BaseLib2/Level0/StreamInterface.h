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
 * $Id$
 *
**/

/** 
 * @file 
 * The base interface for the streams
 */
#ifndef STREAM_INTERFACE_H
#define STREAM_INTERFACE_H

#include "System.h"
#include "TimeoutType.h"

/** size of Standard stream buffering
    The buffers will be allocated on stack.
    Therefore keep the number low */
const uint32 StreamableBufferingSize = 32;

/** A more abstract version of Streamable. It is used to allow referring to streams at lower levels */
class StreamInterface{
public:
    /** the destructor */
    virtual             ~StreamInterface(){};

    // PURE STREAMING
    /** Reads data into buffer. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool        Read(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)=0;

    /** Write data from a buffer to the stream. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool        Write(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)=0;

    /** whether it can be written into */
    virtual bool        CanWrite()=0;

    /** whether it can be  read */
    virtual bool        CanRead()=0;


    /** single character write operation */
    inline bool         PutC(char c)
    {
        uint32 size = 1;
        return (Write(&c, size) && (size == 1));
    }

    /** single character read operation */
    inline bool         GetC(char &c)
    {
        uint32 size = 1;
        return (Read(&c, size) && (size == 1));
    }

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size()=0;

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos)=0;

    /** Returns current position */
    virtual int64       Position(void)=0;

    /** Clip the stream size to a specified point */
    virtual bool        SetSize(int64 size)=0;

    /** can you move the pointer */
    virtual bool        CanSeek()=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    virtual uint32      NOfStreams()=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n)=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name)=0;

    /** how many streams are available */
    virtual uint32      SelectedStream()=0;

    /** the name of the stream we are using */
    virtual bool        StreamName(uint32 n,char *name,int nameSize)=0;

    /**  add a new stream to write to. */
    virtual bool        AddStream(const char *name)=0;

    /**  remove an existing stream . */
    virtual bool        RemoveStream(const char *name)=0;

    // Utility functions.

    /** the function performs the job of a the Read function but gurantees
        the completion. In case of failure size returns the actual data read.
        @param timeOutMs is the total allowed wait time cehcked using HRT.
        Any value below 0 implies infinite wait.*/
    virtual bool        CompleteRead(
                            void*           buffer,
                            uint32 &        size,
                            TimeoutType     msecTimeout = TTInfiniteWait) = 0;

    /** the function performs the job of a the Write function but gurantees
        the completion. In case of failure size returns the actual data read.
        @param timeOutMs is the total allowed wait time checked using HRT.
        Any value below 0 implies infinite wait.
        @param pollMs is the wait between retrials. */
    virtual bool        CompleteWrite(
                            const void*     buffer,
                            uint32 &        size,
                            TimeoutType     msecTimeout = TTInfiniteWait) = 0;

    /** copies @param size bytes to the stream s
        stops on Timeout on reading the input or
        on an empty input  (EOF)
        returns True on success or on EOF reached (if Timeout is InfiniteWait)
        otw False  */
    inline bool CopyTo( StreamInterface &   s,
                        int64 &             size,
                        TimeoutType         msecTimeout     = TTInfiniteWait,
                        char *              extBuffer       = NULL,
                        int                 extBufferSize   = 0){
        char buffer[StreamableBufferingSize];
        if (extBuffer == NULL){
            extBuffer = buffer;
            extBufferSize = StreamableBufferingSize;
        }
        int64 left = size;
        while(left > 0){
            uint32 toRead = extBufferSize;
            if (toRead > left) toRead = left;
            CompleteRead(extBuffer,toRead,msecTimeout);
            if (!s.CompleteWrite(extBuffer,toRead,msecTimeout)){
                size = size - left + toRead;
                return False;
            }
            left = left - toRead;
            // the available information was less than size or Timeout
            if (toRead == 0){
                size = size - left;
                if (msecTimeout == TTInfiniteWait) return True;
                return False;
            }
        }
        return True;
    }
#if 0
    /** will use CompleteWrite */
    inline void operator<<(char *p){
        if (p!=NULL){
            uint32 size= strlen(p);
            CompleteWrite(p,size);
        }
    }
#endif

#if defined(_VXWORKS)
    /** */
    bool Printf(const char *format,...);

    /**   */
    bool SSPrintf(uint32 streamNo,const char *format,...);

    /**   */
    bool SSPrintf(const char *streamName,const char *format,...);
#else

    /** Supported format flags %<pad><size><type>
        type can be o d i X x Lo Ld Li LX Lx f e s c   */
    inline bool Printf(const char *format,...){
        if (format==NULL) return False;

        va_list argList;
        va_start(argList,format);
        bool ret = VPrintf(format,argList);
        va_end(argList);

        return ret;
    }

    /** selects a stream and writes to it, then returns to previous
        buffer is a zero terminated string  */
    inline bool SSPrintf(uint32 streamNo,const char *format,...){
        if (format==NULL) return False;

        uint32 save = SelectedStream();
        bool ret = False;
        if (Switch(streamNo)){
            va_list argList;
            va_start(argList,format);
            ret = VPrintf(format,argList);
            va_end(argList);
        }
        Switch(save);
        return ret;
    }

    /** creates streams with AddStreams and writes into them  */
    inline bool SSPrintf(const char *streamName,const char *format,...){
        if (format==NULL) return False;

        uint32 save = SelectedStream();
        bool ret = False;

        AddStream(streamName);
        if (Switch(streamName)){
            va_list argList;
            va_start(argList,format);
            ret = VPrintf(format,argList);
            va_end(argList);
        }
        Switch(save);
        return ret;
    }

#endif

    /** Printf with vararg format */
    virtual bool VPrintf(const char *format,va_list argList)=0;

    /** extract a token from the stream into a string data until a terminator or 0 is found. maxSize is the buffer size.
        The maximum string size is maxSize -1
        Skips all skip characters even if calssified also as terminators if at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        {BUFFERED}    */
    virtual bool GetToken(      char *              buffer,
                                const char *        terminator,
                                uint32              maxSize,
                                char *              saveTerminator=NULL,
                                const char *        skipCharacters=NULL)=0;

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
        3) skip + terminator    the character is not copied, the string is terminated if not empty */
    virtual bool GetToken(
                                StreamInterface &   output,
                                const char *        terminator,
                                char *              saveTerminator=NULL,
                                const char *        skipCharacters=NULL)=0;

    /** Extract a line */
    virtual bool GetLine(char *buffer,uint32 maxSize,bool skipTerminators=True){
        const char *skip = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skip = "\r";
#else
        if (!skipTerminators) skip = "";
#endif
        return GetToken(buffer,"\n",maxSize,NULL,skip);
    }

    /** @param skipTerminators will skip an empty line or any part of a line termination */
    virtual bool GetLine(StreamInterface &output,bool skipTerminators=True){
        const char *skip = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skip = "\r";
#else
        if (!skipTerminators) skip = "";
#endif
        return GetToken(output,"\n",NULL,skip);
    }

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool SkipTokens(uint32 count,const char *terminator)=0;

};

#endif
