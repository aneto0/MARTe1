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


#ifndef _INPUT_MODULES_SERVICE
#define _INPUT_MODULES_SERVICE

#include "TimeServiceActivity.h"
#include "GCRTemplate.h"
#include "GenericAcqModule.h"
//#include "TimeTriggeringServiceInterface.h"

OBJECT_DLL(InputModulesService)

class InputModulesService: public TimeServiceActivity{

     OBJECT_DLL_STUFF(InputModulesService)

private:

    /** Array of input modules that need to trigger the synchorouns start of acquisition */
    GCRTemplate<GenericAcqModule>    *inputsModules;

    /** Number Of inputs modules */
    int32                             numberOfInputModules;

public:

    /** */
    InputModulesService(){
        inputsModules        = NULL;
        numberOfInputModules = 0;
    };

    /** */
    virtual ~InputModulesService(){
        if(inputsModules != NULL) delete[] inputsModules;
        numberOfInputModules = 0;
    }

    /** */
    virtual bool Trigger(int64 usecTime);

    /** */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

};

#endif
