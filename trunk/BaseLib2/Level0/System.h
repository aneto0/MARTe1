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
 * Place holder for all the System.x definitions
 */
#ifndef SYSTEM_H
#define SYSTEM_H

#if !defined (_CINT)

#if (defined(_RSXNT) || defined(__IBMCPP__) || defined(_MSC_VER) || defined(__EMX__) || defined (_CY32) || defined (_LINUX) || defined (_VXWORKS) || defined(_RTAI) || defined(_SOLARIS) || defined(_MACOSX))
#else
    #error Compiler not supported
#endif

#include "SystemMSC.h"
#include "SystemLinux.h"
#include "SystemVX5100.h"
#include "SystemVX5500.h"
#include "SystemV6X5100.h"
#include "SystemV6X5500.h"
#include "SystemVX68k.h"
#include "SystemRTAI.h"
#include "SystemSolaris.h"
#include "SystemMacOSX.h"

#else
#include "SystemCINT.h"
#endif

#include "GenDefs.h"
#include "Memory.h"



// define REAL_MAIN as compilation flag to avoid having the main function called by an auxiliary main function
extern "C"{
    /** This is the main function after renaming. Note that main MUST have the same prototype */
    int UserMainFunction(int argc,char **argv);

    /** A wrapper around the main function. The wrapper allows trapping the main starting and stopping.
        if REAL_MAIN is defined before System.h in all files then the main function is undisturbed
     */
    int MainHandler(int (*userMainFunction)(int argc,char **argv),int argc,char **argv);
}

#if !defined(REAL_MAIN)

// allow interposing our main function
#define main \
main(int argc,char **argv){\
    return MainHandler(UserMainFunction,argc,argv);\
}\
int UserMainFunction

#else // REAL_MAIN

int UserMainFunction(int argc,char **argv){ return 0;}

#endif

#endif

