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
 * A semaphore used to synchronise several tasks.
 *
 * After being Reset the semaphore is ready to Wait.
 * Once waiting, until a Post arrives all the tasks will wait on 
 * the semaphore. After the post all tasks are allowed to proceed.
 * A Reset is then required to use the semaphore again.
 */
#ifndef EVENT_SEM
#define EVENT_SEM

#include "System.h"
#include "ErrorManagement.h"
#include "SemCore.h"
#include "TimeoutType.h"


#if (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX)) && (defined USE_PTHREAD)

/** Private structure used for solaris adn linux when using pThread. */
class PrivateEventSemStruct{
    // Mutex Handle
    pthread_mutex_t       mutexHandle;

    // Mutex Attributes
    pthread_mutexattr_t   mutexAttributes;

    // Conditional Variable
    pthread_cond_t        eventVariable;

    // boolean semaphore
    bool                  stop;

public:
    //
    PrivateEventSemStruct(){
	stop = True;
    }
    //
    ~PrivateEventSemStruct(){}

    bool Init(){
	stop = True;
        if (pthread_mutexattr_init(&mutexAttributes) != 0)        return False;
        if (pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0) return False;
        if (pthread_cond_init(&eventVariable,NULL)!=0)            return False;
        return True;
    }

    bool Close(){
	Post();
        if(!pthread_mutexattr_destroy(&mutexAttributes))          return False;
        if(pthread_mutex_destroy(&mutexHandle)  != 0)             return False;
        if(pthread_cond_destroy(&eventVariable) != 0)             return False;
        return True;
    }

    bool Wait(TimeoutType msecTimeout = TTInfiniteWait){
        if(msecTimeout == TTInfiniteWait){
            if(pthread_mutex_lock(&mutexHandle) != 0)                          return False;
	        if(stop == True){
                if(pthread_cond_wait(&eventVariable,&mutexHandle) != 0){
                     pthread_mutex_unlock(&mutexHandle);
                     return False;
                }
	        }
            if(pthread_mutex_unlock(&mutexHandle) != 0)                        return False;

        }else{
#if defined(_SOLARIS) || defined(_MACOSX)
            if(pthread_mutex_lock(&mutexHandle) != 0)                          return False;
	        if(stop == True){
                if(pthread_cond_wait(&eventVariable,&mutexHandle) != 0){
                     pthread_mutex_unlock(&mutexHandle);
                     return False;
                }
	        }
            if(pthread_mutex_unlock(&mutexHandle) != 0)                        return False;

#else

            struct timespec timesValues;
            timeb tb;
            ftime( &tb );
            
            double sec = ((msecTimeout.msecTimeout + tb.millitm)*1e-3 + tb.time);
            
            double roundValue = floor(sec);
            timesValues.tv_sec  = (int)roundValue;
            timesValues.tv_nsec = (int)((sec-roundValue)*1E9);
            if(pthread_mutex_timedlock(&mutexHandle, &timesValues) != 0)                 return False;
	        if(stop == True){
                if(pthread_cond_timedwait(&eventVariable,&mutexHandle,&timesValues) != 0){
                    pthread_mutex_unlock(&mutexHandle);
                    return False;
                }
	        }
            if(pthread_mutex_unlock(&mutexHandle) != 0)                                  return False;
#endif

        } 
        
        return True;
    }

    bool Post(){
	if(pthread_mutex_lock(&mutexHandle) != 0)                                        return False;
        stop = False;
        if(pthread_mutex_unlock(&mutexHandle) != 0)                                      return False;
	return (pthread_cond_broadcast(&eventVariable) == 0);
    }

    bool Reset(){
	if(pthread_mutex_lock(&mutexHandle) != 0)                                        return False;
	stop = True;
        if(pthread_mutex_unlock(&mutexHandle) != 0)                                      return False;
    }

};

#endif

/** Definition of an event shemaphore. */
class EventSem: public SemCore{
public:
    /** Creates the semafore */
    bool Create(){
#if defined (_OS2)
        APIRET ret = DosCreateEventSem(NULL,(HEV *)&semH,0,0);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::Create: DosCreateEventSem has failed");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        semH = CreateEvent(NULL,TRUE,FALSE,NULL);
        if (semH == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Create: CreateEvent has failed");
        }
        return (semH!=NULL);
#elif (defined (_VXWORKS))
        semH = (HANDLE)semCCreate(SEM_Q_PRIORITY,0);
        if (semH == (HANDLE)0){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Create: semCCreate has failed");
        }
        return (semH!=(HANDLE)0);
#elif (defined(_RTAI))
        semH=_rt_typed_named_sem_init(get_rt_free_uid(), 0, EVT_SEM);
        if (semH==NULL) return False;
        return True;
#elif defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX)

#if (defined USE_PTHREAD)
        if(semH != (HANDLE)NULL) delete (PrivateEventSemStruct *)semH;
        // Create the Structure
#if !defined(_MACOSX)
        semH = (HANDLE) new PrivateEventSemStruct;
#else
        semH = (uint64) new PrivateEventSemStruct;
#endif
        if(semH == (HANDLE)NULL) return False;
        // Initialize the Semaphore
        bool ret = ((PrivateEventSemStruct *)semH)->Init();
        if(!ret){
            delete (PrivateEventSemStruct *)semH;
            semH = (HANDLE)NULL;
            return False;
        }
#endif
        return True;
#else
        return False;
#endif
    }

    /** Shared must be named. */
    bool CreateShared(const char *name){
#if defined (_OS2)
        FString hvName;
        hvName="";
        if (name!=NULL){
            hvName =  "\\SEM32\\";
            hvName += name;
        }
        APIRET ret = DosCreateEventSem((PSZ)hvName.Buffer(),(HEV*)&semH,DC_SEM_SHARED,0);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::CreateShared: DosCreateEventSem has failed");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        semH = CreateEvent(NULL,TRUE,FALSE,name);
        if (semH == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::CreateShared: CreateEvent has failed");
        }
        return (semH!=NULL);
#elif (defined (_VXWORKS))
        bool ret = Create();
        if (ret){
            SemNameDataBaseAdd((SEM_ID)semH,(char *)name);
        }
        return ret;
#elif (defined (_RTAI))
        bool ret = Create();
        if (ret) {
            SemNameDataBaseAdd((SEM_ID)semH, (char *)name);
        }
        return ret;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
        if(!Create())return False;
        SemNameDataBaseAdd(semH,(char *)name);
#endif
        return True;
#else
        return False;
#endif
    }

    /** Shared must be named. */
    bool OpenExisting(const char *name){
#if defined (_OS2)
        FString hvName;
        hvName="";
        if (name!=NULL){
            hvName =  "\\SEM32\\";
            hvName += name;
        }
        APIRET ret = DosOpenEventSem((PSZ)hvName.Buffer(),(HEV*)&semH);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::OpenExisting: DosOpenEventSem has failed");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        semH = OpenEvent(EVENT_ALL_ACCESS,TRUE,name);
        if (semH == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::OpenExisting: OpenEvent has failed");
        }
        return (semH!=NULL);
#elif (defined (_VXWORKS)|| defined(_RTAI))
        SEM_ID id = SemNameDataBaseUseExisting((char *)name);

        if (id == NULL){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::OpenExisting: semaphore %s not found",name);
            return False;
        }
        semH = (HANDLE)id;
        return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
        SEM_ID id = SemNameDataBaseUseExisting((char *)name);
        if (id == (SEM_ID)0){
            CStaticAssertErrorCondition(OSError,"EventSem::OpenExisting: semaphore %s not found",name);
            return False;
        }
        semH = (HANDLE)id;
#endif
        return True;
#else
        return False;
#endif
    }

    /** closes the semafore */
    bool Close(void){
#if defined (_OS2)
        if(semH == (HANDLE)NULL)return True;
        APIRET ret = DosCloseEventSem((HEV)semH);
        if (ret != 0) CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::Close: DosCloseEventSem has failed");
        semH = NULL;
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        if(semH == (HANDLE)NULL)return True;
        if( CloseHandle(semH)==FALSE){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Close:CloseHandle has failed ");
            return False;
        }
        semH = NULL;
        return True;
#elif (defined (_VXWORKS))
        if(semH == (HANDLE)0)return True;
        if (SemNameDataBaseDelete((SEM_ID)semH)) {
            int ret = semDelete((SEM_ID)semH);
            if (ret != OK){
                CStaticAssertPlatformErrorCondition(OSError,"EventSem::Close: SemNameDataBaseDelete has failed");
                return False;
            }
        }
        semH = 0;
        return True;
#elif (defined (_RTAI))
        if(semH == (HANDLE)NULL)
            return True;
	rt_named_sem_delete((SEM_ID) semH);	
        semH=NULL;
        return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
        bool ret = True;
#if (defined USE_PTHREAD)        
        if(semH == (HANDLE)NULL)return True;
        if (SemNameDataBaseDelete((SEM_ID)semH)) {
        bool ret = ((PrivateEventSemStruct *)semH)->Close();
            if (ret != True){
//                CStaticAssertPlatformErrorCondition(OSError,"EventSem::Close");
                ret = False;
            }
        delete (PrivateEventSemStruct *)semH;
        }
        semH = (HANDLE)NULL;
#endif
        return ret;
#else
        return False;
#endif
    }

    /** wait for an event */
    bool Wait(TimeoutType msecTimeout = TTInfiniteWait){
#if defined (_OS2)
        APIRET ret = DosWaitEventSem((HEV)semH,msecTimeout.msecTimeout);
        if ((ret != 0) && (ret!=640)) CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::Wait: DosWaitEventSem has failed");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        int ret;
        ret = WaitForSingleObject((HEV)semH,msecTimeout.msecTimeout);
        if (ret == (int)WAIT_FAILED){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Wait: WaitForSingleObject has failed");
            return False;
        }
        if (ret == (int)WAIT_TIMEOUT) return False;
        return True;
#elif (defined (_VXWORKS))

        int ret = semTake((SEM_ID)semH,MsecToTicks(msecTimeout.msecTimeout));
        if (ret == OK) {
            if (semGive((SEM_ID)semH) != OK){
                CStaticAssertErrorCondition(OSError,"EventSem::Wait: semGive((SEM_ID)semH) has failed");
            }
            return True;
        }
        if (errnoGet() != 0x3d0004){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Wait");
            return False;
        }
        return False;
#elif (defined(_RTAI))
        int ret;
        if (msecTimeout == TTInfiniteWait) {
            ret = rt_sem_wait((SEM *)semH);
	}else if (msecTimeout == TTNoWait) {
            ret = rt_sem_wait_if((SEM *)semH);
	    if (ret == 0) return False;
        }else{
            ret = rt_sem_wait_timed((SEM *)semH, (RTIME)(nano2count(msecTimeout.msecTimeout * 1000000LL)));
        }

	if((ret != 0xFFFF)&&(ret != 0xFFFE)){
	    return True;
	}
        return False;
#elif (defined (_SOLARIS))
#if (defined USE_PTHREAD)
        if(semH == (HANDLE)NULL)return False;
        return ((PrivateEventSemStruct *)semH)->Wait();
#endif
        return True;
#elif defined (_LINUX) || defined(_MACOSX)
#if (defined USE_PTHREAD)
        if(semH == (HANDLE)NULL)return False;
        return ((PrivateEventSemStruct *)semH)->Wait(msecTimeout);
#endif
    return True;
#else
        return False;
#endif
    }

    /** resets the semafore and then waits*/
    bool ResetWait(TimeoutType msecTimeout = TTInfiniteWait){
#if defined (_OS2)
        ULONG count;
        APIRET ret = DosResetEventSem((HEV)semH,&count);
        if (ret != 0) CStaticAssertPlatformErrorCondition(ret,"EventSem::ResetWait");
        ret = DosWaitEventSem(semH,msecTimeout);
        if (ret != 0) CStaticAssertPlatformErrorCondition(ret,"EventSem::ResetWait2");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        Reset();
        return Wait(msecTimeout);
#elif (defined (_VXWORKS))
        Reset();
        return Wait(MsecToTicks(msecTimeout.msecTimeout));
#elif (defined (_RTAI))
        Reset();
        return Wait(msecTimeout);
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
#if (defined USE_PTHREAD)
        Reset();
        return Wait(msecTimeout);
#endif
    return True;
#else
        return False;
#endif
    }

    /** Send an event to semafore */
    bool Post(void){
#if defined (_OS2)
        APIRET ret = DosPostEventSem((HEV)semH);
        if ((ret != 0) && (ret != 299))
            CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::Post");
        return (ret==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        if (SetEvent((HEV)semH)==FALSE){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Post");
            return False;
        }
        return True;
#elif (defined (_VXWORKS))
        int ret = semGive((SEM_ID)semH);
//        if (ret != OK) CStaticAssertErrorCondition(errnoGet(),"EventSem::Post(%d)",semH);
        return (ret == OK);
#elif (defined (_RTAI))
	int ret = rt_sem_signal((SEM *) semH);	
        return (ret != 0xFFFF);
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
#if (defined USE_PTHREAD)
        if(semH == (HANDLE)NULL)return False;
        return (((PrivateEventSemStruct *)semH)->Post());
#endif
    return True;
#else

        return False;
#endif
    }

    /** reset the semafore to its unposted state */
    bool Reset(void){
#if defined (_OS2)
        ULONG count;
        APIRET ret = DosResetEventSem((HEV)semH,&count);
        if ((ret != 0)&&(ret!=300)){
            CStaticAssertPlatformErrorCondition(OSError,ret,"EventSem::Reset");
            return False;
        }
        return True;
#elif (defined (_WIN32) || defined(_RSXNT))
        if (ResetEvent((HEV)semH) == FALSE){
            CStaticAssertPlatformErrorCondition(OSError,"EventSem::Reset");
            return False;
        }
        return True;
#elif (defined (_VXWORKS))
        while(semTake((SEM_ID)semH,0)==OK);
    return True;
#elif (defined (_RTAI))
        rt_sem_reset((SEM *) semH);
	return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
#if (defined USE_PTHREAD)
        if(semH == (HANDLE)NULL)return False;
        return ((PrivateEventSemStruct *)semH)->Reset();
#endif
    return True;
#endif
    }

    /** */
    EventSem(HANDLE h){
        Init(h);
    }

    /** copies semaphore and special infos as well. */
    void operator=(EventSem &s){
        *this = s;
    }

    /** */
    EventSem(){
    }

    /** */
    ~EventSem(){
        Close();
    }

    /** Just wait without wasting too much time */
    inline bool fastWait(TimeoutType msecTimeout = TTInfiniteWait){
#if defined(_OS2)
        APIRET ret = DosWaitEventSem((HEV)semH,msecTimeout.msecTimeout);
        return ((ret == 0) || (ret!=640));
#elif (defined (_WIN32) || defined(_RSXNT))
        int ret = WaitForSingleObject(semH,msecTimeout.msecTimeout);
        return ((ret!=(int)WAIT_FAILED) &&(ret!=(int)WAIT_TIMEOUT));
#elif (defined (_VXWORKS))
        int ret = semTake((SEM_ID)semH,MsecToTicks(msecTimeout.msecTimeout));
        if (ret==OK) {
            semGive((SEM_ID)semH);
            return True;
        }
        return (errnoGet() == 0x3d0004);
#elif (defined(_RTAI))
        return Wait(msecTimeout);
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
        return Wait(msecTimeout);
#else

#endif
    }

    /** post semafore without wasting time */
    inline bool fastPost(void){
#if defined(_OS2)
        return (DosPostEventSem((HEV)semH)==0);
#elif (defined (_WIN32) || defined(_RSXNT))
        return (SetEvent(semH) == TRUE);
#elif (defined (_VXWORKS))
        int ret = semGive((SEM_ID)semH);
        return (ret == OK);
#elif (defined (_RTAI))
        return rt_sem_signal((SEM *) semH);
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
        return Post();
#endif
    }

    /** reset the semafore without wasting time  */
    inline bool fastReset(void){
#if defined(_OS2)
        ULONG count;
        APIRET ret = DosResetEventSem((HEV)semH,&count);
        return ((ret == 0)||(ret!=300));
#elif (defined (_WIN32) || defined(_RSXNT))
        return (ResetEvent(semH) == TRUE);
#elif (defined (_VXWORKS))
        while(semTake((SEM_ID)semH,0)==OK);
        return True;
#elif (defined (_RTAI))
        return Reset();
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
        return Reset();
#endif
    }
};

#endif

