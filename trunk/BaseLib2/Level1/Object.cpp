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

#include "ObjectMacros.h"
#include "Object.h"
#include "ObjectRegistryDataBase.h"
#include "FastPollingMutexSem.h"
#include "ErrorSystemInstructionItem.h"
#include "Threads.h"
#include "System.h"
#include "StreamInterface.h"

// a global mux to control all activities to static Object members
FastPollingMutexSem globalMux;

#define DEBUG_SEM 0
#if (DEBUG_SEM == 1)

inline bool gMuxLock_(uint32 timeout,int line){
printf("Thread %x at %i try locks for %i\n",ThreadClass::ThreadId(),line,timeout);
    bool ret = globalMux.fastLock(timeout);
if (ret == False)
printf("Thread %x fails\n",ThreadClass::ThreadId());
    return ret;
}

inline bool gMuxUnLock_(int line){
printf("Thread %x at %i Unlocks\n",ThreadClass::ThreadId(),line);
    return globalMux.FastUnLock();
}

#define gMuxLock gMuxLock_(INFINITE,__LINE__)
#define gMuxLock0 gMuxLock_(0,__LINE__)
#define gMuxUnLock gMuxUnLock_(__LINE__)

#else
inline bool gMuxLock_(){ return globalMux.FastLock();}
inline bool gMuxLock0_(){ return globalMux.FastTryLock(); }
inline bool gMuxUnLock_(){ return globalMux.FastUnLock(); }

#define gMuxLock gMuxLock_()
#define gMuxLock0 gMuxLock0_()
#define gMuxUnLock gMuxUnLock_()

#endif

// ###########################################################
// MEMORY ALLOCATION ROUTINES
// ###########################################################

bool allocationMonitoringEnabled=False;

void OBJEnableAllocationMonitoring(){
    allocationMonitoringEnabled = True;
}

void OBJDeleteFun(void *p,ObjectRegistryItem  *info){
    free(p);
    gMuxLock;
    info->nOfAllocatedObjects--;
    gMuxUnLock;
    if (allocationMonitoringEnabled){
#ifndef _RTAI
        fprintf(stderr,"%p %s::delete() : %d left\n",p,info->ClassName(),(int)(info->nOfAllocatedObjects));
        fflush(stderr);
#else
        printf("%p %s::delete() : %d left\n",p,info->ClassName(),(int)(info->nOfAllocatedObjects));
#endif
    }
}

void *OBJNewFun(unsigned int len,ObjectRegistryItem  *info){

    void *p = malloc(len);
    if (p!=NULL){
    if (info != NULL){
            gMuxLock;
            info->nOfAllocatedObjects++;
            gMuxUnLock;
    }
        if (allocationMonitoringEnabled){
            if (info != NULL)
#ifndef _RTAI
                fprintf(stderr,"%p %s::new()  %i objects\n",p,info->ClassName(),(int)(info->nOfAllocatedObjects));
#else
                printf("%p %s::new()  %i objects\n",p,info->ClassName(),(int)(info->nOfAllocatedObjects));
#endif
            else
#ifndef _RTAI
                fprintf(stderr,"%p Unknown ClassName::new() Couldn't access class info\n",p);
                fflush(stderr);
#else
                printf("%p Unknown ClassName::new() Couldn't access class info\n",p);
#endif
            
        }
    }
    return p;
}

// ###########################################################
// ERROR MANAGEMENT ROUTINES
// ###########################################################

ErrorSystemInstructions globalInstructions;

EMFErrorBehaviour globalErrorAction = (EMFErrorBehaviour)(onErrorRememberES | onErrorRemoteLogES);

/**
    sets the behaviour to have on error
    @param behaviour The possible values are:
    *     onErrorSulk     =  on error loop infinitely
    *     onErrorQuit     =  on error exit program
    *     onErrorRemember =  on error remember the error code
    *     onErrorReport   =  on error print the error somewhere
*/
/// Sets the global behaviour
void OBJSetGlobalOnErrorBehaviour(EMFErrorBehaviour behaviour){
    gMuxLock;
    globalErrorAction = behaviour;
    gMuxUnLock;
}

/// Sets the error behaviour for this thread
void OBJSetThreadOnErrorBehaviour(EMFErrorBehaviour behaviour,int32 code){
    gMuxLock;
    globalInstructions.ListInsert(TID_ERRORINSTRUCTION(behaviour,Threads::ThreadId(),code));
    gMuxUnLock;
}

void OBJSetClassOnErrorBehaviour(Object &ob,EMFErrorBehaviour behaviour,int32 code){
    gMuxLock;
    ObjectRegistryItem *info =ob.Info();
    if (info != NULL){
        info->classInstructions.ListInsert(CLASS_ERRORINSTRUCTION(behaviour,ob.ClassName(),code));
    } else {
        globalInstructions.ListInsert(CLASS_ERRORINSTRUCTION(behaviour,ob.ClassName(),code));
    }
    gMuxUnLock;
}

void OBJSetObjectOnErrorBehaviour(Object &ob,EMFErrorBehaviour behaviour,int32 code){
    gMuxLock;
    ObjectRegistryItem *info =ob.Info();
    if (info != NULL){
        info->classInstructions.ListInsert(OBJECT_ERRORINSTRUCTION(behaviour,&ob,code));
    } else {
        globalInstructions.ListInsert(OBJECT_ERRORINSTRUCTION(behaviour,&ob,code));
    }
    gMuxUnLock;
}

ErrorSystemInstructions *OBJGetGlobalInstructions(){
    return &globalInstructions;
}

uint32 OBJGetGlobalErrorAction(){
    return globalErrorAction;
}



#if defined(_VXWORKS)
void Object::AssertErrorCondition(EMFErrorType errorCode,const char *errorDescription/* =NULL */,...) const{
    va_list argList;
    va_start(argList,errorDescription);
    VCAssertErrorCondition(errorCode,this,ClassName(),errorDescription,argList);
    va_end(argList);
}

void Object::ISRAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription/* =NULL */,...){
    va_list argList;
    va_start(argList,errorDescription);
    VCISRAssertErrorCondition(errorCode,this,ClassName(),errorDescription=NULL,argList);
    va_end(argList);
}

void Object::AssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription/* =NULL */,...){
    va_list argList;
    va_start(argList,errorDescription);
    VCAssertPlatformErrorCondition(errorCode,this,ClassName(),errorDescription,argList);
    va_end(argList);
}

void Object::VAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,va_list argList){
    VCAssertErrorCondition(errorCode,this,ClassName(),errorDescription,argList);
}



#endif


OBJECTREGISTER(Object,"$Id: Object.cpp,v 1.11 2011/02/11 08:51:03 astephen Exp $")
