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
 * @brief Queue of QueueAbles
 */
#ifndef QUEUE_HOLDER_H
#define QUEUE_HOLDER_H

#include "StackHolder.h"

// Iterators support
/** A class that can handle a Queue of QueueAbles (FIFO). */
class QueueHolder: protected StackHolder {
protected:
    /** */
    Queueable *insertionPoint;

public:
    /** */
    QueueHolder() {
        insertionPoint = &llhRoot;
    }

    /** Return the oldest inserted element. */
    Queueable *Oldest() {
        return List();
    }

    /** Return the first element. */
    uint32 QueueSize() {
        return llhSize;
    }

    /** Insert an element or a list on the queue. */
    void QueueAdd(Queueable *p) {
        if (p == NULL) {
            return;
        }
        llhSize += p->Size();
        insertionPoint->Insert(p);
        while (p->next != NULL)
            p = p->next;
        insertionPoint = p;
    }

    /** Insert an element or a list on the queue. */
    void QueueInsert(Queueable *p) {
        ListInsert(p);
        if (insertionPoint == &llhRoot && p != NULL) {
            while (p->next != NULL) {
                p = p->next;
            }
            insertionPoint = p;
        }
    }

    /** Removes the oldest elemnt from the queue. */
    Queueable *QueueExtract() {
        Queueable *p = StackPop();
        if (llhRoot.next == NULL)
            insertionPoint = &llhRoot;
        return p;
    }

    /** Removes from the middle. */
    bool QueueExtract(Queueable *p) {
        bool ret = ListExtract(p);
        if (p == insertionPoint) {
            insertionPoint = &llhRoot;
            while (insertionPoint->next != NULL) {
                insertionPoint = insertionPoint->next;
            }
        }
        return ret;
    }

    /** Looks into the queue. */
    Queueable *QueuePeek(uint32 index) {
        return ListPeek(llhSize - index - 1);
    }

    /** Looks into the queue to the last element inserted. */
    Queueable *QueuePeekLast() {
        return insertionPoint;
    }

    /** Reset the queue. */
    void Reset() {
        LinkedListHolder::Reset();
        insertionPoint = &llhRoot;
    }

    /** Insert in the first location the element p. */
    inline void FastQueueInsertSingle(LinkedListable &p) {
        llhSize++;
        insertionPoint->next = &p;
        insertionPoint = &p;
        p.next = NULL;
    }

};

#endif
