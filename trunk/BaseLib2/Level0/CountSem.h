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
 * A counting semaphore implementation (based on EventSem code)
 */

#ifndef COUNT_SEM
#define COUNT_SEM

#include "System.h"
#include "ErrorManagement.h"

#include "SemCore.h"
#include "TimeoutType.h"

//-----------------------------------------------------------------------------
// Helper class for Linux and solaris
//-----------------------------------------------------------------------------
#if (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))  &&  (defined USE_PTHREAD)
class PrivateCountSem{
    // Mutex Handle
    pthread_mutex_t mutexHandle;
    // Mutex Attributes
    pthread_mutexattr_t mutexAttributes;
    // Conditional Variable Handle
    pthread_cond_t condHandle;
    // Conditional Variable Attributes
    pthread_condattr_t condAttributes;

    // counting semaphore
    unsigned int count;
    // waiters number
    unsigned int waiters;

public:
    PrivateCountSem() {
        count   = 0;
        waiters = 0;
    } 

    ~PrivateCountSem() {
    } 

    /**
     * @return the number of available locks
     */
    int32 Count() const { 
        return count;
    } 

    /**
     * @return the number of threads locked in the semaphore
     */
    int32 Waiters() const {
        return waiters;
    }

    /**
     * Initialise the semaphore with a number of available locks
     * @param initial_count initial value of the counting semaphore
     * @return True if correctly initialised
     */
    bool Init(int initial_count = 0) {
        if (pthread_mutexattr_init(&mutexAttributes) != 0){
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Init: pthread_mutexattr_init has failed");
            return False;
        }
        if (pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Init: pthread_mutex_init has failed");
            return False;
        }
        if (pthread_condattr_init(&condAttributes) != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Init: pthread_condattr_init has failed");
            return False;
        }
        if (pthread_cond_init(&condHandle,&condAttributes)!=0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Init: pthread_cond_init has failed");
            return False;
        }

        count   = initial_count;
        waiters = 0;

        return True;
    } 

    /**
     * Destroy the semaphore
     * @return True if the semaphore is successfully destroyed
     */
    bool Destroy() {
        // post until there are waiters on the semaphore (unlock the waiters)
        // note that it is an error to post until count != 0 (post increments count)
        while(waiters){
            Post();
        }

        if(pthread_mutexattr_destroy(&mutexAttributes) != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Destroy: pthread_mutexattr_destroy has failed");	        	
            return False;
        }
        if(pthread_mutex_destroy(&mutexHandle)  != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Destroy: pthread_mutex_destroy has failed");	        	
            return False; 
        }

        if(pthread_condattr_destroy(&condAttributes) != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Destroy: pthread_condattr_destroy has failed");	        	
            return False;
        }
        if(pthread_cond_destroy(&condHandle) != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Destroy: pthread_cond_destroy has failed");	        	
            return False;
        }

        count   = 0;
        waiters = 0;

        return True;
    }

    /**
     * Wait until at least one lock if free
     * @return True if does not exit on timeout or error
     */
    bool Wait(TimeoutType msecTimeout = TTInfiniteWait) {
#ifdef _LINUX
        if(msecTimeout == TTInfiniteWait) {
#endif        	
            // enter mutex
            if(pthread_mutex_lock(&mutexHandle) != 0) {
                CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Wait: pthread_mutex_lock has failed");	  
                return False;
            }
            // increment waiters
            waiters++;
            // wait until the semaphore count is greater than 0
            while(count == 0) {
                if (pthread_cond_wait(&condHandle, &mutexHandle) != 0) {
                    waiters--;
                    pthread_mutex_unlock(&mutexHandle);
                    return False;
                }
            }
            // decrement waiters count
            waiters--;
            // decrement semaphore counter
            count--;
            // exit mutex
            if(pthread_mutex_unlock(&mutexHandle) != 0) {
                CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Wait: pthread_mutex_unlock has failed");	  
                return False;
            }
#ifdef _LINUX
        }
        // timed wait 
        else {
            // absolute timeout
            unsigned long     msec;
            struct   timespec ts;
            struct   timeb    tb;

            // get time in tb
            ftime( &tb );
            // convert it in msec
            msec = (msecTimeout.msecTimeout + tb.millitm);
            // get the absolute timeout
            ts.tv_sec  = tb.time + (msec / 1000);
            ts.tv_nsec = (msec % 1000);

            if(pthread_mutex_timedlock(&mutexHandle, &ts) != 0) {
                return False;
            }
            // increment waiters
            waiters++;
            // wait until the semaphore count is greater then 0
            while ( !(count) ) {
                if (pthread_cond_timedwait(&condHandle, &mutexHandle, &ts) != 0) {
                    waiters--;
                    pthread_mutex_unlock(&mutexHandle);
                    return False;
                }
            }
            // decrement waiters count
            waiters--;
            // decrement semaphore counter
            count--;
            // exit mutex
            if(pthread_mutex_unlock(&mutexHandle) != 0) {
                CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Wait: pthread_mutex_unlock has failed");	  
                return False;
            }
        } 
#endif        
        return True;
    } 

    /**
     * Post the semaphore and increment the number of available locks
     * @return if no error occurred
     */ 
    bool Post() {
        // enter mutex
        if(pthread_mutex_lock(&mutexHandle) != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Post: pthread_mutex_lock has failed");	  
            return False;
        }
        // increment the semaphore counter
        count++;
        // if there are waiter on the semaphore wake up the first
        if(waiters > 0){
            if (pthread_cond_signal(&condHandle) != 0) {
                CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Post: pthread_cond_signal has failed");
                pthread_mutex_unlock(&mutexHandle);
                return False;
            }
        }
        // exit mutex
        if(pthread_mutex_unlock(&mutexHandle) != 0) {
            CStaticAssertPlatformErrorCondition(OSError, "PrivateCountSem::Post: pthread_mutex_unlock has failed");	  
            return False;
        }
        return True;
    } //----------------------------------------------------------------------- Post
};
#endif


class CountSem : public SemCore {
public:
        CountSem(){ 
        }

        CountSem(HANDLE h){ 
            Init(h); 
        }

        ~CountSem(){
            Close(); 
        }

        /** 
         * @return the current semaphore count (available number of locks)
         */
        int Count() const { 
            return ((PrivateCountSem *)semH)->Count(); 
        }

        void operator=(CountSem &s){ 
            *this = s; 
        }

        /** 
         * Creates the semaphore 
         * @param initial_count the initial number of available locks
         * @return True if successful
         */ 
        bool Create(int initial_count = 0) {
#if (defined (_WIN32) || defined(_RSXNT))
            semH = CreateSemaphore(NULL, initial_count, 0xFFFF, NULL);
            if (semH == NULL) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::Create has failed");
            }
            return (semH!=NULL);
#elif (defined (_VXWORKS))
            semH = (HANDLE)semCCreate(SEM_Q_PRIORITY, initial_count);
            if (semH == (HANDLE)0){
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::Create: semCCreate has failed");
            }
            return (semH!=(HANDLE)0);
#elif (defined(_RTAI))
            semH=_rt_typed_named_sem_init(get_rt_free_uid(), initial_count, EVT_SEM);
            if (semH==NULL){
                return False;
            }
            return True;
#elif defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX)

#if (defined USE_PTHREAD)
            if(semH != (HANDLE)NULL) {
                delete (PrivateCountSem *)semH;
            }
            // Create the Structure
#if !defined(_MACOSX)
            semH = (HANDLE)(new PrivateCountSem());
#else
            semH = (uint64)(new PrivateCountSem());
#endif
            if(semH == (HANDLE)NULL) {
                return False;
            }
            // Initialize the Semaphore
            bool ret = ((PrivateCountSem *)semH)->Init(initial_count);
            if(!ret){
                delete (PrivateCountSem *)semH;
                semH = (HANDLE)NULL;
                return False;
            }
#endif
            return True;
#else
            return False;
#endif
        }

        /** 
         * Creates a counting semaphore with a name
         * @param initial_count the initial number of available locks
         * @param name the semaphore name
         * @return True if successful
         */
        bool CreateShared(const char *name, int initial_count = 0) {
#if (defined (_WIN32) || defined(_RSXNT))
            semH = CreateSemaphore(NULL, initial_count, 0xFFFF, name);
            if (semH == NULL) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::CreateShared has failed");
            }
            return (semH!=NULL);
#elif (defined (_VXWORKS))
            bool ret = Create(initial_count);
            if (ret){
                SemNameDataBaseAdd((SEM_ID)semH,(char *)name);
            }
            return ret;
#elif (defined (_RTAI))
            bool ret = Create(initial_count);
            if (ret) {
                SemNameDataBaseAdd((SEM_ID)semH, (char *)name);
            }
            return ret;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
            if(!Create(initial_count)){
                return False;
            }
            SemNameDataBaseAdd(semH,(char *)name);
#endif
            return True;
#else
            return False;
#endif
        }

        /**
         * Open an existing semaphore and access to it using its name
         * @param name the name of the counting semaphore
         * @return True if successful
         */ 
        bool OpenExisting(const char *name) {
#if (defined (_WIN32) || defined(_RSXNT))
            semH = OpenSemaphore(EVENT_ALL_ACCESS,TRUE,name);
            if (semH == NULL) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::OpenExisting: OpenSemaphore has failed");
            }
            return (semH!=NULL);
#elif (defined (_VXWORKS)|| defined(_RTAI))
            SEM_ID id = SemNameDataBaseUseExisting((char *)name);
            if (id == NULL){
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::OpenExisting: semaphore %s not found",name);
                return False;
            }
            semH = (HANDLE)id;
            return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))

#if (defined USE_PTHREAD)
            SEM_ID id = SemNameDataBaseUseExisting((char *)name);
            if (id == (SEM_ID)0){
                CStaticAssertErrorCondition(OSError, "CountSem::OpenExisting: semaphore %s not found",name);
                return False;
            }
            semH = (HANDLE)id;
#endif
            return True;
#else
            return False;
#endif
        }

        /**
         * Closes the semaphore
         * @param return True if successful
         */
        bool Close(void) {
#if (defined (_WIN32) || defined(_RSXNT))
            if(semH == (HANDLE)NULL){
                return True;
            }
            if(CloseHandle(semH)==FALSE) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::Close:CloseHandle has failed ");
                return False;
            }
            semH = NULL;
            return True;
#elif (defined (_VXWORKS))
            if(semH == (HANDLE)0){
                return True;
            }
            if (SemNameDataBaseDelete((SEM_ID)semH)) {
                int ret = semDelete((SEM_ID)semH);
                if (ret != OK){
                    CStaticAssertPlatformErrorCondition(OSError, "CountSem::Close: SemNameDataBaseDelete has failed");
                    return False;
                }
            }
            semH = 0;
            return True;
#elif (defined (_RTAI))
            if(semH == (HANDLE)NULL){
                return True;
            }
            rt_named_sem_delete((SEM_ID) semH);	
            semH=NULL;
            return True;
#elif (defined (_SOLARIS) || defined (_LINUX) || defined(_MACOSX))
            bool ret = True;
#if (defined USE_PTHREAD)        
            if(semH == (HANDLE)NULL)return True;
            if (SemNameDataBaseDelete((SEM_ID)semH)) {
                bool ret = ((PrivateCountSem *)semH)->Destroy();
                delete (PrivateCountSem *)semH;
            }
            semH = (HANDLE)NULL;
#endif
            return ret;
#else
            return False;
#endif
        }

        /**
         * If no more locks (i.e. counts) are available, wait on the semaphore
         * @param msecTimeout maximum timeout time
         * @return True if it does not return from error or timeout
         */
        bool Wait(TimeoutType msecTimeout = TTInfiniteWait) {
#if (defined (_WIN32) || defined(_RSXNT))
            int ret;
            ret = WaitForSingleObject((HEV)semH,msecTimeout.msecTimeout);
            if (ret == (int)WAIT_FAILED) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::Wait: WaitForSingleObject has failed");
                return False;
            }
            if (ret == (int)WAIT_TIMEOUT) return False;
            return True;
#elif (defined (_VXWORKS))
            int ret = semTake((SEM_ID)semH,MsecToTicks(msecTimeout.msecTimeout));
            if (ret == OK) {
                if (semGive((SEM_ID)semH) != OK) {
                    CStaticAssertErrorCondition(OSError, "CountSem::Wait: semGive((SEM_ID)semH) has failed");
                }
                return True;
            }
            if (errnoGet() != 0x3d0004) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::Wait");
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
#elif (defined (_LINUX) || defined (_SOLARIS) || defined(_MACOSX))
#if (defined USE_PTHREAD)
            if(semH == (HANDLE)NULL){
                return False;
            }
            return ((PrivateCountSem *)semH)->Wait(msecTimeout);
#endif
            return True;
#else
            return False;
#endif
        }

        /** 
         * Posts the semaphore and releases one lock
         * @return True if successful
         */
        bool Post(void) {
#if (defined (_WIN32) || defined(_RSXNT))
            if(ReleaseSemaphore((HEV)semH, 1, NULL)==FALSE) {
                CStaticAssertPlatformErrorCondition(OSError, "CountSem::Post");
                return False;
            }
            return True;
#elif (defined (_VXWORKS))
            int ret = semGive((SEM_ID)semH);
            return (ret == OK);
#elif (defined (_RTAI))
            int ret = rt_sem_signal((SEM *) semH);	
            return (ret != 0xFFFF);
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
#if (defined USE_PTHREAD)
            if(semH == (HANDLE)NULL)return False;
            return ((PrivateCountSem *)semH)->Post();
#endif
            return True;
#else
            return False;
#endif
        }
};
#endif

