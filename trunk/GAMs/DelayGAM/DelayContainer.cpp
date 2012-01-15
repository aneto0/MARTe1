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

#include "DelayContainer.h"
#include "CDBExtended.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"

/// This method runs at initialisation
bool DelayContainer::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    SetMessageSender(this); // CFGOBJ
    CreateProperty("DelayInSamples", CL_Int32, &delayInSamples); // CFGOBJ

    ////////////////////////////////////////////////////
    //                Add interfaces to DDB           //
    ////////////////////////////////////////////////////

    if(!AddInputInterface(input,"InputInterface")) {
        AssertErrorCondition(InitialisationError,"DelayContainer::Initialise: %s failed to add input interface", Name());
        return False;
    }

    if(!AddOutputInterface(output,"OutputInterface")) {
        AssertErrorCondition(InitialisationError,"DelayContainer::Initialise: %s failed to add output interface", Name());
        return False;
    }

    cdb->Move("InputSignals");
    if(!input->ObjectLoadSetup(cdb,NULL)) {
	AssertErrorCondition(InitialisationError,"DelayContainer::ObjectLoadSetup: %s: ObjectLoadSetup Failed DDBInterface %s ", Name(), input->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    cdb->Move("OutputSignals");
    if(!output->ObjectLoadSetup(cdb,NULL)) {
	AssertErrorCondition(InitialisationError,"DelayContainer::ObjectLoadSetup: %s: ObjectLoadSetup Failed DDBInterface %s ", Name(), output->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    /* Check for matching number of input and output signals */
    if(input->BufferWordSize() != output->BufferWordSize()) {
	AssertErrorCondition(InitialisationError,"DelayContainer::ObjectLoadSetup: %s number of input signals != number of output signals", Name());
	return False;
    } else {
	numberOfDelays = input->BufferWordSize();
    }

    if(!cdb.ReadInt32(delayInSamples, "DelayInSamples", 0)) {
	AssertErrorCondition(Warning, "DelayContainer::Initialise() %s DelayInSamples not specified, assuming %d", Name(), delayInSamples);
    } else if(delayInSamples < 0) {
	AssertErrorCondition(Warning, "DelayContainer::Initialise() %s DelayInSamples < 0", Name());
	return False;
    }
    if(!cdb.ReadFloat(transitionDefaultValue, "TransitionDefaultValue", 0)) {
	AssertErrorCondition(Warning,"DelayContainer::Initialise() %s TransitionDefaultValue not specified, assuming %d", Name(), transitionDefaultValue);
    }

    /* Create signal holders */
    delayedSignal = (float **)malloc(numberOfDelays*sizeof(float *));
    for(int i = 0 ; i < numberOfDelays ; i++) {
	if(delayInSamples) {
	    *(delayedSignal+i) = (float *)malloc(delayInSamples*sizeof(float));
	} else {
	    *(delayedSignal+i) = NULL;
	}
    }

    /// Create array of pointers
    pointer = (float **)malloc(numberOfDelays*sizeof(float *));
    
    /* Initialise all the delays */
    Reset();

    return True;
}

/// Resets all the delays
void DelayContainer::Reset() {
    /* Initialise signal arrays */
    for(int i = 0 ; i < numberOfDelays ; i++) {
    	for(int j = 0 ; j < delayInSamples ; j++) {
    	    delayedSignal[i][j] = transitionDefaultValue;
    	}
    }

    for(int i = 0 ; i < numberOfDelays ; i++) {
    	pointer[i] = delayedSignal[i];
    }
    counter = 1;
}

/// Called in every control loop
bool DelayContainer::Execute(GAM_FunctionNumbers execFlag) {

    /// Create interface data pointers
    float *inputData  = (float *)input->Buffer();
    float *outputData = (float *)output->Buffer();

    /// Read input signals from the DDB
    input->Read();

    if(!sem.FastTryLock()) { // CFGOBJ
	return True;
    }
    
    if(delayInSamples > 0) {

        switch(execFlag) {
            case GAMPrepulse:
                // Reset all the delays
                Reset();
            break;
        
            case GAMOffline:
            case GAMOnline: 
                /// Calculate and apply delays
	        for(int i = 0 ; i < numberOfDelays ; i++) {
		    outputData[i] = *pointer[i];
		    *pointer[i]   = inputData[i];
		    pointer[i]++;
	        }

	        if(counter < delayInSamples-1) {
		    counter++;
	        } else {
		    for(int i = 0 ; i < numberOfDelays ; i++) {
		        pointer[i] = delayedSignal[i];
		    }
		    counter = 1;
	        }
	    
	    break;
        }

    } else {
	outputData[0] = inputData[0];
    }

    sem.FastUnLock(); // CFGOBJ

    /// Write output to the DDB
    output->Write();

    return True;
}
// CFGOBJ
bool DelayContainer::ProcessMessage(GCRTemplate<MessageEnvelope> envelope) {

    TimeoutType to; // in msecs

    sem.FastLock(to);
    oldDelayInSamples = delayInSamples;
    if(ConfigurableObject::ProcessMessage(envelope)) {
	if(delayInSamples > 0) {
	    for(int i = 0 ; i < numberOfDelays ; i++) {
		float *pointerAux = (float *)calloc(delayInSamples, sizeof(float));
		if(pointerAux == NULL) {
		    AssertErrorCondition(FatalError, "DelayContainer::Initialise() %s unable to allocate memory for new delay", Name());
		    sem.FastUnLock();
		    return False;
		}
		pointer[i] = pointerAux;
		free((void*&)delayedSignal[i]);
		delayedSignal[i] = pointerAux;
	    }
	    counter = 1;
	} else {
	    delayInSamples = oldDelayInSamples;
	    AssertErrorCondition(Warning, "DelayContainer::Initialise() %s value not allowed", Name());
	}
    } else {
	AssertErrorCondition(Warning, "DelayContainer::ProcessMessage() %s timeout on mux or failed CfgLib message parsing", Name());
    }

    sem.FastUnLock();
	
    return True;
}
OBJECTLOADREGISTER(DelayContainer, "$Id: DelayContainer.cpp,v 1.4 2011/01/10 16:53:50 dalves Exp $")
