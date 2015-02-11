/**
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
 * $Id: ExceptionHandlerInterface.h 3 2012-01-15 16:26:07Z aneto $
 *
 */

/**
 * @file
 * Exception handling interface. Place holder for future implementations
 */
#ifndef EXCEPTION_HANDLER
#define EXCEPTION_HANDLER

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


/** Exception handler plugin interface. */
class ExceptionHandlerInterface{
private:
    /** What to do in case of exception. */
    uint32 action;
public:
    /** Pass handling back to OS. */
    static const uint32 NotHandled      = 0x1,
    /** Handled: program can continue. */
    static const uint32 XH_ContinueExec = 0x2,
    /** Handled: jump to thread start and proceed to termination. */
    static const uint32 XH_KillThread   = 0x3,
    /** Handled: jump to thread start and terminate task. */
    static const uint32 XH_KillTask     = 0x4,
    /** Handled: jump to thread start and retry. */
    static const uint32 XH_TryAgain     = 0x5,
    /** Pass handling back to other handlers in stack. */
    static const uint32 XH_TryOther     = 0x6,
    /** do not report error information (ored). */
    static const uint32 XH_NoReport     = 0x10000

public:
    /** Default handler. */
    ExceptionHandlerInterface(uint32 action = XH_NotHandled){
        this->action = action;
    }

    /** This is the action to be performed. */
    virtual uint32 Catch(ExceptionInformation &info){
        return action;
    }
};

#endif

