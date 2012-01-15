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

#ifndef LINUX_VERSION_H
#define LINUX_VERSION_H

/*//You can find this value in your kernel source directory.
#define LINUX_VERSION_CODE 132628
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
    #define blib_asmlinkage __attribute__((regparm(3)))
    #define rt_thread_asmlinkage __attribute__((regparm(0)))   
#else
    #define blib_asmlinkage 
    #define rt_thread_asmlinkage 
#endif*/

#endif
