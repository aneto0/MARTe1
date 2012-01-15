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

#include "CStreamBuffering.h"
#include "StreamInterface.h"

void CStreamBufferingReadFN(CStream *p){
    if (p==NULL) return;
    if (p->context == NULL) return;

    CStreamBuffering *scsc = (CStreamBuffering *)p->context;
    if (scsc->stream       == NULL) return;
    if (scsc->cs.bufferPtr == NULL) return;

    scsc->cs.sizeLeft  = scsc->bufferSize;
    scsc->cs.bufferPtr = scsc->saveBuffer;
    scsc->stream->Read(scsc->cs.bufferPtr,scsc->cs.sizeLeft);
}

void CStreamBufferingSeekFN(CStream *p){
    if (p==NULL) return;
    if (p->context == NULL) return;

    CStreamBuffering *scsc = (CStreamBuffering *)p->context;
    if (scsc->stream == NULL) return;

    scsc->stream->Seek(scsc->stream->Position() - p->sizeLeft);
    scsc->cs.bufferPtr = scsc->saveBuffer;
    scsc->cs.sizeLeft  = 0;
}


void CStreamBufferingWriteFN(CStream *p){
    if (p==NULL) return;
    if (p->context == NULL) return;

    CStreamBuffering *scsc = (CStreamBuffering *)p->context;
    if (scsc->stream == NULL) return;
    if (scsc->cs.bufferPtr == NULL) return;

    uint32 size = scsc->bufferSize - p->sizeLeft;
    scsc->cs.sizeLeft  = scsc->bufferSize;
    scsc->cs.bufferPtr = scsc->saveBuffer;
    if (size>0) scsc->stream->Write(scsc->cs.bufferPtr,size);

}




