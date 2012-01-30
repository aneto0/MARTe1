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
#if !defined(_MARTE_H_)
#define _MARTE_H_

/**
    @file
*/

#include "MessageDispatcher.h"
#include "RealTimeThreadPool.h"
#include "MObjectPool.h"
#include "StateManager.h"

/** @mainpage MaRTe: The JET Real Time Manager 
    @section visions Visions
    MaRTe can be seen as an entity made of several parts. It has
    control channels that can be used to recieve commands and send the
    results of the command execution. It has information channels that
    are simply used to acquire and publish data.
    The model used to program the interaction of the MARTe parts is
    based on messages. 
    
*/

/** Real time manager. */
class MARTe{
public:
    /** The object responsable to send messages. */
    MessageDispatcher messageDispatcher;
    /** The pool of Real Time Threads of execution. */
    RealTimeThreadPool rTTPool;
    /** The pool of MObjects that buils the whole system. */
    MObjectPool mObjectPool;
    /** The state machine. */
    StateManager stateMachine;

    /** Main start. */
    bool Start();
    /** Main stop. */
    bool Stop();
    /** Loads the setup for the MObjects. */
    bool ObjectLoadSetup();
    /** ???? */
    bool Menu();
    /** Send a message. Useful when testing the code from a console. */
    bool Message();
};



#endif

