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

#include "OperationInterface.h"
#include "DDBInputInterface.h"

#if !defined(_SCALE_AND_OFFSET_)
#define _SCALE_AND_OFFSET_

OBJECT_DLL(ScaleAndOffset)
class ScaleAndOffset : public OperationInterface {
OBJECT_DLL_STUFF(ScaleAndOffset)

private:

    DDBInputInterface   *input;
    
    float                scale;
    
    float                offset;
    
public:

    ScaleAndOffset() {
        input = NULL;
    }

    bool  Initialise(ConfigurationDataBase& cdbData);

    float Evaluate();

};

#endif
