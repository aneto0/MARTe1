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

#if !defined (FILTER_CONTAINER_H)
#define FILTER_CONTAINER_H

#include "GAM.h"
#include "Filter.h"

OBJECT_DLL(FilterContainer)
class FilterContainer : public GAM {
OBJECT_DLL_STUFF(FilterContainer)

private:

    /// DDB interface pointers
    DDBInputInterface        *input;
    DDBOutputInterface       *output;
    
    /// Pointer to array of filter objects
    Filter                   *f;

    /// Number of filter objects
    int32                     numberOfFilters;

public:

    /// Constructor
    FilterContainer() {
	input           = NULL;
	output          = NULL;
	numberOfFilters = -1;
    };

    /// Destructor
    ~FilterContainer() {
	if(f != NULL) {
	    delete [] f;
	}
    };
    
    /// Reset all filters
    void  Reset();

    /// GAM configuration
    bool  Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool  Execute(GAM_FunctionNumbers execFlag);
};
#endif
