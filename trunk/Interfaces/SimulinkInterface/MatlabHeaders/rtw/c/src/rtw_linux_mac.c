/* Copyright 2011 The MathWorks, Inc. */
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#ifndef __USE_UNIX98
#define __USE_UNIX98
#endif

extern int   pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);

void rtw_pthread_mutex_init_mac( void** mutexDW )
{
    pthread_mutexattr_t attr;       
    pthread_mutexattr_setprotocol(&attr, 1);
    *(mutexDW) = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init((pthread_mutex_t *)(*(mutexDW)), &attr);
}

void rtw_pthread_sem_create_mac( void** semaphoreDW1, void** semaphoreDW2, long initVal )
{
    static int semcount = 0;
    char* semName = malloc(sizeof(char)*128);
    sprintf(semName, "semaphoreDW%d", semcount++);
    
    *semaphoreDW1 = sem_open(semName, O_CREAT | O_EXCL, 0777, (int)initVal);
    *semaphoreDW2 = (void*)semName;
}

/* LocalWords:  PTHREAD PRIO DW
 */
