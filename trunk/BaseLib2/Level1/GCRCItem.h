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
 * The elements used to store references in the references containers
 */
#if !defined (GC_REFERENCE_CONTAINER_ITEM_H)
#define GC_REFERENCE_CONTAINER_ITEM_H

#include "System.h"
#include "LinkedListable.h"
#include "GCRTemplate.h"
#include "BString.h"

/** abstract class for container of references */
class GCRCItem: public LinkedListable{

public:

    /** get next element
        skip no GCRCItem derivatives */
    GCRCItem *Next();

    /* access the object reference */
    virtual GCReference     Reference() = 0;

    /* access the link */
    virtual const char *    Link() = 0;

    virtual ~GCRCItem(){};

};


/** the basic element of storage */
class GCRCIBasic: public GCRCItem{

    /** the actual reference */
    GCReference gc;

public:
    /** set the reference */
    bool Load(GCReference gc){
        this->gc = gc;
        return gc.IsValid();
    }

    /* access the object reference */
    virtual GCReference Reference(){
        return gc;
    }

    virtual const char *    Link(){
        return NULL;
    }

    virtual ~GCRCIBasic(){};

};

class GCRCILink;

extern "C" {

    void GCRCILReference(GCRCILink &gcrcil, GCReference &gc);

}

/** a soft link to an object  */
class GCRCILink: public GCRCItem{

friend void GCRCILReference(GCRCILink &gcrcil, GCReference &gc);
    /** the actual reference */
    BString objectPath;

public:
    /** set the reference  */
    bool Load(const char * objectPath){
        this->objectPath = objectPath;
        return True;
    }

    /* access the object reference */
    virtual GCReference Reference(){
        GCReference gc;

        GCRCILReference(*this,gc);

        return gc;
    }

    virtual const char *    Link(){
        return objectPath.Buffer();
    }

    /** */
    GCRCILink(){
    }

    virtual ~GCRCILink(){};

};

#if 0

/** the specialised element of storage */
template<typename T>
class GCRCITemplate: public GCRCItem{

    /** the actual reference */
    GCRTemplate<T> gc;

public:
    /** set the reference  */
    bool Load(GCReference ref){
        gc = ref;
        return gc.IsValid();
    }

    /* access the object reference */
    GCReference Reference(){
        return gc;
    }

    /* access the object reference */
    GCRTemplate<T> RefT(){
        return gc;
    }

    /* skip no GCRTemplate<T> derivatives */
    GCRCItem *Next(){
        LinkedListable *ll = LinkedListable::Next();
        GCRTemplate<T> *grcit = NULL;
        if (ll) grcit = dynamic_cast<GCRTemplate<T> *> (ll);
        while ((grcit == NULL) && (ll != NULL)){
            ll = ll->Next();
            if (ll) grcit = dynamic_cast<GCRTemplate<T> *> (ll);
        }

        return reinterpret_cast<GCRCItem *>(grcit);
    }

    virtual ~GCRCITemplate(){};

};

#endif

#endif
