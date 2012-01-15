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
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <rtai_proc_fs.h>  //Very nice macros to help printing information...
#include <rtai_malloc.h>
#include "fcomm_proc.h"
#include "fcomm_mod.h"
#include "fcomm_kapi.h"
#include "../share/fcomm_share.h"

// funcs exported for the baselib2 to set
typedef const char*(*ptfunc)(void);
ptfunc proc_thread_func = NULL;

void set_proc_thread_func(ptfunc toset){
    proc_thread_func = toset;
}
EXPORT_SYMBOL(set_proc_thread_func);

/**
 * Measures the occupation of the CPUS
 */
extern int num_of_cpus;
extern int proc_occup[MAX_NUM_PROCESSOR];

/**
 * The size of the baselib_heap, defined in loader.c of LevelRTAIC++sup
 */
extern int baselib_heap_size;
/**
 * Defined in cfunc_sup.c. Heap where data is allocated.
 */
extern rtheap_t baselib_heap;
/**
 * The size of the large_heap, defined in loader.c of LevelRTAIC++sup
 */
extern int large_heap_size;
/**
 * Defined in cfunc_sup.c. Heap where data is allocated.
 */
extern rtheap_t large_heap;
/**
 * The heap which is used by fcomm
 */
extern rtheap_t rtai_global_heap;

extern int call_remote_function_stats[MAX_FN_NUMBER];

static struct proc_dir_entry *proc_fcomm_root;
static struct proc_dir_entry *proc_stats;
static struct proc_dir_entry *proc_defines;
static struct proc_dir_entry *proc_cpus;
static struct proc_dir_entry *proc_memory;
static char *PROCFS_ROOT_NAME = "fcomm";
static char *PROCFS_STATS_NAME = "stats";
static char *PROCFS_DEFS_NAME = "defines";
static char *PROCFS_CPUS_NAME = "cpus";
static char *PROCFS_MEMORY_NAME = "memory";

int proc_file_memory_usage_read(char *page, char **start, off_t off, int count, int *eof, void *data){
    PROC_PRINT_VARS;
    PROC_PRINT("************************** FCOMM Memory **************************\n\n");
    PROC_PRINT("------------------------------------------------------------------\n");
    PROC_PRINT("|  HEAP   |  NPAGES  | PAGE SIZE |     ALLOCATED     |    MAX    |\n");
    PROC_PRINT("------------------------------------------------------------------\n");
    PROC_PRINT("|  FCOMM  |%10lu|%11lu|%12lu (%3lu%%)|%11lu|\n", rtai_global_heap.npages, rtai_global_heap.pagesize, rtai_global_heap.ubytes, rtai_global_heap.ubytes * 100 / rtai_global_heap.maxcont, rtai_global_heap.maxcont);
    PROC_PRINT("| BASELIB |%10lu|%11lu|%12lu (%3lu%%)|%11lu|\n", baselib_heap.npages, baselib_heap.pagesize, baselib_heap.ubytes, baselib_heap.ubytes * 100 / baselib_heap.maxcont, baselib_heap.maxcont);
    PROC_PRINT("| LARGE   |%10lu|%11lu|%12lu (%3lu%%)|%11lu|\n", large_heap.npages, large_heap.pagesize, large_heap.ubytes, large_heap.ubytes * 100 / large_heap.maxcont, large_heap.maxcont);
    PROC_PRINT("------------------------------------------------------------------\n\n\n");
    PROC_PRINT("The size of the max block of the baselib_heap is: %ld\n", fcomm_get_max_block_size(&baselib_heap));
    PROC_PRINT("The size of the max block of the large_heap is: %ld\n", fcomm_get_max_block_size(&large_heap));
    PROC_PRINT("The size of the max block of the rtai_global_heap is: %ld\n", fcomm_get_max_block_size(&rtai_global_heap));
    PROC_PRINT_DONE;
}

int proc_file_cpus_read(char *page, char **start, off_t off, int count, int *eof, void *data){
    int i = 0;
    PROC_PRINT_VARS;
    PROC_PRINT("\n\n****************** FCOMM CPUs ******************\n\n");
    if (num_of_cpus > 0){
        PROC_PRINT("Number of CPUs: %d\n\n",num_of_cpus);
    	PROC_PRINT("----------------------\n");
    	PROC_PRINT("| CPU |  OCCUPATION  |\n");
    	PROC_PRINT("----------------------\n");
        for (i=1;i<num_of_cpus;i++){
            PROC_PRINT("|%5d|%14d|\n", i, proc_occup[i]);
        }
    	PROC_PRINT("----------------------\n");
	PROC_PRINT("\n");
    }else{
        PROC_PRINT("num_of_cpus module parameter, not specified, no processor occupation information can be supplied.");
    }
    PROC_PRINT_DONE;
}

int proc_file_stats_read(char *page, char **start, off_t off, int count, int *eof, void *data){
    int c = 0;
    const int *threads_ids = get_worker_threads_ids();
    const unsigned long *threads_counter = get_worker_threads_counter();
    const unsigned long long *threads_time_use = get_worker_threads_time();
    PROC_PRINT_VARS;
    PROC_PRINT("******** FCOMM Statistics *********\n\n");
    PROC_PRINT("-----------------------------------\n");
    PROC_PRINT("| N |  ID  |  Count  |  Time(ms)  |\n");
    PROC_PRINT("-----------------------------------\n");
    for(c=0; c<MAX_WORKER_THREADS; c++){        
        PROC_PRINT("|%3d|%6d|%9ld|%12lld|\n", c, threads_ids[c], threads_counter[c], threads_time_use[c]);
    }
    PROC_PRINT("-----------------------------------\n");
    PROC_PRINT("\n\n");
    PROC_PRINT("call_remote_function stats:\n\n");
    for (c=0; c<65; c++) {
        PROC_PRINT("ID[%2d]=%3d",c,call_remote_function_stats[c]);
        if ((c%5)==4) PROC_PRINT("\n");
        else PROC_PRINT("\t");
    }
    PROC_PRINT("\n\n");
    PROC_PRINT_DONE;
}

int proc_file_defines_read(char *page, char **start, off_t off, int count, int *eof, void *data){
    PROC_PRINT_VARS;
    PROC_PRINT("\n\n****************** FCOMM Defines ******************\n\n");
    PROC_PRINT("MAX_PARAMETERS_CALL = %d\n", MAX_PARAMETERS_CALL);
    PROC_PRINT("NULL_PARAMETER_ID = %d\n", NULL_PARAMETER_ID);
    PROC_PRINT("NULL_PARAMETER_ID = %d\n", MAX_WORKER_THREADS);    
    PROC_PRINT("\n\n");
    PROC_PRINT_DONE;
}

int init_proc_output(void){
    proc_fcomm_root = create_proc_entry(PROCFS_ROOT_NAME, S_IFDIR, NULL);	
    if (proc_fcomm_root == NULL){
        remove_proc_entry(PROCFS_ROOT_NAME, proc_fcomm_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_ROOT_NAME);
        return -ENOMEM;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)

    proc_fcomm_root->owner = THIS_MODULE;
#endif    
    proc_stats = create_proc_entry(PROCFS_STATS_NAME, S_IFREG|S_IRUGO|S_IWUSR, proc_fcomm_root);	
    if (proc_stats == NULL){
        remove_proc_entry(PROCFS_STATS_NAME, proc_fcomm_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_STATS_NAME);
        return -ENOMEM;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)

    proc_stats->owner = THIS_MODULE;
#endif
    proc_stats->read_proc = proc_file_stats_read;
    
    proc_defines = create_proc_entry(PROCFS_DEFS_NAME, S_IFREG|S_IRUGO|S_IWUSR, proc_fcomm_root);	
    if (proc_defines == NULL){
        remove_proc_entry(PROCFS_STATS_NAME, proc_fcomm_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_DEFS_NAME);
        return -ENOMEM;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)

    proc_defines->owner = THIS_MODULE;
#endif
    proc_defines->read_proc = proc_file_defines_read;
    
    proc_cpus = create_proc_entry(PROCFS_CPUS_NAME, S_IFREG|S_IRUGO|S_IWUSR, proc_fcomm_root);       
    if (proc_cpus == NULL){
        remove_proc_entry(PROCFS_CPUS_NAME, proc_fcomm_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_CPUS_NAME);
        return -ENOMEM;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)

    proc_cpus->owner = THIS_MODULE;
#endif
    proc_cpus->read_proc = proc_file_cpus_read;

    proc_memory = create_proc_entry(PROCFS_MEMORY_NAME, S_IFREG|S_IRUGO|S_IWUSR, proc_fcomm_root);       
    if (proc_memory == NULL){
        remove_proc_entry(PROCFS_MEMORY_NAME, proc_fcomm_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_MEMORY_NAME);
        return -ENOMEM;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)

    proc_memory->owner = THIS_MODULE;
#endif
    proc_memory->read_proc = proc_file_memory_usage_read;

    return 0;
}

void close_proc_output(void){    
    remove_proc_entry(PROCFS_CPUS_NAME, proc_fcomm_root);
    remove_proc_entry(PROCFS_STATS_NAME, proc_fcomm_root);
    remove_proc_entry(PROCFS_DEFS_NAME, proc_fcomm_root);
    remove_proc_entry(PROCFS_MEMORY_NAME, proc_fcomm_root);
    remove_proc_entry(PROCFS_ROOT_NAME, NULL);
}

