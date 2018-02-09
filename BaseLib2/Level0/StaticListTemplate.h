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
 * A template to customise a StaticListHolder for a specific struct or class
 */
#ifndef STATICLISTTEMPLATE_H
#define STATICLISTTEMPLATE_H

#include "StaticListHolder.h"

template <class T>
class StaticListTemplate:protected StaticListHolder {

public:

    /** the constructor sets aitomatically the object size */
                        StaticListTemplate(TimeoutType  msecTimeout = TTInfiniteWait){
        this->elementSize = sizeof(T)/sizeof(intptr);
        this->msecTimeout = msecTimeout;
    }

    /** Add an element at any position. 0 = add on top, -1 = add at the end
        @param element is a pointer to a buffer of data. The data will be copied into the list    */
    inline bool         ListAdd(const T &element,int position = SLH_EndOfList){
        return StaticListHolder::ListAdd((const intptr *)&element,position);
    }

    /** Removes an element from any position. 0 = removes from the top, -1 = removes from the end
            @param element is a pointer to a buffer of data. The data will be copied into the list */
    inline bool         ListExtract(T &element,int position = SLH_EndOfList){
        return StaticListHolder::ListExtract((intptr *)&element,position);
    }

    /** reads a value without affecting the list
        @param element is a pointer to a buffer of data. The data will be copied into the list */
    inline bool         ListPeek(T &element,int position = SLH_EndOfList){
        return StaticListHolder::ListPeek((intptr *)&element,position);
    }

    /** removes an element from the list using a copy of the element as a search key
        @param element is a pointer to a buffer of data. The data will just be read */
    inline bool         ListDelete(const T &element){
        return StaticListHolder::ListDelete((const intptr *)&element);
    }

    /** finds at what index the specified data is located. -1 means not found
        @param element is a pointer to a buffer of data. The data will just be read */
    inline int          ListFind(const T &element){
        return StaticListHolder::ListFind((const intptr *)&element);
    }

    /** Add a the top */
    inline void         ListInsert(const T &element){
        StaticListHolder::ListAdd((const intptr *)&element,SLH_StartOfList);
    }

    /** removes at the specified position */
    inline bool         ListDelete(int position){
        return (StaticListHolder::ListExtract(NULL,position));
    }

    /** how many elements currently in the list */
    uint32              ListSize()const{
        return StaticListHolder::ListSize();
    }

};

#endif

