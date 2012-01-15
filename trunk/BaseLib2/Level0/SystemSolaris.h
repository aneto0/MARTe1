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

#endif
#endif
