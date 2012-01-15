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
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include "pcieAdc.h"
#include "pcieAdc_proc.h"

static struct proc_dir_entry *pcieAdc_proc;
extern struct pci_dev* global_pdev;
extern int *status_register_addr;

//extern int    interrupt_counter;

int pcieAdc_procinfo(char *buf, char **start, off_t fpos, int length, int *eof, void *data) {
    COMMAND_REG commandReg;
    STATUS_REG  statusReg;
    PCIE_DEV *devpriv;
    //u32 *bufferData = NULL;
    int dacValue = 0;
    char *p;
    //int i;
    //int j = 0;
    p = buf;

    /* try to guess if the board is present */
    if (global_pdev == 0) {
        p += sprintf(p, "pcieAdc master board NOT PRESENT\n");
        goto _exit;
    }

  /* get the device information data */
     devpriv = (PCIE_DEV *) pci_get_drvdata(global_pdev);

    
    //Print command register
    commandReg.reg32=PCIE_READ32((void*) &devpriv->pHregs->command);
    p += sprintf(p, "=== Command Register ===\n");
    p += sprintf(p, "--------------------------------------------------------------------------------\n");
	p += sprintf(p, "|rs0|ACQM|DQTP|LOAD|rsv1|CHN|ILVM|rsv2|HOP|TOP|ACQS|ACQT|ACQK|ACQE|STRG|rvs3|DMAF|DMAE|rsv4|ERRiE|DMAiE|ACQiE|\n");
	p += sprintf(p, "--------------------------------------------------------------------------------\n");
	p += sprintf(p, "|%3d|%4d|%4d|%4d|%4d|%3d|%4d|%4d|%3d|%3d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%5d|%5d|%5d|\n",commandReg.cmdFlds.rsv0,
				commandReg.cmdFlds.ACQM, commandReg.cmdFlds.DQTP, commandReg.cmdFlds.LOAD,
				commandReg.cmdFlds.rsv1, commandReg.cmdFlds.CHN, commandReg.cmdFlds.ILVM,
				commandReg.cmdFlds.rsv2, commandReg.cmdFlds.HOP, commandReg.cmdFlds.TOP,
				commandReg.cmdFlds.ACQS, commandReg.cmdFlds.ACQT, commandReg.cmdFlds.ACQK,
				commandReg.cmdFlds.ACQE, commandReg.cmdFlds.STRG, commandReg.cmdFlds.rsv3,
				commandReg.cmdFlds.DMAF, commandReg.cmdFlds.DMAE, commandReg.cmdFlds.rsv4,
				commandReg.cmdFlds.ERRiE, commandReg.cmdFlds.DMAiE, commandReg.cmdFlds.ACQiE);
	p += sprintf(p, "--------------------------------------------------------------------------------\n");

    statusReg.reg32=PCIE_READ32((void*) &devpriv->pHregs->status);
    p += sprintf(p, "=== Status Register ===\n");
    p += sprintf(p, "-----------------------------------------------------------------------\n");
	p += sprintf(p, "| none |master|rtm|slotNum|rsv0|rsv1|FSH|RST|rsv2|ERR1|ERR0|rsv3|FIFE|FIFF|rsv4|DMAC|ACQC|\n");
	p += sprintf(p, "-----------------------------------------------------------------------\n");
	p += sprintf(p, "|%6d|%6d|%3d|%7d|%4d|%4d|%3d|%3d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|%4d|\n", 
				statusReg.statFlds.none, statusReg.statFlds.master, statusReg.statFlds.rtm, statusReg.statFlds.slotNum,
				statusReg.statFlds.rsv0, statusReg.statFlds.rsv1, statusReg.statFlds.FSH,
				statusReg.statFlds.RST, statusReg.statFlds.rsv2, statusReg.statFlds.ERR1,
				statusReg.statFlds.ERR0, statusReg.statFlds.rsv3, statusReg.statFlds.FIFE,
				statusReg.statFlds.FIFF, statusReg.statFlds.rsv4, statusReg.statFlds.DMAC,
				statusReg.statFlds.ACQC);
	p += sprintf(p, "-----------------------------------------------------------------------\n");
    

    //Print DMA buffer contents    
    
    /*for(i = 0; i<DMA_BUFFS; i++){
        bufferData = (u32 *)devpriv->dmaIO.dmaBuffer[i].addr_v;        
        p += sprintf(p, "bufferData[%d] is at %p\n", i, bufferData);
	if(bufferData == NULL){continue;}
        for(j = 0; j<34; j++){
          p += sprintf(p, "bufferData[%d][%d] = %u\n", i, j, bufferData[j]);
        }
    }*/

    p += sprintf(p, "=== DAC values ===\n");

    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_0);
    p += sprintf(p, "DAC[0] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_1);
    p += sprintf(p, "DAC[1] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_2);
    p += sprintf(p, "DAC[2] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_3);
    p += sprintf(p, "DAC[3] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_4);
    p += sprintf(p, "DAC[4] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_5);
    p += sprintf(p, "DAC[5] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_6);
    p += sprintf(p, "DAC[6] = %d\n", dacValue);
    dacValue=PCIE_READ32((void*) &devpriv->pHregs->DAC_7);
    p += sprintf(p, "DAC[7] = %d\n", dacValue);

    
//    p += sprintf(p, "number of interrupts = %d\n", interrupt_counter);
    
_exit:
	*eof = 1;
	return p - buf;
}


int pcieAdc_proccmd(struct file *file, const char __user *buffer, unsigned long count, void *data) {
	return count;
}


void register_proc(void) {
	pcieAdc_proc = create_proc_entry("pcieAdc", S_IFREG | S_IRUGO, 0);
	pcieAdc_proc->read_proc = pcieAdc_procinfo;
	pcieAdc_proc->write_proc = pcieAdc_proccmd;
}

void unregister_proc(void) {
	remove_proc_entry("pcieAdc", 0);
}
