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

#if !defined (PLOTTING_GAM_H)
#define PLOTTING_GAM_H

#include "GAM.h"
#include "GCRTemplate.h"
#include "PlotWindow.h"

OBJECT_DLL(PlottingGAM)
class PlottingGAM : public GAM, public HttpInterface {
OBJECT_DLL_STUFF(PlottingGAM)

private:

    GCRTemplate<PlotWindow> *gcrtpw;

    uint32                   numberOfPlotWindows;

    int32                    httpRefreshMsecTime;

public:

    /// Constructor
    PlottingGAM() {
	gcrtpw = NULL;
	numberOfPlotWindows = 0;
	httpRefreshMsecTime = 500;
    };

    /// Destructor
    ~PlottingGAM() {
	if(gcrtpw != NULL) {
	    delete [] gcrtpw;
	    gcrtpw = NULL;
	}
    };
    
    /// GAM configuration
    bool  Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool  Execute(GAM_FunctionNumbers execFlag);

    virtual bool ProcessHttpMessage(HttpStream &hStream);
};
#endif
