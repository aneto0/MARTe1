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
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <asm/uaccess.h>

#include "../module/pcieAdc.h"
#include "pcieAdc_proc.h"

#include <rtai.h>
#include <rtai_sched.h>


//The master boards slot number
static int masterSlotNumber = 3;
module_param(masterSlotNumber, int, 0);
MODULE_PARM_DESC(masterSlotNumber, "The master board slot number");

//The rtm board slot number
static int rtmSlotNumber = 3;
module_param(rtmSlotNumber, int, 0);
MODULE_PARM_DESC(rtmSlotNumber, "The rtm board slot number");

//The cycle time in us
static int cycleTime = 50;
module_param(cycleTime, int, 0);
MODULE_PARM_DESC(cycleTime, "The cycle time in microseconds");

static double HRTFrequency = 2.400065;
static int    CPUFrequency = 2400;
module_param(CPUFrequency, int, 0);
MODULE_PARM_DESC(CPUFrequency, "The cpu frequency");

//The number of installed boards
static int numberOfBoards = 0;
// The slot numbers for each of the board indexes
static int boardSlotNumbers[MAX_BOARDS];
module_param_array(boardSlotNumbers, int, &numberOfBoards, 0);
MODULE_PARM_DESC(rtmSlotNumber, "The rtm board slot number");

//The cpu mask
static int rtCPUMask = 8;
module_param(rtCPUMask, int, 0);
MODULE_PARM_DESC(rtCPUMask, "The cpu mask");

//The RT Task
static RT_TASK acqSimTask;

// Global DMA address
int DMA_global_addr[MAX_BOARDS * DMA_BUFFS];

int BUFFER_LENGTH = 0;

static inline long long int HRTCounter(void) {
    long long int perf;
    unsigned int *pperf = (unsigned int *)&perf;
    asm(
        "rdtsc"
	: "=a"(pperf[0]) , "=d"(pperf[1])
	:
    );
    return perf;
}

//Header counter
unsigned int  counter = 0;

int run = 1;
//Thread which simulates the memory filling
int acqSimThread(int ignored){
    int           i              = 0;    
    int           j              = 0;
    int           ticksTime      = cycleTime * 1000 * HRTFrequency;
    RTIME         start          = 0;
    RTIME         totalSleepTime = 0;
    int          *memPtr;
    int           temp = 0;
    
    printk("The ticks time is = %d\n", ticksTime);
    while(run){
        start = HRTCounter();
        //Update all the headers and footers
        for(i=0; i<numberOfBoards; i++){
            memPtr = (int *)(DMA_global_addr[i * DMA_BUFFS + j]);
            *memPtr = counter;
            *(memPtr + BUFFER_LENGTH + HEADER_LENGTH) = counter;
        }        
        
        j++;
        if(j == DMA_BUFFS){
            j = 0;
        }
        counter += cycleTime;
        //Sleep the cycle time
        rt_sleep(nano2count(5000));
        totalSleepTime = HRTCounter() + ticksTime - (HRTCounter() - start);
        while(HRTCounter() < totalSleepTime);
        
        
        if(counter > 1073741824){
            counter = 0;
        }
    }
    
    run = 1;
    return 0;
}

static int SlotNumberToBoardIndex(int slotNum) {
    int i = 0;
    for (i = 0; i < MAX_BOARDS; i++) {
        if (boardSlotNumbers[i] == slotNum) {
            return i;
        }
    }
    return -1;
}

int *GetBufferAddress(void){
    return (int *) DMA_global_addr;
}

int *GetBoardBufferAddress(int slotNum){
    int boardIdx = SlotNumberToBoardIndex(slotNum);
    if (boardIdx == -1) {
    	printk("Could not find the index for the slot number %d\n", slotNum);
        return NULL;
    }
    return &DMA_global_addr[boardIdx * DMA_BUFFS];
}

int  GetNumberOfBoards(void){
    return numberOfBoards;
}
int  GetMasterBoardSlotNumber(void){
    return masterSlotNumber;
}
int  WriteToDAC(int board, int channel, int value){
    return 0;
}

int WriteToDIO(int slotNum, int channel, int value) {
    return 0;
}

int  EnableATCApcieAcquisition(void){
    return 0;
}

int  DisableATCApcieAcquisition(void){
    return 0;
}

int  SetATCApcieExternalTriggerAndClock(int enabled){
    return 0;
}

int  IsRTMPresent(int slotNum){    
    return slotNum == rtmSlotNumber;
}

int *GetBoardsSlotNumbers(void){
    return boardSlotNumbers;
}

int  SendSoftwareTrigger(void){
    counter = 0;
    return 0;
}

int GetNumberOfInputAnalogChannels(int slotNum){
    return IN_ANALOG_CHANNELS;
}

int GetNumberOfInputDigitalChannels(int slotNum){
    return IN_DIGITAL_CHANNELS;
}

int GetNumberOfAnalogueOutputChannels(int slotNum){
    if(IsRTMPresent(slotNum)){
        return N_DACS_PER_BOARDS;
    }
    return 0;
}

int GetNumberOfDigitalOutputChannels(int slotNum){
    if(IsRTMPresent(slotNum)){
        return N_DIOS_PER_BOARD;
    }
    return 0;
}


int initModule(void) {
    int i = 0;
    int j = 0;
    printk("Starting the ATCA fake driver");
    
    if(numberOfBoards < 1){
        printk("No board module identifier was supplied. Using default of 6\n");
        numberOfBoards      = 6;        
        boardSlotNumbers[0] = 3;
        boardSlotNumbers[1] = 5;
        boardSlotNumbers[2] = 7;
        boardSlotNumbers[3] = 9;        
        boardSlotNumbers[4] = 11;        
        boardSlotNumbers[5] = 15;        
    }
    
    printk("Found boards in the slots: ");
    for(i = 0; i<numberOfBoards; i++){
        printk("%d, ", boardSlotNumbers[i]);
    }
    printk("\n");
    printk("Master is on slot %d\n", masterSlotNumber);
    printk("RTM is on slot %d\n", rtmSlotNumber);
    
    //Check if the master board is present
    if(SlotNumberToBoardIndex(masterSlotNumber) == -1){
        printk("Master board was not specified or was specified on an unexisting slot: %d\n", masterSlotNumber);
        return -1;
    }
    
    //For simplicity let the master be the last board
    i = boardSlotNumbers[numberOfBoards - 1];
    j = SlotNumberToBoardIndex(masterSlotNumber);
    boardSlotNumbers[numberOfBoards - 1] = masterSlotNumber;
    boardSlotNumbers[j] = i;
    
    BUFFER_LENGTH = GetNumberOfInputAnalogChannels(masterSlotNumber) + GetNumberOfInputDigitalChannels(masterSlotNumber); 
    
    //create the virtual memory
    for(i=0; i<numberOfBoards; i++){
        for(j=0; j<DMA_BUFFS; j++){            
            DMA_global_addr[i * DMA_BUFFS + j] = (int)kmalloc(sizeof(int) * (2 * HEADER_LENGTH + BUFFER_LENGTH), GFP_DMA);
            printk("Allocating memory at i=%d j=%d 0x%x\n", i, j, DMA_global_addr[i * DMA_BUFFS + j]);
            if(DMA_global_addr[i * DMA_BUFFS + j] == 0){
                printk("Failed allocating memory i=%d j=%d\n", i, j);
                return -1;
            }
        }
    }
   
 
    HRTFrequency = (double)(CPUFrequency) / 1000.0;
    
    rt_set_oneshot_mode();
    start_rt_timer(0);
    
    rt_task_init(&acqSimTask, acqSimThread, NULL, 32000, RT_SCHED_HIGHEST_PRIORITY, 1, 0);
    rt_set_runnable_on_cpus(&acqSimTask, rtCPUMask);
    printk("Going to start in CPUMask = %d\n", rtCPUMask);
    rt_task_resume(&acqSimTask);
   
 
    return 0;
}

void exitModule(void) {
    int i = 0;
    int j = 0;
    
    run = 0;
    while(run != 1){
        msleep(1);
    }
    stop_rt_timer();
    rt_task_delete(&acqSimTask);
    
    //Free all the memory
    for(i=0; i<numberOfBoards; i++){
        for(j=0; j<DMA_BUFFS; j++){                        
            if(DMA_global_addr[i * DMA_BUFFS + j] != 0){
                printk("Trying to free : 0x%x\n", DMA_global_addr[i * DMA_BUFFS + j]);
                kfree((void *)(DMA_global_addr[i * DMA_BUFFS + j]));                
            }
        }
    }
        
}

module_init(initModule);
module_exit(exitModule);

EXPORT_SYMBOL(IsRTMPresent);
EXPORT_SYMBOL(GetBufferAddress);
EXPORT_SYMBOL(GetBoardBufferAddress);
EXPORT_SYMBOL(GetNumberOfBoards);
EXPORT_SYMBOL(GetNumberOfInputAnalogChannels);
EXPORT_SYMBOL(GetNumberOfInputDigitalChannels);
EXPORT_SYMBOL(GetNumberOfAnalogueOutputChannels);
EXPORT_SYMBOL(GetNumberOfDigitalOutputChannels);
EXPORT_SYMBOL(GetMasterBoardSlotNumber);
EXPORT_SYMBOL(WriteToDAC);
EXPORT_SYMBOL(WriteToDIO);
EXPORT_SYMBOL(EnableATCApcieAcquisition);
EXPORT_SYMBOL(DisableATCApcieAcquisition);
EXPORT_SYMBOL(SetATCApcieExternalTriggerAndClock);
EXPORT_SYMBOL(GetBoardsSlotNumbers);
EXPORT_SYMBOL(SendSoftwareTrigger);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("ATCA PCIe FPGA Endpoint Simulator");
MODULE_AUTHOR("Andre' Neto");
