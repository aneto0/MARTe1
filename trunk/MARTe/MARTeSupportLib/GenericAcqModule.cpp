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

#include "GenericAcqModule.h"
#include "CDBExtended.h"


bool GQMObjectLoadSetup(GenericAcqModule &gqm,ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    cdb.ReadInt32(gqm.numberOfInputChannels,"NumberOfInputs",-1);
    cdb.ReadInt32(gqm.numberOfOutputChannels,"NumberOfOutputs",-1);

    if((gqm.numberOfInputChannels == -1) && (gqm.numberOfOutputChannels == -1)){
        gqm.AssertErrorCondition(InitialisationError,"GenericAcqModule::ObjectLoadSetup: %s neither NumberOfInputs nor NumberOfOutputs have been specified",gqm.Name());
        return False;
    }

    return True;
}


bool GQMObjectSaveSetup(GenericAcqModule &gqm,ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    cdb.WriteString(gqm.Name(),"BoardName");
    if(gqm.IsTimeModule())cdb.WriteString("True","IsTimeModule");
    if(gqm.numberOfInputChannels  != -1)cdb.WriteInt32(gqm.numberOfInputChannels,"NumberOfInputs");
    if(gqm.numberOfOutputChannels != -1)cdb.WriteInt32(gqm.numberOfOutputChannels,"NumberOfOutputs");


    return True;
}

OBJECTREGISTER(GenericAcqModule,"$Id$")

