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
#ifndef FCOMM_SHARE_H
#define	FCOMM_SHARE_H

/**
 * Used to pass NULL
 */
#define NULL_PARAMETER_ID -1

/**
 * The maximum number of worker threads in user space
 */
#define MAX_WORKER_THREADS 16

/**
 * The maximum number of parameters
 */
#define MAX_PARAMETERS_CALL 8

/**
 * Error initiating the semaphore
 */
#define ERROR_SEM -7

/**
 * Error initiating the shared memory
 */
#define ERROR_SHM -9

/**
 * Alias for the shared HEAP size
 */
#define SHARED_HEAP_SIZE 0x4000000 //(64 MBs)

/**
 * The shared memory heap name
 */
#define SHARED_HEAP_NAME 4000

/**
 * Alias for the shared semaphore used for calling functions. Synch in kernel space.
 */
#define FUN_CALLER_SEM_KERNEL 5000

/**
 * Alias for the shared semaphore used for calling functions. Synch in user space.
 */
#define FUN_CALLER_SEM_USER 5001

/**
 * Alias for the shared semaphore used for waiting for functions to return. Synch in kernel space.
 */
#define FUN_RETURN_SEM_KERNEL 5002

/**
 * Used for internal synchronization
 */
#define MEM_ALLOC_SYNCH_SEM_KERNEL 5003

/**
 * Used for internal synchronization
 */
#define MEM_BLOCK_SEM_KERNEL 5004

/**
 * Used for internal memory alloc/dealloc synchronization
 */
#define MEM_ALLOC_SEM 5005

/**
 * Waits if no user space workers threads are available
 */
#define CALL_FUNCTION_SEM 5006

/**
 * Protects the user space workers threads counter shared memory
 */
#define CALL_FUNCTION_MEM_INC_SEM 5007

/**
 * Kernel workers semaphore
 */
#define KERNEL_WORKERS_SEM 5008

/**
 * User space initialization finished semaphore
 */
#define USER_SPACE_INIT_FINISH_SEM 5009

/**
 * Alias for the shared memory used to communicate the id of the function to call
 */
#define FUN_CALL_ID_SHM 6000

/**
 * Alias for the shared memory which stores the ids of the worker threads
 */
#define WORKER_THREADS_IDS_SHM 6001

/**
 * Alias for the shared memory which stores the count usage of the worker threads
 */
#define WORKER_THREADS_COUNTER_SHM 6002

/**
 * Alias for the shared memory which stores the time usage of the worker threads
 */
#define WORKER_THREADS_TIME_USE_SHM 6003

/**
 * Alias for the shared memory which stores the number of workers in user space
 */
#define FUN_CALL_SYNCH_SHM 6004

/**
 * Alias for the shared memory which updates the current time
 */
#define FAST_TIME_SHM 6005

/**
 * Used by the user space
 */
#define MAIN_TASK_UID 7000

/**
 * The minimum value that a proc uid may have
 */
#define MIN_PROC_UID 100

/**
 * The minimum value that a shm uid may have
 */
#define MIN_SHM_UID 20000

/**
 * The maximum value that a shm uid may have
 */
#define MAX_SHM_UID UINT_MAX/1000

/*
 * The location of the file with kallsyms
 */
#define KALL_SYMS_LOC "/proc/kallsyms"
/**
 * Function IDS. Enum always start at 0 and ++
 */
#define FUNCTION_LIST                           \
    FUN_SOLVE(FUN_ID_OPEN),                     \
    FUN_SOLVE(FUN_ID_WRITE),                    \
    FUN_SOLVE(FUN_ID_CLOSE),                    \
    FUN_SOLVE(FUN_ID_READ),                     \
    FUN_SOLVE(FUN_ID_SELECT),                   \
    FUN_SOLVE(FUN_ID_DUP),                      \
    FUN_SOLVE(FUN_ID_DUP_2),                    \
    FUN_SOLVE(FUN_ID_PIPE),                     \
    FUN_SOLVE(FUN_ID_SOCKET),                   \
    FUN_SOLVE(FUN_ID_SEND),                     \
    FUN_SOLVE(FUN_ID_SEND_TO),                  \
    FUN_SOLVE(FUN_ID_RECV),                     \
    FUN_SOLVE(FUN_ID_RECV_FROM),                \
    FUN_SOLVE(FUN_ID_FCNTL),                    \
    FUN_SOLVE(FUN_ID_BIND),                     \
    FUN_SOLVE(FUN_ID_LISTEN),                   \
    FUN_SOLVE(FUN_ID_ACCEPT),                   \
    FUN_SOLVE(FUN_ID_GET_SOCK_OPT),             \
    FUN_SOLVE(FUN_ID_SET_SOCK_OPT),             \
    FUN_SOLVE(FUN_ID_RAND),                     \
    FUN_SOLVE(FUN_ID_SRAND),                    \
    FUN_SOLVE(FUN_ID_MKDIR),                    \
    FUN_SOLVE(FUN_ID_RMDIR),                    \
    FUN_SOLVE(FUN_ID_UNLINK),                   \
    FUN_SOLVE(FUN_ID_GET_HOSTNAME),             \
    FUN_SOLVE(FUN_ID_SET_HOSTNAME),             \
    FUN_SOLVE(FUN_ID_GET_DOMAINNAME),           \
    FUN_SOLVE(FUN_ID_SET_DOMAINNAME),           \
    FUN_SOLVE(FUN_ID_FCOMM_GETSERVBYNAME),      \
    FUN_SOLVE(FUN_ID_FCOMM_GETSERVBYPORT),      \
    FUN_SOLVE(FUN_ID_INET_ADDR),                \
    FUN_SOLVE(FUN_ID_INET_ATON),                \
    FUN_SOLVE(FUN_ID_INET_NETWORK),             \
    FUN_SOLVE(FUN_ID_INET_NTOA),                \
    FUN_SOLVE(FUN_ID_INET_LNAOF),               \
    FUN_SOLVE(FUN_ID_INET_NETOF),               \
    FUN_SOLVE(FUN_ID_FSTAT),                    \
    FUN_SOLVE(FUN_ID_LSEEK),                    \
    FUN_SOLVE(FUN_ID_FLOCK),                    \
    FUN_SOLVE(FUN_ID_REMOVE),                   \
    FUN_SOLVE(FUN_ID_GET_PEERNAME),             \
    FUN_SOLVE(FUN_ID_TIME),                     \
    FUN_SOLVE(FUN_ID_CONNECT),                  \
    FUN_SOLVE(FUN_ID_GETCHAR),                  \
    FUN_SOLVE(FUN_ID_OPENDIR),                  \
    FUN_SOLVE(FUN_ID_CLOSEDIR),                 \
    FUN_SOLVE(FUN_ID_READDIR),                  \
    FUN_SOLVE(FUN_ID_STAT),                     \
    FUN_SOLVE(FUN_ID_FOPEN),                    \
    FUN_SOLVE(FUN_ID_FCLOSE),                   \
    FUN_SOLVE(FUN_ID_FREAD),                    \
    FUN_SOLVE(FUN_ID_FWRITE),                   \
    FUN_SOLVE(FUN_ID_FTELL),                    \
    FUN_SOLVE(FUN_ID_FSEEK),                    \
    FUN_SOLVE(FUN_ID_CTIME),                    \
    FUN_SOLVE(FUN_ID_FERROR),                   \
    FUN_SOLVE(FUN_ID_FIND_FUNC_BY_NAME),        \
    FUN_SOLVE(FUN_ID_FGETS),                    \
    FUN_SOLVE(FUN_ID_FFLUSH),                   \
    FUN_SOLVE(FUN_ID_SET_SOCKET_BLOCKING),      \
    FUN_SOLVE(FUN_ID_EXEC_BASH),                \
    FUN_SOLVE(FUN_ID_CON_PERFORM_CHAR_INPUT),   \
    FUN_SOLVE(FUN_ID_GET_INTERNETINFO),         \
    FUN_SOLVE(FUN_ID_FCOMM_GETHOSTBYNAME),      \
    FUN_SOLVE(FUN_ID_FCOMM_GETHOSTBYADDR),      \
    FUN_SOLVE(FUN_ID_FTRUNCATE)

#define FUN_SOLVE(x)    x
enum function_ids{
    FUNCTION_LIST
};
#undef FUN_SOLVE

// Should point to the last element of the array!
#define MAX_FN_NUMBER FUN_ID_FTRUNCATE

typedef struct _mem_parameter
{
    unsigned long id;
    void *parameter;
}mem_parameter;

typedef struct _mem_function
{
    int fun_id;
    unsigned long parameter_ids[MAX_PARAMETERS_CALL];
    int return_value;
    int error_errno;
    int finished;
}mem_function;

#define MAX_NUM_PROCESSOR 10
#endif
