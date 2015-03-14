/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/
/**
 * @file
 * A class that can handle a stack of Stackables
 */
#ifndef STACK_HOLDER_H
#define STACK_HOLDER_H

#include "LinkedListHolder.h"

class  StackHolder: public LinkedListHolder {
public:
    /** Insert on top.
        Can insert a list of elements.   */
    inline void StackFastPushSingle(Stackable *p){
        if (p != NULL){
            llhSize++;
            p->next = llhRoot.next;
            llhRoot.next = p;
        }
    }

    /** Get from Top. */
    inline Stackable *StackFastPop(){
        Stackable *p = llhRoot.next;
        if (p != NULL){
            llhSize--;
            llhRoot.next = p->next;
            p->next = NULL;
        }
        return p;
    }

    /**
        Insert on top.
        Can insert a list of elements.
    */
    void StackPush(Stackable *p){
        ListInsert(p);
    }

    /** Get from Top. */
    Stackable *StackPop(){
        Stackable *p = llhRoot.Next();
        if (p != NULL){
            llhSize--;
            llhRoot.next = p->next;
        }
        if (p != NULL) p->next = NULL;
        return p;
    }

    /** The depth of the stack. */
    uint32 StackDepth(){
        return ListSize();
    }

    /** Looks into the stack: index = 0 is the top. */
    Stackable *StackPeek(uint32 index){
        return ListPeek(index);
    }

    /** Show the top of stack. */
    Stackable *StackTop(){
        return List();
    }

    /** Removes and Gets the n-th element of stack. */
    Stackable *StackExtract(uint32 index){
        return ListExtract(index);
    }

    /**
        Inserts as the n-th element of stack (if possible otherwise as bottom).
        can insert a linked list of elements.
    */
    void StackInsert(Stackable *q,uint32 index){
        ListInsert(q,index);
    }

};
#endif
