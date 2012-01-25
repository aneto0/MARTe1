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
#ifndef RTAICONSOLE_H_
#define RTAICONSOLE_H_

#include <asm/ioctl.h>


#define    RTAICONSOLE_IOCTL_MAGIC          0xc0

#define    RTAIFunctionCall                 _IOWR(RTAICONSOLE_IOCTL_MAGIC, 0, int)

#define    RTAICopyToKernel                 _IOWR(RTAICONSOLE_IOCTL_MAGIC, 1, int)

#define    RTAICopyFromKernel               _IOWR(RTAICONSOLE_IOCTL_MAGIC, 2, int)

#define    RTAIFreeKernelMemory             _IOWR(RTAICONSOLE_IOCTL_MAGIC, 3, int)

#define    RTAIMaxNumberParameters          8

#define    RTAIFunctionNumberOfParameters   (RTAIMaxNumberParameters + 2)

//Please check that this UIDs don't clash with FCOMM UIDS
#define    FUN_CALLER_TASK_UID              6999

#define    FUN_CALLER_SEM_UID               6998

#endif /*RTAICONSOLE_H_*/
