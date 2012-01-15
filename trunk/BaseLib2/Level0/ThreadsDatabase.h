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
 * Threads database, which can be queried to know the number of threads running in the application
 */
#if !defined (THREADS_DB)
#define THREADS_DB

#include "ThreadInitialisationInterface.h"
#include "TimeoutType.h"

extern "C" {

// functions only accessible by Threads.cpp
#if defined (THREADS_LOCAL)

    /** create new TDB entry associated to the tii */
    bool                                TDB_NewEntry(ThreadInitialisationInterface *tii);

    /** destroy TDB entry  */
    ThreadInitialisationInterface *     TDB_RemoveEntry();

    /** access private thread information
        on timeout returns NULL
        tid = 0 --> current TID */
    ThreadInitialisationInterface *     TDB_GetTII(TID tid=0);

#endif

// global functions

    /** must be locked before accessing TDB information */
    bool                                TDB_Lock(TimeoutType tt = TTInfiniteWait);

    /** must be unlocked after accessing TDB information */
    bool                                TDB_UnLock();

    /** how many threads are registered
        value meaningful only between Lock/UnLock*/
    int                                 TDB_NumberOfThreads();

    /** the TID of thread #n
        value meaningful only between Lock/UnLock*/
    TID                                 TDB_GetThreadID(int n);

    /** the TID of thread named name
        value meaningful only between Lock/UnLock*/
    TID                                 TDB_Find(const char *name);

    /** retrieves information about a thread identified either by name or TID or index
        tid = 0 ==> current tid
        to be called between Lock/UnLock*/
    bool                                TDB_GetInfo(ThreadInitialisationInterface &tiiCopy,int n=-1,const char *name=NULL,TID tid=(TID)-1);

    /** retrieve pointer to name in tii for current thread */
    const char *                        TDB_GetName();
}

#endif

