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
#define PCIE_ATCA_ADC_IOCT_ACQ_ENABLE         _IO(PCIE_ATCA_ADC_IOCT_MAGIC, 1)
#define PCIE_ATCA_ADC_IOCT_ACQ_DISABLE        _IO(PCIE_ATCA_ADC_IOCT_MAGIC, 2)
#define PCIE_ATCA_ADC_IOCT_NUM_BOARDS         _IOR(PCIE_ATCA_ADC_IOCT_MAGIC, 3, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_MASTER_SLOT_NUM    _IOR(PCIE_ATCA_ADC_IOCT_MAGIC, 4, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_SET_EXT_CLK_TRG    _IOW(PCIE_ATCA_ADC_IOCT_MAGIC, 5, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_IS_RTM_PRESENT     _IOWR(PCIE_ATCA_ADC_IOCT_MAGIC, 6, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_SEND_SOFT_TRG      _IO(PCIE_ATCA_ADC_IOCT_MAGIC, 7)
#define PCIE_ATCA_ADC_IOCT_GET_BOARD_SLOT_NS  _IOR(PCIE_ATCA_ADC_IOCT_MAGIC, 8, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_ACQ_DEBUG          _IO(PCIE_ATCA_ADC_IOCT_MAGIC, 9)
#define PCIE_ATCA_ADC_IOCT_N_IN_ANA_CHANNELS  _IOWR(PCIE_ATCA_ADC_IOCT_MAGIC, 10, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_N_IN_DIG_CHANNELS  _IOWR(PCIE_ATCA_ADC_IOCT_MAGIC, 11, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_N_OUT_ANA_CHANNELS _IOWR(PCIE_ATCA_ADC_IOCT_MAGIC, 12, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_N_OUT_DIG_CHANNELS _IOWR(PCIE_ATCA_ADC_IOCT_MAGIC, 13, u_int32_t)
#define PCIE_ATCA_ADC_IOCT_MAXNR   13

#endif /* _PCIE_ATCA_ADC_IOCTL_H_ */
