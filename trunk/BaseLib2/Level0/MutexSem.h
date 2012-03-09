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
 *  @file 
 *  Mutex semafore
 */
#ifndef MUTEX_SEM
#define MUTEX_SEM


#include "SemCore.h"
#include "System.h"
#include "ErrorManagement.h"
#include "TimeoutType.h"

#if (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX)) && (defined USE_PTHREAD)
#include <sys/timeb.h>
/** Auxiliary class needed to implement the MutexSem class under
    Solaris and Linux. */
class PrivateMutexSemStruct{
    /**  Mutex Handle */
    pthread_mutex_t       mutexHandle;
    /** Mutex Attributes */
    pthread_mutexattr_t   mutexAttributes;
public:
    /** */
    PrivateMutexSemStruct(){}
    /** */
    ~PrivateMutexSemStruct(){}

    /** */
    bool Init(){
        if(pthread_mutexattr_init(&mutexAttributes) != 0)                              return False;
        if(pthread_mutexattr_setprotocol(&mutexAttributes, PTHREAD_PRIO_INHERIT) != 0) return False;
        if(pthread_mutexattr_settype(&mutexAttributes,PTHREAD_MUTEX_RECURSIVE)!=0)     return False;
        if(pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0)                       return False;
        return True;
    }

    /** */
    bool Close(){
        if(!pthread_mutexattr_destroy(&mutexAttributes))                          return False;
        if(!pthread_mutex_destroy(&mutexHandle))                                  return False;
        return True;
    }

    /** */
    bool Lock(TimeoutType msecTimeout = TTInfiniteWait){
        if(msecTimeout == TTInfiniteWait){
            if(pthread_mutex_lock(&mutexHandle) != 0)                 return False;
        }else{
#if defined(_SOLARIS) || defined(_MACOSX)
            if(pthread_mutex_lock(&mutexHandle) != 0)                 return False;
#else
            struct timespec timesValues;
            timeb tb;
            ftime( &tb );
            double sec        = ((msecTimeout.msecTimeout + tb.millitm)*1e-3 + tb.time);
            double roundValue = floor(sec);
            timesValues.tv_sec  = (int)roundValue;
            timesValues.tv_nsec = (int)((sec-roundValue)*1E9);
            int err = 0;
            if((err = pthread_mutex_timedlock(&mutexHandle, &timesValues)) != 0){
                return False;
            }
#endif
        } 
        return True;
    }

    /** */
    bool UnLock(){
        return (pthread_mutex_unlock(&mutexHandle)==0);
    }

    /** */
    bool TryLock(){
        return (pthread_mutex_trylock(&mutexHandle)==0);
    }


};

#endif

/** a mutual exclusion semaphore */
class MutexSem: public SemCore {

public:
    /** open the semafore with a given initial state */
    bool Create(bool locked=False){
#if defined(_OS2)
        APIRET ret;
        if (locked == True) ret = DosCreateMutexSem(NULL,(HMTX *)&semH,0,1);
        else                ret = DosCreateMutexSem(NULL,(HMTX *)&semH,0,0);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"MutexSem::Create");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        semH = CreateMutex(NULL,(locked==True),NULL);
        if (semH == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Create");
        }
        return (semH!=NULL);
#elif (defined (_VXWORKS))

        semH = (HANDLE)semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
        if (semH == (HANDLE)0)
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Create");
        else
            if (locked) semTake((SEM_ID)semH,-1);
        return (semH!=(HANDLE)0);
#elif (defined (_RTAI))
        // To implement the Mutex Sem in RTAI
	semH=_rt_typed_named_sem_init(get_rt_free_uid(), (locked == True) ? 0 : 1, RES_SEM);
        if (semH == (HANDLE)0)
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Create");
        else if (locked == True)
            rt_sem_wait((SEM_ID)semH);
        
	return (semH != NULL);
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)

    if(semH != (HANDLE)NULL) delete (PrivateMutexSemStruct *)semH;
    // Create the Structure
    semH = (HANDLE) new PrivateMutexSemStruct;
    if(semH == (HANDLE)NULL) return False;
    // Initialize the Semaphore
    bool ret = ((PrivateMutexSemStruct *)semH)->Init();
    if(!ret){
        delete (PrivateMutexSemStruct *)semH;
        semH = (HANDLE)NULL;
        return False;
    }
    if(locked == True){
        ((PrivateMutexSemStruct *)semH)->Lock(TTInfiniteWait);
    }

#endif
    return True;
#endif
    };

    /** Must specify a name for shared sem! This to support NT :-(. */
    bool CreateShared(bool locked,const char *name){
#if defined(_OS2)
        APIRET ret;
        FString mxName;
        mxName="";
        if (name!=NULL){
            mxName =  "\\SEM32\\";
            mxName += (char *)name;
        }
        semH = (HMTX)0;
        if (locked == True) ret = DosCreateMutexSem((PSZ)mxName.Buffer(),(HMTX *)&semH,DC_SEM_SHARED,1);
        else                ret = DosCreateMutexSem((PSZ)mxName.Buffer(),(HMTX *)&semH,DC_SEM_SHARED,0);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"MutexSem::CreateShared");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        semH = CreateMutex(NULL,(locked==True),name);
        if (semH == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::CreateShared");
        }
        return (semH!=NULL);
#elif (defined (_VXWORKS))
        bool ret = Create(locked);
        if (ret){
            SemNameDataBaseAdd((SEM_ID)semH,(char *)name);
        }
        return ret;
#elif (defined (_RTAI))
        semH = _rt_typed_named_sem_init(nam2num(name), (locked==True) ? 1 : 0, RES_SEM);        
        return (semH != NULL);
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
#if (defined USE_PTHREAD)
    if(!Create(locked))return False;
    SemNameDataBaseAdd(semH,(char *)name);
#endif
    return True;

#endif
    };

    /** Must specify a name for shared sem! This to support NT :-(. */
    bool OpenExisting(const char *name){
#if defined(_OS2)
        if (name == NULL) return False;
        char *fullName = malloc(strlen(name)+10);
        strcpy(fullName,"\\SEM32\\");
        strcat(fullName,name);
        semH = (HMTX)0;
        APIRET ret = DosOpenMutexSem((PSZ)fullName,(HMTX *)&semH);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"MutexSem::OpenExisting");
        free(fullName);
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        semH = OpenMutex(EVENT_ALL_ACCESS,TRUE,name);
        if (semH == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::OpenExisting");
        }
        return (semH!=NULL);
#elif (defined (_VXWORKS))
        SEM_ID id = SemNameDataBaseUseExisting((char *)name);

        if (id == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::OpenExisting: semaphore %s not found",name);
            return False;
        }
        semH = (HANDLE)id;
        return True;
#elif (defined (_RTAI))
        SEM *id = (SEM *)rt_get_adr(nam2num(name));
        if (id == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::OpenExisting: semaphore %s not found",name);
            return False;
        }
        semH = (HANDLE)id;
        return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
#if (defined USE_PTHREAD)

        SEM_ID id = SemNameDataBaseUseExisting((char *)name);
        if (id == (SEM_ID)0){
            CStaticAssertErrorCondition(OSError,"MutexSem::OpenExisting: semaphore %s not found",name);
            return False;
        }
        semH = (HANDLE)id;
#endif
        return True;
#else
//........................................//
    return False;
#endif
    };

    /** close the semafore handle */
    bool Close(){
#if defined(_OS2)
        if (semH==(HANDLE)NULL) return True;
        APIRET ret = DosCloseMutexSem((HMTX )semH);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"MutexSem::Close");
        semH=NULL;
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        if (semH==(HANDLE)NULL) return True;
        if( CloseHandle(semH)==FALSE){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Close");
            return False;
        }
        semH=NULL;
        return True;
#elif (defined (_VXWORKS))
        if (semH==(HANDLE)0) return True;
        if (SemNameDataBaseDelete((SEM_ID)semH)) {
            int ret = semDelete((SEM_ID)semH);
            if (ret != OK){
                CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Close");
                return False;
            }
        }
        semH=0;
        return True;
#elif (defined (_RTAI))        
        if (semH==(HANDLE)NULL) 
            return True;        
        int ret=rt_named_sem_delete((SEM *)semH);
        if (ret != 0) 
        {
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Close");
            return False;
        }
        semH = NULL;
        return True;                       
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
#if (defined USE_PTHREAD)
        if (semH==(HANDLE)NULL) return True;
        if (SemNameDataBaseDelete((SEM_ID)semH)) {
        bool ret = ((PrivateMutexSemStruct *)semH)->Close();
            if (ret != True){
//                CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Close");
                return False;
            }
        delete (PrivateMutexSemStruct *)semH;
        }
        semH=(HANDLE)NULL;
#endif
        return True;
#else
#endif
    };

    /** grab the semafore */
    bool Lock(TimeoutType msecTimeout = TTInfiniteWait){
#if defined(_OS2)
        APIRET ret = DosRequestMutexSem((HMTX )semH,msecTimeout.msecTimeout);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"MutexSem::Lock");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        DWORD ret = WaitForSingleObject(semH,msecTimeout.msecTimeout);
        if (ret == WAIT_FAILED){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::Lock");
            return False;
        }
        if (ret == WAIT_TIMEOUT) return False;
        return True;
#elif (defined (_VXWORKS))
        int ret = semTake((SEM_ID)semH,MsecToTicks(msecTimeout.msecTimeout));
        return (ret==OK);
#elif (defined (_RTAI))
        int ret = 0xFFFF;
	if(semH == NULL)
		return False;
        if (msecTimeout == TTInfiniteWait) {
            ret = rt_sem_wait((SEM *)semH);
        }else if (msecTimeout == TTNoWait) {
            ret = rt_sem_wait_if((SEM *)semH);
	    if (ret == 0) return False;
        }else {
            ret = rt_sem_wait_timed((SEM *)semH, nano2count((RTIME)(msecTimeout.msecTimeout*1000000LL)));
        }

        if ((ret == 0xFFFF)||(ret == 0xFFFE)){
            return False;
        }
        return True;

#elif (defined (_SOLARIS) || defined(_MACOSX))
#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->Lock();
#endif
    return True;

#elif defined (_LINUX)
#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->Lock(msecTimeout);
#endif
    return True;
#else
#endif
    };

    /** returns the ownership */
    bool UnLock(void){
#if defined(_OS2)
        APIRET ret = DosReleaseMutexSem((HMTX )semH);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"MutexSem::UnLock");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        if(ReleaseMutex(semH)==FALSE){
            CStaticAssertPlatformErrorCondition(OSError,"MutexSem::UnLock");
            return False;
        }
        return True;
#elif (defined (_VXWORKS))
        int ret = semGive((SEM_ID)semH);
        return (ret==OK);
#elif (defined (_RTAI))
        if(rt_sem_signal((SEM *) semH) == 0xFFFF){
            return False;
        }
        return True;

#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->UnLock();
#endif
    return True;
#else
#endif
    };

    /** constructor */
    MutexSem(HANDLE h){
        Init(h);
//       printf("MutexSem(HANDLE h)\n");
    }

    /** default constructor */
    MutexSem(){
//        printf("MutexSem()\n");
    }

    /** destructor */
    ~MutexSem(){
//        printf("~MutexSem()\n");
        Close();
    }

    /** locks without wasting time */
    inline bool FastLock(TimeoutType msecTimeout = TTInfiniteWait){
#if defined(_OS2)
        return (DosRequestMutexSem((HMTX )semH,msecTimeout.msecTimeout)==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        int ret = WaitForSingleObject(semH,msecTimeout.msecTimeout);
        return ((ret!=(int)WAIT_FAILED) &&(ret!=(int)WAIT_TIMEOUT));
#elif (defined (_VXWORKS))
        int ret = semTake((SEM_ID)semH,MsecToTicks(msecTimeout.msecTimeout));
        return  (ret == OK);
#elif (defined (_RTAI))
        int ret;
        if (msecTimeout == TTInfiniteWait) {
            ret=rt_sem_wait((SEM *)semH);
        }
        else {
            ret=rt_sem_wait_timed((SEM *)semH, nano2count((RTIME)(msecTimeout.msecTimeout * 1000000LL)));
        }
        if (ret == 0xFFFF)
            return False;
        return True;
#elif defined(_SOLARIS) || defined(_MACOSX)

#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->Lock();
#else
    return True;
#endif

#elif defined (_LINUX)

#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->Lock(msecTimeout);
#else
    return True;
#endif

#else

#endif
    }
    /** unlock semafore fast */
    inline bool FastUnLock(void){
#if defined(_OS2)
        return (DosReleaseMutexSem((HMTX )semH)==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        return (ReleaseMutex(semH)==TRUE);
#elif (defined (_VXWORKS))
        int ret = semGive((SEM_ID)semH);
        return  (ret == OK);
#elif (defined (_RTAI))
        int ret;
        ret=rt_sem_signal((SEM *) semH);
        if (ret == 0xFFFF)
            return False;
        return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->UnLock();
#else
    return True;
#endif

#endif
    }

    /** just try to lock it returning immediately */
    inline bool FastTryLock(){
#if defined(_OS2)
        return (DosRequestMutexSem((HMTX )semH,0)==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        int ret = WaitForSingleObject(semH,0);
        return ((ret!=(int)WAIT_FAILED) &&(ret!=(int)WAIT_TIMEOUT));
#elif (defined (_VXWORKS))
        int ret = semTake((SEM_ID)semH,0);
        return  (ret == OK);
#elif (defined (_RTAI))
        int ret = rt_sem_wait_if((SEM *) semH);
        if (ret==0xFFFF)
            return True;
        return False;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
    if(semH == (HANDLE)NULL)return False;
    return ((PrivateMutexSemStruct *)semH)->TryLock();
#endif
    return True;
#else

#endif
    }

};


#endif
