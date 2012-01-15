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

#if !defined(ERRORSYSTEM_INSTRUCTION)
#define ERRORSYSTEM_INSTRUCTION

#include "GenDefs.h"
#include "LinkedListHolder.h"
#include "FastPollingMutexSem.h"
#include "Threads.h"
#include "ErrorSystemInstructionItem.h"

class ObjectRegistryItem;

/** 
 * @file 
 * Holds information regarding the logging system caller
 */ 

/** Searches for ErrorSystemInstructionItem*/
class ESISearchFilter: public SearchFilter{
    /** */
    void                *object;
    /** */
    const char          *className;
    /** */
    int32               errorCode;
    /** */
    TID                 tid;
public:

    /** The constructor*/
    ESISearchFilter(void *object,const char *className,int32 errorCode){
        this->object = object;
        this->errorCode = errorCode;
        this->className = className;
        tid = Threads::ThreadId();
    }

    /** The test function*/
    bool Test(LinkedListable *data){
        ErrorSystemInstructionItem *p= (ErrorSystemInstructionItem *)data;
        bool flag = True;
        if (p->Object()   != ANY_VALUE)         flag &= (p->Object()    == object);
#if defined (_RTAI)
        if (p->Tid()      != (TID)NULL)         flag &= (p->Tid()       == tid);
#else
        if (p->Tid()      != (TID)0)            flag &= (p->Tid()       == tid);
#endif
        if (p->ErrorCode()!= (int32)ANY_VALUE)  flag &= (p->ErrorCode() == errorCode);
        if (p->ClassName()[0]!= 0)              flag &= (strcmp(p->ClassName(),className)==0);
        return  flag;
    }
};

/** A filter to delete ErrorSystemInstructionItem */
class ESIDeleteFilter: public SearchFilter{
    void *object;
public:
    /** teh constructor */
    ESIDeleteFilter(void *object){  this->object = object;  }

    /** the test function */
    bool Test(LinkedListable *data){
        ErrorSystemInstructionItem *p= (ErrorSystemInstructionItem *)data;
        bool flag = True;
        if (p->Object() != ANY_VALUE)
            flag &= (p->Object() == object);
        return  flag;
    }
};

/** A container of ErrorSystemInstruction: what to do in case of an error */
class ErrorSystemInstructions: public LinkedListHolder {

    /** manages the access to the resources */
    FastPollingMutexSem mux;
public:
    /** Searches using one of the 3 keys */
    uint32 SearchErrorAction(void *object,const char *className,int32 errorCode){
        mux.FastLock();
        ESISearchFilter searcher(object,className,errorCode);
        ErrorSystemInstructionItem *p= (ErrorSystemInstructionItem *)ListSearch(&searcher);
        uint32 ret = (uint32)NOT_FOUND;
        if (p != NULL) ret = p->ErrorAction();
        mux.FastUnLock();
        return ret;
    }

    /** Remove ESIs associated to an object */
    void PurgeByObject(void *object){
        mux.FastLock();
        ESIDeleteFilter remover(object);
        ListDelete(&remover);
        mux.FastUnLock();
    }
};

#endif

