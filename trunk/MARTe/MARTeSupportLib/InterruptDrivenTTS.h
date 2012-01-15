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

#if !defined(_INTERRUPT_DRIVEN_TTS_)
#define _INTERRUPT_DRIVEN_TTS_

#include "System.h"
#include "GCRTemplate.h"
#include "GCReferenceContainer.h"

#include "TimeTriggeringServiceInterface.h"
#include "EventSem.h"

/** External Time and Triggering Service Class.
    It's a container of TimeServiceActivity objects */
OBJECT_DLL(InterruptDrivenTTS)

class InterruptDrivenTTS : public TimeTriggeringServiceInterface{

private:

    /** Event Sem to inform the Real Time Thread of the start of a new cycle */
    EventSem                             synchSem;

    /** Semaphore Time Out */
    TimeoutType                          timeOut;
    
public:

    /** Constructor */
    InterruptDrivenTTS(){
        synchSem.Create();
        timeOut = TTInfiniteWait;
    };

    /** Destructor */
    virtual ~InterruptDrivenTTS(){
        // Stop Activities
        Stop();
        // Close the semaphore
        synchSem.Close();
    }

    //////////////////////////////////
    // Interfaces to be implemented //
    //////////////////////////////////

    /** Load External Time And Triggering Service configuration parameters. */
    virtual bool     ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** */
    virtual bool     ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

    /** Synchronizes the system to the cycle time.
        For Triggering methods posted by interrupts the
        Synchronise() function waits on a semaphore to be posted
        by the ISR(), namely the SignalNewCycle() function.
        For Triggering methods based on data arrival without 
        interrupts, the Synchronise() function returns immediately 
        and the synchronisation is performed by the GetData method of
        the synchronising module. 
     */
    virtual bool     Synchronise(){return synchSem.ResetWait(timeOut);}

private:

    /** This function is used within the Trigger() method.
        It performs a set of activities marking the start of the new real-time
        cycle. For interrupt driven Synchronising methods, the SignalNewCycle()
        function posts the semaphore the Synchronise function is waiting on.
        For Triggering methods based on data arrival, the SignalNewCycle returns
        without performing any activity. 
     */
    virtual bool     SignalNewCycle(){return synchSem.Post();};
    

OBJECT_DLL_STUFF(InterruptDrivenTTS)
};

#endif
