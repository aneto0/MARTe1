/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or – as soon they 
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
 * $Id: Atomic.h 75 2012-11-07 09:42:41Z aneto $
 *
 **/

/**
 * @file
 * Routines that are performed by the CPU without interruption.
 * These ones are slower than those in SPAtomic.h, but are granted
 * to work on multiprocessor machines.
 */
#ifndef ATOMIC_P_H
#define ATOMIC_P_H

/** A collector of functions that are executed atomically even on multiprocessor machines. */
/** Atomically increment a 32 bit integer in memory. */
static inline void AtomicIncrement32 (volatile int32 *p ){
    __asm  {
        mov   ebx,p
            lock inc DWORD PTR[ebx]
    }
}

/** Atomically increment a 16 bit integer in memory. */
static inline void AtomicIncrement16 (volatile int16 *p){
    __asm  {
        mov   ebx,p
            lock inc WORD PTR[ebx]
    }
}

/** Atomically increment a 8 bit integer in memory. */
static inline void AtomicIncrement8 (volatile int8 *p){
    __asm  {
        mov   ebx,p
            lock inc [ebx]
    }
}

/** Atomically decrement a 32 bit integer in memory. */
static inline void AtomicDecrement32 (volatile int32 *p){
    __asm  {
        mov   ebx,p
            lock dec DWORD PTR[ebx]
    }
}

/** Atomically decrement a 16 bit integer in memory. */
static inline void AtomicDecrement16 (volatile int16 *p){
    __asm  {
        mov   ebx,p
            lock dec WORD PTR[ebx]
    }
}

/** Atomically decrement a 8 bit integer in memory. */
static inline void AtomicDecrement8 (volatile int8 *p){
    __asm  {
        mov   ebx,p
            lock dec [ebx]
    }
}

/** Atomically exchange the contents of a variable with the specified memory location. */
static inline int32 AtomicExchange32 (volatile int32 *p, int32 v){
    __asm  {
        mov   ebx,p
            mov   eax,v
            lock xchg  DWORD PTR [ebx], eax
    }

    /** Test and set a 32 bit memory location in a thread safe way. */
    static inline bool AtomicTestAndSet32(int32 volatile *p){
        int32 temp;
        __asm  {
            mov   ebx,p
                mov   eax,1
                xchg  DWORD PTR [ebx], eax
                mov   temp,eax
        }
    }

    /** Test and set a 16 bit memory location in a thread safe way. */
    static inline bool AtomicTestAndSet16(int16 volatile *p){
        int16 temp;
        __asm  {
            mov   ebx,p
                mov   ax,1
                xchg  WORD PTR [ebx], ax
                mov   temp,ax
        }
    }

    /** Test and set a 8 bit memory location in a thread safe way. */
    static inline bool AtomicTestAndSet8(int8  volatile *p){
        int8 temp;
        __asm  {
            mov   ebx,p
                mov   al,1
                xchg   [ebx], al
                mov   temp,al
        }
    }

    /**
     * Atomic addition
     */
    static inline void AtomicAdd32 (volatile int32 *p, int32 value) {
        __asm  {
            mov   ebx,p
                mov   eax,p
                lock add DWORD PTR[ebx], eax
        }
    }

    /**
     * Atomic subtraction
     */
    static inline void AtomicSub32 (volatile int32 *p, int32 value) {
        __asm  {
            mov   ebx,p
                mov   eax,p
                lock add DWORD PTR[ebx], eax
        }
    }

#endif

