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

#ifndef WATER_TANK_H_
#define WATER_TANK_H_

#include "GAM.h"
#include "HttpInterface.h"

/**
 * A GAM  implementing the model for a water tank. The water pump enables
 * water to flow in proportionaly to an input voltage and to the tank apperture. Water flows out
 * at a rate proportional to the output apperture and to the water height. The system receives as an input
 * the voltage and outputs an height accordingly to the following equation:
 * dh/dt = (bV - a x sqrt(h)) / A
 * where:
 * b - constant related to flow rate into the tank (cm^3/(V.s))
 * a - constant related to flow rate out of the tank (cm^2.5))
 * h - water height (cm)
 * A - tank area (cm^2)
 */
OBJECT_DLL(WaterTank)
class WaterTank : public GAM, public HttpInterface {
OBJECT_DLL_STUFF(WaterTank)

// DDB Interfaces
private:
    /** Input interface to read data from */
    DDBInputInterface                      *input;
    /** Output interface to write data to */
    DDBOutputInterface                     *output;
    
// Parameters
private:
    /** Last usec time (for the integral) */
    int32                                   lastUsecTime;
    /** Last water height (for the integral) */
    float                                   lastHeight;
    /** Last voltage value after saturation*/
    float                                   lastVoltage;
    /** The input flow rate constant*/
    float                                   bFlowRate;
    /** The output flow rate constant */
    float                                   aFlowRate;
    /** Tank area */
    float                                   tankArea;
    /** Maximum voltage that can be requested */
    float                                   maxVoltage;
    /** Minimum voltage that can be requested */
    float                                   minVoltage;
    /** The contents of the SVG file */
    FString                                 svg;
    /** Enable animation?*/
    bool                                    animationEnabled;

public:
    /** Constructor */
    WaterTank(){
        input         = NULL;
        output        = NULL;
        lastHeight    = 0;
        lastVoltage   = 0;
        lastUsecTime  = 0;
        bFlowRate     = 0;
        aFlowRate     = 0;
        tankArea      = 0;
        maxVoltage    = 0;
        minVoltage    = 0;
    }

    /** Destructor */
    ~WaterTank(){
    }

   /**
    * Loads GAM parameters from a CDB
    * @param cdbData the CDB
    * @return True if the initialisation went ok, False otherwise
    */
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /**
    * GAM main body
    * @param functionNumber The current state of MARTe
    * @return False on error, True otherwise
    */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /**
    * Saves parameters to a CDB
    * @param info the CDB to save to
    * @return True
    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){ return True; };


    /**
    * The message is actually a multi stream (HttpStream)
    * with a convention described in HttpStream
    */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};


#endif /* WATER_TANK_H */

