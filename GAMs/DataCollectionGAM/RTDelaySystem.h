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

#if !defined (_RTDELAYSYS)
#define _RTDELAYSYS

#include "System.h"
#include "Object.h"
#include "QueueHolder.h"
#include "RTCollectionBuffer.h"

/** Real-Time Delay System Class
    It's a queue (FIFO) of data buffers.
    In each data buffer are stored all the signals for
    a particular time-slot.  */
class RTDelaySystem:protected QueueHolder{
private:
    /** Single Fast Acquisiton length.
        It's the delay value expressed in time slots
        and it's equal to the queue size. */
    int32 delay;

public:
    /// Constructor
    RTDelaySystem(){ delay = 0; }

    /// Deconstructor
    ~RTDelaySystem(){ Reset(); }

    /// Export Reset() method
    inline void Reset(){ QueueHolder::Reset(); }

    ///
    void PrepareForNextPulse(){ Reset(); }

    ///
    uint32 Size(){ return QueueSize(); }

    /** Check wether we have reached the desired delay */
    inline bool DelayIsFull(){return (QueueSize() >= delay);}

    /** Load the parameters for the Delay System */
    void  Init(int32 delay){ this->delay = delay; }

    /// Add a copy of a RTDataBuffer object to the queue
    void QueueAdd(RTCollectionBuffer *buf){  QueueHolder::QueueAdd(buf);  }

    ///
    RTCollectionBuffer *QueueExtract(){return (RTCollectionBuffer *)QueueHolder::QueueExtract();}

    bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL){
	s.Printf("PreTigger = %d \n", delay);
	return True;
    }

};

#endif
