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
 * Common functionalities shared by all semaphore types
 */
#ifndef _SEM_CORE
#define _SEM_CORE

#include "System.h"
#include "Iterators.h"
#include "Sleep.h"
#include "TimeoutType.h"

/**
   This class provides basic facilities to use semaphores.
*/
class SemCore {
protected:
    /** A number associated to te semaphore. */
    HANDLE semH;
public:
    /** Set the semaphore handle to 0. */
    void Init(){
        semH = (HANDLE)0;
    }
    /** Set the semaphore. */
    void Init(HANDLE s){
        semH = s;
    }
    /** Calls Init(s). */
    SemCore(HANDLE s){
        Init(s);
    }
    /** Calls Init(). */
    SemCore(){
        Init();
    }
    /** */
    void operator=(SemCore &s){
        semH = s.semH;
    }

    /** the operating system handle */
    inline HANDLE Handle(){
        return semH;
    }

#if (defined (_VXWORKS))
    /** Converts a specified request of Timeout in msecs into ticks*/
    inline int MsecToTicks(uint32 ms){
        // Recalculate Timeout as ticks
        if (ms != TTInfiniteWait.msecTimeout)
            return FastFloat2Int((0.001 * ms * GetSleepFrequency())+0.5);
        return ms;
    }

    /** Converts a specified request of Timeout in msecs into ticks*/
    inline int TTToTicks(TimeoutType msecTimeout){
        return MsecToTicks(msecTimeout.msecTimeout);
    }
    #endif

};

#if ((defined (_VXWORKS)) || (defined(_RTAI)))

#else

/** Defines the semaphore ID as an Integer. */
#define SEM_ID int

#endif

extern "C" {
    /** ?? */
    void SemNameDataBaseErase();

    /** ?? */
    void SemNameDataBaseList();

    /** ?? */
    void SemNameDataBaseAdd(SEM_ID id, char *name,void *data=NULL);

    /** ?? */
    SEM_ID SemNameDataBaseUseExisting2(char *name,void *&data);

    /** ?? */
    SEM_ID SemNameDataBaseUseExisting(char *name);

    /** Returns True if the Semaphore was successfully delete or if the
    semaphore was not shared. Note that the shared semaphore can only
    be delete by the last user (Thread...)
    */
    bool SemNameDataBaseDelete(SEM_ID id);

}

#endif

