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
 * $Id$
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

#include "GenDefs.h"
#include "Sleep.h"

/** A collector of functions that are executed atomically even on multiprocessor machines. */
class Atomic{

#if defined _SOLARIS
// Temporary solution until atomic full implementation on Solaris
private:

    /** The state of a semaphore. */
    static int32 privateSem;

    /** Lock function implemented with a semaphore. */
    static inline bool PrivateLock(){
        while(!Atomic::TestAndSet((int32 *)&privateSem)){
            SleepMsec(1);
        }
        return True;
    }

    /** Unlock function implemented with a semaphore. */
    static inline bool PrivateUnLock(){
        privateSem = 0;
        return True;
    }
#endif

public:
    /** Atomically increment a 32 bit integer in memory. */
    static inline void Increment (volatile int32 *p ){
#if defined(_CINT)
#elif defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            lock inc DWORD PTR[ebx]
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile register int32 MSR,temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lwzx %1,0,%2\n"            // Store *p in a register, otherwise it doesn't work
            "addi %1,%1,1\n"            // Increment
            "stwx %1,0,%2\n"            // Store the result in *p
            "mtmsr %0"                  // Update the MSR
            :"=r" (MSR),"=r"(temp), "=r" (p) : "0" (MSR),"1"(temp), "2" (p)
        );

#elif defined(_VX68K)
        asm volatile(
            "ADDQL #1,(%0)\n"
            : : "d" (p)
        );

#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock incl (%0)\n"
            : : "r" (p)
            );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p + 1;
    Atomic::PrivateUnLock();
#else
#endif
    }

    /** Atomically increment a 16 bit integer in memory. */
    static inline void Increment (volatile int16 *p){
#if defined(_CINT)
#elif defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            lock inc WORD PTR[ebx]
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile register int32 MSR, temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lhzx %1,0,%2\n"            // Store *p in a register, otherwise it doesn't work
            "addi  %1,%1,1\n"           // Increment
            "sthx %1,0,%2\n"            // Store the result in *p
            "mtmsr %0"                  // Update the MSR
            :"=r" (MSR),"=r"(temp), "=r" (p) : "0" (MSR),"1"(temp), "2" (p)

        );
#elif defined(_VX68K)
        asm volatile(
            "ADDQW #1,(%0)\n"
            : : "d" (p)
        );
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock incw (%0)\n"
            : : "r" (p)
            );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p + 1;
    Atomic::PrivateUnLock();
#else
#endif
    }

    /** Atomically increment a 8 bit integer in memory. */
    static inline void Increment (volatile int8 *p){
#if defined(_CINT)
#elif defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            lock inc [ebx]
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile register int32 MSR, temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lbzx %1,0,%2\n"            // Store *p in a register, otherwise it doesn't work
            "addi  %1,%1,1\n"           // Increment
            "stbx %1,0,%2\n"            // Store the result in *p
            "mtmsr %0"                  // Update the MSR
            :"=r" (MSR),"=r"(temp), "=r" (p) : "0" (MSR),"1"(temp), "2" (p)
        );
#elif defined(_VX68K)
        asm volatile(
            "ADDQB #1,(%0)\n"
            : : "d" (p)
        );
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock incb (%0)\n"
            : : "r" (p)
            );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p + 1;
    Atomic::PrivateUnLock();
#else
#endif
    }

    /** Atomically decrement a 32 bit integer in memory. */
    static inline void Decrement (volatile int32 *p){
#if defined(_CINT)
#elif defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            lock dec DWORD PTR[ebx]
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile register int32 MSR, temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lwzx %1,0,%2\n"            // Store *p in a register, otherwise it doesn't work
            "subi  %1,%1,1\n"           // Increment
            "stwx %1,0,%2\n"            // Store the result in *p
            "mtmsr %0"                  // Update the MSR
            :"=r" (MSR),"=r"(temp), "=r" (p) : "0" (MSR),"1"(temp), "2" (p)
        );
#elif defined(_VX68K)
        asm volatile(
            "SUBQL #1,(%0)\n"
            : : "d" (p)
        );
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock decl (%0)\n"
            : : "r" (p)
            );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p - 1;
    Atomic::PrivateUnLock();
#else
#endif
    }

    /** Atomically decrement a 16 bit integer in memory. */
    static inline void Decrement (volatile int16 *p){
#if defined(_CINT)
#elif defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            lock dec WORD PTR[ebx]
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile register int32 MSR, temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lhzx %1,0,%2\n"            // Store *p in a register, otherwise it doesn't work
            "subi  %1,%1,1\n"           // Increment
            "sthx %1,0,%2\n"            // Store the result in *p
            "mtmsr %0"                  // Update the MSR
            :"=r" (MSR),"=r"(temp), "=r" (p) : "0" (MSR),"1"(temp), "2" (p)
        );
#elif defined(_VX68K)
        asm volatile(
            "SUBQW #1,(%0)\n"
            : : "d" (p)
        );
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock decw (%0)\n"
            : : "r" (p)
            );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p - 1;
    Atomic::PrivateUnLock();
#else
#endif
    }

    /** Atomically decrement a 8 bit integer in memory. */
    static inline void Decrement (volatile int8 *p){
#if defined(_CINT)
#elif defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            lock dec [ebx]
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile register int32 MSR, temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lbzx %1,0,%2\n"            // Store *p in a register, otherwise it doesn't work
            "subi  %1,%1,1\n"           // Increment
            "stbx %1,0,%2\n"            // Store the result in *p
            "mtmsr %0"                  // Update the MSR
            :"=r" (MSR),"=r"(temp), "=r" (p) : "0" (MSR),"1"(temp), "2" (p)
        );
#elif defined(_VX68K)
        asm volatile(
            "SUBQB #1,(%0)\n"
            : : "d" (p)
        );
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock decb (%0)\n"
            : : "r" (p)
            );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p - 1;
    Atomic::PrivateUnLock();
#else
#endif
    }

    /** Atomically exchange the contents of a variable with the specified memory location. */
    static inline int32 Exchange (volatile int32 *p, int32 v){
#if defined(_MSC_VER)
        __asm  {
            mov   ebx,p
            mov   eax,v
            lock xchg  DWORD PTR [ebx], eax
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)


        volatile register int32 MSR,temp;
        asm volatile(
            "mfmsr %0\n"                // Gets the MSR
            "rlwinm %1,%0,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %1\n"                // Update the MSR
            "lwarx %1,0,%3\n"           // Lock the resource
            "stwcx. %2,0,%3\n"          // Release the resource
            "bne- $-8\n"                // Repeat until atomic operation successful.
            "mr %2,%1\n"                // Swap
            "mtmsr %0"                  // Update the MSR
            : "=r" (MSR),"+r" (temp),"=r" (v) :"r" (p) , "2" (v)
        );
        return v;
#elif defined(_VX68K)
        int ret = *p;
        asm volatile(
            "MOVEL %1,(%0)"
            : "=d" (p) : "d" (v)
        );
        return ret;
#elif defined(_RTAI)
        volatile int ret = *p;
        asm volatile(
            "lock xchg (%0), %1"
            : : "r" (p), "r" (v)
            );
        return ret;
#elif (defined(_LINUX) || defined(_MACOSX))
        asm volatile(
            "lock xchg (%1), %0"
            :"=r" (v) : "r" (p), "0" (v)
        );
        return v;
#endif
    }

    /** Test and set a 32 bit memory location in a thread safe way. */
    static inline bool TestAndSet(int32 volatile *p){
#if defined(_CINT)
        return 0;
#elif defined(_MSC_VER)
        int32 temp;
        __asm  {
            mov   ebx,p
            mov   eax,1
            xchg  DWORD PTR [ebx], eax
            mov   temp,eax
        }
        return (temp == 0);

#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int32 MSR,out,temp;
        asm volatile(
            "mfmsr %1\n"                // Gets the MSR
            "rlwinm %2,%1,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %2\n"                // Update the MSR
            "lwzx %0,0,%3\n"
            "li %2,1\n"
            "stwx %2,0,%3\n"
            "mtmsr %1"
            :"=r" (out),"=r" (MSR),"=r"(temp),"=r" (p) :"0" (out), "1" (MSR),"2"(temp), "3" (p)

        );
        return (out==0);
#elif (defined(_VX68K))
    volatile int8 out;
    asm volatile(
        "tas (%1)\n"
        "seq %0\n"
        :"=d" (out) : "g" (p)
        );

    return(out!=0);
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    register int32 out=1;
    asm volatile (
        "lock xchg (%2),%1"
        : "=r" (out) : "0" (out), "r" (p)
        );
    return (out==0);
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    register int32 temp = 0;
    __asm__("ldstub [%1], %0"
        :  "=&r"(temp)
        :  "r"(p));

    return (temp==0);
#else
#endif
    }

    /** Test and set a 16 bit memory location in a thread safe way. */
    static inline bool TestAndSet(int16 volatile *p){
#if defined(_CINT)
        return 0;
#elif defined(_MSC_VER)
        int16 temp;
        __asm  {
            mov   ebx,p
            mov   ax,1
            xchg  WORD PTR [ebx], ax
            mov   temp,ax
        }
        return (temp == 0);

#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int32 MSR;
        volatile int16 out,temp;
        asm volatile(
            "mfmsr %1\n"                // Gets the MSR
            "rlwinm %2,%1,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %2\n"                // Update the MSR
            "lhzx %0,0,%3\n"
            "li %2,1\n"
            "sthx %2,0,%3\n"
            "mtmsr %1"
            :"=r" (out),"=r" (MSR),"=r"(temp),"=r" (p) :"0" (out), "1" (MSR),"2"(temp), "3" (p)
        );
        return (out==0);
#elif defined(_VX68K)
    volatile int8 out;
    asm volatile(
        "tas (%1)\n"
        "seq %0"
        :"=d" (out) :"g" (p)
    );
    return(out!=0);
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    register int16 out=1;
    asm volatile (
        "lock xchgw (%2),%1"
        : "=r" (out) : "0" (out), "r" (p)
        );
    return (out==0);
#elif defined(_SOLARIS)
    register int16 temp = 0;
    __asm__("ldstub [%1], %0"
        :  "=&r"(temp)
        :  "r"(p));

    return (temp==0);
#else
#endif
    }

    /** Test and set a 8 bit memory location in a thread safe way. */
    static inline bool TestAndSet(int8  volatile *p){
#if defined(_CINT)
        return 0;
#elif defined(_MSC_VER)
        int8 temp;
        __asm  {
            mov   ebx,p
            mov   al,1
            xchg   [ebx], al
            mov   temp,al
        }
        return (temp == 0);

#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int32 MSR;
        volatile int8 out,temp;
        asm volatile(
            "mfmsr %1\n"                // Gets the MSR
            "rlwinm %2,%1,0,17,15\n"    // Reset the EE flag to disable the interrupts
            "mtmsr %2\n"                // Update the MSR
            "lbzx %0,0,%3\n"
            "li %2,1\n"
            "stbx %2,0,%3\n"
            "mtmsr %1"
            :"=r" (out),"=r" (MSR),"=r"(temp),"=r" (p) :"0" (out), "1" (MSR),"2"(temp), "3" (p)
        );
        return (out==0);
#elif defined(_VX68K)
    volatile int8 out;
    asm volatile(
        "tas (%1)\n"
        "seq %0"
        :"=d" (out)  :"g" (p)
    );
    return(out!=0);
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    register int8 out=1;
    asm volatile (
        "lock xchgb (%2),%1"
        : "=q" (out) : "0" (out), "q" (p)
        );
    return (out==0);
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    register int8 temp = 0;
    __asm__("ldstub [%1], %0"
        :  "=&r"(temp)
        :  "r"(p));

    return (temp==0);
#else
#endif
    }

    /**
     * Atomic addition
     */
    static inline void Add (volatile int32 *p, int32 value) {
#if (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile (
                "lock addl %1, (%0)"
                : /* output */
                :"r" (p), "ir" (value) /* input */
        );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p + value;
    Atomic::PrivateUnLock();
#else
    #error not available in this O.S. Contributions are welcome
#endif
    }

    /**
     * Atomic subtraction
     */
    static inline void Sub (volatile int32 *p, int32 value) {
#if (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        asm volatile (
                "lock subl %1, (%0)"
                : /* output */
                :"r" (p), "ir" (value) /* input */
        );
#elif defined(_SOLARIS)
// This is not appropriate .... but works for the moment...
    Atomic::PrivateLock();
    *p = *p - value;
    Atomic::PrivateUnLock();
#else
    #error not available in this O.S. Contributions are welcome
#endif
    }

};

#endif

