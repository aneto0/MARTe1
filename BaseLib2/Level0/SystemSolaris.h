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
 * Solaris implementation
 */
#ifndef SYSTEM_SOLARIS
#define SYSTEM_SOLARIS

#if defined(_SOLARIS)

#define SOLARIS_PLATFORM

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <setjmp.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <typeinfo>

#if defined USE_PTHREAD
#include <pthread.h>
#include <signal.h>
#include <synch.h>
#include <sys/timeb.h>

#endif

#include <sys/filio.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <sys/stat.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <dirent.h>
#include <dlfcn.h>

#include <fcntl.h>

#include <sys/reboot.h>
#include <semaphore.h>

#include "GenDefs.h"

#define CPPDEFS\
    static void operator delete(void *p){ free(p); } \
    static void *operator new(unsigned int size) { return malloc(size); }

/* Threads Definitions  */

#define __thread_decl
#define TID unsigned int

#include "Memory.h"


/* SemCore Definitions  */
#define HANDLE  uint32
//#define INFINITE -1
//#define SEM_INDEFINITE_WAIT INFINITE

#define _EXTERN_ extern
#define ERROR -1

/* File Definitions*/
#define HFILE HANDLE


#define IEEE_FLOAT_NAN 0x7fC00000
#define IEEE_FLOAT_INF 0x7f800000
#define IEEE_FLOAT_INF_NEG 0xFf800000

/* Socket Definitions*/
#define soclose close
#define sock_errno()  errno
// sock_init() moved to InternetAddress.cpp

#define _snprintf snprintf

//
// The strcasestr() function is a non-standard extension and is not provided by Solaris libc.
// Note that a copy of this function is also provided in the VxWorks system headers.
//
// s1 = haystack, s2 = needle
inline char *strcasestr(const char *s1, const char *s2 ){

    int    hsIdx;
    int    ndIdx;
    int    s1Len;
    int    s2Len;
    int    match = 0;

    s1Len = strlen(s1);
    s2Len = strlen(s2);

    if (s1 == s2) {
	return (char *)s1;
    }

    if ((s1 == NULL) || (s2 == NULL) || (s2Len > s1Len)) return NULL;

    for(hsIdx = 0 ; hsIdx < s1Len-s2Len+1 ; hsIdx++) {
        int result = 1;
	for(ndIdx = 0 ; ndIdx < s2Len ; ndIdx++) {
	    register char c1 = toupper(s1[hsIdx+ndIdx]);
	    register char c2 = toupper(s2[ndIdx]);
	    result &= (c1 == c2);
	}
	if(result) {
	  match = 1;
	  break; 
	}
    }

    if (match) {
      return ((char *)(&s1[hsIdx]));
    } else {
      return NULL;
    }
}


#endif
#endif
