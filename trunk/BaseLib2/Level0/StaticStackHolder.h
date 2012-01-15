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
 * A class that can handle a stack of any type but has a fixed size
 * It is based on a vector of pointers
 */
#ifndef STATICSTACKHOLDER_H
#define STATICSTACKHOLDER_H

#include "GenDefs.h"
#include "System.h"
#include "StaticListHolder.h"

class  StaticStackHolder: protected StaticListHolder {

public:

    /** creates a Stack with the given element size
        note that the size is in 32 bit multiples
    */
                    StaticStackHolder(int elementSize = 1){
        this->elementSize = elementSize;
    };

    /**  Insert on top a single element. When the space is finished the bottom is discarded */
    inline void     StackPush(const intptr *element){ ListAdd(element); }

    /** Get from Top */
    inline bool     StackPop(intptr *element){ return ListExtract(element); }

    /** The depth of the stack */
    inline uint32   StackDepth(){ return ListSize(); }

    /** looks into the stack: index = 0 is the top */
    inline bool     StackPeek(intptr *element,int position){ return ListPeek(element,ListSize()-1-position); }

    /** show the top of stack */
    inline bool     StackTop(intptr *element){ return ListPeek(element); }


};


#endif
