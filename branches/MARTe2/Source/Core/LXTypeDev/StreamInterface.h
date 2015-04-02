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

#include "System.h"
#include "TimeoutType.h"


/** A more abstract version of Streamable. It is used to allow referring to streams at lower levels */
class StreamInterface{

protected:
    // PURE STREAMING
    /** Reads data into buffer. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool        PrivateRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)=0;

    /** Write data from a buffer to the stream. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    virtual bool        PrivateWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)=0;

    /** whether it can be written into */
    virtual bool        PrivateCanWrite()=0;

    /** whether it can be  read */
    virtual bool        PrivateCanRead()=0;

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    virtual int64       PrivateSize()=0;

    /** Moves within the file to an absolute location */
    virtual bool        PrivateSeek(int64 pos)=0;

    /** Returns current position */
    virtual int64       PrivatePosition(void)=0;

    /** Clip the stream size to a specified point */
    virtual bool        PrivateSetSize(int64 size)=0;

    /** can you move the pointer */
    virtual bool        PrivateCanSeek()=0;

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    virtual uint32      PrivateNOfStreams()=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        PrivateSwitch(uint32 n)=0;

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        PrivateSwitch(const char *name)=0;

    /** how many streams are available */
    virtual uint32      PrivateSelectedStream()=0;

    /** the name of the stream we are using */
    virtual bool        PrivateStreamName(uint32 n,char *name,int nameSize)=0;

    /**  add a new stream to write to. */
    virtual bool        PrivateAddStream(const char *name)=0;

    /**  remove an existing stream . */
    virtual bool        PrivateRemoveStream(const char *name)=0;

public:
    /** the destructor */
    virtual             ~StreamInterface(){};

public: 

    // PURE UNBUFFERED STREAMING

    /** Reads data into buffer. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    inline bool         Read(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return PrivateRead(buffer,size,msecTimeout);
    }

    /** Write data from a buffer to the stream. As much as size byte are written, actual size
        is returned in size. msecTimeout is how much the operation should last.
        timeout behaviour is class specific. I.E. sockets with blocking activated wait forever
        when noWait is used .... */
    inline bool         Write(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return PrivateWite(buffer,size,msecTimeout);
    }

    /** whether it can be written into */
    inline bool         CanWrite(){ return PrivateCanWrite(); }

    /** whether it can be  read */
    inline bool         CanRead() { return PrivateCanRead(); }

    // RANDOM ACCESS INTERFACE

    /** The size of the stream */
    inline int64        Size()    { return PrivateSize(); }

    /** Moves within the file to an absolute location */
    inline bool         Seek(int64 pos)
    {
        return PrivateSeek(pos);
    }

    /** Returns current position */
    inline int64        Position() { return PrivatePosition(); }

    /** Clip the stream size to a specified point */
    inline bool         SetSize(int64 size)
    {
        return PrivateSetSize(size);
    }

    /** can you move the pointer */
    inline bool         CanSeek()  { return PrivateCanSeek(); }

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    inline uint32       NOfStreams(){return PrivateNOfStreams(); }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n)
    {
        return PrivateSwitch(n);
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name)
    {
        return PrivateSwitch(name);
    }

    /** how many streams are available */
    virtual uint32      SelectedStream()
    {
        return PrivateSelectedStream();
    }  

    /** the name of the stream we are using */
    virtual bool        StreamName(uint32 n,char *name,int nameSize)
    {
        return PrivateStreamName(n,name,nameSize);
    }

    /**  add a new stream to write to. */
    virtual bool        AddStream(const char *name)
    {
        return PrivateAddStream(name);
    }

    /**  remove an existing stream . */
    virtual bool        RemoveStream(const char *name)
    {
        return PrivateRemoveStream(name);
    }

};

#endif
