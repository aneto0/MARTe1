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
 * $Id: BinaryFileTimeInputGAM.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

#if !defined _BINARY_FILE_TIME_GAM_MODULE
#define _BINARY_FILE_TIME_GAM_MODULE

#include "System.h"
#include "GenericAcqModule.h"
#include "TimeTriggeringServiceInterface.h"
#include "GAM.h"
#include "BinaryFileInputGAM.h"
#include "GCRTemplate.h"
#include "DDBOutputInterface.h"

OBJECT_DLL(BinaryFileTimeInputGAM)

class BinaryFileTimeInputGAM: public BinaryFileInputGAM{
    
    OBJECT_DLL_STUFF(BinaryFileTimeInputGAM)
    
private:

    /** Reference to the triggering service facility */
    GCRTemplate<TimeTriggeringServiceInterface>  trigger;

private:

    /** Initialise the usecTime ddb interface. It is only used if the module
     *  is a standard InputGAM.
     *  */
    virtual bool InitialiseTimeInformation(ConfigurationDataBase& cdbData){return True;}
    
    /** Read Timing informations. It is only used if the module is a standard
     *  InputGAM. 
     * */
    virtual int32 ReadTime(){return trigger->GetPeriodUsecTime() ;}
    
public:

    /** Constructor */
    BinaryFileTimeInputGAM(){ };

    /** Destructor */
    virtual ~BinaryFileTimeInputGAM();

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
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err);

    /** Implements the Dump of the outputs, with their values, names and calibration factors */
    bool InputDump(StreamInterface &outputStream);

    /** Returns True if the module has delayed acquisition or the associated driver is synchronising*/
    virtual bool IsSynchronizing(){
        return inputModule->IsSynchronizing();
    }
    
};



#endif
