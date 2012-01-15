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

#include "DelayGAM.h"
#include "CDBExtended.h"

/// This method runs at GAM initialisation
bool DelayGAM::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    /* Check the number of child objects */
    if(Size() <= 0) {
        AssertErrorCondition(InitialisationError, "DelayGAM::Initialise: %s no delay containers found", Name());
        return False;
    }

    /* Create the list to be executed in real-time*/
    executableList = new GCRTemplate<DelayContainer>[Size()];
    if(executableList == NULL){
        AssertErrorCondition(InitialisationError, "DelayGAM::Initialise: %s failed to allocate the executableList array with %d elements", Name(), Size());
        return False;
    }

    /* Check if all child objects are of the DelayContainer class type */
    for(int i = 0 ; i < Size() ; i++) {
        GCRTemplate<DelayContainer> gcrcdc = Find(i);
        if(!gcrcdc.IsValid()) {
            AssertErrorCondition(InitialisationError, "DelayGAM::Initialise: %s child object is not of the DelayContainer class type", Name());
            return False;
        }
        executableList[i] = gcrcdc;
    }

    return True;
}

/// Called in every control loop
bool DelayGAM::Execute(GAM_FunctionNumbers execFlag) {

    switch(execFlag) {
        case GAMPrepulse:
            // Reset all the filters
            for(int i = 0 ; i < Size() ; i++) {
            executableList[i]->Reset();
	    }
        break;

        case GAMOffline:
        case GAMOnline: 
	    /// Apply filters
	    for(int i = 0 ; i < Size() ; i++) {
            executableList[i]->Execute(execFlag);
	    }
        break;
    }

    return True;
}
OBJECTLOADREGISTER(DelayGAM, "$Id$")
