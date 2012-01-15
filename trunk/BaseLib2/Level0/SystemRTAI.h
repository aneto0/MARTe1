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
 * RTAI implementation
 */
#ifndef SYSTEM_RTAI_H
#define SYSTEM_RTAI_H

//These are also used by the loader
#define RTAI_THREADS_DEFAULT_STACKSIZE  49152
#define DEFAULT_RT_CPU_MASK     0x2
#define RTAI_PRIORITY_MIN       0x1FFFFF
#define RTAI_PRIORITY_MAX       0x00FFFF
#define RTAI_PRIORITY_IDLE      0x1EFFFF
#define RTAI_PRIORITY_NORMAL    0x0FFFFF
#define RTAI_PRIORITY_HIGH      0x08FFFF

//Only used if round robin is enabled
#define RR_QUANTUM_NS 5000000

#if defined(_RTAI)

#define memcpy rtai_memcpy
#define memset rtai_memset
#define memcmp rtai_memcmp
#define strlen rtai_strlen
#define strcmp rtai_strcmp
#define strchr rtai_strchr
#define strstr rtai_strstr
#define strncpy rtai_strncpy
#define strcpy rtai_strcpy
#define strncmp rtai_strncmp
#define strncat rtai_strncat
#define strcat rtai_strcat
#define strtod rtai_strtod
#define toupper rtai_toupper
#define tolower rtai_tolower
#define atoi rtai_atoi
#define atol rtai_atol
#define atof rtai_atof
#define strncat rtai_strncat
#define strpbrk rtai_strpbrk
//TODO Check if ok
#define strtol simple_strtol

//only use in RTAI functions withOUT RTAI_SYSCALL_MODE
#include "../../OSFiles/rtai/C++Sup/linux_version.h"
#include <byteswap.h>
#include <ctype.h>
#include "../../OSFiles/rtai/C++Sup/cfunc_sup.h"
#include "../../OSFiles/rtai/C++Sup/typeinfo"
#include "../../OSFiles/rtai/fcomm/kspace/fcomm_kapi.h"
#include "../../OSFiles/rtai/fcomm/kspace/fcomm_mod.h"

#include <rtai_nam2num.h>    
#include <rtai_types.h>
#include <rtai_math.h>

#define vprintf fcomm_vprintk
#define printf rt_printk
#define sscanf fcomm_sscanf
#define snprintf bl2_snprintf
#define vsnprintf bl2_vsnprintf
#define sprintf bl2_sprintf
#define strnicmp fcomm_strnicmp
#define strncasecmp strnicmp
#define htons fcomm_htons
#define htonl fcomm_htonl

#define STDIN 0
#define stdin STDIN
#define STDOUT 1
#define stdout STDOUT
#define STDERR 2
#define stderr STDERR
#ifndef NULL
#define NULL 0
#endif
        
#define strerror(error) "RTAI"
#define sock_errno() fcomm_get_errno()
#define soclose close

#define KEEP_STATIC_INLINE
#define TID uint32
#define TaskID RT_TASK *
#define HANDLE void *
#define HFILE uint32
#define __thread
#define __thread_decl
#define SEM_INDEFINITE_WAIT -1
#define INTEL_PLATFORM
#define INTEL_BYTE_ORDER
#define _EXTERN_ extern

#define USE_VMALLOC 0

#define IEEE_FLOAT_NAN 0x7fC00000 
#define IEEE_FLOAT_INF 0x7f800000
#define IEEE_FLOAT_INF_NEG 0xFf800000

#define SEM_ID SEM *
#define RT_TASK_STRUCT_SIZE 2000
#define RT_TASK struct rt_task_struct
#define RTE_BASE 0x3FFFFF00
#define RT_TIMOUT 3
#define RTE_TIMOUT (RTE_BASE + RT_TIMOUT)
#define SEM_TIMOUT 0xFFFE


//Defines for rt_get_task_state
#define RT_SCHED_READY        1
#define RT_SCHED_SUSPENDED    2
#define RT_SCHED_DELAYED      4
#define RT_SCHED_SEMAPHORE    8
#define RT_SCHED_SEND        16
#define RT_SCHED_RECEIVE     32
#define RT_SCHED_RPC         64
#define RT_SCHED_RETURN     128
#define RT_SCHED_MBXSUSP    256
#define RT_SCHED_SFTRDY     512


//This allows to pass these parameters as module parameters
extern int rt_cpu_mask;
extern int thread_stack_size;

//RTAI module parameters which state min and max jitter (nanosecs)
extern int RTAI_max_jitter;
extern int RTAI_min_jitter;

#include "System.h"
#include "GenDefs.h"
#include "CStream.h"

extern "C"
{   
    typedef void (*rt_thread)(void *parameters); 
    void rt_printk(const char *s, ...);

    int fcomm_get_prio(RT_TASK* task);
    int fcomm_get_thread_cpu(RT_TASK* task);
    
    //Your main module must call these functions once, not YOU!
    /*RTIME start_rt_timer(int period);
    void stop_rt_timer(void);
    int rt_is_hard_timer_running(void);*/
    struct SEM;
    
    void *rt_shm_alloc(unsigned long name, int size, int suprt);
    RT_TASK *rt_whoami(void);
    int rt_task_init(struct rt_task_struct *task,
                     void (*rt_thread)(long),
		     long data,
		     int stack_size,
		     int priority,
		     int uses_fpu,
		     void(*signal)(void));
    int rt_task_delete(RT_TASK *task);
    void rt_task_yield(void);
    int rt_get_inher_prio(RT_TASK *task);
    int rt_get_prio(RT_TASK *task);
    int rt_get_task_state(RT_TASK *task); 
    void *rt_named_malloc(unsigned long name, int size);
    void rt_named_free(void *address);
    int rt_set_period(struct rt_task_struct *task, RTIME new_period);

    RTIME rt_get_time(void);
    __attribute__((regparm(0))) void rt_free(void *ptr);
    __attribute__((regparm(0))) int rt_shm_free(unsigned long name);
    __attribute__((regparm(0))) int rt_sleep(RTIME delay);
    __attribute__((regparm(0))) RTIME count2nano(RTIME timercounts);
    __attribute__((regparm(0))) RTIME nano2count(RTIME nanosecs);        
    __attribute__((regparm(0))) int rt_task_resume(RT_TASK *task);    
    __attribute__((regparm(0))) int rt_task_suspend(RT_TASK *task);    
    __attribute__((regparm(0))) int rt_change_prio(RT_TASK * task, int priority);
    __attribute__((regparm(0))) void* rt_get_adr(unsigned long name);
    __attribute__((regparm(0))) unsigned long rt_get_name(void *adr);
    __attribute__((regparm(0))) SEM *_rt_typed_named_sem_init(unsigned long sem_name, int value, int type);
    __attribute__((regparm(0))) int rt_named_sem_delete(SEM *sem);
    __attribute__((regparm(0))) int rt_sem_wait(SEM *sem);
    __attribute__((regparm(0))) int rt_sem_reset(SEM *sem);
    __attribute__((regparm(0))) int rt_sem_wait_timed(SEM * sem, RTIME delay);
    __attribute__((regparm(0))) int rt_sem_wait_if(SEM *sem);
    __attribute__((regparm(0))) int rt_sem_signal(SEM *sem);    
    __attribute__((regparm(0))) int rt_sem_broadcast(SEM * sem);                
    __attribute__((regparm(0))) void rt_set_runnable_on_cpus( RT_TASK *task, unsigned int cpu_mask);
    __attribute__((regparm(0))) int rt_task_make_periodic(struct rt_task_struct *task, RTIME start_time, RTIME period);    
    __attribute__((regparm(0))) int rt_task_wait_period();    
    int abs(int j);
    
    void emergency_restart(void);    
}

inline int strcasecmp(const char *s1, const char *s2 ){
    if (s1 == s2) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;

    register char c1 = toupper(*s1);
    register char c2 = toupper(*s2);
    while ((c1!=0) && (c2!=0)){
  	if (c1>c2) return 1;
	if (c1<c2) return -1;
	s1++;
	s2++;
	c1 = toupper(*s1);
	c2 = toupper(*s2);
    }
    if ((c1==0) && (c2==0)) return 0;
    if (c1==0) return -1;

    return 1;
}

inline int strncasecmp(const char *s1, const char *s2, int n ){
    if (s1 == s2) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;
    register char c1= toupper(*s1);
    register char c2= toupper(*s2);
    while ((c1!=0) && (c2!=0) && (n>0)){
        if (c1>c2) return 1;
	if (c1<c2) return -1;
	s1++;
	s2++;
	n--;
	c1= toupper(*s1);
        c2= toupper(*s2);
    }

    if (n==0) return 0;
    if ((c1==0) && (c2==0)) return 0;
    if (c1==0) return -1;
    
    return 1;
}

#endif

/*
 * do_div() is NOT a C function. It wants to return
 * two values (the quotient and the remainder), but
 * since that doesn't work very well in C, what it
 * does is:
 *
 * - modifies the 64-bit dividend _in_place_
 * - returns the 32-bit remainder
 *
 * This ends up being the most efficient "calling
 * convention" on x86.
 */
#ifndef do_div
#define do_div(n,base) ({ \
        unsigned long __upper, __low, __high, __mod, __base; \
        __base = (base); \
        asm("":"=a" (__low), "=d" (__high):"A" (n)); \
        __upper = __high; \
        if (__high) { \
                __upper = __high % (__base); \
                __high = __high / (__base); \
        } \
        asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (__base), "0" (__low), "1" (__upper)); \
        asm("":"=A" (n):"a" (__low),"d" (__high)); \
        __mod; \
})
#endif
#endif

