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
 * A global container of references to objects of type GarbageCollectable + Object.
 *
 * Only a single GlobalObjectData exists per application, if loaded with a configuration 
 * file it will to navigate, search and retrieve a reference anywhere in the tree
 */

#if !defined (GLOBAL_OBJECT_DATABASE_H)
#define GLOBAL_OBJECT_DATABASE_H

#include "GCReferenceContainer.h"
#include "StreamInterface.h"
#include "BString.h"
#include "Iterators.h"



extern "C" {

//    /** retrieve reference to database
//        it will create it if this is the first time it is accessed */
//    GCReferenceContainer &GetGlobalObjectDataBase();

    /** retrieve reference to database
        it will create it if this is the first time it is accessed */
    void _GetGlobalObjectDataBase(GCRTemplate<GCReferenceContainer> &GODB);

    /** @param enable True will enable display of statistics on
        left-over objects at database destruction*/
    void GlobalObjectDataBaseEnableDebugging(bool enable);

}

/** retrieve reference to database
    it will create it if this is the first time it is accessed */
static inline GCRTemplate<GCReferenceContainer>
GetGlobalObjectDataBase(){
    GCRTemplate<GCReferenceContainer> GODB;
    _GetGlobalObjectDataBase(GODB);
    return GODB;
}


/** retrieves an object by name
    if name starts with ( it is assumed to be a unique name
    and is therefore searched recursively
    GODB is the name of the GlobalObjectDataBase
    if unMatched is not NULL then a partial match is allowed.
    The unmatched string is returned in unMatched
    recursive match will also be enabled if @param forceRecurseSearch is True
    */
static inline GCReference GODBFindByName(
    const char *                    name,
    const char **                   unMatched           =   NULL,
    bool                            forceRecurseSearch  =   False
     ){

    if (name == NULL){
        GCReference gc;
        return gc;
    }

    if (strncasecmp(name,"GODB",4)==0){
        name += 4;
        if (name[0] == 0) return GetGlobalObjectDataBase();
        if (name[0] != '.') {
            GCReference gc;
            return gc;
        }
        name ++;
    }

    GCFlagType gcft = GCFT_None;
    // in case of unique name search the whole tree not just the root
    // the logic here was inverted and then the line was commented out...
    if ((name[0] == '(') || forceRecurseSearch) gcft = GCFT_Recurse;
    return GetGlobalObjectDataBase()->Find(name,gcft,unMatched);
}

/** returns the global name of an object in  @param name, if there is one.
The global name contains the name of all the containers from the
GlobalObjectDataBase to the element @param ref */
static inline bool GODBFindGlobalName(BString &name,GCReference ref){

    return False;
}

#endif

