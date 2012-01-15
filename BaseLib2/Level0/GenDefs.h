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
 * Definition of the basic properties and types.
 */

#ifndef GENERIC_DEFS_H
#define GENERIC_DEFS_H

#if defined (__GNUG__)
#define _System
#endif

/** Another name for a float. */
typedef float           real;           // float should be enough

#if defined(__IBMCPP__)
typedef int           bool;
#endif

#if defined(_MSC_VER)
#define           bool int
#endif


#if defined(_CINT)
/** 32 Bit unsigned integer. */
#define uint32 unsigned int
/** 32 Bit signed integer. */
#define int32 int
/** 16 Bit unsigned integer. */
#define uint16 unsigned short
/** 16 Bit signed integer. */
#define int16 short
/** 8 Bit unsigned integer. */
#define uint8 unsigned char
/** 8 Bit signed integer. */
#define int8 char

#else

#if defined(_MSC_VER)
/** 64 Bit unsigned integer. */
typedef unsigned _int64    uint64;
/** 64 Bit signed integer. */
typedef _int64             int64;
#elif defined(__IBMCPP__)

//!!!
//#warning !! the 64 bit support does not work on IBMC
//!!

/** 32 Bit unsigned integer. */
typedef unsigned long int  uint64;
/** 32 Bit signed integer. */
typedef signed long int    int64;
#else
/** 64 Bit unsigned integer. */
typedef unsigned long long uint64;
/** 64 Bit signed integer. */
typedef long long          int64;
#endif
/** Large enought to store a pointer*/
#ifdef __LP64__
typedef unsigned long      intptr;
#elif defined __ILP64__
typedef unsigned long      intptr;
#elif defined __LLP64__
typedef unsigned long long intptr;
#else
typedef unsigned long      intptr;
#endif

#if defined(_V6X5100) || defined(_V6X5500)
/** 32 Bit unsigned integer. */
typedef unsigned long int       uint32;
/** 32 Bit signed integer. */
typedef signed long int         int32;
#else
/** 32 Bit unsigned integer. */
typedef unsigned int       uint32;
/** 32 Bit signed integer. */
typedef signed int         int32;
#endif


/** 16 Bit unsigned integer. */
typedef unsigned short     uint16;
/** 16 Bit signed integer. */
typedef signed   short     int16;
/** 8 Bit unsigned integer. */
typedef unsigned char      uint8;
/** 8 Bit signed integer. */
typedef signed   char      int8;
#endif

#ifndef True
/** Portable definition of true. */
#define True   (1==1)
/** Portable definition of false. */
#define False  (1==0)
#endif

/** The console colours codes*/
#if defined(_RTAI)
/** converts a color to Unix Terminal Colour*/
enum Colours{
    Black = 0,
    Red = 1,
    Green = 2,
    Yellow = 3,
    Blue = 4,
    Purple = 5,
    Cyan = 6,
    White = 7,
    Grey = 8,
    DarkRed = 9,
    DarkGreen = 10,
    DarkYellow = 11,
    DarkBlue = 12,
    DarkPurple = 13,
    DarkCyan = 14,    
    DarkGrey = 15
};

#elif defined(_LINUX) || defined(_MACOSX)

enum Colours{
    Black = 0,
    Red = 1,
    Green = 2,
    Yellow = 3,
    Blue = 4,
    Purple = 5,
    Cyan = 6,
    White = 7,

//do not recognized    

    Grey = 8,
    DarkRed = 9,
    DarkGreen = 10,
    DarkYellow = 11,
    DarkBlue = 12,
    DarkPurple = 13,
    DarkCyan = 14,    
    DarkGrey = 15
    
};
#else

enum Colours{
    /** */
    Black       =  0,
    /** */
    DarkBlue    =  1,
    /** */
    DarkGreen   =  2,
    /** */
    DarkCyan    =  3,
    /** */
    DarkRed     =  4,
    /** */
    DarkPurple  =  5,
    /** */
    DarkYellow  =  6,
    /** */
    Grey        =  7,
    /** */
    DarkGrey    =  8,
    /** */
    Blue        =  9,
    /** */
    Green       = 10,
    /** */
    Cyan        = 11,
    /** */
    Red         = 12,
    /** */
    Purple      = 13,
    /** */
    Yellow      = 14,
    /** */
    White       = 15
};
    
#endif


#if defined(_VXWORKS) || defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX)
/** A tool to find indexes of structures fields.
    1024 has been used to avoid alignment problems. */
#define indexof(type,field) ((intptr)&(((type *)1024)->field) - 1024)
/** A tool to find the size of structures fields.
    1024 has been used to avoid alignment problems. */
#define msizeof(type,field) sizeof(((type *)1024)->field)


#else
/** A tool to find indexes of structures fields. */
#define indexof(type,field) ((intptr)&(((type *)0)->field))
/** A tool to find the size of structures fields. */
#define msizeof(type,field) sizeof(((type *)0)->field)

#endif

/** Builds a 64 bit field with two 32 bit values. */
#define load64(x,a,b)  ((uint32 *)&x)[0] = b; ((uint32 *)&x)[1] = a;

#endif

