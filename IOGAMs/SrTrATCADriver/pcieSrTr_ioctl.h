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
 * This file is an hard copy of the original driver from:
 * http://metis.ipfn.ist.utl.pt/svn/cdaq/ATCA/SR-TR-ATCA/Software/Linux/include/
 */
#ifndef _PCIE_SRTR_IOCTL_H_
#define _PCIE_SRTR_IOCTL_H_

/*
 * IOCTL definitions
 * $Id$
 */
#define PCIE_SRTR_IOC_MAGIC 'l'  // 
/* Please use a different 8-bit number in your code */
/*See  /Documentation/ioctl-number.txt*/

/* S means "Set": thru a pointer
 * T means "Tell": directly with the argument value
 * G menas "Get": reply by setting thru a pointer
 * Q means "Qry": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

/**********************************************************************
 *                         IOCTL FUNCTIONS                            *
 *********************************************************************/
#define PCIE_SRTR_IOCT_INT_ENABLE    _IO(PCIE_SRTR_IOC_MAGIC, 1)
#define PCIE_SRTR_IOCT_INT_DISABLE   _IO(PCIE_SRTR_IOC_MAGIC, 2)
#define PCIE_SRTR_IOCT_ACQ_ENABLE    _IO(PCIE_SRTR_IOC_MAGIC, 3)
#define PCIE_SRTR_IOCT_ACQ_DISABLE   _IO(PCIE_SRTR_IOC_MAGIC, 4)
#define PCIE_SRTR_IOCS_RDOFF         _IOW(PCIE_SRTR_IOC_MAGIC, 5, u_int32_t)
#define PCIE_SRTR_IOCS_RDTMOUT       _IOW(PCIE_SRTR_IOC_MAGIC, 6, u_int32_t)
#define PCIE_SRTR_IOCT_SOFT_TRIG     _IO(PCIE_SRTR_IOC_MAGIC, 7)
#define PCIE_SRTR_IOCS_ACQBYTESIZE   _IOW(PCIE_SRTR_IOC_MAGIC, 9, u_int32_t)
#define PCIE_SRTR_IOCG_STATUS        _IOR(PCIE_SRTR_IOC_MAGIC, 10, u_int32_t)
#define PCIE_SRTR_IOCG_COMMAND       _IOR(PCIE_SRTR_IOC_MAGIC, 11, u_int32_t)
#define PCIE_SRTR_IOCG_ACQC          _IOR(PCIE_SRTR_IOC_MAGIC, 12, u_int32_t)
#define PCIE_SRTR_IOCS_PLLCFG        _IOW(PCIE_SRTR_IOC_MAGIC, 13, u_int32_t)
#define PCIE_SRTR_IOCS_DATA_PROC_K   _IOW(PCIE_SRTR_IOC_MAGIC, 14, u_int32_t) 
#define PCIE_SRTR_IOCS_DATA_PROC_L   _IOW(PCIE_SRTR_IOC_MAGIC, 15, u_int32_t) 
#define PCIE_SRTR_IOCS_DATA_PROC_M   _IOW(PCIE_SRTR_IOC_MAGIC, 16, u_int32_t) 
#define PCIE_SRTR_IOCS_THRESHOLD     _IOW(PCIE_SRTR_IOC_MAGIC, 17, u_int32_t) 
#define PCIE_SRTR_IOCS_DQTP          _IOW(PCIE_SRTR_IOC_MAGIC, 18, u_int32_t) 
#define PCIE_SRTR_IOCS_POST_TRG      _IOW(PCIE_SRTR_IOC_MAGIC, 19, u_int32_t) 
#define PCIE_SRTR_IOCS_PRE_TRG_DATA  _IOW(PCIE_SRTR_IOC_MAGIC, 20, u_int32_t)
#define PCIE_SRTR_IOCS_ILVM          _IOW(PCIE_SRTR_IOC_MAGIC, 21, u_int32_t)
#define PCIE_SRTR_IOCS_CHAN_ON_OFF   _IOW(PCIE_SRTR_IOC_MAGIC, 22, u_int32_t)
#define PCIE_SRTR_IOCS_COMPL         _IOW(PCIE_SRTR_IOC_MAGIC, 24, u_int32_t) 
#define PCIE_SRTR_IOCS_FILEDATA      _IOW(PCIE_SRTR_IOC_MAGIC, 25, u_int32_t)
#define PCIE_SRTR_IOCS_PTRG    	     _IOW(PCIE_SRTR_IOC_MAGIC, 26, u_int32_t)
#define PCIE_SRTR_IOCS_PWIDTH        _IOW(PCIE_SRTR_IOC_MAGIC, 27, u_int32_t)
#define PCIE_SRTR_IOCS_Trg_Acc       _IOW(PCIE_SRTR_IOC_MAGIC, 28, u_int32_t)
#define PCIE_SRTR_IOCS_TRGM          _IOW(PCIE_SRTR_IOC_MAGIC, 29, u_int32_t)
#define PCIE_SRTR_IOCS_INFIR         _IOW(PCIE_SRTR_IOC_MAGIC, 30, u_int32_t)
#define PCIE_SRTR_IOCS_TFIR          _IOW(PCIE_SRTR_IOC_MAGIC, 31, u_int32_t)
#define PCIE_SRTR_IOCG_EVENT_COUNTER _IOR(PCIE_SRTR_IOC_MAGIC, 32, u_int32_t)
#define PCIE_SRTR_IOCG_ACQK          _IO(PCIE_SRTR_IOC_MAGIC, 33)
#define PCIE_SRTR_IOCT_RT_ENABLE     _IO(PCIE_SRTR_IOC_MAGIC, 34)
#define PCIE_SRTR_IOCT_RT_DISABLE    _IO(PCIE_SRTR_IOC_MAGIC, 35)
#define PCIE_SRTR_IOC_MAXNR   35

#endif /* _PCIE_SRTR_IOCTL_H_ */
