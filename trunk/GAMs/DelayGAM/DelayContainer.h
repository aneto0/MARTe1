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

#if !defined (DELAY_CONTAINER_H)
#define DELAY_CONTAINER_H

#include "GAM.h"
#include "FastPollingMutexSem.h" // CFGOBJ
#include "ConfigurableObject.h" // CFGOBJ

OBJECT_DLL(DelayContainer)
class DelayContainer : public GAM, public ConfigurableObject {
OBJECT_DLL_STUFF(DelayContainer)

private:

    /// DDB interface pointers
    DDBInputInterface        *input;
    DDBOutputInterface       *output;
    
    /// Number of delay operations
    int32                     numberOfDelays;

    ///
    int32                     delayInSamples;

    ///
    int32                     oldDelayInSamples; // CFGOBJ

    ///
    float                     transitionDefaultValue;

    ///
    float                   **delayedSignal;

    ///
    float                   **pointer;

    ///
    uint32                    counter;

private:

    FastPollingMutexSem       sem; // CFGOBJ

public:

    /// Constructor
    DelayContainer() {
	sem.Create(); // CFGOBJ

	input                  = NULL;
	output                 = NULL;

	delayedSignal          = NULL;

	numberOfDelays         = -1;
	delayInSamples         = 0;
	transitionDefaultValue = 0;
    };

    /// Destructor
    ~DelayContainer() {
	if(delayedSignal != NULL) {
	    for(int i = 0 ; i < numberOfDelays ; i++) {
		free((void*&)(*(delayedSignal+i)));
		*(delayedSignal+i) = NULL;
	    }
	    free((void*&)(delayedSignal));
	    delayedSignal = NULL;
	}
	if(pointer != NULL) {
	    free((void*&)pointer);
	    pointer = NULL;
	}
    };
    
    /// Reset all delays
    void  Reset();

    /// GAM configuration
    bool  Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool  Execute(GAM_FunctionNumbers execFlag);

    bool  ProcessMessage(GCRTemplate<MessageEnvelope> envelope);
};
#endif
