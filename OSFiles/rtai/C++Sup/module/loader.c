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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <rtai_sched.h>
#include <rtai_malloc.h>
#include <rtai_shm.h>
#include "../global_obj_support.h"
#include "../cfunc_sup.h"
#include "../../../../Level0/SystemRTAI.h"

int rt_cpu_mask = 1;
module_param(rt_cpu_mask, int, 0);
MODULE_PARM_DESC(rt_cpu_mask, "CPUS where RT tasks are allowed to run");

void *__dynamic_cast(void);
void _ZTVN10__cxxabiv121__vmi_class_type_infoE(void);
void _ZTVN10__cxxabiv117__class_type_infoE(void);
void _ZTVN10__cxxabiv120__si_class_type_infoE(void);

EXPORT_SYMBOL(malloc);
EXPORT_SYMBOL(free);
EXPORT_SYMBOL(realloc);
EXPORT_SYMBOL(rtai_memcpy);
EXPORT_SYMBOL(rtai_memset);
EXPORT_SYMBOL(rtai_memcmp);
EXPORT_SYMBOL(rtai_strlen);
EXPORT_SYMBOL(rtai_strcmp);
EXPORT_SYMBOL(rtai_strchr);
EXPORT_SYMBOL(rtai_strstr);
EXPORT_SYMBOL(rtai_strncpy);
EXPORT_SYMBOL(rtai_strcpy);
EXPORT_SYMBOL(rtai_strncmp);
EXPORT_SYMBOL(rtai_strncat);
EXPORT_SYMBOL(rtai_strtod);
EXPORT_SYMBOL(rtai_strcat);
EXPORT_SYMBOL(rtai_toupper);
EXPORT_SYMBOL(rtai_tolower);
EXPORT_SYMBOL(rtai_atoi);
EXPORT_SYMBOL(rtai_atol);
EXPORT_SYMBOL(rtai_atof);
EXPORT_SYMBOL(rtai_strpbrk);

void _Znaj(void);
EXPORT_SYMBOL(_Znaj);
void _ZdaPv(void);
EXPORT_SYMBOL(_ZdaPv);
void _ZdlPv(void);
EXPORT_SYMBOL(_ZdlPv);
void _Znwj(void);
EXPORT_SYMBOL(_Znwj);
void __cxa_pure_virtual(void);
EXPORT_SYMBOL(__cxa_pure_virtual);
void __cxa_bad_typeid(void);
EXPORT_SYMBOL(__cxa_bad_typeid);

EXPORT_SYMBOL(__dynamic_cast);
EXPORT_SYMBOL(_ZTVN10__cxxabiv121__vmi_class_type_infoE);
EXPORT_SYMBOL(_ZTVN10__cxxabiv117__class_type_infoE);
EXPORT_SYMBOL(_ZTVN10__cxxabiv120__si_class_type_infoE);


/**
 * The heap which will be used for malloc/new of the C++ code
 */
struct rtheap baselib_heap;
EXPORT_SYMBOL(baselib_heap);
/**
 * The size of the baselib_heap
 */
int baselib_heap_size = 16777216; 
EXPORT_SYMBOL(baselib_heap_size);
module_param(baselib_heap_size, int, 0);
MODULE_PARM_DESC(baselib_heap_size, "The heap size for the baselib objects");

/**
 * The heap which will be used for large block allocations
 */
struct rtheap large_heap;
EXPORT_SYMBOL(large_heap);
/**
 * The size of the large_heap
 */
int large_heap_size = 134217728; 
EXPORT_SYMBOL(large_heap_size);
module_param(large_heap_size, int, 0);
MODULE_PARM_DESC(large_heap_size, "The heap size for the baselib large objects");
/**
 * The large heap minimum and maximum memory locations
 */
void *large_heap_mem_min_loc;
void *large_heap_mem_max_loc;
EXPORT_SYMBOL(large_heap_mem_min_loc);
EXPORT_SYMBOL(large_heap_mem_max_loc);

/**
 * The baselib heap minimum and maximum memory locations
 */
void *baselib_heap_mem_min_loc;
void *baselib_heap_mem_max_loc;
EXPORT_SYMBOL(baselib_heap_mem_min_loc);
EXPORT_SYMBOL(baselib_heap_mem_max_loc);

/**
 * The threshold size for a malloc request to be allocated in the large_heap 
 */
int largeBlockThreshold = 0x400000;
EXPORT_SYMBOL(largeBlockThreshold);
module_param(largeBlockThreshold, int, 0);
MODULE_PARM_DESC(largeBlockThreshold, "The threshold size for a malloc request to be allocated in the large_heap");

/**
 * Start up processor
 */
static int was_timer_running = 0;
static RT_TASK startup_task;
int rt_cpu_mask = 1;
//THE STACK SIZE OF THE LAUNCHING TASK IS VERY IMPORTANT...
#define STACK_SIZE 64000
void startup(void) {   
    init_all_constructors();
}

/**
 * Tries to create the base lib heap.
 * Return -1 if error, 0 otherwise.
 */
int init_bl_heap(void) {
    baselib_heap_size = (baselib_heap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (rtheap_init(&baselib_heap, NULL, baselib_heap_size, PAGE_SIZE, 0)) {
         rt_printk("Failed to initialize the baselib heap (size=%d bytes).\n", baselib_heap_size);
         return -1;
    }

    return 0;
}

/**
 * Tries to create the large heap.
 * Return -1 if error, 0 otherwise.
 */
int init_large_heap(void) {
    large_heap_size = (large_heap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (rtheap_init(&large_heap, NULL, large_heap_size, PAGE_SIZE, 0)) {
         rt_printk("Failed to initialize the large heap (size=%d bytes).\n", large_heap_size);
         return -1;
    }

    return 0;
}

/**
 * Destroys the baselib heap
 */
void close_bl_heap(void){
    rtheap_destroy(&baselib_heap, 0);
}

/**
 * Destroys the large heap
 */
void close_large_heap(void){
    rtheap_destroy(&large_heap, 0);
}

int init_func(void) {
    printk("Starting RTAI C++ support\n");
    if(init_bl_heap() < 0){
    	return -1;
    }
    if(init_large_heap() < 0){
    	return -1;
    }

    baselib_heap_mem_min_loc = baselib_heap.extents.next;
    baselib_heap_mem_max_loc = baselib_heap_mem_min_loc + baselib_heap_size;
    large_heap_mem_min_loc = large_heap.extents.next;
    large_heap_mem_max_loc = large_heap_mem_min_loc + large_heap_size;

    rt_printk("Initialized the baselib heap with %d bytes [%p %p].\n", baselib_heap_size, baselib_heap_mem_min_loc, baselib_heap_mem_max_loc);
    rt_printk("Initialized the large heap with %d bytes [%p %p].\n", large_heap_size, large_heap_mem_min_loc, large_heap_mem_max_loc);

    if(rt_is_hard_timer_running())
        was_timer_running = 1;
    else
        start_rt_timer(0);
    rt_task_init(&startup_task, startup, 1, STACK_SIZE, 1, 0, 0);
    rt_set_runnable_on_cpus(&startup_task, rt_cpu_mask);
    rt_task_resume(&startup_task);
    
    return 0;
}

void exit_func(void) {
    printk("Ending RTAI C++ support\n");
    if(was_timer_running)
        stop_rt_timer();
    rt_task_delete(&startup_task);
    delete_all_deconstructors();
   
    close_bl_heap();
    close_large_heap();
}

module_init(init_func);
module_exit(exit_func);
