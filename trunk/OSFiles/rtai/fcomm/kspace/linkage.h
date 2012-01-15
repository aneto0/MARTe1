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

#ifndef FCOMM_LINKAGE
#define FCOMM_LINKAGE

/*#include <linux/version.h>

#if defined(__STANDALONE__) && defined(_RTAI)
    #error YOU CANNOT USE THE STANDALONE MODE WITH Baselib2 PLEASE READ THE README AT fcomm/kspace
#endif
 #if defined(blib_asmlinkage)   //This should be here although its the same as of the next #elif
                                //some times the version.h of /usr/include is not up to date and we have problems
                                //SystemRTAI.h knows how to get the correct value...
        #define fcomm_asmlinkage blib_asmlinkage
        #define fcomm_rt_thread_asmlinkage rt_thread_asmlinkage
    #elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19) && !defined(__STANDALONE__)
        //#define fcomm_asmlinkage __attribute__((regparm(3)))
        //#define fcomm_rt_thread_asmlinkage __attribute__((regparm(0)))
        #define fcomm_asmlinkage 
        #define fcomm_rt_thread_asmlinkage 
    #else
        #define fcomm_asmlinkage 
        #define fcomm_rt_thread_asmlinkage 
    #endif*/
#endif

