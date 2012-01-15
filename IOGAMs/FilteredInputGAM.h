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
#ifndef FILTEREDINPUTGAM_H_
#define FILTEREDINPUTGAM_H_

#include "System.h"
#include "InputGAM.h"
#include "GAM.h"
#include "GenericAcqModule.h"

OBJECT_DLL(FilteredInputGAM)

class FilteredInputGAM: public InputGAM{

    OBJECT_DLL_STUFF(FilteredInputGAM)

private:

    /**Buffer */
    float     *buffer;
    
    /** Filter Coefficients */
    float     *filterCoefficients;
    
    /** Size of the filter */
    int32     downSampling;

public:

    /** Constructor */
    FilteredInputGAM(){
        filterCoefficients = NULL;
        buffer             = NULL;
        downSampling       = 0;
    }
    
    /** Destructor */
    virtual ~FilteredInputGAM(){
        if(filterCoefficients != NULL) free((void *&)filterCoefficients);  
        if(buffer             != NULL) free((void *&)buffer);  
    };
    
    /** Initialise the module
        @param cdbData the data base containing the initialisation parameters
        @return True if everything was ok. False if parameters are missing or inconsistent.

        The user must specify the following parameters:
        BoardName                        is the name of the input driver to be used
        UsecTimeSignalName               is the name of the time signal in usec
        Signals                          which contains the informations for the ddb interface and the calibrations (if needed)
        
    */
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /** Implements the Saving of the parameters to Configuration Data Base */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){
        return InputGAM::ObjectSaveSetup(info,err);
    }

    /** Returns True if the module has delayed acquisition or the associated driver is synchronising*/
    virtual bool IsSynchronizing(){
        return inputModule->IsSynchronizing();
    }
    
};

#endif /*FILTEREDINPUTGAM_H_*/
