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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <rtai.h>
#include <rtai_malloc.h>
#include <rtai_sched.h>
#include <rtai_shm.h>
#include <rtai_sem.h>
#include <rtai_nam2num.h>
#include <rtai_registry.h>
#include <asm/rtai_sched.h>

#include "fcomm_mod.h"
#include "fcomm_kapi.h"
#include "fcomm_proc.h"
#include "../share/fcomm_share.h"

int call_remote_function_stats[MAX_FN_NUMBER];

EXPORT_SYMBOL(free_parameters);
EXPORT_SYMBOL(free_parameter);
EXPORT_SYMBOL(free_parameter_from_ptr);
EXPORT_SYMBOL(get_rt_free_uid);

// RTAI max & min jitter
int RTAI_max_jitter = DEFAULT_RTAI_MAX_JITTER;
EXPORT_SYMBOL(RTAI_max_jitter);
module_param(RTAI_max_jitter, int, 0);
MODULE_PARM_DESC(RTAI_max_jitter, "The max (absolute) value of RTAI's jitter (in nanosecs) as shown by RTAI testsuite");

int RTAI_min_jitter = DEFAULT_RTAI_MIN_JITTER;
EXPORT_SYMBOL(RTAI_min_jitter);
module_param(RTAI_min_jitter, int, 0);
MODULE_PARM_DESC(RTAI_min_jitter, "The min (absolute) value of RTAI's jitter (in nanosecs) as shown by RTAI testsuite");

//Number of CPUs available in the system
int num_of_cpus = -1;
/*module_param(num_of_cpus, int, 0);
MODULE_PARM_DESC(num_of_cpus, "The number of CPUs available in the system");*/

volatile int proc_occup[MAX_NUM_PROCESSOR];
volatile int keepAlive[MAX_NUM_PROCESSOR];
RT_TASK occupation_tasks[MAX_NUM_PROCESSOR];

//Measures the RTAI sleep jitter 
//RT_TASK jitter_measurament_task;

void processor_occupation_task(long int proc){
    int processor_number = proc;
    int counter = 0;

    RTIME before = rt_get_time();
    RTIME after = before;

    const RTIME ten_millisec = nano2count(1E7);
    
    rt_printk("Task running on processor %d\n",processor_number);

    while (keepAlive[processor_number] == 1){
        before = rt_get_time();
        while ( (after-before) < ten_millisec ){	
	    counter++; 
	    after = rt_get_time();
	}
        proc_occup[processor_number] = counter;
	rt_sleep(nano2count(99E7));
	counter = 0;
    }
}

/*void jitter_measurament(long ignored){
    long jitter     = 0;
    int  i          = 0;
    RTIME sleepTime = 0;
    RTIME start     = 0;
    RTAI_min_jitter =  1000000000;
    RTAI_max_jitter = -1000000000;        
    
    for(i=0; i<5; i++){
        //sleep 10us
        start = rt_get_time_ns_cpuid(1);
        sleepTime = 10E3;
        rt_sleep(nano2count(sleepTime));
        jitter = (rt_get_time_ns_cpuid(1) - start) - sleepTime;    

        if(jitter > RTAI_max_jitter){
            RTAI_max_jitter = jitter;
        }
        if(jitter < RTAI_min_jitter){
            RTAI_min_jitter = jitter;
        }    

        //sleep 100us
        start = rt_get_time_ns_cpuid(1);
        sleepTime = 100E3;
        rt_sleep(nano2count(sleepTime));
        jitter = (rt_get_time_ns_cpuid(1) - start) - sleepTime;

        if(jitter > RTAI_max_jitter){
            RTAI_max_jitter = jitter;
        }
        if(jitter < RTAI_min_jitter){
            RTAI_min_jitter = jitter;
        }

        //sleep 1ms
        start = rt_get_time_ns_cpuid(1);
        sleepTime = 1E6;
        rt_sleep(nano2count(sleepTime));
        jitter = (rt_get_time_ns_cpuid(1) - start) - sleepTime;   

        if(jitter > RTAI_max_jitter){
            RTAI_max_jitter = jitter;
        }
        if(jitter < RTAI_min_jitter){
            RTAI_min_jitter = jitter;
        }

        //sleep 100ms
        start = rt_get_time_ns_cpuid(1);
        sleepTime = 100E6;
        rt_sleep(nano2count(sleepTime));
        jitter = (rt_get_time_ns_cpuid(1) - start) - sleepTime;

        if(jitter > RTAI_max_jitter){
            RTAI_max_jitter = jitter;
        }
        if(jitter < RTAI_min_jitter){
            RTAI_min_jitter = jitter;
        }

        //sleep 1s
        start = rt_get_time_ns_cpuid(1);
        sleepTime = 1E9;
        rt_sleep(nano2count(sleepTime));
        jitter = (rt_get_time_ns_cpuid(1) - start) - sleepTime;

        if(jitter > RTAI_max_jitter){
            RTAI_max_jitter = jitter;
        }
        if(jitter < RTAI_min_jitter){
            RTAI_min_jitter = jitter;
        }
    }

    if(RTAI_max_jitter < 0){
        rt_printk("RTAI_max_jitter was negative = %lld, setting to 0\n", RTAI_max_jitter);
        RTAI_max_jitter = 0;
    }
        
    rt_printk("RTAI_max_jitter = %ld\n", RTAI_max_jitter);
    rt_printk("RTAI_min_jitter = %ld\n", RTAI_min_jitter);
}*/


//Functions exported to obtain thread data
int fcomm_get_thread_cpu(RT_TASK* task){
    return task->lnxtsk ? CPUMASK((task->lnxtsk)->cpus_allowed) : (1 << task->runnable_on_cpus);
}
EXPORT_SYMBOL(fcomm_get_thread_cpu);

int fcomm_get_prio(RT_TASK* task){
    return task->priority;
}
EXPORT_SYMBOL(fcomm_get_prio);

//Semaphore which locks the uid generator. Beware that this is also synch with the user space uid gen.
static SEM *mem_alloc_synch_sem;

//Semaphore used to wait for function returns
static SEM *fun_return_sem;

//Semaphore used to warn the user space to perform the work
static SEM *fun_caller_user_sem;

//Semaphore used to warn the kernel space that the job has started
static SEM *fun_caller_kernel_sem;

//Semaphore used to wait on call_remote_function. It protects the shared memory block id
static SEM *fun_caller_mem_block_sem;

//Waits for available workers in user space
static SEM *fun_call_synch_sem;

//Protects the available workers counter
static SEM *fun_call_mem_inc_sem;

//Only one task at the time can allocate memory 
static SEM *mem_alloc_sem;

//This semaphore is signalled when the user space has finished initialization
SEM *user_space_init_finish_sem;

//The parameter id. This uniquely identifies each memory block in the system...
static unsigned long memory_current_uid = MIN_SHM_UID;

//Shared memory which stores the memory id of the function to call
static unsigned long *remote_fun_call_id_shm;

//Shared memory which stores the number of workers in use in user space
static int *fun_call_synch_mem;

//Shared memory which stores the ids of the worker threads
static int *worker_threads_ids;

//Shared memory which stores the usage counts of the worker threads
static unsigned long *worker_threads_counter;

//Shared memory which is updated with the latest time
static time_t *time_memory;

//Shared memory which stores the usage time of the worker threads
static unsigned long long *worker_threads_time_use;

//The shared heap
static void *heap;

MODULE_LICENSE("GPL");

int trap_handler(int vec, int signo, struct pt_regs *regs, void *dummy){
    rt_printk("GOT TRAP!!!\n");
    rt_printk("VEC = %d\n", vec);
    rt_printk("SIGNO = %d\n", signo);
    rt_printk("STACK = %p\n", (rt_whoami())->stack);
    dump_stack();
    rt_printk("Suspending task %p\n", rt_whoami());
    rt_task_suspend(rt_whoami());
    return 1;
}

void real_free(void *ptr){
    rt_sem_wait(mem_alloc_sem);
    while(rt_get_name(ptr) != 0)
        rt_named_free(ptr);
    rt_sem_signal(mem_alloc_sem);
}

void *real_alloc(unsigned long id, int size){
    void *mem;
    rt_sem_wait(mem_alloc_sem);
    mem = rt_named_malloc(id, size);
    rt_sem_signal(mem_alloc_sem);
    return mem;
}

int get_rt_free_uid(void){        
    unsigned long mem_uid = 0;    
    //THESE ARE REALLY NEEDED
    rt_sem_wait(mem_alloc_synch_sem);
    rt_sem_wait(mem_alloc_sem);
    memory_current_uid++;
    while(rt_get_adr(memory_current_uid) != 0)
    {
        if(memory_current_uid > MAX_SHM_UID)
            memory_current_uid = MIN_SHM_UID;
        memory_current_uid++;        
    }
    mem_uid = memory_current_uid;
    rt_sem_signal(mem_alloc_sem);
    rt_sem_signal(mem_alloc_synch_sem);    
    return mem_uid;
}

unsigned long get_parameter(int size, void *to_copy){
    void *mem;
    unsigned long id;
    
    if(size == 0 || to_copy == NULL)
        return NULL_PARAMETER_ID;
        
    id = get_rt_free_uid();        
    mem = real_alloc(id, size);
    if(mem != NULL)    {
        memcpy(mem, to_copy, size);
    }
    else{
        rt_printk("COULD NOT ALLOCATE MEMORY FOR PARAMETER...\n");
        id = ERROR_SHM;
    }    
    return id;
}

unsigned long get_str_parameter(char *to_copy){
    if(to_copy == NULL || strlen(to_copy) == 0)
        return NULL_PARAMETER_ID;
    return get_parameter(strlen(to_copy) + 1, to_copy);
}

unsigned long get_parameter_no_copy(int size){
    void *mem;
    unsigned long id;
    
    if(size == 0)
        return NULL_PARAMETER_ID;
            
    id = get_rt_free_uid();        
    //mem = rt_named_malloc(id, size);
    mem = real_alloc(id, size);
    if(mem == NULL)    {
        rt_printk("COULD NOT ALLOCATE MEMORY FOR PARAMETER (NO COPY)...\n");
        id = ERROR_SHM;
    }
    return id;
}

void free_parameter_from_ptr(void *par){
    if(par != NULL)
        real_free(par);
}

void free_parameter(unsigned long id){
    if(id != NULL_PARAMETER_ID)
        free_parameter_from_ptr(real_alloc(id, 0));
}

void free_parameters(unsigned long *id, int par_size){
    int i = 0;
    for(; i<par_size; i++)
        free_parameter(id[i]);
}


/** Checks the number of available workers in user-space*/
int test_call_remote_function(void){
    int ret = 0;
    rt_sem_wait(fun_call_mem_inc_sem);
    ret = *fun_call_synch_mem;
    rt_sem_signal(fun_call_mem_inc_sem);
    return ret;
}

int call_remote_function(int fun_id, unsigned long *par_ids, int par_size){
    mem_function *mem;
    int i = 0;
    int id = 0;
    
    call_remote_function_stats[fun_id]++;

    while(test_call_remote_function() < 1){
        rt_sem_wait(fun_call_synch_sem);
    }
    
    id = get_rt_free_uid();
    
    //Init protection
    rt_sem_wait(fun_caller_mem_block_sem);
    mem = (mem_function *)real_alloc(id, sizeof(mem_function));
    if(mem != 0){
        *remote_fun_call_id_shm = id;
        mem->fun_id = fun_id;
        mem->finished = 0;
        mem->error_errno = 0;        
        for(; i<par_size && i<MAX_PARAMETERS_CALL; i++)
            mem->parameter_ids[i] = par_ids[i];    
    }                
    else{
        rt_printk("MEMORY ERROR!\n");
        rt_sem_signal(fun_caller_mem_block_sem);        
        return ERROR_SHM;
    }
            
    //Call the user-space to the job...
    rt_sem_signal(fun_caller_user_sem);
        
    if(mem == NULL){
        rt_printk("MEMORY IS NULL WTF???\n");
    }
    //Wait for acknowledgment...
    if(mem->finished != 1){
        rt_sem_wait(fun_caller_kernel_sem); 
    }
            
    //end protection
    rt_sem_signal(fun_caller_mem_block_sem);    
    while(mem->finished != 1){
        rt_sem_wait(fun_return_sem);        
    }
        
    //Assign the returned value to i
    i = mem->return_value;
    /*if(i == -1)
        rt_printk("-1 received when calling function with id = %d\n", fun_id);*/
    
    //Since in kernel space we don't use RTAI's ipc, reuse the msg parameter
    //from the RT_TASK to store the errno... !TODO! CHECK if this is ok...(I've looked
    // at the RTAI source code and msg seems only to be used at ipc/msg/msg.c)
    rt_whoami()->msg = mem->error_errno;
    
    //If some one was waiting for available workers...
    rt_sem_signal(fun_call_synch_sem);
    //Free the function memory
    real_free(mem);
    
    return i;        
}

const int *get_worker_threads_ids(void){
    return worker_threads_ids;
}

const unsigned long *get_worker_threads_counter(void){
    return worker_threads_counter;
}

time_t get_fast_time(void){
    return *time_memory;
}

const unsigned long long *get_worker_threads_time(void){
    return worker_threads_time_use;
}

static int fcomm_init_module(void){
    char sem_name[6];
    int i;
    rt_printk("Entering FCOMM Module\n");        

    rt_printk("Using RTAI_max_jitter = %d, RTAI_min_jitter = %d\n",RTAI_max_jitter,RTAI_min_jitter);
   
    for(i=0; i<MAX_FN_NUMBER; i++) {
        call_remote_function_stats[i] = 0;
    }

    if(init_proc_output() != 0)
        return -1;
    
    rt_set_oneshot_mode();
    start_rt_timer(0);

    //Needed to allocate shared memory in runtime (inside tasks)
    //(we can use one of our own, but in that case we must use the heap shm functions. 
    //The 2M value can be changed in the RTAI configuration)
    heap = rt_global_heap_open();//rt_heap_open(SHARED_HEAP_NAME, SHARED_HEAP_SIZE, USE_VMALLOC);

    num2nam(FUN_CALLER_SEM_USER, sem_name);
    if(!(fun_caller_user_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;    
    num2nam(FUN_CALLER_SEM_KERNEL, sem_name);
    if(!(fun_caller_kernel_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    num2nam(FUN_RETURN_SEM_KERNEL, sem_name);
    if(!(fun_return_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;    
    num2nam(MEM_BLOCK_SEM_KERNEL, sem_name);
    if(!(fun_caller_mem_block_sem = rt_typed_named_sem_init(sem_name, 1, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    rt_sem_signal(fun_caller_mem_block_sem);
    num2nam(MEM_ALLOC_SEM, sem_name);
    if(!(mem_alloc_sem = rt_typed_named_sem_init(sem_name, 1, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    rt_sem_signal(mem_alloc_sem);
    num2nam(MEM_ALLOC_SYNCH_SEM_KERNEL, sem_name);
    if(!(mem_alloc_synch_sem = rt_typed_named_sem_init(sem_name, 1, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    rt_sem_signal(mem_alloc_synch_sem);
    num2nam(CALL_FUNCTION_SEM, sem_name);
    if(!(fun_call_synch_sem = rt_typed_named_sem_init(sem_name, 1, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    rt_sem_signal(fun_call_synch_sem);
    num2nam(CALL_FUNCTION_MEM_INC_SEM, sem_name);
    if(!(fun_call_mem_inc_sem = rt_typed_named_sem_init(sem_name, 1, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    rt_sem_signal(fun_call_mem_inc_sem);
    num2nam(USER_SPACE_INIT_FINISH_SEM, sem_name);
    if(!(user_space_init_finish_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    
    if(!(remote_fun_call_id_shm = (unsigned long *)rt_shm_alloc(FUN_CALL_ID_SHM, sizeof(unsigned long), USE_VMALLOC)))
        return ERROR_SHM;    
    if(!(worker_threads_ids = (int *)rt_shm_alloc(WORKER_THREADS_IDS_SHM, MAX_WORKER_THREADS * sizeof(int), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(worker_threads_counter = (unsigned long *)rt_shm_alloc(WORKER_THREADS_COUNTER_SHM, MAX_WORKER_THREADS * sizeof(unsigned long), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(time_memory = (time_t *)rt_shm_alloc(FAST_TIME_SHM, sizeof(time_t), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(worker_threads_time_use = (unsigned long long *)rt_shm_alloc(WORKER_THREADS_TIME_USE_SHM, MAX_WORKER_THREADS * sizeof(unsigned long long), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(fun_call_synch_mem = (int *)rt_shm_alloc(FUN_CALL_SYNCH_SHM, sizeof(int), USE_VMALLOC)))
        return ERROR_SHM;
        
    *remote_fun_call_id_shm = 0;
    
    *fun_call_synch_mem = MAX_WORKER_THREADS - 1;
    
    //Install trap handler
    RT_SET_RTAI_TRAP_HANDLER(trap_handler);
    // Start processor occupation measurement thread(s)
    num_of_cpus = num_online_cpus();
    /*if (num_of_cpus < 1) {
        rt_printk("FCOMM: No or invalid num_of_cpus module parameter specified. CPU occupation measurement not started!\n");
    }else{
        int i;
        rt_printk("FCOMM: Using %d processor!\n",num_of_cpus);
        for (i=1;i<num_of_cpus;i++){
	    int ret;
	    rt_printk("FCOMM: Starting occupation thread %d on processor %x\n",i,1<<i);
	    keepAlive[i] = 1;
	    proc_occup[i] = 0;

        ret = rt_task_init(&occupation_tasks[i],processor_occupation_task,(long int)i,32000,RT_SCHED_LINUX_PRIORITY-1,0,NULL);
	    rt_set_runnable_on_cpus(&occupation_tasks[i],1<<i);
	    ret = rt_task_resume(&occupation_tasks[i]);
	    if (ret != 0) rt_printk("FCOMM: Could not start occupation_task[%d]!\n",i);
        
        ret = rt_task_init(&jitter_measurament_task, jitter_measurament, 0, 32000, 0, 0, NULL);
	    rt_set_runnable_on_cpus(&jitter_measurament_task, 2);
	    ret = rt_task_resume(&jitter_measurament_task);
	    if (ret != 0) rt_printk("FCOMM: Could not start jitter_measurament_task[%d]!\n",i);
	}
    }*/
    return 0;
}

static void fcomm_exit_module(void){
    int i;

    rt_printk("Exiting FCOMM Module\n");
    close_proc_output();
   
    
    for (i=0;i<num_of_cpus;i++){
        keepAlive[i] = 0;
    }
    
    msleep(2000);
    stop_rt_timer();
/*
    for (i=1;i<num_of_cpus;i++){
        rt_task_delete(&occupation_tasks[i]);
    }
*/  
    //rt_task_delete(&jitter_measurament_task);
     
    rt_named_sem_delete(fun_caller_user_sem);
    rt_named_sem_delete(fun_caller_kernel_sem);
    rt_named_sem_delete(fun_return_sem);
    rt_named_sem_delete(mem_alloc_synch_sem);
    rt_named_sem_delete(fun_caller_mem_block_sem);
    rt_named_sem_delete(mem_alloc_sem);
    rt_named_sem_delete(fun_call_mem_inc_sem);    
    rt_named_sem_delete(fun_call_synch_sem);
    rt_named_sem_delete(user_space_init_finish_sem);

    rtai_kfree(FUN_CALL_ID_SHM); 
    rtai_kfree(WORKER_THREADS_IDS_SHM);
    rtai_kfree(WORKER_THREADS_COUNTER_SHM);
    rtai_kfree(FAST_TIME_SHM);
    rtai_kfree(WORKER_THREADS_TIME_USE_SHM);
    rtai_kfree(FUN_CALL_SYNCH_SHM);    
    
    rt_global_heap_close();   
}

module_init(fcomm_init_module);
module_exit(fcomm_exit_module);

