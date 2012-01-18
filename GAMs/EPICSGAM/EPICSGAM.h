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
#if !defined(_EPICS_GAM_)
#define _EPICS_GAM_


#include "System.h"
#include "GAM.h"
#include "HttpInterface.h"
#include "MessageHandler.h"

// TODO --start


#include "GCRTemplate.h"
#include "FString.h"

// TODO also add support for DDBOutputInterface as well
//#include "DDBOutputInterface.h"
#include "DDBInputInterface.h"
#include "DDBIOInterface.h"

//#include "RTDataCollector.h"

#include "SignalInterface.h"
// TODO -- end

#include "EPICSSignalsTable.h"



OBJECT_DLL(EPICSGAM)
;

class EPICSGAM
: public GAM, public HttpInterface, public MessageHandler {

OBJECT_DLL_STUFF(EPICSGAM)


private:
	
	static const char * css;
    /** Get Data Collected during the pulse */
  //  GCRTemplate<SignalInterface>                   GetSignal(const FString &jpfSignalName);
    
    // DDB Interface for the Time
    DDBInputInterface                              *usecTime;

    // Fast Trigger Request signal
    DDBIOInterface                                 *fastTrigger;

    // Jpf Data Collection
    DDBInputInterface                              *jpfData;

    
    
    // Has trigger signal
    bool                                           hasTriggerSignal;

    // Data Collector
//    RTDataCollector                                dataCollector;

    // flag to avoid processing messages during pulse
    bool                                           acceptingMessages;
    

    /** SignalTable Database */
    EPICSSignalsTable        signalTable;
    
public:

    EPICSGAM();
    virtual ~EPICSGAM(){};

    // Initialise the module
    // which is the difference between Initialise and Object Load Setup?
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /** Implements the Saving of the parameters to Configuration Data Base */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err) {
    	return True;
    }

    /** Menu Interface */
//    virtual bool MenuInterface(StreamInterface &in,StreamInterface &out,void *userData) {
//        return dataCollector.ObjectDescription(in, True);
//    }

    /**
    * Builds the webpage.
    * @param hStream The HttpStream to write to.
    * @return False on error, True otherwise.
    */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
    
protected:
    // MESSAGE HANDLER INTERFACE
    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

};

#endif
