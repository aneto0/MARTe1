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

#include "RTDataPool.h"
#include "ConfigurationDataBase.h"
#include "Memory.h"

RTDataPool::RTDataPool(){
    bufferSize  = 0;
    nOfBuffers  = 0;
    memoryPool  = NULL;
}

void RTDataPool::CleanUp(){
    while(ListSize()!=0) ListExtract();
    if(memoryPool!=NULL) free((void *&)memoryPool);
    bufferSize  = 0;
    nOfBuffers  = 0;
    memoryPool  = NULL;
}

bool RTDataPool::RebuildList(){
    StackHolder::Reset();

    if ((memoryPool == NULL) || (nOfBuffers == 0) || (bufferSize == 0)){
        AssertErrorCondition(FatalError,"RTDataPool::RebuildList: memory=%x nOfBuffers=%i bufferSize=%i",memoryPool,nOfBuffers,bufferSize);
        return False;
    }

    // bufferSize + 2 to account for the jpfUsecTime and the fastTrigger request
    uint32 stepSize = sizeof(RTCollectionBuffer)+(bufferSize + 2)*sizeof(uint32);
    char *p = (char *) memoryPool;
    for(int i = 0;i < nOfBuffers;i++){
        RTCollectionBuffer *buf = (RTCollectionBuffer *)(p+stepSize*i);
        buf->Reset();
        ListInsert(buf);
    }
    return True;
}

bool RTDataPool::Init(uint32 bufferSize,uint32 nOfBuffers){
    // Clean up the linked list holder
    CleanUp();

    if ( bufferSize <= 0 ){
        AssertErrorCondition(FatalError,"RTDataPool::Init:Buffer is too small : %i",bufferSize);
        return False;
    }

    if (nOfBuffers == 0){
        AssertErrorCondition(FatalError,"RTDataPool::Init: #of buffers =%i",nOfBuffers);
        return False;
    }

    // bufferSize + 2 to account for the jpfUsecTime and the fastTrigger request
    uint32 totalBufferSize = nOfBuffers * (sizeof(RTCollectionBuffer) + (bufferSize + 2)*sizeof(uint32));
    AssertErrorCondition(Information,"RTDataPool::Init: Allocating %i bytes",totalBufferSize);

    memoryPool = (uint32 *)malloc(totalBufferSize,MEMORYExtraMemory);

    if (memoryPool == NULL){
        AssertErrorCondition(FatalError,"RTDataPool::Init: Memory allocation failed for %d words",totalBufferSize);
        return False;
    }

    this->bufferSize = bufferSize;
    this->nOfBuffers = nOfBuffers;

    return RebuildList();
}



bool RTDataPool::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){

    uint32 totalBufferSize = nOfBuffers * (sizeof(RTCollectionBuffer)+bufferSize*sizeof(uint32));

    s.Printf("TotalMemorySize = %i \n",totalBufferSize);
    s.Printf("SingleBufferSize = %d\n",bufferSize);
    s.Printf("NOfBuffers = %d\n",nOfBuffers);
    s.Printf("FreeBuffers = %d \n",ListSize());
    return True;
}


OBJECTREGISTER(RTDataPool,"$Id: RTDataPool.cpp,v 1.2 2008/02/22 11:16:37 fpiccolo Exp $")
