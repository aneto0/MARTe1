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
 * Linked list implementation
 */
#ifndef LINKED_LISTABLE_H
#define LINKED_LISTABLE_H

#include "GeneralDefinitions.h"
#include "Iterators.h"

/**
    A linked member. Can be used as root of a linked list
    a specific linked list member can be derived from this class
*/
class LinkedListable {
friend class MultiLinkedListHolder;
friend class LinkedListHolder;
friend class StackHolder;
friend class QueueHolder;
protected:
    LinkedListable *next;
public:
    LinkedListable(){
        next = NULL;
    }

    /** Needs to be virtual to allow proper deallocation on derived classes when used on a generic holder. */
    virtual ~LinkedListable(){ next = NULL; }

    /** Return the next element. */
    LinkedListable *Next()const{
        return next;
    }

    /** Return the next element. */
    void SetNext(LinkedListable *p){
        next = p;
    }

    /** The amount of elements in this sub-list the current element included. */
    uint32 Size(){
        LinkedListable *p = this;
        uint32 count = 0;
        while (p!=NULL){ p = p->next;count++;}
        return count;
    }

    /** Bubble Sort the sub-list to the right of this element. */
    void BSort(SortFilter *sorter){
        if (sorter == NULL) return ;
        if (next == NULL) return ;
        uint32 count = 1;
        LinkedListable *p = this;
        while(p->next->next != NULL){
            count++;
            LinkedListable *a1 = p->next;
            LinkedListable *a2 = p->next->next;
            if (sorter->Compare(a1,a2) > 0){
                a1->next = a2->next;
                a2->next = a1;
                p->next = a2;
                p = a2;
            } else p = a1;
        }
        while (count > 2){
            LinkedListable *p = this;
            uint32 index = count;
            while(index > 2){
                index--;
                LinkedListable *a1 = p->next;
                LinkedListable *a2 = p->next->next;
                if (sorter->Compare(a1,a2) > 0){
                    a1->next = a2->next;
                    a2->next = a1;
                    p->next = a2;
                    p = a2;
                } else p = a1;
            }
            count--;
        }
    }

    /** Bubble Sort the sub-list to the right of this element. */
    void BSort(SortFilterFn *sorter){
        if (sorter == NULL) return ;
        if (next == NULL) return ;
        uint32 count = 1;
        LinkedListable *p = this;
        while(p->next->next != NULL){
            count++;
            LinkedListable *a1 = p->next;
            LinkedListable *a2 = p->next->next;
            if (sorter(a1,a2) > 0){
                a1->next = a2->next;
                a2->next = a1;
                p->next = a2;
                p = a2;
            } else p = a1;
        }
        while (count > 2){
            LinkedListable *p = this;
            uint32 index = count;
            while(index > 2){
                index--;
                LinkedListable *a1 = p->next;
                LinkedListable *a2 = p->next->next;
                if (sorter(a1,a2) > 0){
                    a1->next = a2->next;
                    a2->next = a1;
                    p->next = a2;
                    p = a2;
                } else p = a1;
            }
            count--;
        }
    }

    /** Insert in the next location the list p. */
    void Insert(LinkedListable *p){
        if (p == NULL) return ;
        LinkedListable *q = next;
        next = p;
        if (q == NULL) return;
        while(p->next != NULL) p = p->next;
        p->next = q;
    }

    /** Insert sorted all the elements of the sub-list p. */
    void Insert(LinkedListable *p,SortFilter *sorter){
        if (p == NULL) return;
        if (sorter == NULL){
            Insert(p);
            return;
        }
        if (p->next != NULL){
            LinkedListable root;
            root.next = p;
            root.BSort(sorter);
            p = root.next;
        }
        LinkedListable *list = this;
        while ((p != NULL) &&(list->next != NULL)){
            if (sorter->Compare(list->next,p)>0){
                LinkedListable *item = p;
                p = p->next;
                item->next = list->next;
                list->next = item;
            } else list = list->next;
        }
        if (p !=NULL) list->next = p;
    }

    /** Insert sorted all the elements of the sub-list p. */
    void Insert(LinkedListable *p,SortFilterFn *sorter){
        if (p == NULL) return;
        if (sorter == NULL){
            Insert(p);
            return;
        }
        if (p->next != NULL){
            LinkedListable root;
            root.next = p;
            root.BSort(sorter);
            p = root.next;
        }
        LinkedListable *list = this;
        while ((p != NULL) &&(list->next != NULL)){
            if (sorter(list->next,p)>0){
                LinkedListable *item = p;
                p = p->next;
                item->next = list->next;
                list->next = item;
            } else list = list->next;
        }
        if (p !=NULL) list->next = p;
    }

    /** Add at the end of the queue. */
    void Add(LinkedListable *p){
        if (p == NULL) return ;
        LinkedListable *q = this;
        while (q->next != NULL){
	        q = q->next;
		}
        q->next = p;
	    p->next = NULL;
    }

    /** */
    void AddL(LinkedListable *p){
        if (p == NULL) return ;
        LinkedListable *q = this;
        while (q->next != NULL){
	    q = q->next;
	}
        q->next = p;
    }

    /** Search if p is a member. */
    bool Search(LinkedListable *p){
        LinkedListable *q = this;
        while (q!=NULL){
            if (q==p) return True;
            q = q->next;
        }
        return False;
    }

    /** Search an element using a specific criteria. */
    LinkedListable *Search(SearchFilter *filter){
        if (filter == NULL) return NULL;
        LinkedListable *q = this;
        while (q!=NULL){
            if (filter->Test(q)) return q;
            q = q->next;
        }
        return NULL;
    }

    /** Search an element using a specific criteria. */
    LinkedListable *Search(SearchFilterFn *filter){
        if (filter == NULL) return NULL;
        LinkedListable *q = this;
        while (q!=NULL){
            if (filter(q)) return q;
            q = q->next;
        }
        return NULL;
    }

    /** Remove the requested element from list. Start searching from next element. */
    bool Extract(LinkedListable *p){
        if (p == NULL) return False;
        LinkedListable *q = this;
        if (p == q) return False;
        while ((q->next != p) && (q->next != NULL)) q= q->next;
        if (q->next == NULL) return False;
        q->next = q->next->next;
        p->next = NULL;
        return True;
    }

    /** Find and remove one element from list using filter as criteria. */
    LinkedListable *Extract(SearchFilter *filter){
        if (filter == NULL) return NULL;
        LinkedListable *q = this;
        while (q->next != NULL){
            if (filter->Test(q->next)){
                LinkedListable *p = q->next;
                q->next = q->next->next;
                p->next = NULL;
                return p;
            } else q = q->next;
        }
        return NULL;
    }

    /** Find and remove one element from list using filter. */
    LinkedListable *Extract(SearchFilterFn *filter){
        if (filter == NULL) return NULL;
        LinkedListable *q = this;
        while (q->next != NULL){
            if (filter(q->next)){
                LinkedListable *p = q->next;
                q->next = q->next->next;
                p->next = NULL;
                return p;
            } else q = q->next;
        }
        return NULL;
    }

    /** Delete the requested element. Start searching from next element. */
    bool Delete(LinkedListable *p){
        bool ret = Extract(p);
        if (ret) delete p;
        return ret;
    }

    /** Delete elements using a specific criteria. */
    uint32 Delete(SearchFilter *filter){
        uint32 deleted = 0;
        if (filter == NULL) return deleted;
        LinkedListable *q = this;
        while (q->next != NULL){
            if (filter->Test(q->next)){
                LinkedListable *p = q->next;
                q->next = q->next->next;
                delete p;
                deleted++;
            } else q = q->next;
        }
        return deleted;
    }

    /**
       @overload
    */
    uint32 Delete(SearchFilterFn *filter){
        uint32 deleted = 0;
        if (filter == NULL) return deleted;
        LinkedListable *q = this;
        while (q->next != NULL){
            if (filter(q->next)){
                LinkedListable *p = q->next;
                q->next = q->next->next;
                delete p;
                deleted++;
            } else q = q->next;
        }
        return deleted;
    }

    /** Browse through the list. */
    LinkedListable *Peek(uint32 index){
        LinkedListable *p = this;
        while((p != NULL) && (index > 0)) { p = p->next; index--; }
        return p;
    }

    /** For each item in the list do <it>. */
    void Iterate(Iterator *it){
        LinkedListable *p = this;
        while(p != NULL){
            it->Do(p);
            p = p->next;
        }
    }

    /** For each item in the list do <it>. */
    void Iterate(IteratorFn *it){
        LinkedListable *p = this;
        while(p != NULL){
            it(p);
            p = p->next;
        }
    }

};


#endif
