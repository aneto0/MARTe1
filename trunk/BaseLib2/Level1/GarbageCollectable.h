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
 * Inherit from this class to implement garbage collection on a class
 *
 * A class interface for implementing classes whose number of references
 * is always accounted for
 */

#if !defined (_GARBAGECOLLECTABLE_H)
#define _GARBAGECOLLECTABLE_H

#include "Object.h"

class GarbageCollectable;

extern "C"{
    int32 GCDecrement(GarbageCollectable &gc);
    int32 GCIncrement(GarbageCollectable &gc);
}

class GCReference;

class GarbageCollectable{

private:

    friend class GCReference;
    friend int32 GCDecrement(GarbageCollectable &gc);
    friend int32 GCIncrement(GarbageCollectable &gc);

#if defined(_LINUX) || defined (_RTAI) || defined (_MACOSX)
    template<typename T> friend class GCRTemplate;
#endif

    /** The number of references to this object. */
    volatile int32 referenceCounter;


protected:
    /** Constructor.
        Initialises referenceCounter to 0 */
    GarbageCollectable(){referenceCounter=0;};

#if defined (GCRLOADPOINTER)
public:
#endif

    /** Used to atomically increment the reference counter. */
    int32 Increment(){
        return GCIncrement(*this);
        //Atomic::Increment(&referenceCounter);};
    }

    /** Used to atomically decrement the reference counter. */
    int32 Decrement(){
        return GCDecrement(*this);
        //Atomic::Decrement(&referenceCounter);
    };

    /** To allow cloning of objects using references the final class must implement clone. */
    virtual GarbageCollectable* Clone() const {return NULL;}

public:

    /** Gets the number of references. */
    int32 NumberOfReferences() const{return referenceCounter;};

    /** Destructor.
        Only called when the object is actually destroyed.
        The code does nothing here!  */
    virtual ~GarbageCollectable(){};


};


#endif
