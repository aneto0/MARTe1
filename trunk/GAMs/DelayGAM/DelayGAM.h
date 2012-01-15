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

#if !defined (DELAY_GAM_H)
#define DELAY_GAM_H

#include "GAM.h"
#include "DelayContainer.h"

OBJECT_DLL(DelayGAM)
class DelayGAM : public GAM {
OBJECT_DLL_STUFF(DelayGAM)

private:
    /**
     * Array of DelayContainers to execute in real-time
     */
    GCRTemplate<DelayContainer> *executableList;

public:

    DelayGAM(){
        executableList = NULL;
    }

    virtual ~DelayGAM(){
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

