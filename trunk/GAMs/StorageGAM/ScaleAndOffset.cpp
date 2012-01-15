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

#include "ScaleAndOffset.h"

bool ScaleAndOffset::Initialise(ConfigurationDataBase& cdbData) {

    CDBExtended cdb(cdbData);

    if(!AddInputInterface(input, "InputInterface")){
        AssertErrorCondition(InitialisationError,"ScaleAndOffset::Initialise: %s failed to add InputInterface", Name());
        return False;
    }

    if(!cdb->Move("InputSignal")) {
        AssertErrorCondition(InitialisationError,"ScaleAndOffset::Initialise: %s: cannot move to InputSignal", Name());
        return False;
    }
    
    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"ScaleAndOffset::Initialise: %s: ObjectLoadSetup failed %s", Name(), input->InterfaceName());
        return False;
    }
    
    cdb.ReadFloat(scale , "Scale" , 1.0);
    cdb.ReadFloat(offset, "Offset", 0.0);
    
    return True;
}

float ScaleAndOffset::Evaluate() {

    float *inputData = (float *)input->Buffer();
    input->Read();

    return (offset + scale*inputData[0]);
}
OBJECTLOADREGISTER(ScaleAndOffset, "$Id$")
