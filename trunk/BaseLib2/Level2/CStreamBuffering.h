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
 * Maps a StreamInterface to a CStream
 */
#if !defined (CSTREAM_BUFFERING_H)
#define CSTREAM_BUFFERING_H

struct CStream;

#include "System.h"
#include "CStream.h"
#include "StreamInterface.h"

extern "C" {
    /** calls the Read method of StreamInterface */
    void CStreamBufferingReadFN(CStream *p);

    /** calls the Write method of StreamInterface */
    void CStreamBufferingWriteFN(CStream *p);

    /** calls the Seek and Position method of StreamInterface */
    void CStreamBufferingSeekFN(CStream *p);
}

/** provide buffering to CStreams */
class CStreamBuffering{
public:

    /** */
    char *              saveBuffer;

    /** */
    uint32              bufferSize;

    /** The buffered stream*/
    StreamInterface *   stream;

    /** */
    CStream             cs;

    /** replace with an enum */
    int                 mode;

public:
    /** */
    void                ReduceSize(int bufferSize)
    {
        if (this->bufferSize > bufferSize) {
            this->bufferSize = bufferSize;
        }
    }

    /** Creates the object*/
                        CStreamBuffering(
                            StreamInterface *       stream,
                            char *                  buffer,
                            int                     bufferSize)
    {
        this->stream        = stream;
        cs.bufferPtr        = buffer;
        saveBuffer          = buffer;
        this->bufferSize    = bufferSize;
        cs.context          = this;
        cs.sizeLeft         = 0;
        mode                = 2;
        cs.NewBuffer        = CStreamBufferingReadFN;
    }

    /** */
    virtual             ~CStreamBuffering()
    {
        Flush();
        mode            = 0;
    }

    /** */
    CStream *           UseAsOutput()
    {
        if (mode != 1) {
            Flush();
            mode = 1;
        }
        cs.NewBuffer    = CStreamBufferingWriteFN;
        cs.sizeLeft     = bufferSize;
        return &cs;
    }

    /** */
    CStream *           UseAsInput()
    {
        if (mode != 2) {
            Flush();
            mode = 2;
        }
        cs.NewBuffer    = CStreamBufferingReadFN;
        return &cs;
    }

    /** */
    void                Flush()
    {
        if (stream == NULL) return;
        if (mode == 1)  CStreamBufferingWriteFN(&cs);
        if (mode == 2)  CStreamBufferingSeekFN(&cs);
    }

    /** */
    int32               RelativePosition()
    {
        if (mode == 1) return -cs.sizeLeft;
        if (mode == 2) return bufferSize-cs.sizeLeft;
        return 0;
    }
};


#endif
