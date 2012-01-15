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

#include "FilterContainer.h"
#include "CDBExtended.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "LoadCDBObjectClass.h"

/// This method runs at initialisation
bool FilterContainer::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    ////////////////////////////////////////////////////
    //                Add interfaces to DDB           //
    ////////////////////////////////////////////////////

    if(!AddInputInterface(input,"InputInterface")) {
        AssertErrorCondition(InitialisationError,"FilterContainer::Initialise: %s failed to add input interface", Name());
        return False;
    }

    if(!AddOutputInterface(output,"OutputInterface")) {
        AssertErrorCondition(InitialisationError,"FilterContainer::Initialise: %s failed to add output interface", Name());
        return False;
    }

    cdb->Move("InputSignals");
    if(!input->ObjectLoadSetup(cdb,NULL)) {
	AssertErrorCondition(InitialisationError,"FilterContainer::ObjectLoadSetup: %s: ObjectLoadSetup Failed DDBInterface %s ", Name(), input->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    cdb->Move("OutputSignals");
    if(!output->ObjectLoadSetup(cdb,NULL)) {
	AssertErrorCondition(InitialisationError,"FilterContainer::ObjectLoadSetup: %s: ObjectLoadSetup Failed DDBInterface %s ", Name(), output->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    /* Check for matching number of input and output signals */
    if(input->BufferWordSize() != output->BufferWordSize()) {
	AssertErrorCondition(InitialisationError,"FilterContainer::ObjectLoadSetup: %s number of input signals != number of output signals", Name());
	return False;
    } else {
	numberOfFilters = input->BufferWordSize();
    }

    /* Create filter objects */
    f = new Filter[numberOfFilters];

    for(int i = 0 ; i < numberOfFilters ; i++) {
	/** Initialise filter objects */
	if(!f[i].Init(cdb)) {
	    AssertErrorCondition(InitialisationError, "FilterContainer::Initialise: %s Unable to initialise digital filter of index %d", Name(), i);
	    return False;
	}
    }

    return True;
}

/// Resets all the filters
void FilterContainer::Reset() {
    for(int i = 0 ; i < numberOfFilters ; i++) {
	f[i].Reset();
    }
}

/// Called in every control loop
bool FilterContainer::Execute(GAM_FunctionNumbers execFlag) {

    /// Create interface data pointers
    float *inputData  = (float *)input->Buffer();
    float *outputData = (float *)output->Buffer();

    /// Read input signals from the DDB
    input->Read();

    switch(execFlag) {
        case GAMPrepulse:
            // Reset all the filters
            Reset();
	    for(int i = 0 ; i < numberOfFilters ; i++) {
		outputData[i] = f[i].Process(inputData[i]);
	    }
        break;
        
        case GAMOffline:
        case GAMOnline: 
	    /// Apply filters
	    for(int i = 0 ; i < numberOfFilters ; i++) {
		outputData[i] = f[i].Process(inputData[i]);
	    }
	break;
    }

    /// Write output to the DDB
    output->Write();

    return True;
}
OBJECTLOADREGISTER(FilterContainer, "$Id: FilterContainer.cpp,v 1.3 2011/04/09 21:41:57 dalves Exp $")
