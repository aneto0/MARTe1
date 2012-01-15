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
 * CERN ROOT cint support
 */
#ifndef SYSTEM_CINT_H
#define SYSTEM_CINT_H
// it's an intel PLATFORM
#define INTEL_BYTE_ORDER
#define INTEL_PLATFORM

/** Dummy struct */
struct unknown{
    void *nowhere;
};

#define fd_set unknown
#define timeval unknown
#define sockaddr_in unknown
#define servent unknown
#define __cdecl

#define sock_errno() errno
#define psock_errno  perror
#define soclose      closesocket
#define volatile

#define ioctl(a,b,c,d) ioctlsocket(a,b,(u_long FAR *)c)

#define __thread __cdecl
#define __thread_decl __cdecl

#define  vsnprintf  _vsnprintf

#define private public
#define HANDLE long
#define HFILE HANDLE
#define HMTX  HANDLE
#define HEV   HANDLE
#define TID   HANDLE
#define HFILE HANDLE
#define APIRET DWORD
#define HINSTANCE long

#define EWOULDBLOCK WSAEWOULDBLOCK
#define ECONNREFUSED WSAECONNREFUSED
#define EINPROGRESS WSAEINPROGRESS
#define ECONNRESET WSAECONNRESET
#define ETIMEDOUT WSAETIMEDOUT
#define ENOTCONN WSAENOTCONN


#if   defined(_EXPORTING)
        #define DllExport
#elif defined(_IMPORTING)
        #define DllExport
#else
        #define DllExport
#endif

//#define _EXTERN_ __declspec(dllimport)
#define _EXTERN_ extern

//#if !defined(INFINITE)
//#define  SEM_INDEFINITE_WAIT 0xFFFFFFFF
//#define INFINITE 0xFFFFFFFF
//#endif

#endif

