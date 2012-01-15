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

#ifndef PCIEADC_H_
#define PCIEADC_H_

/* board PCI id */
#define PCI_DEVICE_ID_FPGA          0x0007
#define PCIEADC_MAJOR               234

/* board configurable parameters */
#define BAR1_RVAL                   0xB

/* The DMA_NBYTES will only be used for offline acquisition.
 * Realtime acquisiton will always requires 32 * 4 + 4 + 4 <=> 32 channels +
 * + header + footer */
#define IN_DIGITAL_CHANNELS_V1      1
#define IN_DIGITAL_CHANNELS_V2      9
#define IN_ANALOG_CHANNELS          32
#define N_DACS_PER_BOARDS	        8
#define N_DIOS_PER_BOARD            1
#define HEADER_LENGTH     	        1
//#define DMA_NBYTES                  IN_ANALOG_CHANNELS * 4 + IN_DIGITAL_CHANNELS * 4 + 2 * HEADER_LENGTH * 4 + 4
#define GFPORDER                    0

#define PCIE_READ32(addr)           ioread32(addr)
#define PCIE_WRITE32(value, addr)   iowrite32(value, addr)
#define PCIE_FLUSH32()              PCIE_READ32()

#define IRQ_CYCLES                  3


#define DMA_MA_REG                  4
#define DMA_SZE_REG                 8
#define DMA_NINT_REG                0xC
#define DMA_BUFFS                   4 //The number of DMA buffers
#define MAX_BOARDS		            12

#define CHAR_DEVICE_NAME            "pcieATCAAdc"

#ifdef __cplusplus
extern "C"{
#endif
	int *GetBufferAddress(void);
	int *GetBoardBufferAddress(int slotNum);
	int  GetNumberOfBoards(void);
	int  GetMasterBoardSlotNumber(void);
	int  WriteToDAC(int board, int channel, int value);
	int  WriteToDIO(int board, int channel, int value);
	int  EnableATCApcieAcquisition(void);
	int  DisableATCApcieAcquisition(void);
	int  SetATCApcieExternalTriggerAndClock(int enabled);
	int  IsRTMPresent(int board_idx);
	int *GetBoardsSlotNumbers(void);
	int  SendSoftwareTrigger(void);
    int  GetNumberOfInputAnalogChannels(int slotNum);
    int  GetNumberOfInputDigitalChannels(int slotNum);
    int  GetNumberOfAnalogueOutputChannels(int slotNum);
    int  GetNumberOfDigitalOutputChannels(int slotNum);
#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
/**
 * The number of NOOP cycles
 * This is used for synch
 */
#define WAIT_NOOP_CYCLES 100
#define IRQ_NOOP_CYCLES	 3

/**
 * Registers
 */
// BITFIELDS
#ifdef __BIG_ENDIAN_BITFIELD
 #define BTFLD(a,b) b,a
#else
#define BTFLD(a,b) a, b
#endif

//TODO: remove
typedef struct _REVID_FLDS {
    u32 BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(RevId:4, TMR:1), HDR:1), DBG:1), reserved:1),none:24);
} REVID_FLDS;
typedef struct _STATUS_FLDS {
	 u32 BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(
            BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(revID:4, none:4), master:1), rtm:1), slotNum:4), rsv0:2), rsv1:2),
               FSH:1), RST:1), rsv2:2), ERR1:1), ERR0:1), rsv3:2),
              FIFE:1), FIFF:1), rsv4:2), DMAC:1), ACQC:1);
} STATUS_FLDS;


typedef struct _STATUS_REG {
    union
    {
        u32 reg32;

/*        struct _STALFD
        {
            u32 revId:8;
            u32 statWrd:24;
        } Str;*/
        STATUS_FLDS statFlds;
        REVID_FLDS rdIdFlds;
    };
} STATUS_REG;

typedef struct _COMMAND_REG {
    union
    {
        u32 reg32;
        struct  {
	u32 BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(
            BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(BTFLD(
	    BTFLD(BTFLD(BTFLD(BTFLD(EXT_TRG_CLK:1, rsv0:2), ACQM:1),DQTP:2),LOAD:1),rsv1:1),CHN:2),ILVM:2),
                rsv2:6), HOP:1), TOP:1), ACQS:1), ACQT:1),
	       ACQK:1), ACQE:1), STRG:1), rsv3:1), DMAF:1), DMAE:1), rsv4:1),
                              ERRiE:1), DMAiE:1), ACQiE:1);
        } cmdFlds;
    };
} COMMAND_REG;

// 32 bit reg **/
typedef struct _DATA_PROC_REG {
    union
    {
        u32 reg32;
        struct  {
                        u32 BTFLD(BTFLD(BTFLD(BTFLD(K:4, L:6), M:7),T:14),rsv1:1);
        } dpFlds;
    };
} DATA_PROC_REG;

/**
  32 bit reg **/
typedef struct _DMA_REG {
        union
    {
        u32 reg32;
        struct  {
                        u32 BTFLD(Size:16, BuffsNumber:16);
                } dmaFlds;
        };
} DMA_REG;

/**
 * The board has to access modes:
 * RT_ACK_MODE : No DMA interrupts
 * OFF_ACK_MODE: Data is acquired offline with the classic linux char device like access
 */
typedef enum{
    RT_ACK_MODE,
    OFF_ACK_MODE
} BoardAcquisitionMode;


typedef struct _BAR_STRUCT {
    unsigned long start;
    unsigned long end;
    unsigned long len;
    unsigned long flags;
    void* vaddr;
} BAR_STRUCT;

typedef struct _READ_BUF {
    int  count;
    int  total;
    u32 *buf;
} READ_BUF;

typedef struct _DMA_BUF {
    void       *addr_v;
    dma_addr_t  addr_hw;
} DMA_BUF;

typedef struct _DMA_STRUCT {
    unsigned int buf_size;
    DMA_BUF      dmaBuffer[DMA_BUFFS];
} DMA_STRUCT;

/**
 * All the board registers
 */
typedef struct _PCIE_HREGS {
    volatile STATUS_REG status;
    volatile COMMAND_REG command;   /*Offset 0x04 */
    volatile u32 DIO_0;             /*Offset 0x08 */
    volatile DMA_REG dmaReg;        /*Offset 0x0C */
    volatile u32 dmaCurrBuff;       /*Offset 0x10 */
    volatile u32 hwcounter;         /*Offset 0x14 */
    volatile u32 _reserved1;        /*Offset 0x18 */
    volatile u32 _reserved2;        /*Offset 0x1C */
    volatile u32 HwDmaAddr[16];     /*Offsets 0x20 - 0x5C  */
    volatile u32 DAC_0;             /*Offset 0x60 DAC_0 is 16 lsb*/
    volatile u32 DAC_1;             /*Offset 0x64 DAC 1 is 16 lsb*/
    volatile u32 DAC_2;             /*Offset 0x68 DAC 2 is 16 lsb*/
    volatile u32 DAC_3;             /*Offset 0x6C DAC 3 is 16 lsb*/
    volatile u32 DAC_4;             /*Offset 0x70 DAC_4 is 16 lsb*/
    volatile u32 DAC_5;             /*Offset 0x74 DAC 5 is 16 lsb*/
    volatile u32 DAC_6;             /*Offset 0x78 DAC 6 is 16 lsb*/
    volatile u32 DAC_7;             /*Offset 0x7C DAC 7 is 16 lsb*/
} PCIE_HREGS;

typedef struct _PCIE_DEV {
    //char device    
    struct cdev cdev;
    //char device number 
    dev_t devno;
    //pci device 
    struct pci_dev *pdev;
    unsigned char irq;
    spinlock_t irq_lock;
    unsigned int counter;
    unsigned int counter_hw;
    unsigned open_count;
    
    unsigned flags;
    struct semaphore open_sem;
    
    //Read queues
    wait_queue_head_t rd_q;
    
    //read timeout
    long wt_tmout;
    
    //max amount  of data stored in memory
    //unsigned long size;
    
    BAR_STRUCT memIO[2];
    DMA_STRUCT dmaIO;
    
    // buffer struct for read() ops
    READ_BUF bufRD; 
    PCIE_HREGS *pHregs;
} PCIE_DEV ;

// function pointer type for IRQ handlers
typedef int(*irq_handler_func)(unsigned, void*); 

// Prototypes
pci_ers_result_t pcieAdc_error_detected(struct pci_dev *pdev, enum pci_channel_state error);
pci_ers_result_t pcieAdc_mmio_enabled(struct pci_dev *dev);
pci_ers_result_t pcieAdc_link_reset(struct pci_dev *dev);
pci_ers_result_t pcieAdc_slot_reset(struct pci_dev *pdev);
void pcieAdc_resume(struct pci_dev *pdev);
ssize_t pcieAdc_read(struct file *filp, char *buf, size_t count, loff_t *ptr);
ssize_t pcieAdc_write(struct file *file, const char *buf, size_t count, loff_t * ppos);
int pcieAdc_release(struct inode *inode, struct file *filp);
int pcieAdc_open(struct inode *inode, struct file *filp);
irqreturn_t pcieAdc_handler(unsigned irq, void* dev_id);
int pcieAdc_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
int pcieAdc_probe(struct pci_dev *pdev, const struct pci_device_id *id);
void pcieAdc_remove(struct pci_dev *pdev);
int install_irq(struct pci_dev *pdev, PCIE_DEV* device, irq_handler_func pcieAdc_handler);
int uninstall_irq(struct pci_dev *pdev);

ssize_t pcieAdcMmap(struct file *file, struct vm_area_struct *vma);
ssize_t pcieAdcOpen(struct inode *inode, struct file *file);
ssize_t pcieAdcRelease(struct inode *inode, struct file *file);
ssize_t pcieAdcIoctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
ssize_t pcieAdcWrite(struct file *file, const char *buf, size_t count, loff_t * ppos);
#endif //__cplusplus
#endif //PCIEADC_H_

