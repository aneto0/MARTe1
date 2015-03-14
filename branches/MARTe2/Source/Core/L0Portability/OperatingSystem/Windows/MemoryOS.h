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
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
 **/
/**
 * @file
 * Basic memory management
 */
#ifndef MEMORY_OS_H
#define MEMORY_OS_H

#include <stdlib.h>

/**
 * @see Memory::MemoryMalloc
 */
void *MemoryOSMalloc(uint32 size, MemoryAllocationFlags allocFlags) {
    if (size <= 0) {
        return NULL;
    }
    return malloc(size);
}

/** 
 * @see Memory::MemoreFree
 */
void MemoryOSFree(void *&data) {
    if (data != NULL) {
        free(data);
    }
    data = NULL;
}

/** 
 * @see Memory::Realloc
 */
void *MemoryOSRealloc(void *&data, uint32 newSize) {
    return realloc(data, newSize);
}

/** 
 * @see Memory::Strdup
 */
char *MemoryOSStringDup(const char *s) {
    return strdup(s);
}

/** 
 * @see Memory::SharedAlloc
 */
void *MemoryOSSharedAlloc(uint32 key, uint32 size, uint32 permMask) {
    return NULL;
}

/** 
 * @see Memory::SharedFree
 */
void MemoryOSSharedFree(void *address) {
    /* Do nothing */
}

/**
 * @see Memory::MemoryCheck
 */
bool MemoryOSCheck(void *address, MemoryTestAccessMode accessMode,
                   uint32 size) {
    if (accessMode & MTAM_Execute) {
        if (IsBadCodePtr((FARPROC) address))
            return False;
    }
    if (accessMode & MTAM_Read) {
        if (IsBadReadPtr(address, size))
            return False;
    }
    if (accessMode & MTAM_Write) {
        if (IsBadWritePtr(address, size))
            return False;
    }

    return True;
}

#endif

