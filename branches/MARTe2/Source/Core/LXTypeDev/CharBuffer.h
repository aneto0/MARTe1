/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/

/** 
 * @file
 * Implementation of a buffer with allocation and reallocation and access functions
 */
#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include "GeneralDefinitions.h"
#include "Memory.h"


/**
    A basic implementation of a buffer of character.
    A replacement for dealing directly with mallocs and reallocs 
    Access as char * both read-only and read-write
*/
class CharBuffer {

    /**
        allocate or reallocate memory to the desired size
        content is preserved by copy as long as it fits the newsize
        or if contiguus memory is available
        allocationGranularityMask defines how many bits to consider 
        for the buffer size. round up the others
    */
    inline void SetBufferSize(uint32 desiredSize,
                              uint32 allocationGranularityMask = 0xFFFFFFFF){
        // the mask is the 2 complement of the actual granularity
        uint32 allocationGranularity = ~allocationGranularityMask + 1;
        uint32 allocationBoundary    = ~(2 * allocationGranularity - 1);

        // stay within matematical limits
        if (desiredSize > allocationBoundary ) return ;
        uint32 neededMemory = 
	            desiredSize 
                 +  StringGranularity // to increase up to granularity boundaries
                 -  1 // so that 0 still stays below granularity ;
        neededMemory &= StringGranularityMask; 

        char *newBuffer;
        if (buffer == NULL) {
            buffer = (char *) MemoryMalloc(neededMemory);
        }
        else {
            buffer = (char *) MemoryRealloc((void * &) buffer, neededMemory);
        }
        // if the pointer is not NULL it means we have been successful
        if (buffer != NULL){
            allocatedSize = neededMemory;
        }
    }

    /** the size of the allocated memory block  */
    uint32 allocatedSize;

    /**  the memory buffer  */
    char * buffer;

    /** used for constructors */
    void InitMembers() {
        allocatedSize = 0;
        buffer = NULL;
    }

    /** used for destructors */
    void FinishMembers() {
        if (buffer != NULL)
            MemoryFree((void *&) buffer);
        allocatedSize = 0;
    }

public:

    /** Creates a buffer of a given size */
    inline CharBuffer(uint32 desiredSize=0) {
        InitMembers();
        SetBufferSize(desiredSize);
    }

    /** Destructor */
    virtual ~CharBuffer() {
        FinishMembers();
    }

    /** Read Only access to the internal buffer
     @return The pointer to the buffer
     */
    inline const char *Buffer() const {
        return buffer;
    }

    /** Read Write access top the internal buffer
       @return The pointer to the buffer
     */
    inline char *BufferReference() const {
        return buffer;
    }

    /// access to the current memory used by buffer
    inline uint32 BufferAllocatedSize(){
        return allocatedSize;
    }

    /// this is architecture dependent for optimisation purposes. It should be moved.
    /// a non inline version should be created and used instead of the code here
    /// alignment of data is important to be considered too
    inline void CopyBuffer(char *destination,const char *source,int size){
        uint64       *d = (uint64 *)destination;
        const uint64 *s = (uint64 *)source;
        while (size > 32){
            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];
            d[3] = s[3];
            size -= 32;
            d+=4;
            s+=4;
        }
        while (size > 0){
            *destination++ = *source++;
        }
    }

};

#endif

