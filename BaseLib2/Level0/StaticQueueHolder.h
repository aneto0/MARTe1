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

/** 
 * @file
 * A class that can handle a Queue of any type. The maximum length is fixed.
 */
#ifndef STATICQUEHOLDER_H
#define STATICQUEHOLDER_H

#include "StaticStackHolder.h"

class  StaticQueueHolder: protected StaticStackHolder {
protected:

public:

    /** creates a Queue with the given elelent size */
                    StaticQueueHolder(int elementSize32bit = 1){
        this->elementSize = elementSize;
    };

    /** return the last inserted element */
    inline bool     Last(intptr *element){  return StackTop(element);  };

    /** return the n of element on the Q */
    inline uint32   QueueSize(){  return StackDepth();   };

    /** insert an element on the queue */
    inline void     QueueAdd(const intptr *element){ StackPush(element); }

    /** removes the oldest elemnt from the queue */
    inline bool     QueueExtract(intptr *element){ return ListExtract(element,SLH_StartOfList);}

    /** looks into the queue: index = 0 is the most recent added */
    inline bool     QueuePeek(intptr *element, uint32 index){ return StackPeek(element,index); }

    /** looks into the queue to the last element inserted */
    inline bool     QueuePeekLast(intptr *element ){ return QueuePeek(element,0);  }


};




#endif
