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
 * Defines a class structure
 */
#if !defined (CLASS_STRUCTURE_)
#define CLASS_STRUCTURE_

#include "ClassStructureEntry.h"
#include "BasicTypes.h"

/** The type of the ClassStructure flags */
typedef int CSFlags;

/** The ClassStructure has a virtual table*/
const CSFlags CSF_HasVirtual    = 0x1;

/** The ClassStructure describes a BasicType */
const CSFlags CSF_BasicType     = 0x2;

class ClassStructureEntry;


/** The description of a class in term of members */
class ClassStructure{
public:

    /** flags to describe the structure */
    CSFlags flags;

private:
    union {

        struct {
            /** size, type and other descriptors in a 32 bit number
                it is infact a BasicTypeDescriptor    */
            int32  btd;
        } btInfo;

        struct {
            /** total structure size in bytes */
            int  size;

            /** NULL terminated list of members */
            ClassStructureEntry **members;
        } csInfo;


    };

public:
    /** constructor of a Basic Type ClassStructure */
    ClassStructure(
                const char *            name,
                BasicTypeDescriptor     btd)
    {
        flags = CSF_BasicType;
        btInfo.btd = btd.Value();
    }

    /** constructor of a ClassStructure */
    ClassStructure(
                const char *            name,
                int                     size,
                CSFlags                 flags,
                ClassStructureEntry **  members)
    {
        this->flags     = flags;
        csInfo.size     = size;
        csInfo.members  = members;
    }

    /** destructor */
    ~ClassStructure(){
        csInfo.members = NULL;
    }

    /** size (bytes) corrected in all cases*/
    int32 Size() const {
        if ((flags & CSF_BasicType)==0) return csInfo.size;
        BasicTypeDescriptor btd(btInfo.btd);
        return btd.ByteSize();
    }

    /** retrieve components of this structure */
    ClassStructureEntry **Members() const {
        if ((flags & CSF_BasicType)==0) return csInfo.members;
        return NULL;
    }

    /** Checks if of BasicType, and returns the BasicType Equivalent */
    bool IsBasicType(BasicTypeDescriptor &btd){
        if((flags & CSF_BasicType) == 0)  return False;
        BasicTypeDescriptor btd2(btInfo.btd);
        btd = btd2;
        return True;
    }
};

#endif

