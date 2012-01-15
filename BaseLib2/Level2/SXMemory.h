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
 * Streamable eXtension for a buffer. Read and write from/to a memory buffer
 */
#if !defined (SX_MEMORY_H)
#define SX_MEMORY_H

#include "System.h"
#include "Streamable.h"

class SXMemory: public Streamable{
    /** */
    char *memory;

    /** */
    uint32 memorySize;

    /** */
    uint32 memoryPos;

public:
    /** */
    virtual ~SXMemory(){ }

    /** */
    SXMemory(char *mem,uint32 size){
        memory = mem;
        memorySize = size;
        memoryPos = 0;
    }


    // PURE STREAMING
    /** */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if ((memory == NULL) || (buffer == NULL)){
            size = 0;
            return False;
        }
        uint32 free = memorySize -  memoryPos;
        if (size > free) size = free;
        memcpy(buffer,&memory[memoryPos],size);
        memoryPos += size;
        return (size > 0);
    }

    /** */
    virtual bool CanRead(){ return (memory != NULL); };

    /** */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if ((memory == NULL) || (buffer == NULL)){
            size = 0;
            return False;
        }
        uint32 free = memorySize -  memoryPos;
        if (size > free) size = free;
        memcpy(&memory[memoryPos],buffer,size);
        memoryPos += size;
        return (size > 0);
    }

    /** */
    virtual bool CanWrite(){ return (memory != NULL); };

    // RANDOM ACCESS INTERFACE

    /** */
    virtual int64 Size(){
        if (memory == NULL) return -1;
        return memorySize;
    }

    /** */
    virtual bool  Seek(int64 pos){
        if (memory == NULL) return False;
        memoryPos = pos;
        if (memoryPos > memorySize) memoryPos = memorySize;
        return True;
    }

    /** */
    virtual int64 Position(void){
        if (memory == NULL) return -1;
        return memoryPos;
    }

    /** */
    virtual bool SetSize(int64 size){
        if (memory == NULL) return False;
        return False;
    }

    /** */
    virtual bool  CanSeek(){ return (memory != NULL); };
};



#endif
