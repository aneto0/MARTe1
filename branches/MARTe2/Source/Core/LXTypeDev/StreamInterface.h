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


};

#endif
