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

#if !defined (_RTDATAPOOL)
#define _RTDATAPOOL

#include "System.h"
#include "Object.h"
#include "StackHolder.h"
#include "RTCollectionBuffer.h"
//#include "GAPInitMessage.h"


/// Max number of time windows for single Data storage System
static const int MaxNOfTimeWindow = 16;

/** Real-Time Data Pool Class.
    It allocates and holds the memory to collect
    JPF data  */
OBJECT_DLL(RTDataPool)
class RTDataPool:public Object, protected StackHolder{
OBJECT_DLL_STUFF(RTDataPool)
private:
    /// the size of each buffer as 32 bits
    uint32  bufferSize;

    /// Max the number of samples stored in JPF
    uint32  nOfBuffers;

    /// Pointer to the allocated memory
    uint32  *memoryPool;

    /*******************************************************************************************************
    /*
    /* Avoid the user from making copies of the RTDataPool and forgetting to handle the memory allocation
    /*
    ********************************************************************************************************/

    /// Copy constructors (since it is defined private it won't allow a public use!!)
    RTDataPool(const RTDataPool&){};

    /// Operator=  (since it is defined private it won't allow a public use!!)
    RTDataPool& operator=(const RTDataPool&){};

public:
    ///Constructor
    RTDataPool();

    /// Destructor
    ~RTDataPool(){ CleanUp(); };

    /// takes a discarded buffer and puts it back in the stack
    inline RTCollectionBuffer *GetFreeBuffer(){ return (RTCollectionBuffer *)StackFastPop(); }

    /// takes a discarded buffer and puts it back in the stack
    inline void ReturnUnusedBuffer(RTCollectionBuffer *buffer){ StackFastPushSingle(buffer); }

    /// sets all to empty, deallocates memory
    void CleanUp();

    /// rebuilds the list of ready buffers using the memoryPool;
    bool RebuildList();

    /// Load the parameters for the Storage Data System
    bool Init(uint32 bufferSize,uint32 nOfBuffers);

    ///
    bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

};

#endif
