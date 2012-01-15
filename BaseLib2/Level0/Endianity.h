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
 * A NameSpace holder of all the Endianity conversion routines. 
 */
#ifndef ENDIANITY_H
#define ENDIANITY_H

#include "System.h"

class Endianity {
public:

    /** 
     * Swaps the 4 bytes in a 32 bit number.  
     * @param x the number to be swapped
     */
    static inline void Swap32(volatile void *x){
#if defined(_MSC_VER)
        __asm  {
            mov   ebx,x
                mov   eax,DWORD PTR [ebx]
                bswap eax
                mov   DWORD PTR [ebx],eax
                }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int32 *xx = (int32 *)x;
        asm volatile(
            "lwbrx %0,0,%1" : "=r" (*xx): "r" (x)
            );
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        int32 *xx = (int32 *)x;
        register int32 temp=*xx;
        asm(
            "bswap %1"
            : "=r" (temp) : "0" (temp)
            );
        *xx=temp;
#elif defined(_VX68K)
	volatile int32 temp;
	asm(
	    "move    d1, %1 \n"
	    "move.b  (%0), d1 \n"
	    "move.b  3(%0), (%0) \n"
	    "move.b  d1, 3(%0) \n"
	    "move.b  1(%0), d1 \n"
	    "move.b  2(%0), 1(%0) \n"
	    "move.b  d1, 2(%0) \n"
	    "move    %1, d1 \n"
	    : : "a" (x), "r" (temp)
	    );
#else
        unsigned char *p  = (unsigned char *)x;
        unsigned char temp;
        temp = p[0];
        p[0] = p[3];
        p[3] = temp;
        temp = p[1];
        p[1] = p[2];
        p[2] = temp;
#endif
    }

    /** 
     * Swaps the 4 bytes in a 32 bit number for all the elements
     * of a vector
     * @param x the number to be swapped
     * @param sizer the number of elements in the vector
     */
    static inline void Swap32(volatile void *x,uint32 sizer) {
#if defined(_MSC_VER)
        __asm  {
            mov   ecx,sizer
                test  ecx,ecx
                je    SHORT $swap_32_mul_2
                mov   ebx,x
                $swap_32_mul_1:
                mov   eax,DWORD PTR [ebx]
                bswap eax
                mov   DWORD PTR [ebx],eax
                add   ebx,4
                dec   ecx
                jne   SHORT $swap_32_mul_1
                $swap_32_mul_2:
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int32 *xx = (int32 *)x;
        for(uint32 i=0;i<sizer;i++){
            asm volatile(
                "lwbrx %0,0,%1" :"=r" (*xx): "r" (xx)
                );
            xx++;
        }
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_MACOSX))
        register int32 *xx = (int32 *)x;
        for (uint32 i=0; i<sizer; i++) {
            register int32 temp=*xx;
            asm(
                "bswap %1"
                : "=r" (temp) : "0" (temp)
                );
            *xx=temp;
            xx++;
        }
#elif defined(_VX68K)
	volatile int temp = 0;
	volatile int *pointer = (int *)x;
	for(int i = 0; i < sizer; i++){
	    asm(
		"move    d1, %1 \n"
		"move.b  (%0), d1 \n"
		"move.b  3(%0), (%0) \n"
		"move.b  d1, 3(%0) \n"
		"move.b  1(%0), d1 \n"
		"move.b  2(%0), 1(%0) \n"
		"move.b  d1, 2(%0) \n"
		"move    %1, d1 \n"
		: :"a" (pointer), "r" (temp)
		);
	    pointer++;
	}
#else
        unsigned char *p  = (unsigned char *)(x);
        for(int i=0;i<sizer;i++){
            unsigned char temp;
            temp = p[0];
            p[0] = p[3];
            p[3] = temp;
            temp = p[1];
            p[1] = p[2];
            p[2] = temp;
            p+=4;
        }
#endif
    }

    /** 
     * Swaps the 4 bytes while copying a vector of 32 bit numbers
     * @param dest the destination vector (must be allocated in memory)
     * @param src the source vector 
     * @param sizer the number of elements in the vector
     */
    static inline void MemCopySwap32(volatile void *dest,volatile const void *src,uint32 sizer){
#if defined(_MSC_VER)
        __asm  {
            mov   ecx,sizer
                test  ecx,ecx
                je    SHORT $swap_32_copy_mul_2
                mov   ebx,src
                mov   edx,dest
                $swap_32_copy_mul_1:
                mov   eax,DWORD PTR [ebx]
                add   ebx,4
                bswap eax
                mov   DWORD PTR [edx],eax
                add   edx,4
                dec   ecx
                jne   SHORT $swap_32_copy_mul_1
                $swap_32_copy_mul_2:
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int32 *s = (int32 *)src;
        volatile int32 *d = (int32 *)dest;
        for(uint32 i=0;i<sizer;i++){
            asm volatile(
                "lwbrx %0,0,%1" : "=r" (*d): "r" (s)
                );
            s++;
            d++;
        }
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_MACOSX))
        register int32 *s = (int32 *)src;
        register int32 *d = (int32 *)dest;
        for (uint32 i=0; i<sizer; i++) {
            register int32 temp=*s;
            asm(
                "bswap %1"
                : "=r" (temp) : "0" (temp)
                );
            *d=temp;
            d++;
            s++;
        }
#elif defined(_VX68K)
	volatile int temp;
	volatile int *destin = (int *)dest;
	volatile int *source = (int *)src;
	for(int i = 0; i < sizer; i++){
	    asm(
		"move    d1, %2 \n"
		"move.b  (%1), 3(%0)  \n"
		"move.b  1(%1), 2(%0) \n"
		"move.b  2(%1), 1(%0) \n"
		"move.b  3(%1), (%0)  \n"
		"move    %2, d1 \n"
		: : "a" (destin) , "a" (source) , "r" (temp)
		);
	    destin++;
	    source++;
	}
#else
        unsigned char *s  = (unsigned char *)(src);
        unsigned char *d  = (unsigned char *)(dest);
        for(int i=0;i<sizer;i++){
            unsigned char temp;
            d[3] = s[0];
            d[2] = s[1];
            d[1] = s[2];
            d[0] = s[3];
            d+=4;
            s+=4;
        }
#endif
    }

    /** 
     * Swaps the 2 bytes in a 16 bit number.  
     * @param x the number to be swapped
     */
    static inline void Swap16(volatile void *x){
#if defined(_MSC_VER)
        __asm  {
            mov   ebx,x
                mov   cx,WORD PTR [ebx]
                xchg  cl,  ch
                mov   WORD PTR [ebx], cx
                }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int16 *s = (int16 *)x;
        asm volatile(
            "lhbrx %0,0,%1": "=r" (*s): "r" (s)
            );
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_MACOSX))
        asm(
            "movw (%0), %%dx\n"
            "xchgb %%dl, %%dh\n"
            "movw %%dx, (%0)"
            : : "r" (x) :"dx"
            );

#elif defined(_VX68K)
	volatile int32 temp;
	asm(
	    "move    d1, %1 \n"
	    "move.b  (%0), d1 \n"
	    "move.b  1(%0), (%0) \n"
	    "move.b  d1, 1(%0) \n"
	    "move    %1, d1 \n"
	    : : "a" (x), "r" (temp)
	    );
#else
        unsigned char *s  = (unsigned char *)(x);
        unsigned char temp;
        temp = s[0];
        s[0] = s[1];
        s[1] = temp;
#endif
    }

    /** 
     * Swaps the 2 bytes in a 16 bit number for all the elements
     * of a vector
     * @param x the number to be swapped
     * @param sizer the number of elements in the vector
     */
    static inline void Swap16(volatile void *x,uint32 sizer){
#if defined(_MSC_VER)
        __asm  {
            mov   ecx,sizer
                test  ecx,ecx
                je    SHORT $swap_16_mul_2
                mov   ebx,x
                $swap_16_mul_1:
                mov   ax,WORD PTR [ebx]
                xchg  al,  ah
                mov   WORD PTR [ebx],ax
                add   ebx,2
                dec   ecx
                jne   SHORT $swap_16_mul_1
                $swap_16_mul_2:
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int16 *xx = (int16 *)x;
        for(uint32 i=0;i<sizer;i++){
            asm volatile(
                "lhbrx %0,0,%1" :"=r" (*xx): "r" (xx)
                );
            xx++;
        }
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_MACOSX))
        register int16 *xx = (int16 *)x;
        for (uint32 i=0; i<sizer; i++) {
            asm(
                "movw (%0), %%dx\n"
                "xchgb %%dl, %%dh\n"
                "movw %%dx, (%0)"
                : : "r" (xx) :"dx"
                );
            xx++;
        }
#elif defined(_VX68K)
	volatile int temp = 0;
	volatile int16 *pointer = (int16 *)x;
	for(int i = 0; i < sizer; i++){
	    asm(
		"move    d1, %1 \n"
		"move.b  (%0), d1 \n"
		"move.b  1(%0), (%0) \n"
		"move.b  d1, 1(%0) \n"
		"move    %1, d1 \n"
		: :"a" (pointer), "r" (temp)
		);
	    pointer++;
	}
#else
        unsigned char *p  = (unsigned char *)(x);
        for(int i=0;i<sizer;i++){
            unsigned char temp;
            temp = p[0];
            p[0] = p[1];
            p[1] = temp;
            p+=2;
        }
#endif
    }

    /** 
     * Swaps the 2 bytes while copying a vector of 16 bit numbers
     * @param dest the destination vector (must be allocated in memory)
     * @param src the source vector 
     * @param sizer the number of elements in the vector
     */
    static inline void MemCopySwap16(volatile void *dest,volatile const void *src,uint32 sizer){
#if defined(_MSC_VER)
        __asm  {
            mov   ecx,sizer
                test  ecx,ecx
                je    SHORT $swap_16_copy_mul_2
                mov   ebx,src
                mov   edx,dest
                $swap_16_copy_mul_1:
                mov   ax,WORD PTR [ebx]
                add   ebx,2
                xchg  al,  ah
                mov   WORD PTR [edx],ax
                add   edx,2
                dec   ecx
                jne   SHORT $swap_16_copy_mul_1
                $swap_16_copy_mul_2:
        }
#elif defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

        volatile int16 *s = (int16 *)src;
        volatile int16 *d = (int16 *)dest;
        for(uint32 i=0;i<sizer;i++){
            asm volatile(
                "lhbrx %0,0,%1" :"=r" (*d): "r" (s)
                );
            s++;
            d++;
        }
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_MACOSX))
        int16 *s = (int16 *)src;
        int16 *d = (int16 *)dest;
        for(uint32 i=0;i<sizer;i++){
            asm(
                "movw (%0), %%dx\n"
                "xchgb %%dl, %%dh\n"
                "movw %%dx, (%1)"
                : : "r" (s), "r" (d) :"dx"
                );
            s++;
            d++;
        }


#elif defined(_VX68K)
	volatile int temp;
	volatile int16 *destin = (int16 *)dest;
	volatile int16 *source = (int16 *)src;
	for(uint32 i = 0; i < sizer; i++){
	    asm(
		"move    d1, %2 \n"
		"move.b  (%1), 1(%0)  \n"
		"move.b  1(%1), 0(%0) \n"
		"move    %2, d1 \n"
		: : "a" (destin) , "a" (source) , "r" (temp)
		);
	    destin++;
	    source++;
	}
#else

        unsigned char *s  = (unsigned char *)(src);
        unsigned char *d  = (unsigned char *)(dest);
        for(int i=0;i<sizer;i++){
            d[1] = s[0];
            d[0] = s[1];
            d+=2;
            s+=2;
        }
#endif
    }

    /** 
     * Swaps the 8 bytes in a 64 bit number.  
     * @param x the number to be swapped
     */
    static inline void Swap64(volatile void *x){
        uint32 *p = (uint32 *)x;
        Swap32(&p[0]);
        uint32 temp = p[0];
        Swap32(&p[1]);
        p[0] = p[1];
        p[1] = temp;
    }

#if defined(INTEL_BYTE_ORDER)
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile double &x){ Swap64(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile float &x) { Swap32(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile uint64 &x){ Swap64(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile uint32 &x){ Swap32(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile uint16 &x){ Swap16(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile int64 &x) { Swap64(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile int32 &x) { Swap32(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile int16 &x) { Swap16(&x); }
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile double &x){ x = x; }
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile float &x) { x = x; }
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile uint64 &x){ x = x;};
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile uint32 &x){ x = x;};
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile uint16 &x){ x = x;};
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile int64 &x) { x = x;};
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile int32 &x) { x = x;};
    /**
     * NOOP since the system is already little endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile int16 &x) { x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile double &x){ Swap64(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile float &x) { Swap32(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile uint64 &x){ Swap64(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile uint32 &x){ Swap32(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile uint16 &x){ Swap16(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile int64 &x) { Swap64(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile int32 &x) { Swap32(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile int16 &x) { Swap16(&x); }
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile double &x){ x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile float &x) { x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile uint64 &x){ x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile uint32 &x){ x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile uint16 &x){ x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile int64 &x) { x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile int32 &x) { x = x;};
    /** 
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile int16 &x) { x = x;};
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(float *dest,const float *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(uint32 *dest,const uint32 *src,uint32 size) { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(uint16 *dest,const uint16 *src,uint32 size) { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(int32 *dest,const int32 *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(int16 *dest,const int16 *src,uint32 size)   { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(float *dest,const float *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(uint32 *dest,const uint32 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(uint16 *dest,const uint16 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(int32 *dest,const int32 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(int16 *dest,const int16 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(float *dest,const float *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(uint32 *dest,const uint32 *src,uint32 size) { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(uint16 *dest,const uint16 *src,uint32 size) { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(int32 *dest,const int32 *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(int16 *dest,const int16 *src,uint32 size)   { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(float *dest,const float *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(uint32 *dest,const uint32 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(uint16 *dest,const uint16 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(int32 *dest,const int32 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(int16 *dest,const int16 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
#else
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile double &x){ Swap64(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile float &x) { Swap32(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile uint64 &x){ Swap64(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile uint32 &x){ Swap32(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile uint16 &x){ Swap16(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile int64 &x) { Swap64(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile int32 &x) { Swap32(&x); }
    /**
     * Converts a number from little endian to big endian
     * @param x the number to convert
     */
    static inline void FromIntel(volatile int16 &x) { Swap16(&x); }
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile double &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile float &x) { x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile uint64 &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile uint32 &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile uint16 &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile int64 &x) { x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile int32 &x) { x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void FromMotorola(volatile int16 &x) { x = x;};
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile double &x){ Swap64(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile float  &x){ Swap32(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile uint64 &x){ Swap64(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile uint32 &x){ Swap32(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile uint16 &x){ Swap16(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile int64 &x) { Swap64(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile int32 &x) { Swap32(&x); }
    /**
     * Converts a number from big endian to little endian
     * @param x the number to convert
     */
    static inline void ToIntel(volatile int16 &x) { Swap16(&x); }
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile double &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile float  &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile  uint64 &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile uint32 &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile uint16 &x){ x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile int64 &x) { x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile int32 &x) { x = x;};
    /**
     * NOOP since the system is already big endian
     * @param x the number to convert
     */
    static inline void ToMotorola(volatile int16 &x) { x = x;};
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(float *dest,float *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(uint32 *dest,uint32 *src,uint32 size) { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(uint16 *dest,uint16 *src,uint32 size) { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(int32 *dest,int32 *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromIntel(int16 *dest,int16 *src,uint32 size)   { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(float *dest,float *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(uint32 *dest,uint32 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(uint16 *dest,uint16 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(int32 *dest,int32 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromMotorola(int16 *dest,int16 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(float *dest,float *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(uint32 *dest,uint32 *src,uint32 size) { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(uint16 *dest,uint16 *src,uint32 size) { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(int32 *dest,int32 *src,uint32 size)   { MemCopySwap32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToIntel(int16 *dest,int16 *src,uint32 size)   { MemCopySwap16(dest,src,size); }
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(float *dest,float *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(uint32 *dest,uint32 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(uint16 *dest,uint16 *src,uint32 size) { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(int32 *dest,int32 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
    /** 
     * Copies a block of memory but performs no endianity swap since both source and destinations are already big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToMotorola(int16 *dest,int16 *src,uint32 size)   { for (uint32 i = 0;i<size;i++) *dest++ = *src++;}
#endif
};
#endif

