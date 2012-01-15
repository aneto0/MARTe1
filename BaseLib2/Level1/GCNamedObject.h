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
 * Extension of the Object class to include garbage collection and naming
 * Used to store object in the GlobalObjectDataBase
 */
#if !defined (GC_NAMED_OBJECT_H)
#define GC_NAMED_OBJECT_H

#include "GarbageCollectable.h"
#include "Object.h"
#include "BString.h"

class GCNamedObject;

extern "C" {
    /** ObjectloadSetup*/
    bool GCNOObjectLoadSetup(GCNamedObject &gcno,ConfigurationDataBase &info,StreamInterface * err);

    /** ObjectSaveSetup*/
    bool GCNOObjectSaveSetup(GCNamedObject &gcno,ConfigurationDataBase &info,StreamInterface * err);
}


OBJECT_DLL(GCNamedObject)

/** A wrapper class for GCRTemplate with the additional feature to be LinkedListable. */
class GCNamedObject: public Object, public GarbageCollectable{

OBJECT_DLL_STUFF(GCNamedObject)

    friend bool GCNOObjectLoadSetup(GCNamedObject &gcno,ConfigurationDataBase &info,StreamInterface * err);
    friend bool GCNOObjectSaveSetup(GCNamedObject &gcno,ConfigurationDataBase &info,StreamInterface * err);

private:
    /** the name of the object. It always contains a valid pointer
        to allocated memory */
    const char * name;

public:
    /** initialise to no  name */
    GCNamedObject(){
        name = strdup("");
    }

    /** deallocate memory */
    virtual ~GCNamedObject(){
        if (name != NULL) {
            free ((void *&)name);
        }
    }

    /** access the name */
    const char *Name()const{
        return name;
    }

    /** set the name */
    void SetObjectName(const char *newName){
        free ((void *&)name);
        if (newName == NULL) name = strdup("");
        else  name = strdup(newName);
    }

    /** Returns a name that contains also the address
        It begins with (address)NAME */
    void GetUniqueName(BString &name){
        char address[32];
        sprintf(address,"(%8p)",this);
        name = address;
        name += this->Name();
    }

    /** Initialises the name of the object
        If Name is found than the name is taken from there
        if missing then the nodeName is used
        if nodeName start with + then the + is removed
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err){
        return GCNOObjectLoadSetup(*this,info,err);
    }

    /**  */
    virtual     bool                ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err){
        return GCNOObjectSaveSetup(*this,info,err);
    }

};


#endif
