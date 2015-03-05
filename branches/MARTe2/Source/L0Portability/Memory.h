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
#ifndef MEMORY_H
#define MEMORY_H

#include "GeneralDefinitions.h"

/** choice between standard memory and a separate block VX5100/VX5500 24bit mode*/
enum MemoryAllocationFlags {
    /** within 32Mb */
    MemoryStandardMemory = 0x00000000,

    /** above 32Mb */
    MemoryExtraMemory = 0x00000001,
};

/** set of bits indicating desired access modes to be teste for validity */
enum MemoryTestAccessMode {
    /** read and execute */
    MTAM_Execute = 0x00000001,

    /** read  */
    MTAM_Read = 0x00000002,

    /** read  */
    MTAM_Write = 0x00000004

};

/** MemoryTestAccessMode & operator */
static inline MemoryTestAccessMode operator &(MemoryTestAccessMode a,
                                              MemoryTestAccessMode b) {
    return (MemoryTestAccessMode) ((int) a & (int) b);
}

/** MemoryTestAccessMode | operator*/
static inline MemoryTestAccessMode operator |(MemoryTestAccessMode a,
                                              MemoryTestAccessMode b) {
    return (MemoryTestAccessMode) ((int) a | (int) b);
}

extern "C" {

/** Wrap the malloc standard C function.
 @param size The size in byte of the memory to allocate
 @param allocFlags Flag specifying if to enable the memory statistic.
 @return The pointer to the allocated memory. NULL if allocation failed.
 */
void *MemoryMalloc(uint32 size, MemoryAllocationFlags allocFlags =
                           MemoryStandardMemory);

/** Wrap the free standard C function and sets the memory pointer to NULL.
 @param data The memory area to be freed.
 */
void MemoryFree(void *&data);

/** Wrap the realloc standard C function.
 @param data The pointer to the new memory block
 @param newSize The size of the new memory block
 @return The pointer to the new data block. NULL if reallocation failed.
 */
void *MemoryRealloc(void *&data, uint32 newSize);

/** Wrap the strdup standard C function.
 @param s The pointer to the memory to be copied
 @return The pointer to the newly allocated memory which contains a copy of s.
 */
char *MemoryStringDup(const char *s);

/** Displays the Memory Statistics
 @param out The output stream
 */
#ifdef MEMORY_STATISTICS
void MemoryDisplayAllocationStatistics(StreamInterface *out);
#endif

/** Get the memory statistics associated with a thread.
 @param  size   The size of the memory associated with the thread.
 @param  chunks The number of chunks of memory
 @param  tid    The Thread id to be investigated
 @return True if everything was ok. False otherwise.
 */
bool MemoryAllocationStatistics(uint32 &size, uint32 &chunks, TID tid =
                                        (TID) 0xFFFFFFFF);

/** checks for 8/16/24/32 ... bit access validity of
 @param address The pointer to the memory block to be checked
 @param accessMode The type of memory access to perform
 @param size is the numnber of bytes to check
 @return True if the check was successful. False otherwise.
 */
bool MemoryCheck(void *address, MemoryTestAccessMode accessMode,
                 uint32 size = 4);

/** Creates or accesses an area of shared memory which can be used to communicate between different processes
 Even if you are accessing a shared memory created by another process, you should free it using
 MemorySharedFree, as soon as you finish
 @param key the desired identifier for the shared memory
 @param size the size of the shared memory
 @param permMask the process permissions.Usually you will set this as 0666
 @return the address or NULL if it fails to allocate the memory
 */
void *MemorySharedAlloc(uint32 key, uint32 size, uint32 permMask = 0666);

/** Frees an area of shared memory which was previously created with MemorySharedAlloc
 @param address the address for the shared memory
 */
void MemorySharedFree(void *address);
}

#endif

