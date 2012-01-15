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
 * A Database of classes 
 */
#if !defined (OBJECT_REGISTRY_DATABASE_H)
#define OBJECT_REGISTRY_DATABASE_H

#include "StreamInterface.h"

class ObjectRegistryItem;
class ClassStructure;
class ClassStructureEntry;
class Object;


extern "C" {

    /** Add a ObjectRegistryItem entry to the database */
    void ObjectRegistryDataBaseAdd(ObjectRegistryItem *p);

    /** remove a ObjectRegistryItem entry */
    void ObjectRegistryDataBaseDelete(ObjectRegistryItem *p);

    /** Lists the classes that have been registered
    @param onlyAllocated shows only classes with instances */
    void DisplayRegisteredClasses(StreamInterface *stream=NULL,bool onlyAllocated=False);

    /** Lists the classes that have instances */
    void DisplayAllocatedObjects(StreamInterface *stream=NULL);

    /** Find information about a class */
    ObjectRegistryItem *ObjectRegistryDataBaseFind(const char *className);

    /** Find data structure description of class */
    ClassStructure *ObjectRegistryDataBaseFindStructure(const char *className);

    /** Find information about a class specified by its class id*/
    ObjectRegistryItem *ObjectRegistryDataBaseFindByCode(uint32 classId);

    /** Find information about the class that is associated to a certain file extension */
    ObjectRegistryItem *ObjectRegistryDataBaseFindByExtension(const char *objectName);

    /** Get the linked list */
    ObjectRegistryItem *ObjectRegistryDataBaseList();

    /** Create an object of class name that must be one of the registered classes
        It needs RTTI up casting to be usable  */
    Object *OBJObjectCreateByName(const char *name);

    /** Create an object of one of the registered classes specified by classId
        It needs RTTI up casting to be usable  */
    Object *OBJObjectCreateById(uint32 classId);

}


#endif


