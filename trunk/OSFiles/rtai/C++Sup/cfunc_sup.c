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
 * In real time, the RTAI protected memory management must be used, mostly because
 * of context switchings between the two schedulers. 
 */

#include "cfunc_sup.h"
#include <stddef.h>

/**
 * RT printf
 */
void rt_printk(const char *format, ...);

/**
 * Kernel malloc
 */
void *vmalloc(size_t size);

/**
 * Kernel free
 */
void vfree(void *ptr);

/** 
 * This function converts the initial part of the string in nptr to a long 
 * integer value according to the given base, which must be between 2 and 36 inclusive
 * or be the special value 0
 */
//long simple_strtol(const char *cp, char **endp, unsigned int base);

/**
 * Real time heap functions (used internally)
 */
struct rtheap;
typedef struct rtheap rtheap_t;
extern rtheap_t baselib_heap;
extern rtheap_t large_heap;
void *rtheap_alloc(rtheap_t*, int, int);
int rtheap_free(rtheap_t*, void*);

extern int largeBlockThreshold;
/**
 * The large heap minimum and maximum memory locations
 */
extern void *large_heap_mem_min_loc;
extern void *large_heap_mem_max_loc;

/**
 * frees the memory space pointed to by ptr, which must have been returned by 
 * a previous call to malloc(), calloc() or  realloc(). It decides in what heap to request the free
 * accordingly to the heap boundaries
 */
void free(void *ptr){
    if(ptr < large_heap_mem_min_loc || ptr > large_heap_mem_max_loc){
        rtheap_free(&baselib_heap, ptr);
    }
    else{
        rtheap_free(&large_heap, ptr);
    }
}

/**
 * C++ wrappper
 */
void _Z4freePv(void *ptr){
    free(ptr);
}

/**
 * allocates size bytes and returns a pointer to the allocated memory.  
 * The memory is not cleared.  If size is 0, then malloc()
 * returns either NULL, or a unique pointer value that can later be successfully passed to free()
 * If the requested size is superior to largeBlockThreshold it allocated on the large heap. Otherwise it 
 * allocates on the baselib heap
 */
void *malloc(size_t size){
    if(size > largeBlockThreshold){
        return rtheap_alloc(&large_heap, size, 0);
    }
    else{
        return rtheap_alloc(&baselib_heap, size, 0);
    }
}

/**
 * changes the size of the memory block pointed to by ptr to size bytes.  
 * The contents will be unchanged to the minimum of the
 * old and new sizes; newly allocated memory will be uninitialized.  
 */
void *realloc(void *data, size_t newSize){
    //no realloc in kernel space! Ugly solution...
    void *tempData = malloc(newSize);
    if(!tempData) {
        free(data);
        return data;
    }
    rtai_memcpy(tempData, data, newSize);
    free(data);
    data = tempData;
    return data;
}

/**
 * The memcmp() function compares the first n bytes of the memory areas cs and ct.  
 * It returns an integer less than, equal to, or greater
 * than zero if cs is found, respectively, to be less than, to match, or be greater than ct.
 */
int rtai_memcmp(const void *cs, const void *ct, size_t count){
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
            if ((res = *su1 - *su2) != 0)
                    break;
    return res;
}

/**
 * The  memcpy()  function copies n bytes from memory area src to memory area dest.  
 * The memory areas should not overlap.
 */
void *rtai_memcpy(void *dest, const void *src, size_t n){
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while(n--)
            *d++ = *s++;
    return dest;
}

/**
 * The memset() function fills the first n bytes of the memory area pointed to by src 
 * with the constant byte c.
 */
void *rtai_memset(void *src, int c, size_t n){
    char *s = (char *)src;
    while(n--)
        *s++ = (char)c;
    return src;
}

/**
 * The atoi() function converts the initial portion of the string pointed to by nptr to int.
 */
int rtai_atoi(const char *cp){
    return (int)simple_strtol(cp, NULL, 10 );
}

/**
 * The atol() and atoll() functions behave the same as atoi(), except that 
 * they convert the initial portion of the string to their return
 * type of long or long long.
 */
long rtai_atol(const char *cp){
    return simple_strtol(cp, NULL, 10 );
}

/**
 * The  abort()  function causes abnormal program termination unless the signal 
 * SIGABRT is caught and the signal handler does not return.
 */
void abort(void){
    rt_printk("cfunc_sup.c ABORT CALLED!\n");
    while(1){}
}

/**
 * C++ wrapper
 */
void _ZTISt9exception(void) {
}

/**
 * C++ wrapper
 */
void _ZNKSt9exception4whatEv(void) {
}

/**
 * C++ wrapper
 */
void _ZNSt9exceptionD2Ev(void) {
}

/**
 * C++ wrapper
 */
void __cxa_bad_typeid(void){    
    rt_printk("cfunc_sup.c! __cxa_bad_typeid CALLED\n");
}

/**
 * C++ wrapper
 */
void __cxa_pure_virtual(void) {
    rt_printk("cfunc_sup.c! __cxa_pure_virtual CALLED\n");
}

/**
 * This function  compares the two strings cs and ct.  
 * It returns an integer less than, equal to, or greater than zero if cs is
 * found, respectively, to be less than, to match, or be greater than ct
 */
int rtai_strcmp(const char * cs, const char * ct) {
    int d0, d1;
    register int __res;
    __asm__ __volatile__(
    "1:\tlodsb\n\t"
    "scasb\n\t"
    "jne 2f\n\t"
    "testb %%al,%%al\n\t"
    "jne 1b\n\t"
    "xorl %%eax,%%eax\n\t"
    "jmp 3f\n"
    "2:\tsbbl %%eax,%%eax\n\t"
    "orb $1,%%al\n"
    "3:"
    :"=a" (__res), "=&S" (d0), "=&D" (d1)
    :"1" (cs), "2" (ct)
    :"memory");
    return __res;
}

/**
 * This function returns a pointer to the first occurrence of the character c in the string s.
 */
char *rtai_strchr(const char * s, int c) {
    int d0;
    register char * __res;
    __asm__ __volatile__(
    "movb %%al,%%ah\n"
    "1:\tlodsb\n\t"
    "cmpb %%ah,%%al\n\t"
    "je 2f\n\t"
    "testb %%al,%%al\n\t"
    "jne 1b\n\t"
    "movl $1,%1\n"
    "2:\tmovl %1,%0\n\t"
    "decl %0"
    :"=a" (__res), "=&S" (d0)
    :"1" (s), "0" (c)
    :"memory");
    return __res;
}

/**
 * This function calculates the length of the string s, not including the terminating ’\0’ character.
 */
size_t rtai_strlen(const char * s) {
    int d0;
    register int __res;
    __asm__ __volatile__(
    "repne\n\t"
    "scasb\n\t"
    "notl %0\n\t"
    "decl %0"
    :"=c" (__res), "=&D" (d0)
    :"1" (s), "a" (0), "0" (0xffffffffu)
    :"memory");
    return __res;
}

/**
 * This function  copies  the string pointed to by src, including the terminating null byte (’\0’), to the buffer pointed to by
 * dest.  The strings may not overlap, and the destination string dest must be large enough to receive the copy.
 */
char *rtai_strcpy(char * dest, const char *src) {
    int d0, d1, d2;
    __asm__ __volatile__(
    "1:\tlodsb\n\t"
    "stosb\n\t"
    "testb %%al,%%al\n\t"
    "jne 1b"
    : "=&S" (d0), "=&D" (d1), "=&a" (d2)
    :"0" (src), "1" (dest) : "memory");
    return dest;
}

/**
 * This function is similar to strcpy , except that at most n bytes of src are copied
 */
char *rtai_strncpy(char * dest, const char *src, size_t count) {
    int d0, d1, d2, d3;
    __asm__ __volatile__(
    "1:\tdecl %2\n\t"
    "js 2f\n\t"
    "lodsb\n\t"
    "stosb\n\t"
    "testb %%al,%%al\n\t"
    "jne 1b\n\t"
    "rep\n\t"
    "stosb\n"
    "2:"
    : "=&S" (d0), "=&D" (d1), "=&c" (d2), "=&a" (d3)
    :"0" (src), "1" (dest), "2" (count) : "memory");
    return dest;
}

/**
 * This function finds the first occurrence of the substring s2 in the string s1.  
 * The terminating ’\0’ characters are not compared.
 */
char *rtai_strstr(const char *s1, const char *s2) {
    int l1, l2;
    
    l2 = rtai_strlen(s2);
    if (!l2)
        return (char *)s1;
    l1 = rtai_strlen(s1);
    while (l1 >= l2) {
        l1--;
        if (!rtai_memcmp(s1, s2, l2))
            return (char *)s1;
        s1++;
    }
    return NULL;
}

/**
 * This function appends the src string to the dest string, overwriting the 
 * null byte at the end of dest, and then adds a terminating null byte. 
 * The strings may not overlap, and the dest string must have enough space for the result.
 */
char *rtai_strcat(char *dest, const char *src){
        char *tmp = dest;

        while (*dest)
            dest++;
        while ((*dest++ = *src++) != '\0');

        return tmp;
}

/**
 * strncmp - Compare two length-limited strings
 * @cs: One string
 * @ct: Another string
 * @count: The maximum number of bytes to compare
 */
int rtai_strncmp(const char *cs, const char *ct, size_t count) {
    signed char __res = 0;
    
    while (count) {
        if ((__res = *cs - *ct++) != 0 || !*cs++)
            break;
        count--;
    }
    return __res;
}

int rtai_toupper(int c) {
    if (c >= 'a' && c <= 'z')
        return c - ('a' - 'A');
    else
        return c;
}

int rtai_tolower(int c) {
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');
    else
        return c;
}

/**
 * strncat - Append a length-limited, %NUL-terminated string to another
 * @dest: The string to be appended to
 * @src: The string to append to it
 * @count: The maximum numbers of bytes to copy
 *
 * Note that in contrast to strncpy, strncat ensures the result is
 * terminated.
 */
char *rtai_strncat(char *dest, const char *src, size_t count)
{
        char *tmp = dest;

        if (count) {
                while (*dest)
                        dest++;
                while ((*dest++ = *src++) != 0) {
                        if (--count == 0) {
                                *dest = '\0';
                                break;
                        }
                }
        }
        return tmp;
}

char *rtai_strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;
        for (sc1 = cs; *sc1 != '\0'; ++sc1) {
	        for (sc2 = ct; *sc2 != '\0'; ++sc2) {
		        if (*sc1 == *sc2)
			        return (char *)sc1;
		}
	}
	return NULL;
}
char *rtai_strcasestr(const char *haystack, const char *needle)
{
	size_t hay_len = rtai_strlen(haystack);
	size_t needle_len = rtai_strlen(needle);
	while (hay_len >= needle_len) {
		if (rtai_strncmp(haystack, needle, needle_len) == 0) 
		    return (char *) haystack;

		haystack++;
		hay_len--;
	}
	return NULL;
}

