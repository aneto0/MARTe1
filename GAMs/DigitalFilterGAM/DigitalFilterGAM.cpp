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

#include "DigitalFilterGAM.h"
#include "CDBExtended.h"

/// This method runs at GAM initialisation
bool DigitalFilterGAM::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    /* Check the number of child objects */
    if(Size() <= 0) {
	AssertErrorCondition(InitialisationError, "DigitalFilterGAM::Initialise: %s no filter containers found", Name());
	return False;
    }

    executableList = new GCRTemplate<FilterContainer>[Size()];
    if(executableList == NULL){
        AssertErrorCondition(InitialisationError, "ExpEvalGAM::Initialise: %s failed to allocate the executableList array with %d elements", Name(), Size());
        return False;
    }
    /* Check if all child objects are of the FilterContainer class type */
    for(int i = 0 ; i < Size() ; i++) {
        GCRTemplate<FilterContainer> gcrcfc = Find(i);
        if(!gcrcfc.IsValid()) {
            AssertErrorCondition(InitialisationError, "DigitalFilterGAM::Initialise: %s child object is not of the filter container type", Name());
            return False;
        }
        executableList[i] = gcrcfc;
    }

    return True;
}

/// Called in every control loop
bool DigitalFilterGAM::Execute(GAM_FunctionNumbers execFlag) {

    for(int i = 0 ; i < Size() ; i++) {
        executableList[i]->Execute(execFlag);
    }

    return True;
}
OBJECTLOADREGISTER(DigitalFilterGAM, "$Id$")
