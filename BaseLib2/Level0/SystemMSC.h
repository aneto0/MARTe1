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
 * Microsoft windows definition
 */
#ifndef SYSTEM_MSC_H
#define SYSTEM_MSC_H

#if defined(_MSC_VER)

// it's an intel PLATFORM
#define INTEL_BYTE_ORDER
#define INTEL_PLATFORM

// we can use RTTI
#define RTTI_AVAILABLE

// exception handler installed
#define EH_AVAILABLE

// start logger at application startup
#define LOGGER_AUTOSTART

// check boundary on MATRIX calls
#define MATRIX_BOUNDARY_CHECK

// NT 4 compatibility level
#define _WIN32_WINNT 0x400

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <float.h>
#include <math.h>
#include <io.h>
#include <sys/types.h>
#include <windows.h>
#include <winuser.h>
#include <winbase.h>
#include <process.h>
#include <conio.h>
#include <typeinfo.h>

#if (_MSC_VER < 1400)
#include <largeint.h>
#include <winsock2.h>
#endif

#if !defined(_WIN32)
#define _WIN32
#endif

#include "GenDefs.h"

// sock_init() implemented in InternetAddress.cpp
extern WSADATA globalWsaData; // implemented in InternetAddress.cpp

#define sock_errno() errno
#define psock_errno  perror
#define soclose      closesocket

#define ioctl(a,b,c,d) ioctlsocket(a,b,(u_long FAR *)c)


#define __thread __cdecl
#define __thread_decl __cdecl

#define  vsnprintf  _vsnprintf
#define  snprintf   _snprintf

#define HANDLE void *
#define HFILE HANDLE
#define HMTX  HANDLE
#define HEV   HANDLE
#define TID   HANDLE
#define HFILE HANDLE
#define APIRET DWORD

#define EWOULDBLOCK WSAEWOULDBLOCK
#define ECONNREFUSED WSAECONNREFUSED
#define EINPROGRESS WSAEINPROGRESS
#define ECONNRESET WSAECONNRESET
#define ETIMEDOUT WSAETIMEDOUT
#define ENOTCONN WSAENOTCONN


#if   defined(_EXPORTING)
        #define DllExport    __declspec(dllexport)
#elif defined(_IMPORTING)
        #define DllExport    __declspec(dllimport)
#else
        #define DllExport
#endif

//#define _EXTERN_ __declspec(dllimport)
#define _EXTERN_ extern


//#if !defined(SEM_INDEFINITE_WAIT)
//#define  SEM_INDEFINITE_WAIT INFINITE
//#endif
#undef INFINITE

inline int strcasecmp(const char *s1, const char *s2 ){
    if (s1 == s2) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;

    register char c1= toupper(*s1);
    register char c2= toupper(*s2);
    while ((c1!=0) && (c2!=0)){
        if (c1>c2) return 1;
        if (c1<c2) return -1;
        s1++;
        s2++;
        c1= toupper(*s1);
        c2= toupper(*s2);
    }
    if ((c1==0) && (c2==0)) return 0;
    if (c1==0) return -1;
    return 1;
}


inline int strncasecmp(const char *s1, const char *s2, int n ){
    if (s1 == s2) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;

     register char c1= toupper(*s1);
     register char c2= toupper(*s2);
     while ((c1!=0) && (c2!=0) && (n>0)){
         if (c1>c2) return 1;
         if (c1<c2) return -1;
         s1++;
         s2++;
         n--;
         c1= toupper(*s1);
         c2= toupper(*s2);
     }
     if (n==0) return 0;
     if ((c1==0) && (c2==0)) return 0;
     if (c1==0) return -1;
     return 1;
}

//
// The strcasestr() function is a non-standard extension and is not provided by MSC.
// Note that a copy of this function is also provided in the VxWorks system headers.
//
// s1 = haystack, s2 = needle
inline char *strcasestr(const char *s1, const char *s2 ){

    int32  hsIdx;
    int32  ndIdx;
    int32  s1Len;
    int32  s2Len;
    int32  match = 0;

    s1Len = strlen(s1);
    s2Len = strlen(s2);

    if (s1 == s2) {
        return (char *)s1;
    }

    if ((s1 == NULL) || (s2 == NULL) || (s2Len > s1Len)) return NULL;

    for(hsIdx = 0 ; hsIdx < s1Len-s2Len+1 ; hsIdx++) {
        int32 result = 1;
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

#define IEEE_FLOAT_NAN 0x7fC00000
#define IEEE_FLOAT_INF 0x7f800000
#define IEEE_FLOAT_INF_NEG 0xFf800000

#endif
#endif
