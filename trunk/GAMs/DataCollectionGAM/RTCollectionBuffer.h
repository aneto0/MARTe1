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

#if !defined (_RTCOLLECTIONBUFFER)
#define _RTCOLLECTIONBUFFER

#include "System.h"
#include "Iterators.h"

/// Used to collect data for post pulse analisys
class RTCollectionBuffer:public LinkedListable{

public:
    ///
    RTCollectionBuffer(){ }

    ///
    ~RTCollectionBuffer(){}

    /// the data begins after this structure
    uint32 *Data(){ return (uint32 *)(this+1); }

    /// since this object is not allocated but cast, use this to reset it to initial state
    void Reset(){ next = NULL; }

    ///
    RTCollectionBuffer *Next(){return (RTCollectionBuffer *)LinkedListable::Next();}

    /// does copying and labeling
    inline void Copy(const uint32 *ddbInterfaceBuffer ,uint32 size,uint32 usecTime, bool fastTrigger){

        uint32 *jpfUsecTime = (uint32 *)Data();
        uint32 *trigger     = jpfUsecTime + 1;
        // Set local Copy of the usec Time
        *jpfUsecTime        = usecTime;
        // Set local Copy of the Fast Trigger
        if(fastTrigger) *trigger            = 1;
        else            *trigger            = 0;

        const uint32 *src       =  ddbInterfaceBuffer;
        // Skipt the first entry which is the time the buffer was acquired
        uint32 *dst             =  trigger + 1;
        const uint32 *endSrc    =  src + size;
        // Copy Data
        while (src < endSrc) *dst++ = *src++;

    }

    /// Gets the sample time from JPF header
    inline uint32 PacketUsecTime(){
        uint32 *jpfUsecTime = (uint32 *)Data();
        return *jpfUsecTime;
    }

    /// Fast Trigger Request
    inline bool PacketFastTriggerRequest(){
        uint32 *trigger = (uint32 *)Data() + 1;
        return (*trigger != 0);

    }
};

#endif
