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
 * Definitions for the exception handling
 */ 

#ifndef EXCEPTION_HANDLER_DEFINITIONS
#define EXCEPTION_HANDLER_DEFINITIONS

#include "System.h"
/** Exeception handling options. */
enum ExceptionHandlerBehaviour {
    /** Pass handling back to OS. */
    XH_NotHandled     = 0x1,
    /** Handled: program can continue. */
    XH_ContinueExec   = 0x2,
    /** Handled: jump to thread start and proceed to termination. */
    XH_KillThread     = 0x3,
    /** Handled: jump to thread start and terminate task. */
    XH_KillTask       = 0x4,
    /** Handled: jump to thread start and retry. */
    XH_TryAgain       = 0x5,
    /** Pass handling back to other handlers in stack. */
    XH_TryOther       = 0x6,
    /** do not report error information (ored). */
    XH_NoReport       = 0x10000
};



#endif
