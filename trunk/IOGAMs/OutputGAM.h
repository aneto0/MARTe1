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



#if !defined _OUTPUT_GAM_MODULE
#define _OUTPUT_GAM_MODULE

#include "System.h"
#include "GenericAcqModule.h"
#include "GAM.h"
#include "GCRTemplate.h"

OBJECT_DLL(OutputGAM)

class OutputGAM: public GAM{

OBJECT_DLL_STUFF(OutputGAM)

protected:
    /** Reference to one of the Acquisition Modules */
    GCRTemplate<GenericAcqModule>               outputModule;

    /** Interface with the DDB. Points to the Time Base
        signal. */
    DDBInputInterface                           *usecTime;

    /** Interface with the DDB. Points to the area
        where the inputs that should be written are */
    DDBInputInterface                           *input;

private:
    /** Size of the packet to be written on the board in words */
    int32                                       dataWordSize;

    /** Calibration Factor to convert the data to the board format */
    float                                       *cal0;

    /** Calibration Factor to convert the data to the board format */
    float                                       *cal1;

    /** Maximum output value */
    float                                       *maxOutputValue;

    /** Minimum output value */
    float                                       *minOutputValue;

    /** Flag used to specify it the signal needs calibration. */
    bool                                        *needsCalibration;

    /** If true it will write to the GACQM in the GAMPrepulse. The default is not to write*/
    bool                                         writeInPrepulse;
public:

    /** Constructor */
    OutputGAM(){
        // Interfaces
        usecTime                           = NULL;
        input                              = NULL;
        // Packet Size
        dataWordSize                       = 0;
        // Calibration factors
        cal0                               = NULL;
        cal1                               = NULL;
        // Max and Min
        maxOutputValue                     = NULL;
        minOutputValue                     = NULL;
        needsCalibration                   = NULL;
        writeInPrepulse                    = False;
    };

    /** Destructor */
    virtual ~OutputGAM();

    /** Initialise the module */
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /** Implements the Saving of the parameters to Configuration Data Base */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err);

    /** Implements the Dump of the outputs, with their values, names and calibration factors */
    bool OutputDump(StreamInterface &outputStream);

    /** Returns True if the module has delayed acquisition or the associated driver is synchronising*/
    virtual bool IsSynchronizing(){
	return outputModule->IsSynchronizing();
    }

};



#endif
