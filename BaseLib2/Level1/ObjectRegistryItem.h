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
 * Container for object properties
 */
#if !defined (_OBJECT_REGISTRY_ITEM_H)
#define _OBJECT_REGISTRY_ITEM_H

#include "System.h"
#include "LinkedListable.h"
#include "ErrorSystemInstructions.h"
#include "ClassStructure.h"
#include "LoadableLibrary.h"


class Object;

/** create an object of the specific type. */
typedef Object *(ObjectBuildFn )();

/** some functions that can be used on an object if the name of the object is known (or type) */
class ObjectTools {
public:
    /** calls the object constructor */
    ObjectBuildFn       *buildFn;

    /** constructor */
    ObjectTools(ObjectBuildFn *buildFn ){
        this->buildFn       = buildFn;
    }
};

extern "C" {
    /** */
    uint32 NameToClassId(const char *name);
    /** */
    void ORIInit(ObjectRegistryItem *ori);
    /** */
    void ORISetup(ObjectRegistryItem *ori,
                    const char *className,
                    const char *version,
                    const char *defaultExtension,
                    uint32 defaultFlags,
                    ObjectTools *tools);
    /** */
    void ORISetupStructure(ObjectRegistryItem *ori,
                    const char *className,
                    ClassStructure *structure);
    /** */
    void ORIFinish(ObjectRegistryItem *ori);

}

/** a class to store information about a class type */
class ObjectRegistryItem: public LinkedListable {
    //
    friend void ORIInit(ObjectRegistryItem *ori);
    //
    friend void ORISetup(ObjectRegistryItem *ori,
                    const char *className,
                    const char *version,
                    const char *defaultExtension,
                    uint32 defaultFlags,
                    ObjectTools *tools);
    //
    friend void ORIFinish(ObjectRegistryItem *ori);

public:
    /** the name of the class */
    const char *        className;

    /** the extension used as default */
    const char *        defaultExtension;

    /** the relative position of Object within the class */
    int32               objectOffset;

    /** some default attributes for the class */
    uint32              defaultFlags;

    /** the version of this class */
    char                version[8];

    /** an user definable class Id the default is a checksum of the name based on SUM(x) sum(x^2) sum(x^3) sum(x^4) */
    uint32              classId;

    /** the structure of this class */
    ClassStructure *    structure;

    /** the function to create this class */
    ObjectTools *       tools;

    /** the dll containing the object code */
    LoadableLibrary *   ll;

    /** the initializer */
    void Setup(const char *className,
               const char *version,
               const char *defaultExtension = NULL,
               uint32 defaultFlags = 0,
               ObjectTools *tools = NULL){
        ORISetup(this,className,version,defaultExtension,defaultFlags,tools);
    };

    /** the DUMMY initializer */
    ObjectRegistryItem(){  ORIInit(this);  }

public:

    /** on error behaviour */
    ErrorSystemInstructions classInstructions;

    /** the initializer
        will register into the DB */
    ObjectRegistryItem(const char *className,
                       const char *version,
                       int          objectOffset,
                       const char *defaultExtension = NULL,
                       uint32 defaultFlags = 0,
                       ObjectTools *tools = NULL){
        this->objectOffset = objectOffset;
        ORISetup(this,className,version,defaultExtension,defaultFlags,tools);
    }

    /** will init the structure details of a DB record using the structure information
        will add this ORI to the DB after calling the main constructor if a record of
        class className is not found  in the DB*/
    ObjectRegistryItem(const char *className,
                       ClassStructure *structure){
        this->objectOffset = objectOffset;
        ORISetupStructure(this,className,structure);
    }

    /** the DUMMY initializer */
    ~ObjectRegistryItem(){
        ORIFinish(this);
    }

    /** Returns the name of the class */
    const char *ClassName()  {
        return className;
    }

    /** Returns the type of the class. This identification code is unique within an application */
    intptr ClassType()       {
        return (intptr)className;
    }

    /** The class version as reported by CVS*/
    const char *Version()    {
        return version;
    }

    /** An file extension associated to this class */
    const char *DefaultExtension() {
        return defaultExtension;
    }

    /** Some properties of this class*/
    const uint32 DefaultFlags() {
        return defaultFlags;
    }

    /** This identification code is with an high probability unique among different applications */
    uint32 ClassId()         {
        return classId;
    }

    /**  to override the default setting of the class id  */
    void   SetClassId(uint32 id) {
        classId = id;
    }

    /** returns the create function */
    ObjectTools *Tools()     {
        return tools;
    }

    /** only if the memory allocation routines are in debugging mode */
    uint32 nOfAllocatedObjects;

    /** if the class structure is known, then returns the class size */
    int Size(){
        if (structure == NULL) return 0;
        return structure->Size();
    }

    /** the dll containing the object code */
    LoadableLibrary *   Library(){ return ll; }

};
#endif

