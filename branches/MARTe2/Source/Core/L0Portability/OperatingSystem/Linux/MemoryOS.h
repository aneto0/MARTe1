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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

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
    key_t keyid = (key_t) key;
    int32 shmid = shmget(keyid, size, IPC_CREAT | permMask);
    if (shmid == -1) {
        return NULL;
    }
    void *shm = shmat(shmid, NULL, 0);
    if (shm == (char *) -1) {
        return NULL;
    }
    return shm;
}

/** 
 * @see Memory::SharedFree
 */
void MemoryOSSharedFree(void *address) {
    shmdt(address);
}

/**
 * Currently not implemented in Linux
 * @see Memory::MemoryCheck
 */
bool MemoryOSCheck(void *address, MemoryTestAccessMode accessMode,
                   uint32 size) {
    return True;
}

//Copy source to destination.
bool MemoryOsCopy(void* destination, void* source, uint32 size) {
    if (source == NULL || destination == NULL) {
        return False;
    }

    return memcpy(destination, source, size) != NULL;
}

// <0 -> mem1 < mem2      >0 -> mem1 > mem2       =0 -> mem1 = mem2
int32 MemoryOsCompare(const void* mem1, const void* mem2, uint32 size) {
    if (mem1 == NULL || mem2 == NULL) {
        return -1;
    }
    int32 ret = memcmp(mem1, mem2, size);
    if (ret < 0) {
        return 1; // 1 if mem1<mem2
    }
    if (ret > 0) {
        return 2; // 2 if mem1>mem2
    }
    return ret; //0 if mem1=mem2

}

//Return a pointer to the first occurence of c in mem
void* MemoryOsSearch(void* mem, int32 c, uint32 size) {
    if (mem == NULL) {
        return NULL;
    }
    return memchr(mem, c, size);
}

//Move size bytes from source to destination
bool MemoryOsMove(void* destination, const void* source, uint32 size) {
    if (source == NULL || destination == NULL) {
        return False;
    }
    return memmove(destination, source, size) != NULL;

}

//Copy c in the first size locations pointed by mem
bool MemoryOsSet(void* mem, int32 c, uint32 size) {
    if (mem == NULL) {
        return False;
    }
    return memset(mem, c, size) != NULL;
}

#endif

