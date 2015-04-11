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


/** A more abstract version of Streamable. It is used to allow referring to streams at lower levels */
class StreamInterface{

protected:
    // PURE STREAMING
    /** 
        Reads data into buffer. 
        As much as size byte are read, 
        actual read size is returned in size. 
        msecTimeout is how much the operation should last - no more
        timeout behaviour depends on class characteristics and sync mode. 
        I.E. sockets with blocking activated wait forever when noWait is used .... 
    */
    virtual bool        UnBufferedRead(
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
        I.E. sockets with blocking activated wait forever when noWait is used .... 
    */
    virtual bool        UnBufferedWrite(
                            const char*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault,
                            bool                complete        = false)=0;


    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       UnBufferedSize()=0;

    /** Moves within the file to an absolute location */
    virtual bool        UnBufferedSeek(int64 pos)=0;

    /** Returns current position */
    virtual int64       UnBufferedPosition()=0;

    /** Clip the stream size to a specified point */
    virtual bool        UnBufferedSetSize(int64 size)=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        UnBufferedSwitch(uint32 n)=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        UnBufferedSwitch(const char *name)=0;
    
    virtual bool        UnBufferedRemoveStream(const char *name)=0;


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
                            bool                complete        = false)
    {
        return UnBufferedRead(buffer,size,msecTimeout);
    }

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
                            bool                complete        = false)
    {
        return UnBufferedWrite(buffer,size,msecTimeout);
    }

    /** whether it can be written into */
    virtual bool        CanWrite()=0;

    /** whether it can be  read */
    virtual bool        CanRead()=0;

    // SYNCHRONISATION INTERFACE

    /** 
       whether it can wait to complete operation - msecTimeout is used to limit the blocking time
    */
    virtual bool        CanBlock()=0;
    
    /** activates blocking mode */
    virtual bool        SetBlocking(bool flag)=0;

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       Size()    { return UnBufferedSize(); }

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos)
    {
        return UnBufferedSeek(pos);
    }

    /** Returns current position */
    virtual int64       Position() { return UnBufferedPosition(); }

    /** Clip the stream size to a specified point */
    virtual bool        SetSize(int64 size)
    {
        return UnBufferedSetSize(size);
    }

    /** can you move the pointer */
    virtual bool        CanSeek()=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    virtual uint32      NOfStreams()=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n)
    {
        return UnBufferedSwitch(n);
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name)
    {
        return UnBufferedSwitch(name);
    }

    /** how many streams are available */
    virtual uint32      SelectedStream()=0;

    /** the name of the stream we are using */
    virtual bool        StreamName(uint32 n,char *name,int nameSize)=0;

    /**  add a new stream to write to. */
    virtual bool        AddStream(const char *name)=0;

    /**  remove an existing stream . */
    virtual bool        RemoveStream(const char *name)=0;

    
    
};

#endif
