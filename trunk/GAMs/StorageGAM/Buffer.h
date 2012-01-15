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
#if !defined(_BUFFER_)
#define _BUFFER_

class Buffer : public Object, public GarbageCollectable {
/// Core data
protected:

    /// Buffer size in bytes
    uint32                bufferByteSize;
    
    /// Pointer to start of data buffer
    char                 *data;

/// Core data derived quantities
protected:

    /// End of buffer's memory address
    char                 *maxDataAddress;

/// Buffer navigation quantities
protected:
    
    /// Address of the write pointer
    char                 *writePtr;

    /// Address of the read pointer
    char                 *readPtr;

/// Non-public memory handling methods
protected:

    /// CleanMemory
    void CleanMemory() {
        if(data != NULL) {
            free((void*&)data);
            data = NULL;
        }
    };
    
    /// AllocateMemory
    bool AllocateMemory() {
        if(data != NULL) {
            CleanMemory();
        }

        if(bufferByteSize) {
            if((data = (char *)malloc(bufferByteSize)) == NULL) {
                AssertErrorCondition(InitialisationError, "Buffer::AllocateMemory(): Unable to allocate memory for data buffer");
                return False;
            }
            maxDataAddress = data + bufferByteSize;
        } else {
            data = NULL;
            AssertErrorCondition(InitialisationError, "Buffer::AllocateMemory(): bufferByteSize is zero");
            return False;
        }
        
        return True;
    };
    
/// Non-virtual public methods
public:

    /// InitialiseMemory
    bool InitialiseMemory(int32 value = 0) {
        if(data == NULL) {
            AssertErrorCondition(FatalError, "Buffer::InitialiseMemory(): data buffer = null");
            return False;
        }
        memset(data, value, bufferByteSize);
        return True;
    };

    /// Initialise object
    bool Initialise(uint32 bufferByteSize, int32 arrayInitialisationValue = 0) {
        CleanMemory();
        this->bufferByteSize = bufferByteSize;
        if(!AllocateMemory()) {
            AssertErrorCondition(InitialisationError, "Buffer::Initialise(): AllocateMemory() failed");
            return False;
        }
        if(!InitialiseMemory(arrayInitialisationValue)) {
            AssertErrorCondition(InitialisationError, "Buffer::Initialise(): InitialiseMemory() failed");        
            return False;
        }
        writePtr = data;
        readPtr  = data;
        return True;
    };

    /// Constructor
    Buffer(uint32 bufferByteSize = 0, int32 arrayInitialisationValue = 0) {
        data = NULL;
        if(bufferByteSize) {
            Initialise(bufferByteSize, arrayInitialisationValue);
        }
    };
    
    /// Destructor
    ~Buffer() {
        CleanMemory();
    };

    /// Reset acquisition
    bool Reset(int32 arrayInitialisationValue = 0) {
        if(data != NULL) {
            writePtr = data;
            readPtr  = data;
        } else {
            AssertErrorCondition(FatalError, "Buffer::Reset(): data pointer = null");
            return False;
        }
        if(!InitialiseMemory(arrayInitialisationValue)) {
            AssertErrorCondition(InitialisationError, "Buffer::Reset(): InitialiseMemory() failed");
            return False;
        }
        return True;
    };

    /// Pointer to buffer holding data
    const void *DataBuffer() {
        return ((void *)data);
    };

    /// Current write pointer address
    const void *WritePtr() {
        return ((void *)writePtr);
    };

    /// Current read pointer address
    const void *ReadPtr() {
        return ((void *)readPtr);
    };
    
    /// Store external data in buffer
    bool Store(const void *extData, uint32 numberOfBytesToStore) {
        if(extData == NULL) {
            AssertErrorCondition(FatalError, "Buffer::Store(): external data pointer = null");
            return False;
        }
        if(writePtr <= maxDataAddress-numberOfBytesToStore) {
            memcpy(writePtr, extData, numberOfBytesToStore);
        } else {
            AssertErrorCondition(FatalError, "Buffer::Store(): writePtr in forbidden area");
            return False;
        }

        /* printf("->%d\n", (uint32)(*(uint32 *)(writePtr))); */
        /* printf("%d\n", (uint32)(*(uint32 *)(writePtr)+1)); */
        /* printf("%d\n", (uint32)(*(uint32 *)(writePtr)+2)); */

        return True;
    };

    /// Copy internal data to external memory block
    bool Retrieve(void *extData, uint32 numberOfBytesToRetrieve) {
        if(extData == NULL) {
            AssertErrorCondition(FatalError, "Buffer::Retrieve(): external data pointer = null");
            return False;
        }
        if(readPtr <= maxDataAddress-numberOfBytesToRetrieve) {
            memcpy(extData, readPtr, numberOfBytesToRetrieve);
        } else {
            AssertErrorCondition(FatalError, "Buffer::Retrieve(): readPtr in forbidden area");
            return False;
        }
        
        return True;
    };

    /// Store external data in buffer and move internal pointer fwd
    /// Should be used for linear storage just like filling an array
    /// sequentially and monotonically
    bool StoreAndAdvancePointer(const void *extData, uint32 numberOfBytesToStore) {
        //printf("numberOfBytesToStore = %d\n", numberOfBytesToStore);
        if(!Store(extData, numberOfBytesToStore)) {
            AssertErrorCondition(FatalError, "Buffer::StoreAndAfvancePointer(): Store() failed");
            return False;
        }
        /* printf("-> StoreAndAdvancePointer() called\n"); */
        /* printf("data = %d\twritePtr = %d\n", data, writePtr); */
        /* printf("%d\n", (uint32)(*(uint32 *)(writePtr))); */
        /* printf("%d\n", (uint32)(*(uint32 *)((uint32 *)(writePtr)+1))); */
        /* printf("%d\n", (uint32)(*(uint32 *)((uint32 *)(writePtr)+2))); */
        writePtr += numberOfBytesToStore;
        /* printf("data = %d\twritePtr = %d\n", data, writePtr); */

        return True;
    };

    /// Copy internal data to external memory block
    /// and advance internal read pointer
    bool RetrieveAndAdvancePointer(void *extData, uint32 numberOfBytesToRetrieve) {
        if(!Retrieve(extData, numberOfBytesToRetrieve)) {
            AssertErrorCondition(FatalError, "Buffer::RetrieveAndAfvancePointer(): Retrieve() failed");
            return False;
        }
        readPtr += numberOfBytesToRetrieve;

        return True;
    };

    /// Store external data in buffer keeping internal pointer at the same location
    /// The data is shifted to the left by the number of bytes that were just stored
    /// Data already at the left limit of the buffer is lost
    /// This "solution" should be used for a circular buffer
    bool StoreAndShiftData(const void *extData, uint32 numberOfBytesToStore) {
        if(!Store(extData, numberOfBytesToStore)) {
            AssertErrorCondition(FatalError, "Buffer::StoreAndShiftData(): Store() failed");
            return False;
        }
        memmove(data, data+numberOfBytesToStore, bufferByteSize-numberOfBytesToStore);

        return True;
    };

    /// Set the internal pointer position within the data buffer
    bool SetWritePointerPosition(uint32 bytePosition) {
        char *writePtrAux = data + bytePosition;
        if(writePtrAux > maxDataAddress) {
            AssertErrorCondition(FatalError, "Buffer::SetWritePointerPosition(): attempt to place writePtr in forbidden position");
            return False;
        }
        writePtr = writePtrAux;
        return True;
    };

    /// Set the internal pointer position within the data buffer
    bool AdvanceWritePointerPosition(uint32 advanceBytes) {
        char *writePtrAux = writePtr + advanceBytes;
        if(writePtrAux > maxDataAddress) {
            AssertErrorCondition(FatalError, "Buffer::AdvanceWritePointerPosition(): attempt to place writePtr in forbidden position");
            return False;
        }
        writePtr = writePtrAux;
        return True;
    };

    /// Set the internal pointer position within the data buffer
    bool SetReadPointerPosition(uint32 bytePosition) {
        char *readPtrAux = data + bytePosition;
        if(readPtrAux > maxDataAddress) {
            AssertErrorCondition(FatalError, "Buffer::SetReadPointerPosition(): attempt to place readPtr in forbidden position");
            return False;
        }
        readPtr = readPtrAux;
        return True;
    };

    /// Set the internal pointer position within the data buffer
    bool AdvanceReadPointerPosition(uint32 advanceBytes) {
        char *readPtrAux = readPtr + advanceBytes;
        if(readPtrAux > maxDataAddress) {
            AssertErrorCondition(FatalError, "Buffer::AdvanceReadPointerPosition(): attempt to place readPtr in forbidden position");
            return False;
        }
        readPtr = readPtrAux;
        return True;
    };

};

#endif
