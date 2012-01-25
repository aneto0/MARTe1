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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "RTAIConsoleAPI.h"
#include "../module/RTAIConsole.h"
#include <rtai_sem.h>
#include <rtai_shm.h>
#include <rtai_registry.h>

static char *consoleLocation = "/dev/RTAIConsole0";

int CallRemoteFunction(int *remoteFunctionArguments){
    int err     = 0;
    int fd      = open(consoleLocation,O_RDWR);
    int i = 0;
    char semName[6];
    RT_TASK *main_task;
    SEM *functionCallerSem;

    if(remoteFunctionArguments == 0){
        printf("remoteFunctionArguments are NULL\n");
        return -1;
    }
    
    if(fd == -1){
        printf("Failed opening RTAIConsole0 file. \n");
        return -1;
    }
    
    if (!(main_task = rt_task_init_schmod(FUN_CALLER_TASK_UID, 0, 0, 0, SCHED_FIFO, 2))){
            printf("Could not create the RT TASK!\n");
            return -1;
    }
    
    num2nam(FUN_CALLER_SEM_UID, semName);
    functionCallerSem = rt_typed_named_sem_init(semName, 0, BIN_SEM | PRIO_Q);
    if(functionCallerSem == 0){
        printf("Could not create the semaphore!\n");
        return -1;
    }               

    err = ioctl(fd, RTAIFunctionCall, remoteFunctionArguments);
    err = rt_sem_wait(functionCallerSem);

    close(fd);
    rt_task_delete(main_task);
    return err;
}

int CopyToKernel(int address, int size){
    int err     = 0;
    int fd      = open(consoleLocation,O_RDWR);
    int args[2] = {address, size};
    
    err = ioctl(fd, RTAICopyToKernel, args);
    
    close(fd);    
    return err;
}

int CopyFromKernel(int userSpaceAddress, int kernelSpaceAddress, int size){
    int err     = 0;
    int fd      = open(consoleLocation,O_RDWR);
    int args[3] = {userSpaceAddress, kernelSpaceAddress, size};
    
    err = ioctl(fd, RTAICopyFromKernel, args);
    
    close(fd);    
    return err;
}

int FreeKernelMemory(int address){
    int err     = 0;
    int fd      = open(consoleLocation,O_RDWR);
    int args[1] = {address};
    
    err = ioctl(fd, RTAIFreeKernelMemory, args);
    
    close(fd);    
    return err;
}


void SetConsoleLocation(const char *newConsoleLocation){
    consoleLocation = (char *)newConsoleLocation;
}

