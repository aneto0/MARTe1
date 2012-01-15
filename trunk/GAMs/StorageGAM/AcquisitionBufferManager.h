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
#include "Buffer.h"

#if !defined(_ACQUISITION_BUFFER_MANAGER_)
#define _ACQUISITION_BUFFER_MANAGER_

static const int32 MAX_NUMBER_OF_CONSECUTIVE_STORE_OPERATIONS = 10;
static const int32 NUMBER_OF_SAFETY_BUFFERS = 10;

class AcquisitionBufferManager : public GarbageCollectable {

protected:

    /// Pointer to the DDB input interface
    DDBInputInterface   *ddbInputInterfacePtr;

    /// Total number of bytes to store signal
    /// This should be the size of the linear buffer
    uint32               totalNumberOfBytes;

    /// Address of where the copy from circ to lin buffer
    /// starts
    char                *markPtr;

    /// Address of where the copy from circ to lin buffer
    /// ends
    char                *storePtr;

public:

    /// Linear buffer, stores data points
    /// sequentially and monotonically
    Buffer               linearBuffer;

    /// Circular buffer
    Buffer               circularBuffer;

    /// Number of bytes per cycle
    uint32               bytesPerCycle;

    /// Number of cycles per each store operation
    uint32               cyclesPerStoreOperation;

public:

    /// Constructor
    AcquisitionBufferManager() {
        ddbInputInterfacePtr = NULL;
        markPtr              = NULL;
        storePtr             = NULL;
    };
    
    /// Configure object and buffers
    bool Config(DDBInputInterface *ddbii, uint32 bytesPerCycle, uint32 cyclesPerStoreOperation, uint32 totalNumberOfBytes) {
        if(ddbii == NULL) {
            CStaticAssertErrorCondition(InitialisationError, "AcquisitionBufferManager::Config: DDBInputInterface pointer = null");
            return False;
        }
        
        ddbInputInterfacePtr          = ddbii;
        this->totalNumberOfBytes      = totalNumberOfBytes;
        this->bytesPerCycle           = bytesPerCycle;
        this->cyclesPerStoreOperation = cyclesPerStoreOperation;
        
        if(!linearBuffer.Initialise(totalNumberOfBytes+NUMBER_OF_SAFETY_BUFFERS*bytesPerCycle)) {
            CStaticAssertErrorCondition(InitialisationError, "AcquisitionBufferManager::Config: Error initialising the linear buffer object");
        }
        if(!circularBuffer.Initialise(cyclesPerStoreOperation*bytesPerCycle*MAX_NUMBER_OF_CONSECUTIVE_STORE_OPERATIONS)) {
            CStaticAssertErrorCondition(InitialisationError, "AcquisitionBufferManager::Config: Error initialising the circular buffer object");
        }

        //Reset();
        
        return True;
    };

    /// Reset buffers
    void Reset() {
        linearBuffer.InitialiseMemory(0xFFFFFFFF);
        circularBuffer.InitialiseMemory();

        linearBuffer.SetWritePointerPosition(0);
        circularBuffer.SetWritePointerPosition((cyclesPerStoreOperation*MAX_NUMBER_OF_CONSECUTIVE_STORE_OPERATIONS-1)*bytesPerCycle);
    };

    /// Update circular buffer with fresh data
    void UpdateData() {
        if(!circularBuffer.StoreAndShiftData(ddbInputInterfacePtr->Buffer(), bytesPerCycle)) {
            CStaticAssertErrorCondition(InitialisationError, "AcquisitionBufferManager::UpdateData: circularBuffer.StoreAndShiftData() failed");
        }
        if(markPtr != NULL) {
            char *temp = markPtr-bytesPerCycle;
            if(temp < circularBuffer.DataBuffer()) {
                CStaticAssertErrorCondition(InitialisationError, "AcquisitionBufferManager::UpdateData: markPtr in forbidden address");
            } else {
                markPtr = temp;
            }
        }
    };

    /// Set the start address of the memory copy from
    /// the circ to the lin buffer
    void Mark(int32 cycleOffset) {
        char *markPtrAux = (char *)(circularBuffer.WritePtr()) + cycleOffset*bytesPerCycle;
        if(markPtrAux < circularBuffer.DataBuffer()) {
            CStaticAssertErrorCondition(FatalError, "AcquisitionBufferManager::Store: markPtr < data buffer pointer");
        } else {
            markPtr = markPtrAux;
        }
    };
    
    /// Set the end address of the memory copy from
    /// the circ to the lin buffer
    void Store(int32 cycleOffset) {
        storePtr = (char *)(circularBuffer.WritePtr()) + cycleOffset*bytesPerCycle;
        if(storePtr <= markPtr) {
            CStaticAssertErrorCondition(FatalError, "AcquisitionBufferManager::Store: storePtr <= markPtr");
        } else {
            linearBuffer.StoreAndAdvancePointer(markPtr, storePtr-markPtr);
        }
        markPtr  = NULL;
        storePtr = NULL;
    };

    /// Force the store of an orphan mark
    void Flush() {
        if(markPtr) {
            Store(0);
        }
    };

    void *MarkPtr() {
        return markPtr;
    };

    void *StorePtr() {
        return storePtr;
    };
};

#endif
