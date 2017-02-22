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
 * $Id: BinaryFileInputGAM.h 3 2012-01-15 16:26:07Z aneto $
 *
**/
#if !defined _BINARY_FILE_INPUT_GAM_MODULE
#define _BINARY_FILE_INPUT_GAM_MODULE

#include "System.h"
#include "GenericAcqModule.h"
#include "GAM.h"
#include "GCRTemplate.h"
#include "DDBIOInterface.h"
#include "HttpInterface.h"
#include "HttpStream.h"

OBJECT_DLL(BinaryFileInputGAM)

class BinaryFileInputGAM: public GAM, public HttpInterface, public MessageHandler{

OBJECT_DLL_STUFF(BinaryFileInputGAM)

protected:
    
    /** Reference to one of the Acquisition Modules */
    GCRTemplate<GenericAcqModule>               inputModule;
 
    /** Interface with the DDB. Points to the area
         where the read inputs should be copied */
     DDBOutputInterface                          *output;
    
    /** Interface with the DDB. Points to the Time Base
        signal. */
    DDBIOInterface                              *usecTime;
 
    /** Size of the packet to be written on the board in words */
    int32                                       numberOfInputs;

    /** Calibration Factor to convert the data to the board format */
    float                                       *cal0;

    /** Calibration Factor to convert the data to the board format */
    float                                       *oldCal0;

    /** Calibration Factor to convert the data to the board format */
    float                                       *cal1;

    /** Flag used to specify it the signal needs calibration. */
    bool                                        *needsCalibration;

    /** Flag used to specify it the signal needs calibration. */
    int32                                       *noOffsetAdjustment;

    /**
     * Automatic Offset Compensation 
     *                                */

    /** Enable automatic offset compensations */
    bool                                         automaticOffsetCompensation;
    
    /** Maximum allowed percentage correction */
    float                                        percentageCorrection;
    
    /** Automatic offset compensation.
     * It is equal to the average of the input signal during offline
     * operations and computed when during GAMStartUp phase */
    float                                       *offsetCompensation;
    
    /** Default offset value */
    float                                       *physicalOffsetValue;

    /** The maximum value of an signed 32 bits variable */
    static const int32                           MAX32BitsValue;
    
    /** Flag to mark the transition between GAMStartUp and GAMOffline. 
     *  At this point the code performs the maximum correction check.
     * If the check fails, the gam return False, causing the RealTimeThread
     * to perform safety operations.
     * */
    bool                                         performOffsetCheck;
    
    /** Number of cycles */
    int32                                        startUpCycleNumber;
    
    /** The number of cycles performed for each calibration*/
    int32                                        offsetCompensationNumberOfCycles;

    /** Flag set to True when a data calibration is requested*/
    bool                                         calibrationRequested;

    /** Current execution state*/
    GAM_FunctionNumbers                          currentExecutionState;

    bool ReadConfigurationFromCDB(CDBExtended &cdb);

protected:

    /** Initialise the usecTime ddb interface. It is only used if the module
     *  is a standard BinaryFileInputGAM.
     *  */
    virtual bool InitialiseTimeInformation(ConfigurationDataBase& cdbData);
    
    /** Read Timing informations. It is only used if the module is a standard
     *  BinaryFileInputGAM. 
     * */
    virtual int32 ReadTime();
    
public:

    /** Constructor */
    BinaryFileInputGAM(){
        output                             = NULL;
        usecTime                           = NULL;
        cal0                               = NULL;
        oldCal0                            = NULL;
        numberOfInputs                     = 0;
        cal1                               = NULL;
        needsCalibration                   = NULL;
        noOffsetAdjustment                 = NULL;
        
        automaticOffsetCompensation        = False;
        calibrationRequested               = False;
        percentageCorrection               = 0.0;
        offsetCompensation                 = NULL;
        physicalOffsetValue                = NULL;
        offsetCompensationNumberOfCycles   = 0;
        performOffsetCheck                 = False;
        startUpCycleNumber                 = 0;
        currentExecutionState              = GAMOffline;
        
    };

    /** Destructor */
    virtual ~BinaryFileInputGAM();

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

    /**
     *  Builds the webpage.
     * @param hStream The HttpStream to write to.
     * @return False on error, True otherwise.
     */
    virtual bool ProcessHttpMessage(HttpStream &hStream);

    /**
     * Accepts a message with initialisation requirement
     */
    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope>    envelope);
};



#endif
