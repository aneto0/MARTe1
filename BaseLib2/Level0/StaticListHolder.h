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
 * A list of pointers that can grow and shrink
 */
#ifndef STATICLISTHOLDER_H
#define STATICLISTHOLDER_H

#include "GenDefs.h"
#include "System.h"
#include "TimeoutType.h"
#include "FastPollingMutexSem.h"

/** the list wil grow at the pace of 64 elements */
static const int SLH_Granularity = 64;

/** use this to insert at beginning of list */
static const int SLH_StartOfList = 0;

/** use this to add at the end of list */
static const int SLH_EndOfList   = -1;

///** use this to specify not to use the semaphore */
//static const int SLHT_Unprotected = -2;

///** use this to specify not to wait */
//static const int SLHT_NoWait      = 0;

///** use this to specify not to use the semaphore */
//static const int SLHT_WaitForEver = SEM_INDEFINITE_WAIT;

/** a container of pointers that can grow or shrink. It allows inserting and removing of elements.
It uses realloc. It also offers the possibility to control the data access via a sempahore */
class StaticListHolder{
protected:
    /** the vector of elements */
    intptr *            elements;

    /** the size of one element as number */
    int                 elementSize;

    /** how many elements are in the list*/
    int                 numberOfElements;

    /** A semaphore to allow multiple threads protected access to the data. */
    FastPollingMutexSem mux;

    /** how much to wait for a resource in milliseconds */
    TimeoutType         msecTimeout;

    /** increases the list size by 1. Manages the reallocation of memory when necessary */
    bool                IncreaseListSize(){
       if ((numberOfElements % SLH_Granularity)==0){
            int newSize =  numberOfElements + SLH_Granularity;
            if (elements == NULL) {
                elements = (intptr *)malloc(sizeof(intptr *) * newSize * elementSize);
            } else {
                elements = (intptr *)realloc((void *&)elements,sizeof(intptr *) * newSize * elementSize);
            }
            if (elements == NULL){
                numberOfElements = 0;
                CStaticAssertErrorCondition(OSError,"StaticListHolder::IncreaseListSize:malloc(%i bytes) failed",newSize * sizeof(intptr *) * elementSize);
                return False;
            }
       }
       numberOfElements++;
       return True;
    }

    /** decreases the list size by 1. Reallocation of memory is not performed */
    bool                DecreaseListSize(){
       numberOfElements--;
       return True;
    }

    /** copies data from a buffer to the position */
    inline void         Copy(intptr *destination,const intptr *source){
        for (int j = 0; j < elementSize ;j++){
            destination[j] = source[j];
        }
    }

    /** compares data between source and destination. True means equal */
    inline bool         Compare(const intptr *destination,const intptr *source){
        for (int j = 0; j < elementSize ;j++){
            if (destination[j] != source[j]) return False;
        }
        return True;
    }

    /** finds data in list. -1 means not found */
    inline int          Find(const intptr *data){
        int index = 0;
        while (!Compare(GetPointer(index),data) &&
               (index < numberOfElements)) index++;
        if (index == numberOfElements) return -1;
        return index;
    }

    /** retrieves address of data element */
    inline intptr*      GetPointer(int position){
        return &elements[position * elementSize];
    }

    /** moves all the pointers from position to the right. Assumes that the last position is empty */
    inline void          RightShiftListFrom(int position){
        for (int i = (numberOfElements-1);i > position ;i--){
            Copy(GetPointer(i),GetPointer(i-1));
        }
    }

    /** removes the element in position and shifts all the elements at the right of it to the left */
    inline void         LeftShiftListTo(int position){
        for (int i = position; i < (numberOfElements-1);i++){
            Copy(GetPointer(i),GetPointer(i+1));
        }
    }


public:
    /** creates a List with the given elelent size as multiple of 32 bits! */
                        StaticListHolder(int elementSize = 1){
        elements                = NULL;
        numberOfElements        = 0;
        msecTimeout             = TTUnProtected;
        this->elementSize  = elementSize;
        mux.Create();
    }

    /** a virtual destructor . This means that all descendants will be virtual */
    virtual             ~StaticListHolder(){
        if (elements != NULL ) free ((void *&)elements);
        numberOfElements = 0;
    }

    /** set access control policies */
    void                SetAccessTimeout(TimeoutType  msecTimeout = TTInfiniteWait)
    {
        this->msecTimeout = msecTimeout;
    }

    /** how many elements currently in the list */
    uint32              ListSize()const
    {
        return numberOfElements;
    }

    /** Add an element at any position. 0 = add on top, -1 = add at the end
        @param element is a pointer to a buffer of data. The data will be copied into the list    */
    bool                ListAdd(const intptr *element,int position = SLH_EndOfList){
        if (msecTimeout != TTUnProtected) {
            if (!mux.FastLock(msecTimeout)) {
                CStaticAssertErrorCondition(Timeout,"StaticListHolder::ListAdd:access Timeout( %i ) ",msecTimeout.msecTimeout);
                return False;
            }
        }

        if (position == SLH_EndOfList) position = numberOfElements;

        if ((position > numberOfElements) || (position < 0)){
            CStaticAssertErrorCondition(ParametersError,"StaticListHolder::ListAdd:poistion outside range: %i [0 %i]",position,numberOfElements);
            if (msecTimeout != TTUnProtected) mux.FastUnLock();
            return False;
        }

        if (!IncreaseListSize()){
            CStaticAssertErrorCondition(OSError,"StaticListHolder::ListAdd:Failed allocating memory");
            if (msecTimeout != TTUnProtected) mux.FastUnLock();
            return False;
        }

        RightShiftListFrom(position);

        Copy(GetPointer(position),element);

        if (msecTimeout != TTUnProtected) mux.FastUnLock();

        return True;
    }

    /** Removes an element from any position. 0 = removes from the top, -1 = removes from the end
            @param element is a pointer to a buffer of data. The data will be copied into the list */
    bool                ListExtract(intptr *element=NULL,int position = SLH_EndOfList){
        if (msecTimeout != TTUnProtected) {
            if (!mux.FastLock(msecTimeout)) {
                CStaticAssertErrorCondition(Timeout,"StaticListHolder::ListExtract:access Timeout( %i ) ",msecTimeout.msecTimeout);
                return False;
            }
        }

        if (position == SLH_EndOfList) position = numberOfElements - 1;
        if ((position >= numberOfElements) || (position < 0)){
            CStaticAssertErrorCondition(FatalError,"StaticListHolder::ListExtract:poistion outside range: %i [0 %i)",position,numberOfElements);
            if (msecTimeout != TTUnProtected) mux.FastUnLock();
            return False;
        }

        bool ret = True;
        if (element) Copy(element,GetPointer(position));
        LeftShiftListTo(position);

        if (!DecreaseListSize()){
            CStaticAssertErrorCondition(OSError,"StaticListHolder::ListExtract:Failed freeing memory");
            ret = False;
        }

        if (msecTimeout != TTUnProtected) mux.FastUnLock();
        return ret;
    }

    /** reads a value without affecting the list
        @param element is a pointer to a buffer of data. The data will be copied into the list */
    bool                ListPeek(intptr *element=NULL,int position = SLH_EndOfList){
        if (msecTimeout != TTUnProtected) {
            if (!mux.FastLock(msecTimeout)) {
                CStaticAssertErrorCondition(Timeout,"StaticListHolder::ListPeek:access Timeout( %i ) ",msecTimeout.msecTimeout);
                return False;
            }
        }

        if (position == SLH_EndOfList) position = numberOfElements - 1;

        bool ret = False;
        if ((position > numberOfElements) || (position < 0)){
            CStaticAssertErrorCondition(ParametersError,"StaticListHolder::ListPeek:poistion outside range: %i [0 %i)",position,numberOfElements);
        } else {
            ret = True;
            if (element) Copy(element,GetPointer(position));
        }

        if (msecTimeout != TTUnProtected) mux.FastUnLock();
        return ret;
    }

    /** removes an element from the list using a copy of the element as a search key
        @param element is a pointer to a buffer of data. The data will just be read */
    bool                    ListDelete(const intptr *element){
        if (msecTimeout != TTUnProtected) {
            if (!mux.FastLock(msecTimeout)) {
                CStaticAssertErrorCondition(Timeout,"StaticListHolder::ListDelete:access Timeout( %i ) ",msecTimeout.msecTimeout);
                return False;
            }
        }

        int position = Find(element);

        bool ret = False;
        if (position >= 0) {
            ret = True;
            LeftShiftListTo(position);

            if (!DecreaseListSize()){
                CStaticAssertErrorCondition(OSError,"StaticListHolder::ListDelete:Failed freeing memory");
                ret = False;
            }
        }

        if (msecTimeout != TTUnProtected) mux.FastUnLock();
        return ret;
    }

    /** finds at what index the specified data is located. -1 means not found
        @param element is a pointer to a buffer of data. The data will just be read */
    int                 ListFind(const intptr *element)
    {
        if (msecTimeout != TTUnProtected) {
            if (!mux.FastLock(msecTimeout)) {
                CStaticAssertErrorCondition(Timeout,"StaticListHolder::ListFind:access Timeout( %i ) ",msecTimeout.msecTimeout);
                return -1;
            }
        }

        int position = Find(element);

        if (msecTimeout != TTUnProtected) mux.FastUnLock();
        return position;
    }

    /** Add a the top */
    inline void         ListInsert(intptr *element)
    {
        ListAdd(element,0);
    }

    /** removes at the specified position */
    inline bool         ListDelete(int position)
    {
        return (ListExtract(NULL,position));
    }

};


#endif

