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
 * Linux definitions
 */
#ifndef SYSTEM_LINUX_H
#define SYSTEM_LINUX_H

#if defined(_LINUX)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <float.h>
#include <math.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#if defined(_NCURSES)
#include <ncurses.h>
#endif

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/shm.h>
#include <dirent.h>
#include <ctype.h>
#include <dlfcn.h>
#include <typeinfo>

#include <sys/timeb.h>
#include <termio.h>

#include "GenDefs.h"

#define TID pthread_t
#define __thread_decl
#define HANDLE intptr
//#define SEM_INDEFINITE_WAIT -1
#define HFILE HANDLE
//#define INFINITE -1

#define soclose close
#define sock_errno()  errno
// sock_init() moved to InternetAddress.cpp

#define INTEL_BYTE_ORDER
#define INTEL_PLATFORM

#define _EXTERN_ extern

#define IEEE_FLOAT_NAN 0x7fC00000
#define IEEE_FLOAT_INF 0x7f800000
#define IEEE_FLOAT_INF_NEG 0xFf800000

#define _snprintf snprintf

//#define LOGGER_AUTOSTART

#define STDOUT 1
#define STDIN 0

#endif
#endif

