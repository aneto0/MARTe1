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
#ifndef ATOMIC_H
#define ATOMIC_H

#include "GeneralDefinitions.h"
#include INCLUDE_FILE_ARCHITECTURE(ARCHITECTURE,AtomicP.h)

/** A collector of functions that are executed atomically even on multiprocessor machines. */
class Atomic{

public:
    /** Atomically increment a 32 bit integer in memory. */
    static inline void Increment (volatile int32 *p ){
        AtomicIncrement32(p);
    }

    /** Atomically increment a 16 bit integer in memory. */
    static inline void Increment (volatile int16 *p){
        AtomicIncrement16(p);
    }

    /** Atomically increment a 8 bit integer in memory. */
    static inline void Increment (volatile int8 *p){
        AtomicIncrement8(p);
    }

    /** Atomically decrement a 32 bit integer in memory. */
    static inline void Decrement (volatile int32 *p){
        AtomicDecrement32(p);
    }

    /** Atomically decrement a 16 bit integer in memory. */
    static inline void Decrement (volatile int16 *p){
        AtomicDecrement16(p);
    }

    /** Atomically decrement a 8 bit integer in memory. */
    static inline void Decrement (volatile int8 *p){
        AtomicDecrement8(p);
    }

    /** Atomically exchange the contents of a variable with the specified memory location. */
    static inline int32 Exchange (volatile int32 *p, int32 v){
        AtomicExchange32(p, v);
    }

    /** Test and set a 32 bit memory location in a thread safe way. */
    static inline bool TestAndSet(int32 volatile *p){
        AtomicTestAndSet32(p);
    }

    /** Test and set a 16 bit memory location in a thread safe way. */
    static inline bool TestAndSet(int16 volatile *p){
        AtomicTestAndSet16(p);
    }

    /** Test and set a 8 bit memory location in a thread safe way. */
    static inline bool TestAndSet(int8  volatile *p){
        AtomicTestAndSet8(p);
    }

    /**
     * Atomic addition
     */
    static inline void Add (volatile int32 *p, int32 value) {
        AtomicAdd32(p, value);
    }

    /**
     * Atomic subtraction
     */
    static inline void Sub (volatile int32 *p, int32 value) {
        AtomicSub32(p, value);
    }

};

#endif

