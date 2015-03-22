/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
 permissions and limitations under the Licence. 
 *
 * $Id: $
 *
 **/

/**
 * @file
 * @brief Multi-thread support
 *
 * Framework threading definition.
 */
#ifndef THREADS_H
#define THREADS_H

#include "GeneralDefinitions.h"
#include "ThreadInformation.h"
#include "ExceptionHandler.h"
#include "ProcessorType.h"

/**
 * Defines the default stack size for a thread.
 */
#ifndef THREADS_DEFAULT_STACKSIZE
#define THREADS_DEFAULT_STACKSIZE 32768
#endif

/**
 * Assign default initialisation. When a new thread is created, before calling the user callback entry function
 * this function is called. This allows to keep track of how many threads are running in the application at any
 * given time and to do house keeping (see also ThreadsDatabase).
 */
extern ThreadInformationConstructorType threadInitialisationInterfaceConstructor;

extern "C" {
/** @brief See Threads::BeginThread */
TID ThreadsBeginThread(ThreadFunctionType function,
                       void *parameters = NULL,
                       uint32 stacksize = THREADS_DEFAULT_STACKSIZE,
                       const char *name = NULL,
                       uint32 exceptionHandlerBehaviour = ExceptionHandler::NotHandled,
                       ProcessorType runOnCPUs = PTUndefinedCPUs);

/** @brief See Threads::EndThread */
void ThreadsEndThread();

/** @brief See Threads::Id */
TID ThreadsId();

/** @brief See Threads::Kill */
bool ThreadsKill(TID tid);

/** @brief See Threads::IsAlive */
bool ThreadsIsAlive(TID tid);

/** @see Threads::Name*/
const char *ThreadsName(TID tid);

/** @see Threads::GetCPUs*/
int32 ThreadsGetCPUs(TID tid);

/** @see Threads::GetState*/
uint32 ThreadsGetState(TID tid);

/** @see Threads::GetPriorityLevel*/
uint32 ThreadsGetPriorityLevel(TID tid);

/** @see Threads::GetPriorityClass*/
uint32 ThreadsGetPriorityClass(TID tid);

/** @see Threads::SetPriorityLevel*/
void ThreadsSetPriorityLevel(TID tid, uint32 level);

/** @see Threads::SetPriorityClass*/
void ThreadsSetPriorityClass(TID tid, uint32 priotityClass);

/** Allows to set the thread initialisation method */
void ThreadsSetInitialisationInterfaceConstructor(ThreadInformationConstructorType threadInitialisationInterfaceConstructor);

/** This function allows to call a subroutine within an exception handler protection */
bool ThreadProtectedExecute(ThreadFunctionType userFunction, void *userData, ExceptionHandler *eh);
}

// Forward declaration.
/** This is the default TII object instantiator eventually used in the BeginThread method.
 @param userThreadFunction The thread entry point.
 @param userData A pointer to data that can be passed to the thread.
 @param threadName The thread name.
 @param exceptionHandlerBehaviour Describes the behaviour of threads when an exception occurr.
 */
ThreadInformation * DefaultThreadInformationConstructor(ThreadFunctionType userThreadFunction, void *userData, const char *threadName,
                                                        uint32 exceptionHandlerBehaviour);

/**
 * This class provides a common layer among different OS for using threads.
 */
class Threads {
public:

    friend void ThreadsSetInitialisationInterfaceConstructor(ThreadInformationConstructorType tiic);
    friend void ThreadsSetPriorityLevel(TID tid, uint32 level);
    friend void ThreadsSetPriorityClass(TID tid, uint32 priotityClass);
    friend TID ThreadsBeginThread(ThreadFunctionType function, void *parameters, uint32 stacksize, const char *name, uint32 exceptionHandlerBehaviour,
                                  ProcessorType runOnCPUs);
    friend TID ThreadsId();
    friend void ThreadsEndThread();
    friend bool ThreadsKill(TID tid);
    friend bool ThreadsIsAlive(TID tid);
    friend void ThreadsSetNormalClass();
    friend void ThreadsSetRealTimeClass();
    friend void ThreadsSetIdleClass();
    friend void ThreadsSetHighClass();

public:
//Thread states
//The three main states are: READY, PENDING and SUSPENDED 
//All the other states are substates of these and may or not be available depending on the OS

    static const uint32 STATE_UNKNOWN = -1;
    static const uint32 STATE_READY = 1024;
    static const uint32 STATE_PEND = 512;
    static const uint32 STATE_SUSP = 256;
    static const uint32 STATE_BLOCKED = 2;
    static const uint32 STATE_SEM = 4;
    static const uint32 STATE_DELAY = 8;
    static const uint32 STATE_TOUT = 16;
    static const uint32 STATE_RUN = 32;
    static const uint32 STATE_DEAD = 64;

    /**
     * List of possible priority classes in ascending order of priority
     */
    static const uint32 PRIORITY_CLASS_UNKNOWN = 0;
    static const uint32 PRIORITY_CLASS_IDLE = 1;
    static const uint32 PRIORITY_CLASS_NORMAL = 2;
    static const uint32 PRIORITY_CLASS_HIGH = 3;
    static const uint32 PRIORITY_CLASS_REAL_TIME = 4;

    /**
     * List of possible priorities. Each of the above classes can contain any of the 
     * these subpriorities. e.g. REAL_TIME_PRIORITY_CLASS with PRIORITY_LOWEST has
     * less priority then REAL_TIME_PRIORITY_CLASS with PRIORITY_BELOW_NORMAL but 
     * has more priority than HIGH_PRIORITY_CLASS with PRIORITY_TIME_CRITICAL
     */
    static const uint32 PRIORITY_UNKNOWN = 0;
    static const uint32 PRIORITY_IDLE = 1;
    static const uint32 PRIORITY_LOWEST = 2;
    static const uint32 PRIORITY_BELOW_NORMAL = 3;
    static const uint32 PRIORITY_NORMAL = 4;
    static const uint32 PRIORITY_ABOVE_NORMAL = 5;
    static const uint32 PRIORITY_HIGHEST = 6;
    static const uint32 PRIORITY_TIME_CRITICAL = 7;

    /** Sets the function used to build the thread initialisation interface.
     An initialisation interface object is created using either the default value
     or the parameter passed to this function by the BeginThread method.
     @param tiic A pointer to the function to be used in the BeginThread method.
     */
    static void SetThreadInformationConstructor(ThreadInformationConstructorType tiic) {
        ThreadsSetInitialisationInterfaceConstructor(tiic);
    }

    /** Change thread priority. Applies only to current thread 0-31 (on windows it is actually/4) */
    static void SetPriorityLevel(TID tid, uint32 level) {
        ThreadsSetPriorityLevel(tid, level);
    }

    /** Change thread priority class. */
    static void SetPriorityClass(TID tid, uint32 priorityClass) {
        ThreadsSetPriorityClass(tid, priorityClass);
    }

    /** Called implicitly at the end of the main thread function. Calling this leaves some allocated memory unfreed */
    static void EndThread() {
        ThreadsEndThread();
    }

    /**
     * @brief Starts a new thread.
     *
     * This will start a new thread and callback the function set by the user.
     * This function will dynamically allocate an object of type ThreadInformation using the function hook
     * ThreadInformationConstructor. If the DefaultThreadInformationConstructor is used the thread will automatically
     * be registered in the ThreadDatabase and the user function called afterwards.
     *
     * @param function The function main for the thread.
     * @param parameters A pointer passed to the thread main function.
     * @param stacksize The size of the stack.
     * @param name The name of the thread.
     * @param exceptionAction The action to perform when an exception occurs.
     * @return The thread identification number.
     */
    static TID BeginThread(ThreadFunctionType function,
                           void *parameters = NULL,
                           uint32 stacksize = THREADS_DEFAULT_STACKSIZE,
                           const char *name = NULL,
                           uint32 exceptionHandlerBehaviour = ExceptionHandler::NotHandled,
                           ProcessorType runOnCPUs = PTUndefinedCPUs) {
        return ThreadsBeginThread(function, parameters, stacksize, name, exceptionHandlerBehaviour, runOnCPUs);
    }

    /** Gets the current thread id; */
    static TID Id() {
        return ThreadsId();
    }

    /** Asynchronous thread kill */
    static bool Kill(TID tid) {
        return ThreadsKill(tid);
    }

    /** Check whether thread still alive */
    static bool IsAlive(TID tid) {
        return ThreadsIsAlive(tid);
    }

    /** Retrieve thread name */
    static const char *Name(TID tid) {
        return ThreadsName(tid);
    }

    /**
     * Returns the task state. This can be a masked combination of any of the
     * defined THREAD_STATE. So for instance a value of "6" means:
     * THREAD_STATE_BLOCKED + THREAD_STATE_SEM
     * @param tid the thread identifier
     * @return the thread state(s)
     */
    static uint32 GetState(TID tid) {
        return ThreadsGetState(tid);
    }

    /** @return the thread priority level*/
    static int32 GetPriorityLevel(TID tid) {
        return ThreadsGetPriorityLevel(tid);
    }

    /** @return the thread priority class*/
    static int32 GetPriorityClass(TID tid) {
        return ThreadsGetPriorityClass(tid);
    }

};
// end class Thread

#endif

