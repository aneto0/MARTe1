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
 * Information to be stored associated with a specific thread
 */
#ifndef THREAD_INITIALISATION_INTERFACE
#define THREAD_INITIALISATION_INTERFACE

#include "ExceptionHandlerDefinitions.h"
#include "EventSem.h"

class ExceptionHandlerInterface;

#if defined (_CINT)

#define ThreadFunctionType void *

#else

/** The type of a function that can be used for a thread.
 */
typedef void (*ThreadFunctionType)(void *parameters);

#endif

/** This class is an interface used to implement a common thread initialisation procedure.
 *  Despite the name this class is also provides a minimal set of
 *  functionalities. For this reason has not been made pure virtual and can
 *  be istantiated.
 */
class ThreadInitialisationInterface{
protected:
    /** The user thread entry point. */
    ThreadFunctionType  userThreadFunction;

    /** A pointer to a structure containing thread data. */
    void                *userData;

    /** The name of the thread. */
    const char          *name;

    /** enables the operating system to perform some housekeeping 
      * before releasing the thread to the user code
      */
    EventSem            startThreadSynchSem;

public:
    /** the thread number */
    TID                 tid;

    /** operating system thread id */
    TID                 osTid;

    /** */
    ThreadInitialisationInterface(){
        userThreadFunction  = NULL;
        userData            = NULL;
        name                = NULL;
        tid                 = (TID) 0;
        osTid               = (TID) 0;
        startThreadSynchSem.Create();
        startThreadSynchSem.Reset();
    }

    /** */
    ThreadInitialisationInterface(ThreadInitialisationInterface &tii){
        userThreadFunction  = tii.userThreadFunction;
        userData            = tii.userData;
        name                = strdup(tii.name);
        tid                 = tii.tid;
        osTid               = tii.osTid;
    }

    /** */
    void operator=(ThreadInitialisationInterface &tii){
        userThreadFunction  = tii.userThreadFunction;
        userData            = tii.userData;
        name                = strdup(tii.name);
        tid                 = tii.tid;
        osTid               = tii.osTid;
    }

    /** Constructor.
        @param userThreadFunction Actually the thread that has to be executed.
        @param userData A pointer to a structure containing thread data.
        @param name The name of the thread.        */
    ThreadInitialisationInterface(ThreadFunctionType userThreadFunction,void *userData,const char *name){
        this->userThreadFunction = userThreadFunction;
        this->userData           = userData          ;
        if (name != NULL){
            this->name = strdup(name);
        } else {
            this->name = strdup("Unknown");
        }
        tid                 = (TID) 0;
        osTid               = (TID) 0;
        startThreadSynchSem.Create();
        startThreadSynchSem.Reset();
    }

    /** Normal class destructor. It just frees the memory allocated for the name string. */
    virtual ~ThreadInitialisationInterface(){
        free((void *&)name);
        startThreadSynchSem.Close();
    };

    /** The function representing the thread. This is the most basic implementation */
    virtual void UserThreadFunction(){
        if (userThreadFunction != NULL)
            userThreadFunction(userData);
    }

    /** Function to get the name of the thread.
        @return A reference to the dynamically allocated string representing the name of the thread.
        An internal structure that can potentially be referenced after the destruction of the object. */
    virtual const char *GetThreadName(){
        return name;
    }

    /** This function allows to call a subroutine within an exception handler protection.
        This implementation is a dummy one.
    */
    bool ExceptionProtectedExecute(ThreadFunctionType userFunction,void *userData, ExceptionHandlerInterface *eh){
        userFunction(userData);
        return True;
    }

    /**
     * Locks the thread until all OS housekeeping is terminated
     */
    inline void ThreadWait(){
        startThreadSynchSem.Wait();
    }
    
    /**
     * Releases the thread as soon as any OS housekeeping is terminated
     */
    inline void ThreadPost(){
        startThreadSynchSem.Post();
    }
};

/** The type of function that can be used to instantiate a thread initialisation interface object.
    @param userFunction The thread entry point.
    @param userData A pointer to data that can be passed to the thread.
    @param threadName The thread name.
    @param exceptionHandlerBehaviour Describes the behaviour of threads when an exception occurr.
*/
typedef ThreadInitialisationInterface * (*ThreadInitialisationInterfaceConstructorType)(
    ThreadFunctionType          userFunction,
    void                        *userData,
    const char                  *threadName,
    ExceptionHandlerBehaviour   exceptionHandlerBehaviour);

#endif
