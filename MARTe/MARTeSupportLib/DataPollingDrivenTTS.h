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

#if !defined(_DATA_POLLING_DRIVEN_TTS_)
#define _DATA_POLLING_DRIVEN_TTS_

#include "System.h"
#include "GCRTemplate.h"
#include "GCReferenceContainer.h"

#include "TimeTriggeringServiceInterface.h"

/** External Time and Triggering Service Class.
    It's a container of TimeServiceActivity objects */
OBJECT_DLL(DataPollingDrivenTTS)

class DataPollingDrivenTTS : public TimeTriggeringServiceInterface{

public:

    /** Constructor */
    DataPollingDrivenTTS(){
        polledDataReady = False;
	reportSyncError = True;
    };

    /** Destructor */
    ~DataPollingDrivenTTS(){
        // Stop Activities
        Stop();
    }

    //////////////////////////////////
    // Interfaces to be implemented //
    //////////////////////////////////

    /** Load External Time And Triggering Service configuration parameters. */
    virtual bool     ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** */
    virtual bool     ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

    /** Synchronizes the system to the cycle time.
        The synchronization is done by the GetData function in the 
        GenericAcqModule. 
     */
    virtual bool     Synchronise(){
        bool ok = False;
        polledDataReady = False;
        while(!polledDataReady){
            ok = timeModule->Poll();
            if(!ok){
		if(reportSyncError == True) {
	    		AssertErrorCondition(FatalError, "DataPollingDrivenTTS::Syncronise exiting with false due to timing module.");
			reportSyncError == False;
		}
                break;
            } else {
		reportSyncError = True;
	    }
        }
        return ok;
    }

private:

    /** This function is used within the Trigger() method.
        It performs a set of activities marking the start of the new real-time
        cycle. For interrupt driven Synchronising methods, the SignalNewCycle()
        function posts the semaphore the Synchronise function is waiting on.
        For Triggering methods based on data arrival, the SignalNewCycle returns
        without performing any activity. 
     */
    virtual bool     SignalNewCycle(){
        polledDataReady = True;
        return True;
    };
    
    bool             polledDataReady;

    bool	     reportSyncError;
OBJECT_DLL_STUFF(DataPollingDrivenTTS)
};

#endif
