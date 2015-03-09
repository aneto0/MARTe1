/* Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they
 will be approved by the European Commission - subsequent
 versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 Licence.
 * You may obtain a copy of the Licence at:
 *
 * http: //ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in
 writing, software distributed under the Licence is
 distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id:$
 *
 **/

#include "GeneralDefinitions.h"
#include "MemoryTest.h"

bool stringComp(char* string1, char* string2) {
    int32 i = 0;
    while (1) {
        if (string1[i] != string2[i]) {
            return False;
        }
        if (string1[i] == '\0' && string2[i] == '\0') {
            return True;
        }
        if (string1[i] == '\0' || string2[i] == '\0') {
            return False;
        }
        i++;
    }
}

void InitializeSharedMemory(MemoryTest myTestMemory) {
    int32* sharedInt = (int32*) MemorySharedAlloc(1, sizeof(int32));
    bool* sharedBool = (bool*) MemorySharedAlloc(2, sizeof(bool));
    *sharedBool = False;
    (*sharedInt) = 1;
    myTestMemory.eventSem.Post();
}

void IncrementSharedMemory(MemoryTest myTestMemory) {
    int32* sharedInt = (int32*) MemorySharedAlloc(1, sizeof(int32));
    bool* sharedBool = (bool*) MemorySharedAlloc(2, sizeof(bool));
    *sharedBool = True;
    (*sharedInt)++;
}

//Test the malloc function.
bool MemoryTest::TestMallocAndFree(int32 size) {

    //allocate a space of size integers
    int32* allocated = (int32*) MemoryMalloc(size * sizeof(int32));
    int32 i = 0;

    //check if the pointers to these memory locations are valid
    while (i < size) {
        if ((allocated + i) == NULL) {
            return False;
        }
        i++;
    }

    //free the allocated memory
    MemoryFree((void*&) allocated);
    return allocated == NULL;
}

//Tests the realloc function.
bool MemoryTest::TestRealloc(int32 size1, int32 size2) {
    //allocate size1 integers	
    int32* allocated = (int32*) MemoryMalloc(size1 * sizeof(int32));

    //check if the pointers to these memory locations are valid
    for (int32 i = 0; i < size1; i++) {
        if ((allocated + i) == NULL) {
            return False;
        }
        allocated[i] = i;
    }

    //reallocate the memory adding size2 integers locations
    allocated = (int32*) MemoryRealloc((void*&) allocated,
                                       (size1 + size2) * sizeof(int32));

    //check if pointers of new memory are valid and if the old memory is not corrupted
    for (int32 i = 0; i < size2; i++) {
        if ((allocated + size1 + i) == NULL) {
            return False;
        }
        if (allocated[i] != i) {
            return False;
        }
    }

    return True;
}

//Test if the string s is copied without errors.
bool MemoryTest::TestMemoryStringDup(const char* s) {
    return stringComp((char*) MemoryStringDup(s), (char*) s);
}

//Test the behavior of the shared memory
bool MemoryTest::TestSharedMemory() {
    //reset an event sem
    eventSem.Reset();

    //launch two threads, one initialize the shared int to one and the shared bool to false.
    Threads::BeginThread((ThreadFunctionType) InitializeSharedMemory, this);
    //wait the inizialization of the shared memory
    eventSem.Wait();
    //this thread increment the shared integer and impose true the shared bool
    Threads::BeginThread((ThreadFunctionType) IncrementSharedMemory, this);

    //obtain the pointers to the shared memories
    bool* sharedBool = (bool*) MemorySharedAlloc(2, sizeof(bool));
    int32* sharedInt = (int32*) MemorySharedAlloc(1, sizeof(int32));
    int32 j = 0;

    //wait that the second thread increments to two the shared int
    while ((*sharedInt) < 2) {
        if (j++ > 100) {
            return False;
        }
        SleepSec(10e-3);
    }
    bool returnValue = *sharedBool;
    //release the shared memory

    MemorySharedFree(sharedBool);
    MemorySharedFree(sharedInt);

    //else return false
    return returnValue;
}
