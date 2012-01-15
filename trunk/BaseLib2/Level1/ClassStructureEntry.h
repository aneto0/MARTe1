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

#if !defined (CLASS_STRUCTURE_ENTRY_)
#define CLASS_STRUCTURE_ENTRY_

#include "GenDefs.h"

/** 
 * @file
 * Describes an entry within a class structure
 */

/** */
const int CSE_Const         = 0x1;

/** */
const int CSE_Static        = 0x2;

#define CSE_MAXSIZE 4

class ClassStructureEntry{
public:
    /** */
    const char *type;

    /** */
    const char *modif;

    /** */
    int  sizes[CSE_MAXSIZE];

    /** */
    int  flags;

    /** */
    const char *name;

    /** */
    int size;

    /** */
    int pos;

    /** */
    void SetUp(const char *type,const char *modif,int size0,int size1,int size2,int size3,int flags,const char *name,int size,int pos){
        this->name      = name;
        this->sizes[0]  = size0;
        this->sizes[1]  = size1;
        this->sizes[2]  = size2;
        this->sizes[3]  = size3;
        this->flags     = flags;
        this->type      = type;
        this->modif     = modif;
        this->size      = size;
        this->pos       = pos;
        if (this->sizes[0] <= 0){
            this->sizes[0] = 1;
        }
    }

    /** */
    void Copy(ClassStructureEntry &x){
        SetUp(x.type,x.modif,x.sizes[0],x.sizes[1],x.sizes[2],x.sizes[3],x.flags,x.name,x.size,x.pos);
    }

    /** */
    ClassStructureEntry(const char *type="",const char *modif="",int size0=0,int size1=0,int size2=0,int size3=0,int flags=0,const char *name="",int size=0,int pos=0){
        SetUp(type,modif,size0,size1,size2,size3,flags,name,size,pos);
    }


    /** */
    ClassStructureEntry &operator=(ClassStructureEntry &x){
        Copy(x);
        return *this;
    }

    /** */
    int NumberOfDimensions(){
        int ix = 0;
        while ((ix < CSE_MAXSIZE) && (sizes[ix]>0))ix++;
        return ix;
    }

};



#endif
