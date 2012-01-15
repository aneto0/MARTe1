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
 * Create classes that are joint GCNamedObject and user defined class
 * Used to add GarbageCollection and Name to existing classes
 */

#if !defined (GC_NAMED_OBJECT_EXTENDER_H)
#define GC_NAMED_OBJECT_EXTENDER_H

#include "GCNamedObject.h"

/** Extends objects to be GCNamedObject */
template <class T>
class GCNOExtender: public GCNamedObject, public T
{
public:

};

/** Extends objects to be GCNamedObject */
template <class T>
class GCNOExtender2: public GCNamedObject, public T
{
public:

    static void operator delete(void *p){
        T::operator delete(p);
    }

    static void *operator new (size_t len){
        return T::operator new(len);
    }

};
#endif

