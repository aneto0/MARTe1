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
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>

#include "pcieAdc.h"
#include "pcieAdcIoctl.h"
#include "pcieAdc_proc.h"

static int firmwareRevision   = 0;

static int current_board      = -1;
static int master_board_index = -1;

static int DMA_NBYTES         = 0;


// Static structures
static struct pci_error_handlers pcieAdc_err_handler = {
    .error_detected = pcieAdc_error_detected,
    .mmio_enabled = pcieAdc_mmio_enabled, 
    .link_reset = pcieAdc_link_reset,
    .slot_reset = pcieAdc_slot_reset, 
    .resume = pcieAdc_resume,
};

static struct pci_device_id ids[] = {
    { 
        PCI_DEVICE(PCI_VENDOR_ID_XILINX,
        PCI_DEVICE_ID_FPGA)
    },
    { 0,
    },
};

MODULE_DEVICE_TABLE(pci, ids);

static struct pci_driver pcieAdc_driver = {
  .name = "pcieAdc", 
  .id_table = ids,
  .probe = pcieAdc_probe, 
  .remove = pcieAdc_remove,
  .err_handler = &pcieAdc_err_handler,
};

struct pci_dev* global_pdev;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
static struct file_operations pcieAdcDrvFileOps = {
    owner:   THIS_MODULE,
    mmap:    pcieAdcMmap,
    open:    pcieAdcOpen,
    release: pcieAdcRelease,
    ioctl:   pcieAdcIoctl,
    write:   pcieAdcWrite
};
#else
/* the ordinary device operations */
static struct file_operations pcieAdcDrvFileOps = {
    owner:   THIS_MODULE,
    mmap:    pcieAdcMmap,
    open:    pcieAdcOpen,
    release: pcieAdcRelease,
    unlocked_ioctl:   pcieAdcIoctl, 
	write:   pcieAdcWrite
};
#endif
//char device
static struct cdev charDevice;
//char device number
static dev_t  deviceNumber;

// Global DMA address
int DMA_global_addr[MAX_BOARDS * DMA_BUFFS];

// DAC locations
int DAC_addr[MAX_BOARDS * N_DACS_PER_BOARDS];

// DIO locations
int DIO_addr[MAX_BOARDS * N_DIOS_PER_BOARD];

// Command register locations (in order to export the enable and disable acquisition function)
int command_register_addr[MAX_BOARDS];

// Status register locations
int status_register_addr[MAX_BOARDS];

// The slot numbers for each of the board indexes
int board_slot_numbers[MAX_BOARDS];

static int SlotNumberToBoardIndex(int slotNum) {
    int i = 0;
    for (i = 0; i < MAX_BOARDS; i++) {
        if (board_slot_numbers[i] == slotNum) {
            return i;
        }
    }
    return -1;
}

//****************************
//* DMA management functions *
//****************************

int pcieAdcEnableDMAonboard(struct pci_dev *pdev) {
    int ret;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    ret = pci_dma_supported(pdev, DMA_32BIT_MASK);
    if (!ret) {
        printk(KERN_DEBUG "pcieAdc: DMA not supported. Aborting.\n");
        return ret;
    }
    ret = pci_set_dma_mask(pdev, DMA_32BIT_MASK);
    if (ret) {
        printk(KERN_DEBUG "pcieAdc: pci_set_dma_mask error [%d]. Aborting.\n", ret);
        return ret;
    }
    ret = pci_set_consistent_dma_mask(pdev, DMA_32BIT_MASK);
    if (ret) {
        printk(KERN_DEBUG
                "pcieAdc: pci_set_consistent_dma_mask error [%d]. Aborting.\n",
                ret);
        return ret;
    }
#else
    ret = pci_dma_supported(pdev, DMA_BIT_MASK(32));
    if (!ret) {
        printk(KERN_DEBUG "pcieAdc: DMA not supported. Aborting.\n");
        return ret;
    }
    ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
    if (ret) {
        printk(KERN_DEBUG "pcieAdc: pci_set_dma_mask error [%d]. Aborting.\n", ret);
        return ret;
    }
    ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
    if (ret) {
        printk(KERN_DEBUG
                "pcieAdc: pci_set_consistent_dma_mask error [%d]. Aborting.\n",
                ret);
        return ret;
    }
#endif
    //setting DMA mastering mode
    (void) pci_set_master(pdev);
    ret = pci_set_mwi(pdev);
    if (ret) {
        printk(KERN_DEBUG "pcieAdc: pci_set_mwi error [%d]. Aborting.\n", ret);
        return ret;
    }

    return 0;
}

int disableDMAonboard(struct pci_dev *pdev) {
    pci_clear_mwi(pdev);

    return 0;
}

void setupBoardParameters(PCIE_DEV *pcieDev) {
    STATUS_REG statusReg;

    // Find if current board is the master
    statusReg.reg32 = PCIE_READ32((void*) & pcieDev->pHregs->status);
    if (statusReg.statFlds.master == 1) {
        master_board_index = current_board;
        global_pdev = pcieDev->pdev;
        printk("Found master board, index = %d\n", master_board_index);
   }

    if(DMA_NBYTES == 0){
        firmwareRevision = statusReg.statFlds.revID;
        printk("The firmware version is = %d\n", firmwareRevision);
        DMA_NBYTES = IN_ANALOG_CHANNELS * 4 + GetNumberOfInputDigitalChannels(statusReg.statFlds.slotNum) * 4 + 2 * HEADER_LENGTH * 4 + 4;
    }

    DAC_addr[N_DACS_PER_BOARDS * current_board]     = (int) (&pcieDev->pHregs->DAC_0);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 1] = (int) (&pcieDev->pHregs->DAC_1);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 2] = (int) (&pcieDev->pHregs->DAC_2);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 3] = (int) (&pcieDev->pHregs->DAC_3);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 4] = (int) (&pcieDev->pHregs->DAC_4);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 5] = (int) (&pcieDev->pHregs->DAC_5);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 6] = (int) (&pcieDev->pHregs->DAC_6);
    DAC_addr[N_DACS_PER_BOARDS * current_board + 7] = (int) (&pcieDev->pHregs->DAC_7);

    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_0);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_1);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_2);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_3);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_4);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_5);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_6);
    PCIE_WRITE32(0, (void*) & pcieDev->pHregs->DAC_7);

    DIO_addr[N_DIOS_PER_BOARD * current_board] = (int)(&pcieDev->pHregs->DIO_0);

    command_register_addr[current_board] = (int) (&pcieDev->pHregs->command);
    status_register_addr[current_board] = (int) (&pcieDev->pHregs->status);
    board_slot_numbers[current_board] = statusReg.statFlds.slotNum;
    printk("board slot number is %d\n", board_slot_numbers[current_board]);
}

int setupDMA(PCIE_DEV *pcieDev, DMA_REG dmaReg, COMMAND_REG commandReg) {
    int i = 0;

    pcieDev->dmaIO.buf_size = DMA_NBYTES;

    // allocating DMA buffers
    for (i = 0; i < DMA_BUFFS; i++) {
        // set up a coherent mapping through PCI subsystem
        pcieDev->dmaIO.dmaBuffer[i].addr_v = pci_alloc_consistent(
                pcieDev->pdev, pcieDev->dmaIO.buf_size,
                &(pcieDev->dmaIO.dmaBuffer[i].addr_hw));
        if (!pcieDev->dmaIO.dmaBuffer[i].addr_v || !pcieDev->dmaIO.dmaBuffer[i].addr_hw) {
            printk(
                    "pcieAdc: pci_alloc_consistent error (v:%p hw:%p). Aborting.\n",
                    (void*) pcieDev->dmaIO.dmaBuffer[i].addr_v,
                    (void*) pcieDev->dmaIO.dmaBuffer[i].addr_hw);
            return -ENOMEM;
        }
        memset((void*) (pcieDev->dmaIO.dmaBuffer[i].addr_v), 0,
                pcieDev->dmaIO.buf_size);
    }

    // assign addresses to board
    pcieDev->flags = 0;
    for (i = 0; i < DMA_BUFFS; i++) {
        PCIE_WRITE32(pcieDev->dmaIO.dmaBuffer[i].addr_hw, (void*) & pcieDev->pHregs->HwDmaAddr[i]);
    }

    dmaReg.reg32 = 0;
    dmaReg.dmaFlds.Size = DMA_NBYTES;
    dmaReg.dmaFlds.BuffsNumber = DMA_BUFFS;
    printk("DMAREG: 0x%08x\n", dmaReg.reg32);
    PCIE_WRITE32(dmaReg.reg32, (void*) & pcieDev->pHregs->dmaReg);
    pcieDev->counter = PCIE_READ32((void*) & pcieDev->pHregs->hwcounter);
    commandReg.reg32 = PCIE_READ32((void*) & pcieDev->pHregs->command);

    // add DMA address to global structure
    for (i = 0; i < DMA_BUFFS; i++) {
        DMA_global_addr[current_board * DMA_BUFFS + i] = (int) pcieDev->dmaIO.dmaBuffer[i].addr_v;
        SetPageReserved(virt_to_page(pcieDev->dmaIO.dmaBuffer[i].addr_v));
	    memset(pcieDev->dmaIO.dmaBuffer[i].addr_v, 0, pcieDev->dmaIO.buf_size);
    }
    return 0;
}

int cleanupDMA(PCIE_DEV *pcieDev) {
    int i = 0;

    for (i = 0; i < DMA_BUFFS; i++) {
        ClearPageReserved(virt_to_page(pcieDev->dmaIO.dmaBuffer[i].addr_v));
        pci_free_consistent(pcieDev->pdev, pcieDev->dmaIO.buf_size,
                pcieDev->dmaIO.dmaBuffer[i].addr_v,
                pcieDev->dmaIO.dmaBuffer[i].addr_hw);
    }

    // clear DMA addresses on board
    for (i = 0; i < DMA_BUFFS; i++) {
        PCIE_WRITE32(0, (void*) & pcieDev->pHregs->HwDmaAddr[i]);
    }

    return 0;
}

int resetBoard(PCIE_DEV *pcieDev) {
    STATUS_REG statusReg;
    int i;

    // reset board
    statusReg.reg32 = PCIE_READ32((void*) & pcieDev->pHregs->status);
    statusReg.statFlds.RST = 1;
    PCIE_WRITE32(statusReg.reg32, (void*) & pcieDev->pHregs->status);

    //wait for reset acknowledgment
    for (i = 0; i < WAIT_NOOP_CYCLES; i++) {
        udelay(10000);
        statusReg.reg32 = PCIE_READ32((void*) & pcieDev->pHregs->status);
        if (!(statusReg.statFlds.RST))
            break;
    }

    return 0;
}

int pcieAdcConfigurePCI(struct pci_dev *pdev, PCIE_DEV *pcieDev) {
    u16 reg16 = 0;
    int i = 0;
    int ret = 0;

    //set command register
    pci_read_config_word(pdev, PCI_COMMAND, &reg16);
    reg16 &= ~PCI_COMMAND_IO; // disable IO port access
    reg16 |= PCI_COMMAND_PARITY; // enable parity error hangs
    reg16 |= PCI_COMMAND_SERR; // enable addr parity error
    pci_write_config_word(pdev, PCI_COMMAND, reg16);

    //PCI reading IO memory spaces and set virtual addresses
    for (i = 0; i < 2; i++) {
        pcieDev->memIO[i].start = pci_resource_start(pdev, i);
        pcieDev->memIO[i].end = pci_resource_end(pdev, i);
        pcieDev->memIO[i].len = pci_resource_len(pdev, i);
        pcieDev->memIO[i].flags = pci_resource_flags(pdev, i);
        // virtual addr
        pcieDev->memIO[i].vaddr = ioremap_nocache(pcieDev->memIO[i].start,
                pcieDev->memIO[i].len);
        if (!pcieDev->memIO[i].vaddr) {
            printk(KERN_DEBUG "pcieAdc: error in ioremap_nocache [%d]. Aborting.\n", ret);
            return -ENOMEM;
        }
    }
    //virtual pointer to board registers
    pcieDev->pHregs = (PCIE_HREGS *) pcieDev->memIO[1].vaddr;

    return 0;
}

//*******************************
//* Device management functions *
//*******************************

/**
 * Enables acquisition on a board by starting the on board DMA state machine
 * return 0 if successful
 */
int EnableATCApcieAcquisition(void) {
    COMMAND_REG commandReg;
   
    if (master_board_index == -1) {
        printk("pcie adc driver: master board not found!\n");
	return -1;
    }

    commandReg.reg32 = PCIE_READ32((void*) command_register_addr[master_board_index]);
    commandReg.cmdFlds.DMAE = 0;
    commandReg.cmdFlds.ACQE = 0;
    PCIE_WRITE32(commandReg.reg32, (void*) command_register_addr[master_board_index]);

    commandReg.reg32 = PCIE_READ32((void*) command_register_addr[master_board_index]);
    commandReg.cmdFlds.DMAE = 1;
    commandReg.cmdFlds.ACQE = 1;
    PCIE_WRITE32(commandReg.reg32, (void*) command_register_addr[master_board_index]);
	
    if(firmwareRevision > 1){
        SendSoftwareTrigger();
    }

    return 0;
}

/**
 * Perfoms a software trigger on the boards
 */
int SendSoftwareTrigger(void){
    COMMAND_REG commandReg;
    if (master_board_index == -1) {
        printk("pcie adc driver: master board not found!\n");
        return -1;
    }

    if(firmwareRevision > 1){
        commandReg.reg32 = PCIE_READ32((void*) command_register_addr[master_board_index]);
        commandReg.cmdFlds.STRG = 0;
        PCIE_WRITE32(commandReg.reg32, (void*) command_register_addr[master_board_index]);
    }

    commandReg.reg32 = PCIE_READ32((void*) command_register_addr[master_board_index]);
    commandReg.cmdFlds.STRG = 1;
    PCIE_WRITE32(commandReg.reg32, (void*) command_register_addr[master_board_index]);
    return 0;
}

/**
 * Disables the acquisition in the boards
 * @return 0 if successful
 */
int DisableATCApcieAcquisition(void) {

    COMMAND_REG commandReg;
    if (master_board_index == -1) {
        printk("pcie adc driver: master board not found!\n");
        return -1;
    }
    commandReg.reg32 = PCIE_READ32((void*) command_register_addr[master_board_index]);	
    commandReg.cmdFlds.DMAE = 0;
    commandReg.cmdFlds.ACQE = 0;
    commandReg.cmdFlds.STRG = 0;
    commandReg.cmdFlds.DMAiE = 0;
    commandReg.cmdFlds.ERRiE = 0;
    PCIE_WRITE32(commandReg.reg32, (void*) command_register_addr[master_board_index]);

    return 0;
}

/**
 * Enables/Disables external trigger and clock
 * @param enabled 1 if to enable
 * @return 0 if successful
 */
int SetATCApcieExternalTriggerAndClock(int enabled) {
    COMMAND_REG commandReg;
    if (master_board_index == -1) {
        printk("pcie adc driver: master board not found!\n");
        return -1;
    }
    commandReg.reg32 = PCIE_READ32((void*) command_register_addr[master_board_index]);
    commandReg.cmdFlds.EXT_TRG_CLK = enabled;
    PCIE_WRITE32(commandReg.reg32, (void*) command_register_addr[master_board_index]);
    return 0;
}

/**
 * Writes the desired value to a board DAC
 * @param slotNum the board slot number
 * @param channel the DAC channel
 * @param value the value to write to DAC. It will be masked to 16 bits
 * @return 0 if successful
 */
int WriteToDAC(int slotNum, int channel, int value) {
    int board_idx = SlotNumberToBoardIndex(slotNum);
    if (board_idx == -1) {
        return -1;
    }
    if(channel >= N_DACS_PER_BOARDS){
        return -2;
    }

    if(firmwareRevision < 2){
        value &= 0x7FFF;
    }
    else{
        value &= 0xFFFF;
    }
    PCIE_WRITE32(value, (void*) DAC_addr[N_DACS_PER_BOARDS * board_idx + channel]);

    return 0;
}

/**
 * Writes the desired value in the digital IO
 * @param slotNum the board slot number
 * @param channel the IO channel
 * @param value the value to write. It will be masked to 4 bits
 * @return 0 if successful
 */
int WriteToDIO(int slotNum, int channel, int value) {
    int board_idx = SlotNumberToBoardIndex(slotNum);
    if (board_idx == -1) {
        return -1;
    }
    if(channel >= N_DIOS_PER_BOARD){
        return -2;
    }

    value &= 0xF;

    PCIE_WRITE32(value, (void*) DIO_addr[N_DIOS_PER_BOARD * board_idx + channel]);

    return 0;
}

/**
 * Checks if the RTM is present for a given board
 * @param slotNum the board slot number
 * @return true if rtm is present
 */
int IsRTMPresent(int slotNum) {
    STATUS_REG statusReg;
    int board_idx = SlotNumberToBoardIndex(slotNum);
    if (board_idx == -1) {
        return 0;
    }
    statusReg.reg32 = PCIE_READ32((void*) status_register_addr[board_idx]);
    return (statusReg.statFlds.rtm == 1);
}

/**
 * Returns global DMA addresses (exported)
 * @return all the DMA buffer addresses
 */
int *GetBufferAddress(void) {
    return (int *) DMA_global_addr;
}

/**
 * @param slotNum the board slot number
 * @return the DMA addresses for the board in this slot
 */
int *GetBoardBufferAddress(int slotNum) {
    int board_idx = SlotNumberToBoardIndex(slotNum);
    if (board_idx == -1) {
    	printk("Could not find the index for the slot number %d\n", slotNum);
        return NULL;
    }
    return &DMA_global_addr[board_idx * DMA_BUFFS];
}

/**
 * @return the number of boards installed
 */
int GetNumberOfBoards(void) {
    return current_board + 1;
}

/**
 * @return the master number index
 */
int GetMasterBoardSlotNumber(void) {
    STATUS_REG statusReg;
    if (master_board_index == -1) {
        return -1;
    }
    statusReg.reg32 = PCIE_READ32((void*) status_register_addr[master_board_index]);
    return statusReg.statFlds.slotNum;
}

int *GetBoardsSlotNumbers(void){
    return board_slot_numbers;
}


//**********************
//* Hardware functions *
//**********************

/*
 * pcieAdc_error_detected
 * TODO
 */
pci_ers_result_t pcieAdc_error_detected(struct pci_dev *pdev,
        enum pci_channel_state error) {
    printk(
            "pcieAdc_error_detected PCI detect error: %s\n",
            (error == pci_channel_io_normal) ? "PCI channel IO normal" : // 1
            (error == pci_channel_io_frozen) ? "PCI channel is blocked"
            : // 2
            (error == pci_channel_io_perm_failure) ? "PCI channel card is dead"
            : "unknow"); // 3

    return PCI_ERS_RESULT_NONE;
}

/*
 * pcieAdc_mmio_enabled
 * TODO
 */
pci_ers_result_t pcieAdc_mmio_enabled(struct pci_dev *dev) {
    printk("pcieAdc_mmio_enabled PCI detect error\n");

    return PCI_ERS_RESULT_NONE;
}

/*
 * pcieAdc_link_reset
 * TODO
 */
pci_ers_result_t pcieAdc_link_reset(struct pci_dev *dev) {
    printk("pcieAdc_link_reset PCI detect error\n");

    return PCI_ERS_RESULT_NONE;
}

/*
 * pcieAdc_slot_reset
 * TODO
 */
pci_ers_result_t pcieAdc_slot_reset(struct pci_dev *pdev) {
    printk("pcieAdc_slot_reset PCI detect error\n");

    return PCI_ERS_RESULT_NONE;
}

/*
 * pcieAdc_resume
 * TODO
 */
void pcieAdc_resume(struct pci_dev *pdev) {
    printk("pcieAdc_resume PCI detect error\n");

    return;
}

/*
 * probe
 */
int pcieAdc_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
    int ret = 0;
    PCIE_DEV *pcieDev = NULL;
    COMMAND_REG commandReg;
    DMA_REG dmaReg;

    current_board++;

    printk("pcieADC: Variable current_board is = %d\n", current_board);

    //allocate the device instance block
    pcieDev = kzalloc(sizeof (PCIE_DEV), GFP_KERNEL);
    if (!pcieDev) { 
      printk(KERN_DEBUG "pcieADC: Failed to allocate memory");
      return -ENOMEM;
    }

    //enable PCI board
    ret = pci_enable_device(pdev);
    if (ret != 0) {
        printk(KERN_DEBUG "pcieADC: pcieAdc_probe pci_enable_device error(%d). EXIT\n", ret);
        return ret;
    }

    //enable DMA transfers
    ret = pcieAdcEnableDMAonboard(pdev);
    if (ret != 0) {
        printk(KERN_DEBUG "pcieAdc: Error in DMA initialization. Aborting.\n");
        return ret;
    }

    // configure PCI and remap I/O
    ret = pcieAdcConfigurePCI(pdev, pcieDev);
    if (ret != 0) {
        printk(KERN_DEBUG "pcieAdc: error in PCI configuration. Aborting.\n");
        return ret;
    }

    //init spinlock
    //not so sure as the spin_lock_init macro has been available even before 2.0.40
    //but old driver used direct assignment
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    pcieDev->irq_lock = SPIN_LOCK_UNLOCKED;
#else
    spin_lock_init(&pcieDev->irq_lock);
#endif
    // reset board
    //resetBoard(pcieDev);

    pcieDev->pdev = pdev;
    pcieDev->wt_tmout = 20 * HZ;
    pci_set_drvdata(pdev, pcieDev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
    init_MUTEX(&pcieDev->open_sem);
#else
    mutex_init(&pcieDev->open_mutex);
#endif

#ifdef _MSI_ENABLE    
    ret = pci_enable_msi(pdev);
    if (ret) {
        printk(KERN_WARNING "pcieADC: pci_enable_msi %d error[%d]\n", pcieDev->pdev->irq, ret);
        return ret;
    } else {
	printk("pcieADC: pci_enable_msi IRQ %d\n", pdev->irq);
    }
#endif
    // Waitqueue initialization
    init_waitqueue_head(&pcieDev->rd_q);

    setupBoardParameters(pcieDev);

    //Set up DMA
    ret = setupDMA(pcieDev, dmaReg, commandReg);
    if (ret != 0) {
        printk("pcieAdc: error in DMA setup. Aborting.\n");
        return ret;
    }

    //Default is to setup 
    //setATCApcieExternalTriggerAndClock(1);

    // TODO: error chain
    return 0;
}

/*
 * remove
 */
void pcieAdc_remove(struct pci_dev *pdev) {
    int i;
    PCIE_DEV * pcieDev;
    //get the device information data 
    pcieDev = (PCIE_DEV *) pci_get_drvdata(pdev);

#ifdef _MSI_ENABLE    
    pci_disable_msi(pcieDev->pdev);
#endif  
    //Reset 
    resetBoard(pcieDev);

    //deregistering DMAable areas
    cleanupDMA(pcieDev);

    //disable PCI board, deregister virtual addresses for the board 
    for (i = 0; i < 2; i++) {
        iounmap(pcieDev->memIO[i].vaddr);
    }

    kfree(pcieDev);
    pci_set_drvdata(pdev, NULL);

    // disable DMA on board
    disableDMAonboard(pdev);

    pci_disable_device(pdev);
    printk(KERN_DEBUG "pcieADC removed. \n");
}


/**
 *Setup the char device interface
 */
static int SetupCharDeviceInterface(void){
    int ret   = 0;
    int major = 0;
    int minor = 0;

    deviceNumber = 0;
    /* allocate  the device  number*/
    ret = alloc_chrdev_region(&deviceNumber, minor, 1, CHAR_DEVICE_NAME);
    if (ret) {
        printk(KERN_ERR "probe: alloc_chrdev_region() failure - unable to allocate char devices\n");
        return -EIO;
    }

    major = MAJOR(deviceNumber);

    cdev_init(&charDevice, &pcieAdcDrvFileOps);
    charDevice.owner = THIS_MODULE;
    charDevice.ops   = &pcieAdcDrvFileOps;
    ret = cdev_add (&charDevice, deviceNumber, 1); // count =1
    if (ret < 0) {
        printk(KERN_NOTICE "Error %d adding pcieAdc", ret);
        return -EIO;
    }

    return ret;
}

/*
 * Clean up the char device interface
 */
static void CleanUpCharDeviceInterface(void){
    cdev_del(&charDevice);    
    unregister_chrdev_region(deviceNumber, 1);
}

//*******************************
//* Module management functions *
//*******************************

int __init pcieAdcInit(void) {
    int ret;
    int i = 0;
    for (i = 0; i < MAX_BOARDS; i++) {
        board_slot_numbers[i] = -1;
    }
    /* registering the board */
    ret = pci_register_driver(&pcieAdc_driver);
    if (ret) {
        printk(KERN_ALERT "pcieAdcInit pci_register_driver error(%d).\n", ret);
        return ret;
    }
    /* registering proc fs */
    register_proc();

    //Register the char device interface
    ret = SetupCharDeviceInterface();
    if(ret < 0){
        return ret;
    }
    return 0;
}

void pcieAdcExit(void) {
    /* unregistering proc fs */
    unregister_proc();
    /* unregistering the board */
    pci_unregister_driver(&pcieAdc_driver);
    // unregister char device
    CleanUpCharDeviceInterface();
}

/* device mmap */
ssize_t pcieAdcMmap(struct file *file, struct vm_area_struct *vma){
    int ret             = 0;
    int i               = 0;
    int j               = 0;
    unsigned long page  = 0;
    unsigned long start = vma->vm_start;
    printk("pcieAdc: Mapping memory for %d boards\n", GetNumberOfBoards());
    if((vma->vm_end - vma->vm_start) < (PAGE_SIZE * DMA_BUFFS * GetNumberOfBoards())){
        printk("Not enough memory to mmap vma->vm_start = %ld vma->vm_end = %ld required = %d!\n",
                vma->vm_start,
                vma->vm_end,
                (DMA_NBYTES * DMA_BUFFS * GetNumberOfBoards() * sizeof(int)));
        return -EAGAIN;
    }

    for(i = 0; i<GetNumberOfBoards(); i++){
        for(j = 0; j<DMA_BUFFS; j++){
	    page = virt_to_phys((void *)DMA_global_addr[i * DMA_BUFFS + j]) >> PAGE_SHIFT;
            ret = remap_pfn_range(vma,
                        start,
                        page,
                        PAGE_SIZE,
                        PAGE_SHARED);
            start += PAGE_SIZE;
            if(ret){
                printk("mmap failed: %d\n", ret);
                return -EAGAIN;
            }
        }
    }

    /* we do not want to have this area swapped out, lock it */
    vma->vm_flags |= VM_LOCKED;    
    
    return 0;
}

//write to the dacs
ssize_t pcieAdcWrite(struct file *file, const char *buf, size_t count, loff_t * ppos) {
    int *values = (int *)buf;
    if(count != (sizeof(u32) * 4)){
        printk("pcieadc Error in write. The function arguments are: int board, int channel, int value, int mode\n");
    }    

    if(values[3] == 1){
        return WriteToDIO(values[0], values[1], values[2]);
    }
    return WriteToDAC(values[0], values[1], values[2]);
}


//device open
ssize_t pcieAdcOpen(struct inode *inode, struct file *file) {
    printk("pcieadc device driver access opened\n");
    return 0;
}

//device close
ssize_t pcieAdcRelease(struct inode *inode, struct file *file) {
    printk("pcieadc device driver access closed(released)\n");
    return 0;
}

//ioctl

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)
ssize_t pcieAdcIoctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg){
#else
long pcieAdcIoctl(struct file *file, unsigned int cmd, unsigned long arg){
#endif
    int err       = 0;
    u32 tempValue = 0;

    //extract the type and number bitfields, and don't decode
    //wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != PCIE_ATCA_ADC_IOCT_MAGIC)
        return -ENOTTY;
    if (_IOC_NR(cmd) > PCIE_ATCA_ADC_IOCT_MAXNR)
        return -ENOTTY;

    //the direction is a bitmask, and VERIFY_WRITE catches R/W
    //transfers. `Type' is user-oriented, while
    //access_ok is kernel-oriented, so the concept of "read" and
    //"write" is reversed
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;

    switch(cmd) {
        case PCIE_ATCA_ADC_IOCT_ACQ_ENABLE:
            EnableATCApcieAcquisition();
            break;
        case PCIE_ATCA_ADC_IOCT_ACQ_DISABLE:
            DisableATCApcieAcquisition();
            break;
        case PCIE_ATCA_ADC_IOCT_NUM_BOARDS:
            tempValue = GetNumberOfBoards();
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32)))
                return -EFAULT;
            break;
        case PCIE_ATCA_ADC_IOCT_MASTER_SLOT_NUM:
            tempValue = GetMasterBoardSlotNumber();
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32))){
                return -EFAULT;
            }
            break;
        case PCIE_ATCA_ADC_IOCT_SET_EXT_CLK_TRG:
            tempValue = 0;
            if(copy_from_user(&tempValue, (void __user *)arg, sizeof(u32))){
                return -EFAULT;
            }
            if(tempValue < 0 || tempValue > 1){
                return -EFAULT;
            }
            SetATCApcieExternalTriggerAndClock(tempValue);
            break;
        case PCIE_ATCA_ADC_IOCT_IS_RTM_PRESENT:
            tempValue = 0;
            if(copy_from_user(&tempValue, (void __user *)arg, sizeof(u32))){
                return -EFAULT;
            }
            tempValue = IsRTMPresent(tempValue);
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32))){
                return -EFAULT;
            }
            break;
        case PCIE_ATCA_ADC_IOCT_N_IN_ANA_CHANNELS:
            tempValue = 0;
            if(copy_from_user(&tempValue, (void __user *)arg, sizeof(u32))){
                return -EFAULT;
            }
            tempValue = GetNumberOfInputAnalogChannels(tempValue);
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32))){
                return -EFAULT;
            }
            break;
        case PCIE_ATCA_ADC_IOCT_N_IN_DIG_CHANNELS:
             tempValue = 0;
            if(copy_from_user(&tempValue, (void __user *)arg, sizeof(u32))){
                return -EFAULT;
            }
            tempValue = GetNumberOfInputDigitalChannels(tempValue);
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32))){
                return -EFAULT;
            }               
            break;
        case PCIE_ATCA_ADC_IOCT_N_OUT_ANA_CHANNELS:
            tempValue = 0;
            if(copy_from_user(&tempValue, (void __user *)arg, sizeof(u32))){
                return -EFAULT;
            }
            tempValue = GetNumberOfAnalogueOutputChannels(tempValue);
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32))){
                return -EFAULT;
            }
            break;
        case PCIE_ATCA_ADC_IOCT_N_OUT_DIG_CHANNELS:
            tempValue = 0;
            if(copy_from_user(&tempValue, (void __user *)arg, sizeof(u32))){
                return -EFAULT;
            }
            tempValue = GetNumberOfDigitalOutputChannels(tempValue);
            if(copy_to_user((void __user *)arg, &tempValue, sizeof(u32))){
                return -EFAULT;
            }
            break;
        case PCIE_ATCA_ADC_IOCT_SEND_SOFT_TRG:
            SendSoftwareTrigger();
            break;
        case PCIE_ATCA_ADC_IOCT_GET_BOARD_SLOT_NS:
            if(access_ok(VERIFY_WRITE, (void __user *)arg, GetNumberOfBoards() * sizeof(u32))){
                if(copy_to_user((void __user *)arg, board_slot_numbers, GetNumberOfBoards() * sizeof(u32))){
                    return -EFAULT;
                }
            }
            else{
                printk("Bad memory provided to copy ");
                return -EFAULT;
            }
            break;
        case PCIE_ATCA_ADC_IOCT_ACQ_DEBUG:
	    printk("DMA_global_addr[0] = %d\n", *((u32 *)DMA_global_addr[0]));
	    printk("DMA_global_addr[1] = %d\n", *((u32 *)DMA_global_addr[1]));
	    printk("DMA_global_addr[2] = %d\n", *((u32 *)DMA_global_addr[2]));
	    printk("DMA_global_addr[3] = %d\n", *((u32 *)DMA_global_addr[3]));
            break;
    }
    return 0;
}

int GetNumberOfInputAnalogChannels(int slotNum){
    return IN_ANALOG_CHANNELS;
}

int GetNumberOfInputDigitalChannels(int slotNum){
    if(firmwareRevision < 2){
        return IN_DIGITAL_CHANNELS_V1;
    }
    return IN_DIGITAL_CHANNELS_V2;
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

module_init(pcieAdcInit);
module_exit(pcieAdcExit);

EXPORT_SYMBOL(IsRTMPresent);
EXPORT_SYMBOL(GetBufferAddress);
EXPORT_SYMBOL(GetBoardBufferAddress);
EXPORT_SYMBOL(GetNumberOfBoards);
EXPORT_SYMBOL(GetMasterBoardSlotNumber);
EXPORT_SYMBOL(WriteToDAC);
EXPORT_SYMBOL(WriteToDIO);
EXPORT_SYMBOL(EnableATCApcieAcquisition);
EXPORT_SYMBOL(DisableATCApcieAcquisition);
EXPORT_SYMBOL(SetATCApcieExternalTriggerAndClock);
EXPORT_SYMBOL(GetBoardsSlotNumbers);
EXPORT_SYMBOL(SendSoftwareTrigger);
EXPORT_SYMBOL(GetNumberOfInputAnalogChannels);
EXPORT_SYMBOL(GetNumberOfInputDigitalChannels);
EXPORT_SYMBOL(GetNumberOfAnalogueOutputChannels);
EXPORT_SYMBOL(GetNumberOfDigitalOutputChannels);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Test Module for PCIe FPGA Endpoint");
MODULE_AUTHOR("Bernardo Carvalho, Antonio Barbalace, Andre' Neto, Riccardo Vitelli");

