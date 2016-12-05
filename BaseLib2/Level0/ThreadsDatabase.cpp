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

#define THREADS_LOCAL

#include "System.h"
#include "Threads.h"
#include "ThreadsDatabase.h"
#include "Atomic.h"
#include "HRT.h"
#include "TimeoutType.h"
#include "ErrorManagement.h"

static const int                        TDB_Granularity     = 64;

/* fast ram semafore using TestAnsSet*/
static int32                            TDB_Sem             = 0;

/* actual number of entries used */
static int                              TDB_NOfEntries      = 0;

/* max number of entries currently possible */
static int                              TDB_MaxNOfEntries   = 0;

/* vector of tii pointers */
static ThreadInitialisationInterface **  TDB_entries        = NULL;

#if defined (_WIN32)

// a large value to force error from TlsGetValue when not allocated!
static int                              TDB_TLSInited       = 0;

//
static uint32                           ThreadIPTLS         = 0;

static void                             CheckTLS()
{
    if (TDB_TLSInited == 0){
        ThreadIPTLS = TlsAlloc();
        TDB_TLSInited = 1;
    }
}

#endif


/* alloc more space for TDB adding TDB_Granularity entries */
static bool                             TDB_AllocMore(){
    // no need
    if (TDB_MaxNOfEntries > TDB_NOfEntries) return True;

    // first time?
    if (TDB_entries == NULL){
        TDB_entries = (ThreadInitialisationInterface **)malloc(sizeof(ThreadInitialisationInterface *) * TDB_Granularity);
        if (TDB_entries != NULL){
            TDB_MaxNOfEntries = TDB_Granularity;
            TDB_NOfEntries = 0;
        } else {
            CStaticAssertErrorCondition(FatalError,"TDB:TDB_AllocMore failed allocating %i entries",TDB_Granularity);
            return False;
        }
    } else {
        TDB_entries = (ThreadInitialisationInterface **)realloc((void *&)TDB_entries,sizeof(ThreadInitialisationInterface *) * (TDB_Granularity+TDB_MaxNOfEntries));
        if (TDB_entries != NULL){
            TDB_MaxNOfEntries += TDB_Granularity;
        } else {
            CStaticAssertErrorCondition(FatalError,"TDB:TDB_AllocMore failed re-allocating to %i entries",TDB_Granularity+TDB_MaxNOfEntries);
            return False;
        }
    }

    // clean new memory
    int i ;
    for (i= (TDB_MaxNOfEntries - TDB_Granularity) ; i<TDB_MaxNOfEntries ;i++){
        TDB_entries[i] = NULL;
    }
    return True;
}

/** create new TDB entry associated to the tii
    tid = 0 --> current TID */
bool                                    TDB_NewEntry(ThreadInitialisationInterface *tii)
{
    if (!TDB_AllocMore()){
        CStaticAssertErrorCondition(FatalError,"TDB:TDB_NewEntry failed (re-)allocating memory");
        return False;
    }

    // no space
    if (TDB_MaxNOfEntries <= TDB_NOfEntries) {
        CStaticAssertErrorCondition(FatalError,"TDB:TDB_NewEntry no space for new entry");
        return False;
    }

    // search for empty space staring from guess
    int index = TDB_NOfEntries+1;
    if (index >= TDB_MaxNOfEntries) index -= TDB_MaxNOfEntries;
    while (index != TDB_NOfEntries){
        if (TDB_entries[index] == NULL){
            TDB_entries[index] = tii;
            tii->tid   = ThreadsThreadId();
            tii->osTid = ThreadsThreadOsId();
            TDB_NOfEntries ++;

#if defined (_WIN32)
            CheckTLS();
            // store as TLS as well
            TlsSetValue(ThreadIPTLS,tii);
#endif
            return True;
        }
        index++;
        // roll-over
        if (index >= TDB_MaxNOfEntries) index -= TDB_MaxNOfEntries;
    }

    CStaticAssertErrorCondition(FatalError,"TDB:TDB_NewEntry could not find empty slot!!");
    return False;

}

/** destroy TDB entry  */
ThreadInitialisationInterface *          TDB_RemoveEntry(){
    TID tid = ThreadsThreadId();

    // search for empty space staring from guess
    int index = 0;
    while (index < TDB_MaxNOfEntries){
        ThreadInitialisationInterface *tii = TDB_entries[index];
        if (tii != NULL){
            if (tii->tid == tid){
                TDB_entries[index]= NULL;

#if defined (_WIN32)
                CheckTLS();
                // store as TLS as well
                TlsSetValue(ThreadIPTLS,NULL);
#endif
                TDB_NOfEntries--;

                // free at the end
                if (TDB_NOfEntries == 0){
                    free ((void *&)TDB_entries);
                    TDB_entries = NULL;
                    TDB_MaxNOfEntries = 0;
                }
                return tii;
            }
        }
        index++;
    }

    CStaticAssertErrorCondition(FatalError,"TDB:TDB_RemoveEntry could not find/remove entry TID=%08x ",tid);
    return NULL;

}

/** access private thread information
    on timeout returns NULL
    tid = 0 --> current TID */
ThreadInitialisationInterface *          TDB_GetTII(TID tid){

    // search for empty space staring from guess
    int index = 0;
    while (index < TDB_MaxNOfEntries){
        ThreadInitialisationInterface *tii = TDB_entries[index];
        if (tii != NULL){
            if (tii->tid == tid){
                return tii;
            }
        }
        index++;
    }

    CStaticAssertErrorCondition(FatalError,"TDB:TDB_GetTII could not find entry TID=%08x ",tid);
    return NULL;
}

// global functions
/** must be locked before accessing TDB information */
bool                                    TDB_Lock(TimeoutType tt)
{
    int64 ticksStop = tt.HRTTicks();
    ticksStop += HRTRead64();
    while (!Atomic::TestAndSet(&TDB_Sem)) {
        if (tt != TTInfiniteWait){
            int64 ticks = HRTRead64();
            if (ticks > ticksStop)  return False;
        }
        // yield CPU
        SleepMsec(1);
    }
    return True;

}

/** must be unlocked after accessing TDB information */
bool                                    TDB_UnLock()
{
    TDB_Sem = 0;
    return True;
}

/** how many threads are registered
    value meaningful only between Lock/UnLock*/
int                                     TDB_NumberOfThreads()
{
    return TDB_NOfEntries;
}

/** the TID of thread #n
    value meaningful only between Lock/UnLock*/
TID                                     TDB_GetThreadID(int n)
{

    if ((n<0) ||(n >= TDB_NOfEntries)){
        CStaticAssertErrorCondition(FatalError,"TDB:TDB_GetThreadID(%i) index out of range",n);
        return 0;
    }

    // search for empty space staring from guess
    int index = 0;
    while (index < TDB_MaxNOfEntries){
        if (TDB_entries[index] != NULL){
            if (n == 0){
                return TDB_entries[index]->tid;
            }
            n--;
        }
        index++;
    }

    CStaticAssertErrorCondition(FatalError,"TDB:TDB_GetThreadID(%i) mismatch between actual entries and TDB_NOfEntries");
    return 0;
}

/** the TID of thread named name
    value meaningful only between Lock/UnLock*/
TID                                 TDB_Find(const char *name){

    if ((name == NULL) || (name[0] == 0)){
        CStaticAssertErrorCondition(FatalError,"TDB:TDB_GetThreadID(0x%08x) name is NULL or empty ",name);
        return 0;
    }

    // search for empty space staring from guess
    int index = 0;
    while (index < TDB_MaxNOfEntries){
        if (TDB_entries[index] != NULL){
            if (strcmp(TDB_entries[index]->GetThreadName(),name) == 0){
                return TDB_entries[index]->tid;
            }
        }
        index++;
    }

    CStaticAssertErrorCondition(FatalError,"TDB:TDB_GetThreadID(%i) mismatch between actual entries and TDB_NOfEntries");
    return 0;

}

/** retrieves information about a thread identified either by name or TID or index
    to be called between Lock/UnLock*/
bool                                TDB_GetInfo(ThreadInitialisationInterface &tiiCopy,int n,const char *name,TID tid)
{
    if (n>=0){
        TID tid = TDB_GetThreadID(n);
        ThreadInitialisationInterface *tii = TDB_GetTII(tid);
        if (tii == NULL) return False;
        tiiCopy = *tii;
        return True;
    } else
    if (name != NULL){
        TID tid = TDB_Find(name);
        ThreadInitialisationInterface *tii = TDB_GetTII(tid);
        if (tii == NULL) return False;
        tiiCopy = *tii;
        return True;
    } else
    if (tid != (TID)-1)
    {
        if (tid == 0) tid = ThreadsThreadId();
        ThreadInitialisationInterface *tii = TDB_GetTII(tid);
        if (tii == NULL) return False;
        tiiCopy = *tii;
        return True;
    }  else
    {
        CStaticAssertErrorCondition(FatalError,"TDB:TDB_GetThreadID(%i) mismatch between actual entries and TDB_NOfEntries ");
    }
    return False;
}


const char *                        TDB_GetName()
{
#if defined _VXWORKS
        return(taskName(taskIdSelf()));
#elif defined _WIN32
    int32 tmp = (int32)TlsGetValue(ThreadIPTLS);
    ThreadInitialisationInterface *tii = (ThreadInitialisationInterface *)tmp;
    if (tii == NULL) return "TII_NULL";
    return tii->GetThreadName();
#elif (((defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX)) && defined (USE_PTHREAD)) || defined(_RTAI))
    TID tid = ThreadsThreadId();
    ThreadInitialisationInterface *tii = TDB_GetTII(tid);
    if (tii == NULL) return "TII_NULL";
    return tii->GetThreadName();
#else
    return "TII_NULL";
#endif
}


