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
#ifndef FCOMM_MOD_H
#define	FCOMM_MOD_H

#include "linkage.h"

#define DEFAULT_RTAI_MAX_JITTER 0
#define DEFAULT_RTAI_MIN_JITTER 0

#if defined(__cplusplus)
extern "C"
{
#endif
/*
 * Copies a parameter to the shared memory. Returns an unique identifier. If to_copy is null, return the NULL_PARAMETER_ID
 * @param size The size of the memory to allocate
 * @param to_copy Pointer to the value which is to be copied
 * @return The memory unique identifier on success. ERROR_SHM if there was a problem allocating memory and NULL_PARAMETER_ID if one wants to pass a pseudo null value
 */
    unsigned long get_parameter(int size, void *to_copy);
/*
 * Allocates a block of shared memory and returns an unique identifier. Useful to pass parameters by reference (like buffers).
 * @param size The size of the memory to allocate
 * @return The memory unique identifier on success. ERROR_SHM if there was a problem allocating memory and NULL_PARAMETER_ID if one wants to pass a pseudo null value
 */
    unsigned long get_parameter_no_copy(int size);
/*
 * Copies a string to the shared memory. Returns an unique identifier. If to_copy is null, return the NULL_PARAMETER_ID
 * @param to_copy Pointer to the string which is to be copied
 * @return The memory unique identifier on success. ERROR_SHM if there was a problem allocating memory and NULL_PARAMETER_ID if one wants to pass a pseudo null value*/
    unsigned long get_str_parameter(char *to_copy);

/*
 * Calls a remote function identified by a unique number.
 * @param fun_id The function unique identifier.
 * @param par_ids An array with all the memory identifiers for each of the requested parameters.
 * @param par_size The number of parameters.
 * @return If no errors occur it returns whatever value the user-space function returned (in case it is a function which returns void, it returns 0). Returns ERROR_SHM if there was a problem allocating memory
 */
    int call_remote_function(int fun_id, unsigned long *par_ids, int par_size);

/*
 * Returns an available unique identifier in the real-time space. This function is multi-thread safe.
 * @return An unique number in the real-time space
 */
    int get_rt_free_uid(void);

/*
 * Frees all the parameters memory from the shared heap.
 * @param id An array with all the parameter identifiers.
 * @param par_size The number of parameters.
 */
    void free_parameters(unsigned long *id, int par_size);

/*
 * Frees a parameter memory from the shared heap.
 * @param id The parameter unique identifier.
 */
    void free_parameter(unsigned long id);

/*
 * Frees a parameter memory from the shared heap.
 * @param par The parameter memory address.
 */
    void free_parameter_from_ptr(void *par);

/*
 * Returns the ids of the worker threads.
 * @return the user-space worker threads identifiers.
 */
    const int *get_worker_threads_ids(void);

/*
 * The number of worker threads available (not the number of workers available!)
 * @return The number of worker threads which were declared
 */
    const unsigned long *get_worker_threads_counter(void);

/*
 * An array with working time for each worker thread
 * @return The amount of time each worker thread has used.
 */
    const unsigned long long *get_worker_threads_time(void);

/**
 * This functions returns the time which is updated by a user space function
 * @return the time as updated by a user space thread
 */
    time_t get_fast_time(void);

#if defined(__cplusplus)
}
#endif
#endif

