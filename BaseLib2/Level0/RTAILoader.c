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
 * Utility loader for all the RTAI modules
 */

#ifdef _RTAI
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <rtai_sched.h>
#include <rtai_shm.h>
#include "../../OSFiles/rtai/C++Sup/global_obj_support.h"
#include "SystemRTAI.h"

#ifdef _USES_MAIN_
extern int main(int argc, char** argv);
static RT_TASK call_main_task;
static int main_called = 0;

int rt_cpu_mask = DEFAULT_RT_CPU_MASK;
module_param(rt_cpu_mask, int, 0);
MODULE_PARM_DESC(rt_cpu_mask, "CPUS where RT tasks are allowed to run");

int thread_stack_size = RTAI_THREADS_DEFAULT_STACKSIZE;
module_param(thread_stack_size, int, 0);
MODULE_PARM_DESC(thread_stack_size, "The start up thread stack size");

struct main_params {
	int nOfParams;
	int Parameters[9];
} params;

char * filename = "RTAIMain";

static void call_main(long unused){

    main(params.nOfParams, params.Parameters);
}

int RTAIMain(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9){   
    int tmp=0;
    int i = 0;
    
    params.nOfParams = 0;

    rt_printk("Called RTAIMain\n");

    if (p1==INT_MAX) params.nOfParams = 1;
    else if (p2==INT_MAX) params.nOfParams = 2;
    else if (p3==INT_MAX) params.nOfParams = 3;
    else if (p4==INT_MAX) params.nOfParams = 4;
    else if (p5==INT_MAX) params.nOfParams = 5;
    else if (p6==INT_MAX) params.nOfParams = 6;
    else if (p7==INT_MAX) params.nOfParams = 7;
    else if (p8==INT_MAX) params.nOfParams = 8;
    else params.nOfParams = 9;

    params.Parameters[0]=filename;
    params.Parameters[1]=p1;
    params.Parameters[2]=p2;
    params.Parameters[3]=p3;
    params.Parameters[4]=p4;
    params.Parameters[5]=p5;
    params.Parameters[6]=p6;
    params.Parameters[7]=p7;
    params.Parameters[8]=p8;
    
    for (i=1;i<9;i++) {
    	tmp+=params.Parameters[i];
    }

    if (tmp ==0 ) params.nOfParams = 1;
    
    //rt_printk("nOfParams==%d\n",params.nOfParams);

    rt_task_init(&call_main_task, call_main, NULL, thread_stack_size, RTAI_PRIORITY_NORMAL, 1, 0);
    rt_set_runnable_on_cpus(&call_main_task, rt_cpu_mask);
    main_called = 1;
    return rt_task_resume(&call_main_task);
}
#endif

int init_func(void)
{
    //printk("Init RTAILoader ...\n");
    return 0;
}

void exit_func(void)
{
#ifdef _USES_MAIN_    
    if(main_called == 1){
        rt_task_delete(&call_main_task);
    }
    //printk("Exiting RTAI Loader\n");
#endif
}

module_init(init_func);
module_exit(exit_func);
#endif

