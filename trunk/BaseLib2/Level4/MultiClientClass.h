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
 * Keeps information about the connection medium. Needs be derived
 * into a specialized class to cater for the actual needs of a client server app
 */
#if !defined (MULTI_CLIENT_CLASS)
#define MULTI_CLIENT_CLASS

#include "GenDefs.h"
#include "Threads.h"
#include "MutexSem.h"
#include "Object.h"

enum MCCExitType{
    MCCTerminate=0,
    MCCContinue=1
};

class MultiClientClass;


class MultiClientClassConnectionInfo{
public:
    /** reference to the main class */
    MultiClientClass *base;

    /** destructor must be virtual */
    virtual ~MultiClientClassConnectionInfo(){
        base = NULL;
    }
};

extern "C" {
    /** */
    void MCCService (void *arg);

    /** */
    void MCCListener(void *arg);

    /** */
    void MCCListenerThread(MultiClientClass &mcc);

    /** */
    bool MCCStart(MultiClientClass &mcc);

    /** */
    bool MCCStop(MultiClientClass &mcc);

    /** */
    void MCCInit(MultiClientClass &mcc);
};

//OBJECT_DLL(MultiClientClass)

/** a class to build multi client servers */
class MultiClientClass/*:virtual public Object*/ {
//OBJECT_DLL_STUFF(MultiClientClass)
    friend void  __thread_decl MCCService (void *info);
    friend void  __thread_decl MCCListener(void *info);
    friend void MCCListenerThread(MultiClientClass &mcc);
    friend bool MCCStart(MultiClientClass &mcc);
    friend bool MCCStop(MultiClientClass &mcc);
    friend void MCCInit(MultiClientClass &mcc);

    /** */
    virtual bool UserListenerInitialize(){
        return False;
    }

    /** */
    virtual void UserListenerTerminate(){
    }

    /** called upon connection by ListenerThread */
    virtual bool UserListenerThread(MultiClientClassConnectionInfo *&info){
        return False;
    }

    /** called by the service thread after having been spawned */
    virtual void UserServiceThread(MultiClientClassConnectionInfo *info){
    }

    /** called by the service thread after having been spawned. Extended version*/
    virtual MCCExitType UserServiceThreadEx(MultiClientClassConnectionInfo *info){
        UserServiceThread(info);
        return MCCTerminate;
    }

    /**
     * The listener name
     */
    virtual const char *ListenerName(){
        return "MCCLISTENER";
    }

    /**
     * The service name
     */
    virtual const char *ServiceName(){
        return "MCCSERVICE";
    }

protected:
    /** stack size for listener thread */
    uint32 listenerStackSize;

    /** stack size for service thread */
    uint32 serviceStackSize;

    /** while the listening activity should continue */
    bool shouldContinue;

    /** */
    uint32 activeThreads;

    /** */
    MutexSem mux;

    /** the code executed by thread doing the listening */
    void ListenerThread(){
        MCCListenerThread(*this);
    }

public:
    /** */
    bool Start(){
        return MCCStart(*this);
    }

    /** */
    bool Stop(){
        return MCCStop(*this);
    }

    /** */
    MultiClientClass(){
        MCCInit(*this);
    }

    /** */
    virtual ~MultiClientClass(){
        Stop();
    }
};



#endif
