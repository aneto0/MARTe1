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

#include "TimeServiceActivity.h"
#include "CDBExtended.h"

TimeServiceActivity::~TimeServiceActivity(){
    CStaticAssertErrorCondition(Information,"TimeServiceActivity being destroyed");
}

bool TimeServiceActivity::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    GCReferenceContainer::ObjectLoadSetup(info, err);

    // Load time service activity period
    if(!cdb.ReadInt32(aUsecPeriod,"AUsecPeriod")){
        CStaticAssertErrorCondition(InitialisationError,"TimeServiceActivity::ObjectLoadSetup: %s AUsecPeriod not declared",Name());
        return False;
    }

    // Load time service activity phase
    if(!cdb.ReadInt32(aUsecPhase,"AUsecPhase")){
        CStaticAssertErrorCondition(InitialisationError,"TimeServiceActivity::ObjectLoadSetup: %s AUsecPhase not declared",Name());
        return False;
    }

    return True;
}

bool TimeServiceActivity::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("Time Activity %s \n",Name());
    s.Printf("Activity Usec Period --> %d\n",aUsecPeriod);
    s.Printf("Activity Usec Phase  --> %d\n",aUsecPhase);
    return True;
}

OBJECTREGISTER(TimeServiceActivity,"$Id: TimeServiceActivity.cpp,v 1.3 2011/05/20 15:03:46 aneto Exp $")
