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

#include "MultiClientClass.h"

void MCCListenerThread(MultiClientClass &mcc){
    if (mcc.UserListenerInitialize() == False) return;
    MultiClientClassConnectionInfo *info = NULL;
    while(mcc.shouldContinue){
        info = NULL;
        if (mcc.UserListenerThread(info)){
            if (info != NULL){
                info->base = &mcc;
                Threads::BeginThread(MCCService,(void *)info,mcc.serviceStackSize,mcc.ServiceName());
            }
        } else SleepMsec(100);
    }
    mcc.UserListenerTerminate();
}


bool MCCStart(MultiClientClass &mcc){
    if (mcc.mux.Lock()==False){
        CStaticAssertErrorCondition(FatalError,"MultiClientClass::Start::Cannot Lock mux");
        return False;
    }
    if (mcc.activeThreads!=0){
        CStaticAssertErrorCondition(IllegalOperation,"MultiClientClass::Start::Already running threads");
        return False;
    }
    mcc.mux.UnLock();
    mcc.shouldContinue = True;
    Threads::BeginThread(MCCListener,(void *)&mcc,mcc.listenerStackSize,mcc.ListenerName());
    return True;
}

bool MCCStop(MultiClientClass &mcc){
    mcc.shouldContinue = False;
    while(True){
        SleepMsec(100);
        if (mcc.mux.Lock()==False){
            CStaticAssertErrorCondition(FatalError,"MultiClientClass::Stop::Cannot Lock mux");
            return False;
        }
        if (mcc.activeThreads==0){
            mcc.mux.UnLock();
            return True;
        }
        mcc.mux.UnLock();
    }
}

void MCCInit(MultiClientClass &mcc){
    mcc.mux.Create();
    mcc.shouldContinue = False;
    mcc.activeThreads = 0;
    mcc.listenerStackSize = THREADS_DEFAULT_STACKSIZE;
    mcc.serviceStackSize = THREADS_DEFAULT_STACKSIZE;
}


//OBJECTREGISTER(MultiClientClass,"$Id$")

void MCCListener(void *arg){
    MultiClientClass *info = (MultiClientClass *)arg;
    if (info->mux.Lock()){
        info->activeThreads++;
        info->mux.UnLock();
        info->ListenerThread();
        info->mux.Lock();
        info->activeThreads--;
        info->mux.UnLock();
    } else {
        CStaticAssertErrorCondition(FatalError,"Thread MCCListener: cannot Lock mux");
    }
}

void MCCService (void *arg){
    MCCExitType                     exit     = MCCContinue;
    MultiClientClassConnectionInfo *info     = (MultiClientClassConnectionInfo *)arg;
    int32                           muxRetry = 0;
    while((exit == MCCContinue) && (info->base->shouldContinue == True)){
       if (info->base->mux.Lock()){
            muxRetry = 0;
            info->base->activeThreads++;
            info->base->mux.UnLock();
            exit = info->base->UserServiceThreadEx(info);        
            info->base->mux.Lock();
            info->base->activeThreads--;
            info->base->mux.UnLock();
        }
        else {
            CStaticAssertErrorCondition(FatalError,"Thread MCCService: cannot Lock mux");
            muxRetry++;
            if(muxRetry == 3){
                CStaticAssertErrorCondition(FatalError,"Thread MCCService: cannot Lock mux. Tried %d times. Breaking...", muxRetry);
                break;
            }
        }
    }     
    delete info;
}


