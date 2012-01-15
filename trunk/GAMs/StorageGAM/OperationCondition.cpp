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

#include "OperationCondition.h"

bool OperationCondition::ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err) {

    CDBExtended cdb(info);
    
    if((cdb->Exists("LargerThan")) && (cdb->Exists("LargerOrEqualThan"))) {
        AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: LargerThan and LargerOrEqualThan cannot coexist", Name());
        return False;
    } else if(cdb->Exists("LargerThan")) {
        if((excludingLowerThresh = (float *)malloc(sizeof(float))) == NULL) {
            AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: unable to allocate memory for excludingLowerThresh", Name());
            return False;
        }
        cdb.ReadFloat(*excludingLowerThresh, "LargerThan");
    } else if(cdb->Exists("LargerOrEqualThan")) {
        if((includingLowerThresh = (float *)malloc(sizeof(float))) == NULL) {
            AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: unable to allocate memory for includingLowerThresh", Name());
            return False;
        }
        cdb.ReadFloat(*includingLowerThresh, "LargerOrEqualThan");
    }

    if((cdb->Exists("SmallerThan")) && (cdb->Exists("SmallerOrEqualThan"))) {
        AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: SmallerThan and SmallerOrEqualThan cannot coexist", Name());
        return False;
    } else if(cdb->Exists("SmallerThan")) {
        if((excludingUpperThresh = (float *)malloc(sizeof(float))) == NULL) {
            AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: unable to allocate memory for excludingUpperThresh", Name());
            return False;
        }
        cdb.ReadFloat(*excludingUpperThresh, "SmallerThan");
    } else if(cdb->Exists("SmallerOrEqualThan")) {
        if((includingUpperThresh = (float *)malloc(sizeof(float))) == NULL) {
            AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: unable to allocate memory for includingUpperThresh", Name());
            return False;
        }
        cdb.ReadFloat(*includingUpperThresh, "SmallerOrEqualThan");
    }


    if(excludingUpperThresh && excludingLowerThresh) {
        if(*excludingUpperThresh < *excludingLowerThresh) {
            AssertErrorCondition(Warning, "OperationCondition::ObjectLoadSetup: %s: *excludingUpperThresh < *excludingLowerThresh", Name());
        }
    } else if(excludingUpperThresh && includingLowerThresh) {
        if(*excludingUpperThresh < *includingLowerThresh) {
            AssertErrorCondition(Warning, "OperationCondition::ObjectLoadSetup: %s: *excludingUpperThresh < *includingLowerThresh", Name());
        }
    } else if(includingUpperThresh && includingLowerThresh) {
        if(*includingUpperThresh < *includingLowerThresh) {
            AssertErrorCondition(Warning, "OperationCondition::ObjectLoadSetup: %s: *includingUpperThresh < *includingLowerThresh", Name());
        }
    } else if(includingUpperThresh && excludingLowerThresh) {
        if(*includingUpperThresh < *excludingLowerThresh) {
            AssertErrorCondition(Warning, "OperationCondition::ObjectLoadSetup: %s: *includingUpperThresh < *excludingLowerThresh", Name());
        }
    }


    if(cdb->Exists("EqualTo")) {
        if(excludingLowerThresh || excludingUpperThresh || includingLowerThresh || includingUpperThresh) {
            AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: LargerThan and/or LargerOrEqualThan and/or SmallerThan and/or SmallerOrEqualThan are incompatible with EqualTo", Name());
            return False;
        } else {
            if((equalTo = (float *)malloc(sizeof(float))) == NULL) {
                AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: unable to allocate memory for equalTo", Name());
                return False;
            }
            cdb.ReadFloat(*equalTo, "LowerThan");
        }
    }

    if(!GCReferenceContainer::ObjectLoadSetup(info, err)) {
        AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: GCReferenceContainer::ObjectLoadSetup failed", Name());
        return False;
    }
    
    if(Size() != 1) {
        AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: only one child object allowed", Name());
        return False;
    }
    operation = Find(0);
    if(!operation.IsValid()) {
        AssertErrorCondition(InitialisationError, "OperationCondition::ObjectLoadSetup: %s: child object is not of the OperationInterface type", Name());
        return False;
    }
    
    return True;
}

void OperationCondition::Reset() {
    operation->Reset();
}
    
bool OperationCondition::IsTrue() {

    float operationResult = operation->Evaluate();

    conditionResult = True;
    if(equalTo) {
        conditionResult &= ((*equalTo) == operationResult);
    } else {
        if(excludingLowerThresh) {
            conditionResult &= (operationResult >  (*excludingLowerThresh));
        }
        if(excludingUpperThresh) {
            conditionResult &= (operationResult <  (*excludingUpperThresh));
        }
        if(includingLowerThresh) {
            conditionResult &= (operationResult >= (*includingLowerThresh));
        }
        if(includingUpperThresh) {
            conditionResult &= (operationResult <= (*includingUpperThresh));
        }
    }
    
    return conditionResult;
}
OBJECTLOADREGISTER(OperationCondition, "$Id$")
