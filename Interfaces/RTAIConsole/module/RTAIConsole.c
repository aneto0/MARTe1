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
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/pci.h>


#include <asm/uaccess.h>
#include <asm/io.h>

#include <rtai_sched.h>
#include <rtai_sem.h>

#include "RTAIConsole.h"

#define NAME			"RTAIConsole"

MODULE_AUTHOR("Fabio Piccolo, Andre Neto");
MODULE_DESCRIPTION("RTAI Console Driver");
MODULE_LICENSE("GPL");

static int RTAIConsoleMajor;
static int RTAIConsoleMinor;
module_param(RTAIConsoleMajor, int, 0);
MODULE_PARM_DESC(RTAIConsoleMajor, "Major device number");

static int RTAIConsoleDeviceNumber;
module_param(RTAIConsoleDeviceNumber, int, 1);
MODULE_PARM_DESC(RTAIConsoleDeviceNumber, "Number of devices");

static int          *functionProtoParameters;
static struct cdev  RTAIConsoleCharDev;
static int          functionReturnResult = 0;

//Was the timer running before starting the console?
static int wasTimerRunning = 0;

//This semaphore will synchronize the task function executor
static SEM functionExecutorSem;

//This semaphore will synchronize the task function executor
static SEM *functionCallerSem;

//The function executor pool
static RT_TASK functionExecutorTask;

typedef int(*ExecutorFunctionPrototype) (int, int, int, int, int, int, int, int);

//The function executor
int FunctionExecutor(int args){
    unsigned int                functionPointer = 0;
    int                         *fPA            = 0;
    unsigned int                nParameters     = 0;
	ExecutorFunctionPrototype   function        = 0;
    int i = 0;
    
    while(1){
        rt_sem_wait(&functionExecutorSem);
        functionPointer = (unsigned int)functionProtoParameters[0];
        nParameters     = (unsigned int)functionProtoParameters[1];
        		        
        //Execute the jobfunctionProtoParameters
        fPA      = functionProtoParameters;
        function = (ExecutorFunctionPrototype)functionPointer;

        for(i=RTAIFunctionNumberOfParameters - 1; i>(2 + nParameters - 1); i--)
        {
            //This is really to protect faulty vargs implementations (like printk)
            //which can't receive 0 values in the arguments list (NULL pointers)
            if(nParameters > 0)
                fPA[i] = INT_MAX; //fPA[2];
            else
                fPA[i] = 0;
        }
    
        //for(i=0; i<RTAIFunctionNumberOfParameters; i++)
        //    rt_printk("fPA[%d] = %x\n", i, fPA[i]);
    
        functionReturnResult = function(fPA[2],fPA[3],fPA[4],fPA[5],fPA[6],fPA[7],fPA[8],fPA[9]);        
                
        //Task completed
        rt_sem_signal(functionCallerSem);
    }
    
    return 0;
}

/* Declaration of memory.c functions */
int RTAIConsoleOpen(struct inode *inode, struct file *filp){
	/* the mask says which pins are usable by this driver */
#ifdef _RTAI_VERBOSE
	printk("RTAIConsoleOpen\n");
#endif
	return 0;
}

int RTAIConsoleRelease(struct inode *inode, struct file *filp){
#ifdef _RTAI_VERBOSE
	printk("RTAIConsoleRelease\n");
#endif
	return 0;
}

ssize_t RTAIConsoleRead(struct file *filp, char *buf, size_t count, loff_t *f_pos){
	printk("RTAIConsoleRead: \n");	
	return 1;
}

static int CopyToKernel(unsigned long argp){
    int copyToKernelArgs[2];
    int err = copy_from_user(copyToKernelArgs, (const void __user*)argp, sizeof(int) * 2);
    void *allocatedMemory = 0;
	if(err != 0){
		printk("In CopyToKernel: Failed copying data from user space: err = %d\n", err);
		return -1;
	}
    if(copyToKernelArgs[1] < 1){
		printk("In CopyToKernel: Wrong arguments. copyToKernelArgs[0] = %d copyToKernelArgs[1] = %d\n", copyToKernelArgs[0], copyToKernelArgs[1]);
		return -1;
	}
    
    allocatedMemory = vmalloc(copyToKernelArgs[1]);
    if(allocatedMemory == 0){
		printk("In CopyToKernel: Failed to allocate %d bytes in kernel space\n", copyToKernelArgs[1]);
		return -1;
	}
    
    err = copy_from_user(allocatedMemory, (const void __user*)copyToKernelArgs[0], copyToKernelArgs[1]);
    if(err != 0){
		printk("In CopyToKernel: Failed copying the parameter datta from user space:  copyToKernelArgs[0] = %d copyToKernelArgs[1] = %d\n", copyToKernelArgs[0], copyToKernelArgs[1]);
		return -1;
	}
    
    rt_printk("Allocated memory address is %p\n", (int)allocatedMemory);
    rt_printk("The string is ]%s[\n",allocatedMemory);
    return (int)allocatedMemory;
}

static int CopyFromKernel(unsigned long argp){
    int copyFromKernelArgs[3];
    int err = copy_from_user(copyFromKernelArgs, (const void __user*)argp, sizeof(int) * 3);
    if(err != 0){
		printk("In CopyFromKernel: Failed copying data from user space: err = %d\n", err);
		return -1;
	}
    
    if(copyFromKernelArgs[0] < 1 || copyFromKernelArgs[1] < 1 || copyFromKernelArgs[2] < 1){
		printk("Wrong arguments for CopyToKernel. copyFromKernelArgs[0] = %d copyFromKernelArgs[1] = %d copyFromKernelArgs[2] = %d\n", copyFromKernelArgs[0], copyFromKernelArgs[1], copyFromKernelArgs[2]);
		return -1;
	}
        
    return copy_to_user((void __user*)copyFromKernelArgs[0], (void *)copyFromKernelArgs[1], copyFromKernelArgs[2]);
}

static int CallFunction(unsigned long argp){
    /* Copy the user parameters in kernel space*/
	int err = copy_from_user(functionProtoParameters, (const void __user*)argp, sizeof(int) * RTAIFunctionNumberOfParameters);
	if(err != 0){
		printk("CallFunction: Failed copying data from user space: err = %d\n", err);
        //Task completed warn user space
        rt_sem_signal(functionCallerSem);
		return -1;
	}
	
    if(functionProtoParameters[0] == 0){
        printk("CallFunction: Function Pointer is NULL\n");
        //Task completed warn user space
        rt_sem_signal(functionCallerSem);
        return -1;
    }	              
		
    err = rt_sem_signal(&functionExecutorSem);
#ifdef _RTAI_VERBOSE
    printk("CallFunction: finished\n");
#endif
    return err;
}

static void FreeKernelMemory(unsigned long argp){
    int freeKernelMemArgs[1];
    int err = copy_from_user(freeKernelMemArgs, (const void __user*)argp, sizeof(int));
    if(err != 0){
		printk("In FreeKernelMemory: Failed copying data from user space: err = %d\n", err);
        return;
	}
    
    vfree((void *)freeKernelMemArgs[1]);
}

int RTAIConsoleIoctl(struct inode *inode, struct file *filp, unsigned int fun, unsigned long argp){
	
	if(_IOC_TYPE(fun) != RTAICONSOLE_IOCTL_MAGIC)
        return -ENOTTY;
	if(!access_ok(VERIFY_WRITE, (void __user*)argp, _IOC_SIZE(fun) ))
        return -EFAULT;
	
    switch(fun)
    {
        case RTAICopyToKernel:
            return CopyToKernel(argp);
        case RTAICopyFromKernel:
            return CopyFromKernel(argp);
        case RTAIFunctionCall:
#ifdef _RTAI_VERBOSE
	    printk("IOCTL: CallFunction\n");
#endif
            return CallFunction(argp);
        case RTAIFreeKernelMemory:
            FreeKernelMemory(argp);
            return 0;
        default:
            return -ENOTTY;
    }            	
    
    return -ENOTTY;	
}


static ssize_t RTAIConsoleWrite(struct file *filp, const char *buf, size_t count, loff_t *f_pos){
	printk("RTAIConsoleWrite: \n");	
	return count;
}

static void RTAIConsoleExit(void);
static int  RTAIConsoleInit(void);

/* Structure that declares the usual file */
/* access functions */
static const struct file_operations  RTAIConsoleFileOps = {
  read:     RTAIConsoleRead,
  write:    RTAIConsoleWrite,
  ioctl:    RTAIConsoleIoctl,
  open:     RTAIConsoleOpen,
  release:  RTAIConsoleRelease
};


static int RTAIConsoleInit(void)
{
    int     result = 0;
    int     i = 0;
    dev_t	dev_id;
    char semName[6];
    
    /* Allocate Device */	
	if (RTAIConsoleMajor) {
		dev_id = MKDEV(RTAIConsoleMajor, 0);
		result = register_chrdev_region(dev_id, 1,NAME);
	} else {
		result = alloc_chrdev_region(&dev_id,0, 1,NAME);
		RTAIConsoleMajor = MAJOR(dev_id);
		RTAIConsoleMinor = MINOR(dev_id);
	}

	if (result < 0) {
		printk("%s: alloc_chrdev_region failed\n", NAME);
		return -1;
	}

    functionProtoParameters = kmalloc(RTAIFunctionNumberOfParameters * sizeof(int), GFP_KERNEL);
    if(!functionProtoParameters){
    	printk("%s: Failed allocating memory\n",NAME);
    	goto initError;
    }
    memset(functionProtoParameters, 0, RTAIFunctionNumberOfParameters * sizeof(int));
       
	cdev_init(&RTAIConsoleCharDev, &RTAIConsoleFileOps);
	RTAIConsoleCharDev.owner = THIS_MODULE;
	RTAIConsoleCharDev.ops      = &RTAIConsoleFileOps;
	result = cdev_add(&RTAIConsoleCharDev, dev_id, 1); 
	if( result < 0){
		printk("%s: cdev_add failed: Major %d Minor %d \n", NAME, RTAIConsoleMajor, RTAIConsoleMinor);
		return -1;
	}

	printk("%s: Initialized: Major %d Minor %d\n", NAME, RTAIConsoleMajor, RTAIConsoleMinor);
            
    rt_typed_sem_init(&functionExecutorSem, 0, BIN_SEM | PRIO_Q);
        
    num2nam(FUN_CALLER_SEM_UID, semName);
    functionCallerSem = rt_typed_named_sem_init(semName, 0, BIN_SEM | PRIO_Q);        
    if(&functionExecutorSem == 0 || functionCallerSem == 0){
        printk("Could not create the semaphores!\n");
        goto initError;
    }
    
    if(rt_is_hard_timer_running())
        wasTimerRunning = 1;
    else
        start_rt_timer(0);        
     
    //Launch the RT Task
    result = rt_task_init(&functionExecutorTask, FunctionExecutor, i, 65536, 0x1FFFFF, 1, 0);
    if(result != 0){
        printk("Could not init RT executor task error = %d\n", result);
        goto initError;
    }
    rt_set_runnable_on_cpus(&functionExecutorTask, 2);
    rt_task_resume(&functionExecutorTask);
    if(result != 0){
        printk("Could not resume task %d\n", i);
        goto initError;
    }    
    
    return 0;
    
initError:
    RTAIConsoleExit();
    return -1;
}

static void RTAIConsoleExit(void)
{
    dev_t dev_id;
	dev_id = MKDEV(RTAIConsoleMajor, RTAIConsoleMinor);
	unregister_chrdev_region(dev_id, 1);
	cdev_del(&RTAIConsoleCharDev);
	if(functionProtoParameters) 
        kfree(functionProtoParameters);    

    //if it was the console to start the timer
    if(!wasTimerRunning)
        stop_rt_timer();
        
    rt_task_delete(&functionExecutorTask);
        
    rt_sem_delete(&functionExecutorSem);
    rt_sem_delete(functionCallerSem);    
    
    printk(KERN_ALERT "RTAI Console exiting %d %d\n", RTAIConsoleMajor, RTAIConsoleDeviceNumber);
}

module_init(RTAIConsoleInit);
module_exit(RTAIConsoleExit);
