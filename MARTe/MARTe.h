//******************************************************************************
//
//      MARTe
//      $Log: MARTe.h,v $
//      Revision 1.2  2006/05/25 09:12:17  asopp
//      Added little modifications to several files.
//
//      Revision 1.1  2006/05/24 11:24:13  asopp
//      *** empty log message ***
//
//******************************************************************************


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

