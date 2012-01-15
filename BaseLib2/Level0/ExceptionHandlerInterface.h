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
 * $Id$
 *
 */

/**
 * @file
 * Exception handling interface.
 */
#ifndef EXCEPTION_HANDLER
#define EXCEPTION_HANDLER

#include "System.h"
#include "LinkedListable.h"
#include "ExceptionHandlerDefinitions.h"

class ExceptionInformation;

/** Exception handler plugin interface. */
class ExceptionHandlerInterface: public LinkedListable{
    /** What to do in case of exception. */
    uint32 action;
public:
#if defined(_WIN32)
    /** Saves the return context here! */
    _CONTEXT                context;
#endif

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

