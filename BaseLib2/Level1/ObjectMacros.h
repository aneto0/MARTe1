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
 * Macros to automatically register objects with the original class and the object name
 */
#if !defined (_OBJECT_MACROS_H)
#define _OBJECT_MACROS_H


#define ObjectClassOffset(className)  ((size_t)((Object *)((className *)0x1000))-0x1000)

class ObjectRegistryItem;

/** To allow registering a class contained in a DLL.
    Use before the class declaration outside the class */
#define OBJECT_DLL(name)                                     \
    extern "C" {                                             \
        ObjectRegistryItem *Get_private_ ## name ## Info();  \
    }

/** To allow registering a class contained in a DLL. Use inside the class.
    remember to set the public/private/protected afterwards */
#define OBJECT_DLL_STUFF(name)                               \
public:                                                      \
    virtual ObjectRegistryItem  *Info() const{                \
        return Get_private_ ## name ## Info();               \
    }                                                        \
    static void operator delete(void *p){                    \
        OBJDeleteFun(p,Get_private_ ## name ## Info());      \
    }                                                        \
    static void *operator new (size_t len){            \
        return OBJNewFun(len,Get_private_ ## name ## Info());\
    }                                                        \
    friend Object * name ## BuildFn__ ();


/** To allow registering a class. Use inside the class.
    remember to set the public/private/protected afterwards */
#define OBJECT_STUFF(name)                                   \
public:                                                      \
    friend  ObjectRegistryItem *Get_private_ ## name ## Info();  \
    virtual ObjectRegistryItem  *Info() const{                 \
        return Get_private_ ## name ## Info();               \
    }                                                        \
    static void operator delete(void *p){                    \
        OBJDeleteFun(p,Get_private_ ## name ## Info());      \
    }                                                        \
    static void *operator new (size_t len){            \
        return OBJNewFun(len,Get_private_ ## name ## Info());\
    }

/** Register a class with its version. Use in the .cpp file  */
#define OBJECTREGISTER(name,ver)                                                                             \
    static ObjectRegistryItem _private_ ## name ## Info( #name ,ver, ObjectClassOffset(name));               \
    ObjectRegistryItem *Get_private_ ## name ## Info(){                                                      \
        return &_private_ ## name ## Info;                                                                   \
    }

/** Register a class with its version. Use in the .cpp file.
    Automatically creates the Build function */
#define OBJECTLOADREGISTER(name,ver)                                                                         \
    Object * name ## BuildFn__ (){                                                                           \
        name *p = new name () ;                                                                           \
        Object *o = p;                                                                                       \
        return o;                                                                                            \
    }                                                                                                        \
    static ObjectTools name ## ObjectTools(&  name ## BuildFn__);                                          \
    static ObjectRegistryItem _private_ ## name ## Info( #name ,ver, ObjectClassOffset(name),"",0,& name ## ObjectTools);   \
    ObjectRegistryItem *Get_private_ ## name ## Info(){                                                      \
        return &_private_ ## name ## Info;                                                                   \
    }


/** Register a class with its version. Use in the .cpp file.
    Automatically creates the Build function.
    Sets the flags an the file extension. */
#define OBJECTLOADREGISTERFLAGS(name,ext,flags,ver)                                                          \
    Object * name ## BuildFn__ (){                                                                           \
        name *p = new name () ;                                                                              \
        Object *o = p;                                                                                       \
        return o;                                                                                            \
    }                                                                                                        \
    static ObjectTools name ## ObjectTools(& name ## BuildFn__);                                             \
    static ObjectRegistryItem _private_ ## name ## Info( #name ,ver, ObjectClassOffset(name),ext,flags,& name ## ObjectTools);   \
    ObjectRegistryItem *Get_private_ ## name ## Info() {                                                     \
        return &_private_ ## name ## Info;                                                                   \
    }

/** Register a structure with its fields (structure).
    Use in the .cpp file.
 */
#define STRUCTREGISTER(name,structure)                                                                       \
    static ObjectRegistryItem  structure ## __ORI__( name ,&structure);

/** Register a basic type (int float ...).
    Use in the .cpp file.
 */
#define BASICTYPEREGISTER(name,type)                                                                         \
    static ClassStructure     name ## __CS__ (#name,type);                                                   \
    static ObjectRegistryItem name ## __ORI__(#name ,& name ## __CS__);                                      \
    ObjectRegistryItem *Get_ ## name ## __ORI__() {                                                             \
        return & name ## __ORI__;                                                                             \
    }

/** Register a basic type with a modifier in the name (short int long inr=t ...).
    Use in the .cpp file.
 */
#define BASICTYPEREGISTER2(mod,name,type )                                                                   \
    static ClassStructure      mod ## _ ## name ## __CS__ (#mod " " #name,type);                             \
    static ObjectRegistryItem  mod ## _ ## name ## __ORI__(#mod " " #name,& mod ## _ ##name ## __CS__);      \
    ObjectRegistryItem *Get_ ## mod ## _ ## name ## __ORI__() {                                              \
        return & mod ## _ ## name ## __ORI__;                                                                 \
    }

/** Register a basic type with 2 modifiers in the name (short int long inr=t ...).
    Use in the .cpp file.
 */
#define BASICTYPEREGISTER3(mod,mod2,name,type )                                                              \
    static ClassStructure      mod ## _ ## mod2 ## _ ## name ## __CS__ (#mod " " #mod2 " " #name,type);      \
    static ObjectRegistryItem  mod ## _ ## mod2 ## _ ## name ## __ORI__(#mod " " #mod2 " " #name,& mod ## _ ## mod2 ## _ ##name ## __CS__); \
    ObjectRegistryItem *Get_ ## mod ## _ ## mod2 ## _ ## name ## __ORI__() {                                 \
        return & mod ## _ ## mod2 ## _ ## name ## __ORI__;                                                    \
    }


#endif

