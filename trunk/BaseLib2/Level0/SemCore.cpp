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

#include "SemCore.h"
#include "LinkedListHolder.h"
#include "FastPollingMutexSem.h"

#if (defined (_VXWORKS) || defined(_RTAI) || defined (_LINUX) || defined (_SOLARIS) || defined(_MACOSX))

///
class SemInfoBit : public LinkedListable{
    ///
    char *name;

    ///
    SEM_ID  id;

    /// some specific semaphore information
    void *data;

public:

    ///
    int32 useCount;

    ///
    char *Name(){
        return name;
    }

    ///
    SEM_ID Id(){
        return id;
    }

    ///
    SemInfoBit(SEM_ID id, char *name,void *data){
        this->id   = id;
        if (name == NULL)   this->name = NULL;
        else                this->name = strdup(name);
        this->data = data;
        useCount = 1;
    }

    ~SemInfoBit(){
        if (name != NULL) free((void *&)name);
        name = NULL;
    }

    ///
    void *Data(){
        return data;
    }
};

///
class SemNameFinder : public SearchFilter{
    ///
    char *name;
    ///
    bool checkCount;
public:
    ///
    SemNameFinder(char *name,bool checkCount = False){
        if (name == NULL)   this->name = NULL;
        else                this->name = strdup(name);
        this->checkCount = checkCount;
    }
    ///
    virtual ~SemNameFinder(){
        if (name != NULL) free((void *&)name);
        name = NULL;
    }
    ///
    bool Test(LinkedListable *data){
        SemInfoBit *p = (SemInfoBit *)data;
        if (strcmp(name,p->Name())==0){
            return True;
        }
        return False;
    }
};

///
class SemPurger : public SearchFilter{
    ///
    SEM_ID id;
    ///
    bool found;
public:
    ///
    SemPurger(SEM_ID id){
        this->id = id;
        found = False;
    }
    ///
    bool Test(LinkedListable *data){
        SemInfoBit *p = (SemInfoBit *)data;
        if (id == p->Id()){
            found = True;
            p->useCount--;
            if (p->useCount <=0){
                return True;
            }
        }
        return False;
    }
    ///
    bool Found(){
        return found;
    }
};

//
static class SemNameDB : public LinkedListHolder{
public:
    ///
    void Add(SEM_ID id, char *name,void *data=NULL){
        ListAdd(new SemInfoBit(id,name,data));
    }
    ///
    SEM_ID UseExisting(char *name,void *&data){
        SemNameFinder f(name);
        SemInfoBit *p = (SemInfoBit *)ListSearch(&f);
        if (p != NULL){
            data = p->Data();
            p->useCount++;
            return p->Id();
        } else return (SEM_ID)0;
    }
    ///
    bool Delete(SEM_ID id){
        SemPurger f(id);
        bool done = ListDelete(&f);        
        done |= !f.Found();
        return done;
    }
} SemNameDataBase;

bool TrueFinder(LinkedListable *data){ return True; }

///
void SemNameDataBaseList(){
    SemInfoBit *list = (SemInfoBit *)SemNameDataBase.List();

    int ix = 0;
    while(list != NULL){
        printf("sem %i: semid %08x %s\n",ix++,list->Id(),list->Name());
        list = (SemInfoBit *)list->Next();
    }
}

///
void SemNameDataBaseErase(){
    SemNameDataBase.ListDelete(TrueFinder);
}

///
void SemNameDataBaseAdd(SEM_ID id, char *name,void *data){
    SemNameDataBase.Add(id, name,data);
}

///
SEM_ID SemNameDataBaseUseExisting(char *name){
    void *p = NULL;
    return SemNameDataBaseUseExisting2(name,p);
}

///
SEM_ID SemNameDataBaseUseExisting2(char *name,void *&data){
    return SemNameDataBase.UseExisting(name,data);
}

///
bool SemNameDataBaseDelete(SEM_ID id){
    return SemNameDataBase.Delete(id);
}

/*#elif defined(_RTAI)

void SemNameDataBaseErase(){}

///
void SemNameDataBaseList(){}

///
void SemNameDataBaseAdd(int id, char *name,void *data){}

///
SEM_ID SemNameDataBaseUseExisting2(char *name,void *&data){ return 0; }

///
SEM_ID SemNameDataBaseUseExisting(char *name){ return 0; }

///
bool SemNameDataBaseDelete(int id){return False;}*/


#else

void SemNameDataBaseErase(){}

///
void SemNameDataBaseList(){}

///
void SemNameDataBaseAdd(int id, char *name,void *data){}

///
int SemNameDataBaseUseExisting2(char *name,void *&data){ return 0; }

///
int SemNameDataBaseUseExisting(char *name){ return 0; }

///
bool SemNameDataBaseDelete(int id){return False;}


#endif

