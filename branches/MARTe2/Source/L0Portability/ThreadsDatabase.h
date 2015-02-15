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
 * Threads database, which can be queried to know the number of threads running in the application
 */
#if !defined (THREADS_DATABASE_H)
#define THREADS_DATABASE_H

#include "ThreadInformation.h"
#include "TimeoutType.h"

extern "C" {

    /** @see ThreadsDatabase::DatabaseNewEntry */
    bool ThreadsDatabaseNewEntry(ThreadInformation *ti);

    /** @see ThreadsDatabase::RemoveEntry */
    ThreadInformation *ThreadsDatabaseRemoveEntry();

    /** @see ThreadsDatabase::GetThreadInformation */
    ThreadInformation *ThreadsDatabaseGetThreadInformation(TID tid);

    /** @see ThreadsDatabase::Lock */
    bool ThreadsDatabaseLock(TimeoutType tt);

    /** @see ThreadsDatabase::UnLock*/
    bool ThreadsDatabaseUnLock();

    /** @see ThreadsDatabase::NumberOfThreads*/
    int32 ThreadsDatabaseNumberOfThreads();

    /** @see ThreadsDatabase::GetThreadId*/
    TID ThreadsDatabaseGetThreadID(int32 n);

    /** @see ThreadsDatabase::GetInfo*/
    bool ThreadsDatabaseGetInfo(ThreadInformation &tiCopy, int32 n, TID tid);

}

class ThreadsDatabase{
private:
    /** Number of ThreadInformation objects that are created each time 
      a request for more space in the entries array is needed*/
    static const int32 GRANULARITY = 64;

    /* Fast ram semafore using Atomic TestAndSet*/
    static int32 atomicSem;

    /* actual number of entries used */
    static int32 nOfEntries;

    /* Max number of entries currently possible */
    static int maxNOfEntries;

    /* vector of ti pointers */
    static ThreadInformation **entries;

    /** Allocate more space in the database*/
    static bool AllocMore();
public:
    friend bool               ThreadsDatabaseNewEntry(ThreadInformation *ti);
    friend ThreadInformation *ThreadsDatabaseRemoveEntry();
    friend ThreadInformation *ThreadsDatabaseGetThreadInformation(TID tid);
    friend bool               ThreadsDatabaseLock(TimeoutType tt);
    friend bool               ThreadsDatabaseUnLock();
    friend int32              ThreadsDatabaseNumberOfThreads();
    friend TID                ThreadsDatabaseGetThreadID(int32 n);
    friend TID                ThreadsDatabaseFind(const char *name);
    friend bool               ThreadsDatabaseGetInfo(ThreadInformation &tiCopy, int32 n, TID tid);

public:
    /** create new TDB entry associated to the ti */
    static bool NewEntry(ThreadInformation *ti){
        return ThreadsDatabaseNewEntry(ti);
    }

    /** destroy TDB entry  */
    static ThreadInformation *RemoveEntry(){
        return ThreadsDatabaseRemoveEntry();
    }

    /** access private thread information
        on timeout returns NULL
        tid = 0 --> current TID */
    static ThreadInformation *GetThreadInformation(TID tid){
        return ThreadsDatabaseGetThreadInformation(tid);
    }

    /** must be locked before accessing TDB information */
    static bool Lock(TimeoutType tt = TTInfiniteWait){
        return ThreadsDatabaseLock(tt);
    }

    /** must be unlocked after accessing TDB information */
    static bool UnLock(){
        return ThreadsDatabaseUnLock();
    }

    /** how many threads are registered
        value meaningful only between Lock/UnLock*/
    static int32 NumberOfThreads(){
        return ThreadsDatabaseNumberOfThreads();
    }

    /** the TID of thread #n
        value meaningful only between Lock/UnLock*/
    static TID GetThreadID(int32 n){
        return ThreadsDatabaseGetThreadID(n);
    }

    /** retrieves information about a thread identified either by name or TID or index
        tid = 0 ==> current tid
        to be called between Lock/UnLock*/
    static bool GetInfo(ThreadInformation &tiCopy, int32 n=-1, TID tid=(TID)-1){
        return ThreadsDatabaseGetInfo(tiCopy, n, tid);
    }
};

#endif

