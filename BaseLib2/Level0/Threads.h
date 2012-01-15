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
 * Multi-thread support
 */
#ifndef THREADS
#define THREADS

#include "System.h"
#include "ErrorManagement.h"
#include "ExceptionHandlerDefinitions.h"
#include "ThreadInitialisationInterface.h"
#include "ProcessorType.h"
#define THREADS_LOCAL
#include "ThreadsDatabase.h"

class ExceptionHandlerInterface;

/** Defines the default stack size for a thread. */
#ifndef _RTAI
#define THREADS_DEFAULT_STACKSIZE 32768
#else
#define THREADS_DEFAULT_STACKSIZE RTAI_THREADS_DEFAULT_STACKSIZE
#endif

//Thread states
//The three main states are: READY, PENDING and SUSPENDED 
//All the other states are substates of these and may or not be available depending on the OS
enum THREAD_STATE{
    THREAD_STATE_UNKNOWN = -1,
    THREAD_STATE_READY   = 1024,
    THREAD_STATE_PEND    = 512,
    THREAD_STATE_SUSP    = 256,
    THREAD_STATE_BLOCKED = 2,
    THREAD_STATE_SEM     = 4,
    THREAD_STATE_DELAY   = 8,
    THREAD_STATE_TOUT    = 16,
    THREAD_STATE_RUN     = 32,
    THREAD_STATE_DEAD    = 64 
};

extern "C"
{
    /** Set this thread priority level 0-31 */
    void ThreadsSetPriorityLevel(uint32 level);

    /** Start a thread */
    TID  ThreadsBeginThread(ThreadFunctionType function,void *parameters = NULL,uint32 stacksize= THREADS_DEFAULT_STACKSIZE,const char *name=NULL,ExceptionHandlerBehaviour exceptionHandlerBehaviour=XH_NotHandled,ProcessorType runOnCPUs=PTUndefinedCPUs);

    /** Retrieve current thread id */
    TID  ThreadsThreadId();

    /** Retrieve current operating system thread id */
    TID  ThreadsThreadOsId();

    /** Kills a thread by id */
    bool ThreadsKill(TID tid);

    /** Checks for thread life */
    bool ThreadsIsAlive(TID tid);

    /** Medium low group of threads priority */
    void ThreadsSetNormalClass();

    /** Highest group of thread priorities */
    void ThreadsSetRealTimeClass();

    /** Lowest group of thread priorities */
    void ThreadsSetIdleClass();

    /** Medium high group of thread priorities */
    void ThreadsSetHighClass();

    /** Returns the a mask with the CPU(s) where the task is running*/
    int32 ThreadsGetCPUs(TID tid);

    /**
     * Returns the task state. This can be a masked combination of any of the
     * defined THREAD_STATE. So for instance a value of "6" means:
     * THREAD_STATE_BLOCKED + THREAD_STATE_SEM
     * @param tid the thread identifier
     * @return the thread state(s)
     */
    uint32 ThreadsGetState(TID tid);

    /** Returns the task priority*/
    int ThreadsGetPriority(TID tid);

    /** Allows to set the thread initialisation method */
    void ThreadsSetInitialisationInterfaceConstructor(ThreadInitialisationInterfaceConstructorType threadInitialisationInterfaceConstructor);

    /** Retrieves the ThreadInitialisationInterface used to start this thread */
    ThreadInitialisationInterface *ThreadsGetInitialisationInterface();

    /** This function allows to call a subroutine within an exception handler protection */
    bool ThreadProtectedExecute(ThreadFunctionType userFunction,void *userData, ExceptionHandlerInterface *eh);

    /** information taken from parsing argv[] option -httpRelay
        used in Level4 BasicHttpService
    */
    const char *GetHttpRelayURL();

    /** information taken from parsing argv[0]
        used in Level0 Processes*/
    const char *GetProcessName();

    /** information taken from parsing argv[] option -loggerServer
        used in LoggeService */
    const char *GetLoggerServerURL();

    /** */
    void SetLoggerServerURL(const char *url);

}

// Forward declaration.
/** This is the default TII object instantiator eventually used in the BeginThread method.
    @param userThreadFunction The thread entry point.
    @param userData A pointer to data that can be passed to the thread.
    @param threadName The thread name.
    @param exceptionHandlerBehaviour Describes the behaviour of threads when an exception occurr.
*/
ThreadInitialisationInterface * DefaultThreadInitialisationInterfaceConstructor(
    ThreadFunctionType          userThreadFunction,
    void                        *userData,
    const char                  *threadName,
    ExceptionHandlerBehaviour   exceptionHandlerBehaviour);

/** Assign default initialisation. */
extern ThreadInitialisationInterfaceConstructorType threadInitialisationInterfaceConstructor;

/**
 * This class provides a common layer among different OS for using threads.
 */
class Threads {
public:

    friend void ThreadsSetInitialisationInterfaceConstructor(ThreadInitialisationInterfaceConstructorType tiic);
    friend void ThreadsSetPriorityLevel(uint32 level);
    friend TID  ThreadsBeginThread(ThreadFunctionType function,void *parameters,uint32 stacksize,const char *name,ExceptionHandlerBehaviour exceptionHandlerBehaviour,ProcessorType runOnCPUs);
    friend TID  ThreadsThreadId();
    friend bool ThreadsKill(TID tid);
    friend bool ThreadsIsAlive(TID tid);
    friend void ThreadsSetNormalClass();
    friend void ThreadsSetRealTimeClass();
    friend void ThreadsSetIdleClass();
    friend void ThreadsSetHighClass();


public:

    /** Sets the function used to build the thread initialisation interface.
        An initialisation interface object is created using either the default value
        or the parameter passed to this function by the BeginThread method.
        @param tiic A pointer to the function to be used in the BeginThread method.
     */
    static void SetThreadInitialisationInterfaceConstructor(ThreadInitialisationInterfaceConstructorType tiic){
        ThreadsSetInitialisationInterfaceConstructor(tiic);
    };

    /** Change thread priority. Applies only to current thread 0-31 (on windows it is actually/4) */
    static void SetPriorityLevel(uint32 level){
        ThreadsSetPriorityLevel(level);
    }

    /** Called implicitly at the end of the main thread function. Calling this leaves some allocated memory unfreed */
    static void EndThread(){
        CStaticAssertErrorCondition(Warning,"thread %s (0x%x) terminating via EndThread. Some memory will not be deallocated",GetName(),ThreadId());
#if (defined(_OS2) || defined(_RSXNT) || defined(_WIN32))
        _endthread();
#elif defined(_VXWORKS)
        exit(0);
#elif defined(_RTAI)
        return (void)0;
#elif (defined(_LINUX) || (defined _SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
        pthread_exit(0);
#endif
    }

    /**
       A call to this function will start a thred.
       @param[in] function The function main for the thread.
       @param[in] parameters A pointer passed to the thread main function.
       @param[in] stacksize The size of the stack.
       @param[in] name The name of the thread.
       @param[in] exceptionAction The action to perform when an exception occurs.
       @return The thread identification number.

       This function will dynamically allocate an object of type
       ThreadInitialisationInterface using the function hook ThreadInitialisationInterfaceConstructor.
       This allows the programmer to choose which constructor has to be used in the case
       a ThreadInitialisationInterface derived class had been used.
    */
    static TID BeginThread(ThreadFunctionType function,void *parameters = NULL,uint32 stacksize=THREADS_DEFAULT_STACKSIZE,const char *name=NULL,ExceptionHandlerBehaviour exceptionHandlerBehaviour=XH_NotHandled,ProcessorType runOnCPUs=PTUndefinedCPUs){
        return ThreadsBeginThread(function,parameters,stacksize,name,exceptionHandlerBehaviour,runOnCPUs);
    }

    /** Normal range of priorities. Sets the base priority to [8 Windows,192 vxWorks] to all threads  (ON Os/2 and vxWorks only on newer ) */
    static void SetNormalClass(){
        ThreadsSetNormalClass();
    }

    /** High range of priorities. Sets the base priority to [24 Windows,64 vxWorks] to all threads (ON Os/2 and vxWorks only on newer ) */
    static void SetRealTimeClass(){
        ThreadsSetRealTimeClass();
    }

    /** Low range of priorities. Sets the base priority to [4 Windows,256 vxWorks] to all threads  (ON Os/2 and vxWorks only on newer ) */
    static void SetIdleClass(){
        ThreadsSetIdleClass();
    }

    /** medium range of priorities, above normal. Sets the base priority to [13 Windows,128 vxWorks] to all threads (ON Os/2 and vxWorks only on newer ) */
    static void SetHighClass(){
        ThreadsSetHighClass();
    }

    /** Gets the current thread id; */
    static TID ThreadId(){
        return ThreadsThreadId();
    }

    /** Asynchronous thread kill */
    static bool Kill(TID tid){
        return ThreadsKill(tid);
    }

    /** Check whether thread still alive */
    static bool IsAlive(TID tid){
        return ThreadsIsAlive(tid);
    }

    /** Retrieve thread name */
    static const char *GetName(){
        return TDB_GetName();
    }

}; // end class Thread

#undef THREADS_LOCAL

#endif


