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

#ifndef __SINE_OPERATION__
#define __SINE_OPERATION__

#include "UnaryOperation.h"

class SinOperation : public UnaryOperation {

public:

    SinOperation() {
	defaultType = BTDFloat;
	result.SetType(defaultType);
    }
    
    virtual BasicTypeData &ExecuteLocal() {

	float dv, v;

	if(!data.GetData(defaultType, &dv)) {
	    printf("Sine: ExecuteLocal(): unable to get data value\n");
	}

	v = sin(dv);
	if(!result.UpdateData(&v)) {
	    printf("Sine: ExecuteLocal(): unable to update BasicTypeData value\n");
	}
	
	return (result);
    }
    
};
#endif
