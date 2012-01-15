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
 * Provides full buffering support to a Streamable by wrapping the non buffering supporting functions
 */
#if !defined (BUFFERED_STREAM_H)
#define BUFFERED_STREAM_H

#include "Streamable.h"
#include "System.h"


OBJECT_DLL(BufferedStream)

class BufferedStream:public Streamable{
OBJECT_DLL_STUFF(BufferedStream)

    /** the stream being buffered */
    Streamable *stream;

    /** the buffer */
    char *buffer;

public:
    /** allocates memory and sets the stream */
    BufferedStream(Streamable *stream,int32 bufferSize){
        buffer = NULL;
        this->stream = NULL;
        if (stream==NULL) return;
        this->stream = stream;

         if (stream->CanSeek()){
            buffer = (char *)malloc(bufferSize);
            if (buffer==NULL) return;
            csbIn  = new CStreamBuffering(stream,buffer,bufferSize);
            csbOut = csbIn;
        } else {
            buffer = (char *)malloc(bufferSize*2);
            if (buffer==NULL) return;
            csbIn  = new CStreamBuffering(stream,buffer,bufferSize);
            csbOut = new CStreamBuffering(stream,buffer+bufferSize,bufferSize);
        }
    }


    /**  the destructor */
    virtual ~BufferedStream(){
        if ((csbOut != csbIn) && (csbOut != NULL)) delete csbOut;
        if (csbIn != NULL) delete csbIn;
        if (buffer != NULL) free((void *&)buffer);
        buffer = NULL;
    }

    // PURE STREAMING
    /** Uses Streamable::BufferedRead. */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (stream == NULL) return False;
        return stream->BufferedRead(buffer, size);
    }


    /** Uses Streamable::BufferedWrite. */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (stream == NULL) return False;
        return stream->BufferedWrite(buffer, size);
    }


    /** */
    virtual bool CanRead(){
        if (stream == NULL) return False;
        return stream->CanRead();
    }


    /** */
    virtual bool CanWrite(){
        if (stream == NULL) return False;
        return stream->CanWrite();
    }

    // RANDOM ACCESS INTERFACE

    /** */
    virtual int64 Size(){
        if (stream == NULL) return False;
        return stream->Size();
    }


    /** */
    virtual bool  Seek(int64 pos){
        if (stream == NULL) return False;
        Flush();
        return stream->Seek(pos);
    }


    /** */
    virtual int64 Position(void){
        if (stream == NULL) return -1;
        if (csbIn == NULL) return stream->Position();
        return stream->Position() + csbIn->RelativePosition();
    }


    /** */
    virtual bool  SetSize(int64 size){
        if (stream == NULL) return False;
        Flush();
        return stream->SetSize(size);
    }

    /** */
    virtual bool  CanSeek(){
        if (stream == NULL) return False;
        return stream->CanSeek();
    }


    // Extended Attributes or Multiple Streams INTERFACE

    /** */
    virtual uint32 NOfStreams(){
        if (stream == NULL) return 0;
        return stream->NOfStreams();
    }

    /** */
    virtual bool Switch(uint32 n){
        Flush();
        if (stream == NULL) return False;
        return stream->Switch(n);
    }

    /** */
    virtual bool Switch(const char *name){
        Flush();
        if (stream == NULL) return False;
        return stream->Switch(name);
    }

    /** */
    virtual uint32 SelectedStream(){
        if (stream == NULL) return 0xFFFFFFFF;
        return stream->SelectedStream();
    }


    /** */
    virtual bool StreamName(uint32 n,char *name,int nameSize){
        if (stream == NULL) return False;
        return stream->StreamName(n,name,nameSize);
    }


    /** */
    virtual bool AddStream(const char *name){
        Flush();
        if (stream == NULL) return False;
        return stream->AddStream(name);
    }

    /** */
    virtual bool RemoveStream(const char *name){
        Flush();
        if (stream == NULL) return False;
        return stream->RemoveStream(name);
    }


};



#endif
