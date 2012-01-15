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

#if !defined (DIGITAL_FILTER_GAM_H)
#define DIGITAL_FILTER_GAM_H

#include "GAM.h"
#include "FilterContainer.h"

OBJECT_DLL(DigitalFilterGAM)
class DigitalFilterGAM : public GAM {
OBJECT_DLL_STUFF(DigitalFilterGAM)

private:
    /**
     * Array of FilterContainers to execute in real-time
     */
    GCRTemplate<FilterContainer> *executableList;

public:

    DigitalFilterGAM(){
        executableList = NULL;
    }

    virtual ~DigitalFilterGAM(){
        if(executableList != NULL){
            delete []executableList;
        }  
    }

    /// GAM configuration
    bool  Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool  Execute(GAM_FunctionNumbers execFlag);
};
#endif
