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

#include "ObjectRegistryDataBase.h"
#include "ObjectRegistryItem.h"
#include "ClassStructureEntry.h"
#include "LoadableLibrary.h"
#include "StreamAttributes.h"
#include "PrintfStreamInterface.h"

static inline int cmpExt(const char *a,const char *b){

    if (a==b) return 0;
    if (b==NULL) return 1;
    if (a==NULL) return -1;

    int sza = strlen(a);
    int szb = strlen(b);
    if (sza>szb){
        a += (sza-szb);
    } else return -1;

    return strcmp(a,b);
}

/**
    the system-wide class database
    Can be used to save and load classes creating recogniseable objects
*/
class ObjectRegistryDataBase : private LinkedListHolder{
    // this should point to itself. If not the object is not initialized
    void *initCheck;

    // this keeps a copy of the list
    ObjectRegistryItem *saveRoot;


public:
    ///
    ObjectRegistryDataBase(){
        // during initialization the root of the list is wiped
        // since this object could be initialized later than it was used, a trick had to be used
        if (initCheck != &initCheck){
            initCheck = &initCheck;
        }
        // if the list has been already initialized then recover the lost list
        else{
            ListAddL(saveRoot);
        }
    }

    ///
    ~ObjectRegistryDataBase(){
        Reset();
        initCheck = NULL;
    }

    void Delete(ObjectRegistryItem *p){
        if (initCheck == &initCheck)
        ListExtract(p);
    }

    /// all the actions are aimed at allowing insertion to the list even if it has not been yet initialized
    void Add(ObjectRegistryItem *p){
        //
        if (initCheck != &initCheck){
            initCheck = &initCheck;
            // forces a list initialization
            Reset();
        }
        ObjectRegistryItem *q = (ObjectRegistryItem *)List();
        while(q!=NULL){
            if (strcmp(q->ClassName(),p->ClassName())==0){
                p->structure    = q->structure;
                ListExtract(q);
                q = NULL;
            } else
                q = (ObjectRegistryItem *)p->Next();
        }

        ListAdd(p);
        // save continuously the top of the list
        saveRoot = (ObjectRegistryItem *)List();
    }

    ///
    void DisplayRegisteredClasses(StreamInterface *stream,bool onlyAllocated){
        ObjectRegistryItem *p = (ObjectRegistryItem *)List();

        PrintfStreamInterface psi;
        if (stream == NULL) stream = &psi;

        if (stream == NULL){
            printf("className           version     size   #objects\n");
        } else {
            stream->Printf("className           version     size   #objects\n");
            stream->SSPrintf(ColourStreamMode,"%i %i",Green,Black);
            stream->Printf("[ A = allocated L = DLL B= Buildable ]\n");
            stream->SSPrintf(ColourStreamMode,"%i %i",Grey,Black);
        }

        while(p!=NULL){
            if ((!onlyAllocated)||(p->nOfAllocatedObjects>0)){
                if (stream == NULL) {
                    printf("%30s %20s %x\n",p->ClassName(),p->Version(),p->Size());
                } else {

                    stream->SSPrintf(ColourStreamMode,"%i %i",Grey,Black);
                    stream->Printf("[");
                    stream->SSPrintf(ColourStreamMode,"%i %i",Green,Black);
                    if (p->Tools() != NULL){
                        if (p->Tools()->buildFn != NULL){
                            stream->Printf("B");
                        } else stream->Printf(" ");
                    } else stream->Printf(" ");
                    if (p->Library() != NULL){
                        stream->Printf("L");
                    } else stream->Printf(" ");
                    if (p->nOfAllocatedObjects > 0){
                        stream->Printf("A");
                    } else stream->Printf(" ");

                    stream->SSPrintf(ColourStreamMode,"%i %i",Grey,Black);
                    stream->Printf("]");

                    stream->SSPrintf(ColourStreamMode,"%i %i",Red,Black);
                    stream->Printf("%30s ",p->ClassName());
                    stream->SSPrintf(ColourStreamMode,"%i %i",DarkRed,Black);
                    stream->Printf("%10s ",p->Version());
                    stream->SSPrintf(ColourStreamMode,"%i %i",Red,Black);
                    stream->Printf("%5i ",p->Size());

                    stream->SSPrintf(ColourStreamMode,"%i %i",DarkRed,Black);
                    if (p->nOfAllocatedObjects > 0){
                        stream->Printf("%6i",p->nOfAllocatedObjects);
                    } else stream->Printf("      ");

                    //stream->Printf("%30s %20s %x\n",p->ClassName(),p->Version(),p->Size());
                    stream->SSPrintf(ColourStreamMode,"%i %i",Grey,Black);
                    stream->Printf("\n");
                }
            }
            p = (ObjectRegistryItem *)p->Next();
        }
    }


    ///
    ObjectRegistryItem *Find(const char *className){
        const char *hasDllName = strstr(className,"::");
        const int maxSize = 128 + 1;
        char dllName_[maxSize];
        const char *dllName;

        // check for empty className
        if (className[0] == 0) return NULL;

        // check for dllName::className syntax
        if (hasDllName){
            int size = hasDllName-className;			
            if (size > maxSize) size = maxSize;
            dllName_[0] = 0;
            strncat(dllName_,className,size);
            dllName = dllName_;
			className = hasDllName + 2;            
        } else {
            dllName = className;
        }

        if (className == NULL) return NULL;
        ObjectRegistryItem *p = (ObjectRegistryItem *)List();
        while (p != NULL){
            if (strcmp(p->ClassName(),className)==0){
                return p;
            }
            p = (ObjectRegistryItem *)p->Next();
        }

        // add extension
        char *fullName = NULL;
        fullName = (char *)malloc(strlen(dllName)+5);
        LoadableLibrary *ll = new LoadableLibrary;
        bool ret = False;
//        if (!ret) {
//            sprintf(fullName,"%s.bem",className);
//            ret = ll->Open(fullName);
//        }

#if defined(_MACOSX)
        // MAC OS X NAME FOR DLL
        if (!ret) {
            sprintf(fullName,"%s.dylib",dllName);
            ret = ll->Open(fullName);
        }
#else
        // LINUX NAME FOR DLL
        if (!ret) {
            sprintf(fullName,"%s.so",dllName);
            ret = ll->Open(fullName);
        }
        // STD NAME FOR DLL
        if (!ret) {
            sprintf(fullName,"%s.dll",dllName);
            ret = ll->Open(fullName);
        }
#endif
        // GENERAL APPLICATION MODULE
        if (!ret) {
            sprintf(fullName,"%s.gam",dllName);
            ret = ll->Open(fullName);
        }
        // HIGH LEVEL DRIVER (GENERAL ACQUISITION MODULE)
        if (!ret) {
            sprintf(fullName,"%s.drv",dllName);
            ret = ll->Open(fullName);
        }

        free((void *&)fullName);
        if (!ret) return NULL;

        p = (ObjectRegistryItem *)List();
        while (p != NULL){
            if (strcmp(p->ClassName(),className)==0){
                p->ll = ll;
                return p;
            }
            p = (ObjectRegistryItem *)p->Next();
        }

        delete ll;

        return NULL;
    }

    ///
    ObjectRegistryItem *FindByCode(uint32 classId){
        if (classId == 0) return NULL;
        ObjectRegistryItem *p = (ObjectRegistryItem *)List();
        while(p!=NULL){
            if (p->ClassId() == classId) return p;
            p = (ObjectRegistryItem *)p->Next();
        }
        return NULL;
    }

    /// pass the full object file name or just its extension
    ObjectRegistryItem *FindByExtension(const char *objectName){
        if (objectName == NULL) return NULL;
        ObjectRegistryItem *p = (ObjectRegistryItem *)List();
        while(p!=NULL){
            if (cmpExt(objectName,p->DefaultExtension())==0){
                return p;
            }
            p = (ObjectRegistryItem *)p->Next();
        }
        return NULL;
    }

    ///
    ObjectRegistryItem *List(){ return(ObjectRegistryItem *)LinkedListHolder::List(); }

} ObjectRegistryDataBaseInstance ;

///
void ObjectRegistryDataBaseAdd(ObjectRegistryItem *p){
    ObjectRegistryDataBaseInstance.Add(p);
}

///
void ObjectRegistryDataBaseDelete(ObjectRegistryItem *p){
    ObjectRegistryDataBaseInstance.Delete(p);
}

///
void DisplayRegisteredClasses(StreamInterface *stream,bool showStructures){
    ObjectRegistryDataBaseInstance.DisplayRegisteredClasses(stream,showStructures);
}

///
ObjectRegistryItem *ObjectRegistryDataBaseFind(const char *className){
    return ObjectRegistryDataBaseInstance.Find(className);
}

ClassStructure *ObjectRegistryDataBaseFindStructure(const char *className){
    ObjectRegistryItem * ori = ObjectRegistryDataBaseFind(className);
    if (ori == NULL) return NULL;
    return ori->structure;
}

ObjectRegistryItem *ObjectRegistryDataBaseFindByCode(uint32 classId){
    return ObjectRegistryDataBaseInstance.FindByCode(classId);
}

ObjectRegistryItem *ObjectRegistryDataBaseFindByExtension(const char *objectName){
    return ObjectRegistryDataBaseInstance.FindByExtension(objectName);
}

ObjectRegistryItem *ObjectRegistryDataBaseList(){
    return ObjectRegistryDataBaseInstance.List();
}

Object *OBJObjectCreateByName(const char *name){
    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(name);

    if (ori == NULL) return NULL;
    ObjectTools *t = ori->Tools();

    if (t == NULL){
        CStaticAssertErrorCondition(Warning,"OBJObjectCreateByName::Class %s has no tools: cannot create object",name);
        return NULL;
    }

    if (t->buildFn == NULL){
        CStaticAssertErrorCondition(Warning,"OBJObjectCreateByName::Class %s tools have n creation Function: cannot create object",name);
        return NULL;
    }

    Object *ob = t->buildFn();
    return ob;
}

Object *OBJObjectCreateById(uint32 classId){
    ObjectRegistryItem *ori = ObjectRegistryDataBaseFindByCode(classId);

    if (ori == NULL) return NULL;
    ObjectTools *t = ori->Tools();
    if (t == NULL) return NULL;
    if (t->buildFn == NULL) return NULL;

    Object *ob = t->buildFn();
    return ob;
}


