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
#include <rtai_sem.h>
#include <rtai_shm.h>
#include <rtai_registry.h>
#include <pthread.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../share/fcomm_share.h"
#include "../../../../Level0/SystemRTAI.h"
#include "api.h"
#include "fcomm_ipc.h"

//Semaphore used to wait for function returns
static SEM *fun_return_sem;

//Semaphore used to warn the user space to perform the work
static SEM *fun_caller_user_sem;

//Semaphore used to warn the kernel space that the job has started
static SEM *fun_caller_kernel_sem;

//Semaphore for the function memory uid allocation synch in the user space
static SEM *mem_alloc_synch_sem;

//Waits for available workers in user space
static SEM *fun_call_synch_sem;

//Protects the available workers counter
static SEM *fun_call_mem_inc_sem;

//This semaphore is signalled when the user space has finished initialization
static SEM *user_space_init_finish_sem;

//Shared memory which stores the id of the function to call
unsigned long *remote_fun_call_id_shm;

//Sem inited ok?
static int sem_init_ok = 0;

//Shm inited ok?
static int shm_init_ok = 0;

//Responsible for calling the functions
static void *function_executor_thread(void *);

//updates the time memory
static void *time_update_thread(void *);

//This task is needed for the small part of lxrt used
static RT_TASK *main_task;

//The running flag
static int running = 0;

//The proc counter
static int proc_uid_counter = MIN_PROC_UID;

//The cpu mask to put the task on specific cpu(s)
// 1: on cpu 0 only, 2: on cpu 1 only, 3: on any;
static int user_cpu_mask = 1;

//Shared memory which stores the ids of the worker threads
static int *worker_threads_ids;

//Shared memory which stores the usage counts of the worker threads
static unsigned long *worker_threads_counter;

//Shared memory which is updated every second with the current time
static time_t *time_memory;

//Shared memory which stores the usage time of the worker threads
static unsigned long long *worker_threads_time_use;

//Shared memory which stores the number of workers in use in user space
static int *fun_call_synch_mem;

static unsigned long memory_current_uid = MAX_SHM_UID / 2;

//sched policy
#ifdef _ENABLE_ROUND_ROBIN_
int sched_policy = RT_SCHED_RR;
#else
int sched_policy = SCHED_FIFO;
#endif

/**Enum to string*/
#define FUN_SOLVE(x)    #x
const char *function_names[] =
{
	FUNCTION_LIST
};
#undef FUN_SOLVE

/**The debug socket*/
static int                  debugConnectionSocket = 0;
static struct sockaddr_in   loggerAddress;
int                         loggerVerboseLevel    = 0;

int initDebugSocket(const char *remoteLoggerAddr, int remoteLoggerPort, int level){
    int iaddr = 0;
    debugConnectionSocket = socket(PF_INET,SOCK_DGRAM,0);
    if (debugConnectionSocket < 0){
        printf("Could not init debug socket!");
        return -1;
    }
    
    iaddr = inet_addr((char *)remoteLoggerAddr);
    if (iaddr != 0xFFFFFFFF){
        loggerAddress.sin_addr.s_addr = iaddr;
    }
    else{
        struct hostent *h = gethostbyname(remoteLoggerAddr);
        if (h == NULL){
            loggerAddress.sin_addr.s_addr = *((int *)(h->h_addr_list[0]));
        }
        else{
            printf("Could not find the remote address: %s\n", remoteLoggerAddr);
            return -1;
        }
    }
    loggerAddress.sin_family = AF_INET;
    loggerAddress.sin_port = htons(remoteLoggerPort);
        
    loggerVerboseLevel = level;
    return 0;
}

void closeDebugSocket(void){
    close(debugConnectionSocket);
}

static void sendUDPMessage(const char *message){
    sendto(debugConnectionSocket, message, strlen(message), 0, (struct sockaddr *)&loggerAddress, sizeof(loggerAddress));
}

void logUDPStandardMessage(int errorLevel, char *msg){
    char message[1024];
    int currentTime = time(NULL);
    sprintf(message, "|TM=%x|P=%x|T=0|E=1|EX=1|D=%s|", currentTime, getpid(), msg);
    sendUDPMessage(message);
}

static void logUDPFunctionCall(int function, int taskID){
    char    message[1024];
    int     currentTime = time(NULL);
    char    taskName[24];
    sprintf(taskName, "Pool %d", taskID);
    sprintf(message, "|TM=%x|P=%x|T=%x|N=%s|E=1|EX=1|D=Function %s called|", currentTime, getpid(), taskID, taskName, function_names[function]);
    sendUDPMessage(message);
}

int get_rt_free_uid(void)
{        
    unsigned long mem_uid = 0;
    rt_sem_wait(mem_alloc_synch_sem);
    memory_current_uid++;
    while(rt_get_adr(memory_current_uid) != 0)
    {
        if(memory_current_uid == MAX_SHM_UID)
            memory_current_uid = MIN_SHM_UID;
        memory_current_uid++;        
    }

    mem_uid = memory_current_uid;
    rt_sem_signal(mem_alloc_synch_sem);
    return mem_uid;
}

int init_shm()
{
    char sem_name[6];
    int c = 0;

    if(loggerVerboseLevel > VERBOSE_ALL){
        logUDPStandardMessage(1, "Starting FCOMM user space");
    }
    if (!(main_task = rt_task_init_schmod(MAIN_TASK_UID, 0, 0, 0, SCHED_FIFO, user_cpu_mask)))
        return ERROR_MAIN_TASK;    
    //Open the global heap
    //(we can use one of our own, but in that case we must use the heap shm functions. 
    //The 2M value can be changed in the RTAI configuration)
    rt_global_heap_open();
    
    num2nam(FUN_CALLER_SEM_USER, sem_name);
    if(!(fun_caller_user_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    
    num2nam(FUN_CALLER_SEM_KERNEL, sem_name);
    if(!(fun_caller_kernel_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;
    
    num2nam(FUN_RETURN_SEM_KERNEL, sem_name);
    if(!(fun_return_sem = rt_typed_named_sem_init(sem_name, 0, BIN_SEM | PRIO_Q)))
        return ERROR_SEM;    
    
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
            
    sem_init_ok = 1;
    
    if(!(remote_fun_call_id_shm = (unsigned long *)rt_shm_alloc(FUN_CALL_ID_SHM, 0, USE_VMALLOC)))
        return ERROR_SHM;
    if(!(worker_threads_ids = (int *)rt_shm_alloc(WORKER_THREADS_IDS_SHM, 0, USE_VMALLOC)))
        return ERROR_SHM;
    if(!(worker_threads_counter = (unsigned long *)rt_shm_alloc(WORKER_THREADS_COUNTER_SHM, MAX_WORKER_THREADS * sizeof(unsigned long), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(time_memory = (time_t *)rt_shm_alloc(FAST_TIME_SHM, sizeof(time_t), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(worker_threads_time_use = (unsigned long long*)rt_shm_alloc(WORKER_THREADS_TIME_USE_SHM, MAX_WORKER_THREADS * sizeof(unsigned long long), USE_VMALLOC)))
        return ERROR_SHM;
    if(!(fun_call_synch_mem = (int *)rt_shm_alloc(FUN_CALL_SYNCH_SHM, sizeof(int), USE_VMALLOC)))
        return ERROR_SHM;
    
    init_api();
    
    shm_init_ok = 1;
    running = 1;
    pthread_t threads[MAX_WORKER_THREADS + 1];
    for(c=0; c < MAX_WORKER_THREADS; c++)
        pthread_create(&threads[c], NULL, function_executor_thread, (void *)c);
    
    pthread_create(&threads[MAX_WORKER_THREADS], NULL, time_update_thread, NULL);

    rt_sem_broadcast(user_space_init_finish_sem);
    
    return 0;
}

void clean_close_shm(void) 
{    
    running = 0;
    close_api();
    
    if(sem_init_ok)
    {
        rt_named_sem_delete(fun_caller_user_sem);
        rt_named_sem_delete(fun_caller_kernel_sem);
        rt_named_sem_delete(fun_return_sem);
        rt_named_sem_delete(mem_alloc_synch_sem);
        rt_named_sem_delete(fun_call_synch_sem);
        rt_named_sem_delete(fun_call_mem_inc_sem);
        rt_named_sem_delete(user_space_init_finish_sem);        
    }
    
    if(shm_init_ok)
    {
        rt_shm_free(FUN_CALL_ID_SHM);
        rt_shm_free(WORKER_THREADS_IDS_SHM);
        rt_shm_free(WORKER_THREADS_COUNTER_SHM);
        rt_shm_free(FAST_TIME_SHM);
        rt_shm_free(WORKER_THREADS_TIME_USE_SHM);        
    }
    
    rt_task_delete(main_task);    
        
    rt_global_heap_close();
}

//Time update thread
static void *time_update_thread(void* args){
    while(running){
    	*time_memory = time(NULL);
	sleep(1);
    }

    return NULL;
}

//thread pool
static void* function_executor_thread(void* args) 
{
    volatile unsigned long fun_mem_id = 0;    
    int idx = (int)args;
    unsigned long ret = 0;
    mem_function *mem;
    void *heap;
    RT_TASK *fun_exec_task;
    RTIME start = 0;
    
    worker_threads_ids[idx] = proc_uid_counter;
    if (!(fun_exec_task = rt_task_init_schmod(proc_uid_counter++, 0, 0, 0, sched_policy, user_cpu_mask)))
        return NULL;
           
    //open the heap
    heap = rt_global_heap_open();    
    while(running) 
    {                  
        //wait to see if the semaphore is free. the kernel will wake it when there is something to write
        rt_sem_wait(fun_caller_user_sem);
        //OK received request from kernel... 
        rt_sem_wait(fun_call_mem_inc_sem);
        //Decrement the worker counter
        (*fun_call_synch_mem)--;
        rt_sem_signal(fun_call_mem_inc_sem);
        
        //get the memory id and reset it
        fun_mem_id = *remote_fun_call_id_shm;
        *remote_fun_call_id_shm = 0;
        if(fun_mem_id == 0)
        {
            if(loggerVerboseLevel > VERBOSE_ALL){
                char message[128];
                sprintf(message, "MEMORY IS NULL... while calling a function! Thread pool id = %d\n", idx);
                logUDPStandardMessage(-1, message);
            }            
            rt_sem_wait(fun_call_mem_inc_sem);
            (*fun_call_synch_mem)++;
            rt_sem_signal(fun_call_mem_inc_sem);
            continue;                       
        }
        start = rt_get_cpu_time_ns();
        worker_threads_counter[idx]++;        
        mem = (mem_function *)rt_named_malloc(fun_mem_id, 0);
        //Warn the kernel, that we already have the information!
        rt_sem_signal(fun_caller_kernel_sem);        
        if(mem != 0)
        {
            if(loggerVerboseLevel > VERBOSE_ALL){
                logUDPFunctionCall(mem->fun_id, idx);
            }
            ret = gen_function_table[mem->fun_id](mem);
            //set the answer
            mem->return_value = ret;    
            mem->finished = 1;
            mem->error_errno = errno;            
        }
        else{
            if(loggerVerboseLevel > VERBOSE_ALL){
                char message[128];
                sprintf(message, "MEMORY IS NULL ... THREAD %d -     fun_mem_id=%ld \n", idx, fun_mem_id);
                logUDPStandardMessage(-1, message);
            }            
        }
        worker_threads_time_use[idx] += (rt_get_cpu_time_ns() - start)/1000000L;
        rt_sem_wait(fun_call_mem_inc_sem);
        (*fun_call_synch_mem)++;
        rt_sem_signal(fun_call_mem_inc_sem);
        //send the answer
        rt_sem_broadcast(fun_return_sem);
        //This worker thread has finished
        rt_sem_signal(fun_call_synch_sem);        
    }    
    
    //close the heap    
    rt_global_heap_close();
    rt_task_delete(fun_exec_task);
    
    return NULL;
}
