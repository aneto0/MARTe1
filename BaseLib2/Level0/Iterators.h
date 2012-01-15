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
 * Definition of iterator/filters classes
 */

#ifndef _ITERATORS_H
#define _ITERATORS_H

#include "GenDefs.h"

/** the return codes when a Test2 is called in a SearchFilterT or
    when Do2 is called in IteratorT */
enum SFTestType{

    /** No meaning associated */
    SFTTNull      = 0x0000,

    /** This is exactly what I was looking for! */
    SFTTFound     = 0x1001,

    /** This is not what I was looking for */
    SFTTNotFound  = 0x1000,

    /** When recursive search where a path has
        been specified is used This is not the right path */
    SFTTWrongPath = 0x1002,

    /** When recursing this is to notify of a node beginning of
        recursion (used only for Do2) */
    SFTTRecurse   = 0x2000,

    /** When recursing this is to notify of the return
        to the previous level used also for Do2 */
    SFTTBack      = 0x2001
};
// Iterators support

class LinkedListable;

/** A class template to build an iterator filter. */
class  Iterator{
public:
/** The function performing the action linked to the iterator. */
    virtual void Do (LinkedListable *data)=0;
};

/** A class template to build an iterator filter. */
template <typename T>
class IteratorT{
public:
/** The function performing the action linked to the iterator. */
    virtual void Do (T data)=0;

    /** A more specialised form of the Do function to be used on certain
        applications. */
    virtual void Do2(T data, SFTestType mode = SFTTNull){
        Do(data);
    }
};

/** The type of a function to be iterated on a set.
    @param data */
typedef void (IteratorFn )(LinkedListable *data);

/**  A class template to build search filters. */
class  SearchFilter{
public:
    /** the function that performs the search on a set of data */
    virtual bool Test (LinkedListable *data)=0;
};



/** A class template to build an iterator filter. */
template <typename T>
class SearchFilterT{
public:
    /** The function performing the action linked to the iterator. */
    virtual bool Test (T data)=0;

    /** A more specialised form of the Test function to be used on certain
        applications. */
    virtual SFTestType Test2(T data, SFTestType mode = SFTTNull){
        if (Test(data)) return SFTTFound;
        return SFTTNotFound;
    }
};

/** The type of a function to be used to search on a set.
    @param data */
typedef bool (SearchFilterFn )(LinkedListable *data);

/**  A class template to build search filters. */
class  SortFilter{
public:
    /** A function that can be used to compare two object. */
    virtual int32 Compare(LinkedListable *data1,LinkedListable *data2)=0;
};

/** The type of a function to be used to sort a set.
    @param data1
    @param data2
*/
typedef int32 (SortFilterFn )(LinkedListable *data1,LinkedListable *data2);


/** just a rename of LinkedListable

    STACK  ->######    (Push)
           <-######    (Pop)
             ######
             <-|       (Peek)

*/
typedef LinkedListable Stackable ;

/*****************************************************************************/

/** just a rename of LinkedListable

    QUEUE    ######<-  (Add)
           <-######    (Extract)
*/
typedef LinkedListable Queueable ;

#endif

