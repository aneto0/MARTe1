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
 * A simple StreamInterface using printf on the console
 */
#ifndef PRINTF_STREAM_INTERFACE_H
#define PRINTF_STREAM_INTERFACE_H

#include "StreamInterface.h"

/** a simple implementation of Stream Interface that uses printf */
class PrintfStreamInterface: public StreamInterface{
public:

    // PURE STREAMING
    /** the buffer is written into */
    virtual bool Read(void* buffer, uint32 &sizel,TimeoutType msecTimeout){return False;}

    /** whether it can be  read */
    virtual bool CanRead(){return False;}

    /** buffer is read and copied into the stream */
    virtual bool Write(const void* buffer, uint32 &size,TimeoutType msecTimeout){return False;}

    /** whether it can be written into */
    virtual bool CanWrite(){return False;}

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64 Size(){return 0;}

    /** Moves within the file to an absolute location */
    virtual bool Seek(int64 pos){return False;}

    /** Returns current position */
    virtual int64 Position(void){return 0;}

    /** Clip the stream size to a specified point */
    virtual bool SetSize(int64 size){return False;}

    /** can you move the pointer */
    virtual bool CanSeek(){return False;}

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    virtual uint32 NOfStreams(){return 0;}

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool Switch(uint32 n){return False;}

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool Switch(const char *name){return False;}

    /** how many streams are available */
    virtual uint32 SelectedStream(){return 0;}

    /** the name of the stream we are using */
    virtual bool StreamName(uint32 n,char *name,int nameSize){return False;}

    /**  add a new stream to write to. */
    virtual bool AddStream(const char *name){return False;}

    /**  remove an existing stream . */
    virtual bool RemoveStream(const char *name){return False;}

    // Utility functions.

    virtual bool        CompleteRead(
                            void*           buffer,
                            uint32 &        size,
                            TimeoutType     msecTimeout = TTInfiniteWait)
    {
        return False;
    }

    /** the function performs the job of a the Write function but gurantees
        the completion. In case of failure size returns the actual data read.
        @param timeOutMs is the total allowed wait time checked using HRT.
        Any value below 0 implies infinite wait.
        @param pollMs is the wait between retrials. */
    virtual bool        CompleteWrite(
                            const void*     buffer,
                            uint32 &        size,
                            TimeoutType     msecTimeout = TTInfiniteWait)
    {
        return False;
    }


    /** */
    virtual bool Printf(const char *format,...){
        if (format==NULL) return False;

        va_list argList;
        va_start(argList,format);
        bool ret = vprintf(format,argList);
        va_end(argList);

        return ret;
    }


    /** */
    virtual bool VPrintf(const char *format,va_list argList){
        return (vprintf(format,argList) != 0);
    };

    /** selects a stream and writes to it, then returns to previous
        buffer is a zero terminated string  */
    virtual bool SSPrintf(uint32 streamNo,const char *format,...){
        if(streamNo == 0){
            if (format==NULL) return False;

            va_list argList;
            va_start(argList,format);
            bool ret = vprintf(format,argList);
            va_end(argList);

            return ret;
        }
        return False;
    }

    /** extract a token from the stream into a string data until a terminator or 0 is found. maxSize is the buffer size.
        The maximum string size is maxSize -1
        Skips all skip characters even if calssified also as terminators if at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        {BUFFERED}    */
    virtual bool GetToken(char *buffer,const char *terminator,uint32 maxSize,char *saveTerminator=NULL,const char *skipCharacters=NULL){return False;}

    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters even if calssified also as terminators if at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        {BUFFERED}    */
    virtual bool GetToken(
                                StreamInterface &   output,
                                const char *        terminator,
                                char *              saveTerminator=NULL,
                                const char *        skipCharacters=NULL)
    {
        return False;
    }

    /** */
    virtual bool GetLine(char *buffer,uint32 maxSize,bool skipTerminators=True){return False;}

    /** @param skipTerminators will skip an empty line or any part of a line termination */
//    virtual bool GetLine(Streamable &output,bool skipTerminators=True)=0;

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}      */
    virtual bool SkipTokens(uint32 count,const char *terminator){return False;}

};

#endif

