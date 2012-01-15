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

#ifndef PID_H_
#define PID_H_

#include "GAM.h"
#include "HttpInterface.h"


/**
 * A GAM  implementing a PID controller with antiwindup
 * and saturation with slew rate
 */
OBJECT_DLL(PIDGAM)
class PIDGAM : public GAM, public HttpInterface {
OBJECT_DLL_STUFF(PIDGAM)

// DDB Interfaces
private:
    /** Input interface to read data from */
    DDBInputInterface                      *input;
    /** Output interface to write data to */
    DDBOutputInterface                     *output;

// Parameters
private:
    /** True if the controller is enabled */
    bool                                   controllerEnabled;
    /** Control Start Time */
    float                                  *tStart;
    /** Control Start Time in Microseconds */
    uint32                                 *tStartUsec;
    /** Control End Time */
    float                                  *tEnd;
    /** Control End Time in Microsconds */
    uint32                                 *tEndUsec;
    /** Number of time windows */
    int                                     numberOfTimeWindows;
    /** Current time window */
    int                                     currentTimeWindow;
    /** The proportional gain */
    float                                   Kp;
    /** The integral gain */
    float                                   Ki;
    /** The derivative gain */
    float                                   Kd;
    /** The sampling time */
    float                                   Ts;
    /** True if antiwindup is enabled */
    bool                                    antiwindupIsEnabled;
    /** Antiwindup gain */
    float                                   antiwindupGain;
    /** Upper control saturation */
    float                                   upperControlSaturation;
    /** Lower control saturation */
    float                                   lowerControlSaturation;
    /** Fast discharge enabled */
    bool                                    fastDischargeEnabled;
    /** Strength of the fast discharge action */
    float                                   fastDischargeAbsoluteErrorScalingFactor;
    /** Maximum fast discharge absolute error contribution */
    float                                   fastDischargeAbsoluteErrorMaximumContribution;

    /** Output gain value */
    float                                   outputGain;

    /** Slew-Rate limit on control quantity */
    float                                   absSlewRateLimitInAuPerSec;

    /** Check that, for the correspondent time window, the previous control
     sample is available to apply slew-rate limit */
    bool                                    IsPreviousControlValueAvailable;

    /** Control value in the previous cycle */
    float                                   previousControlValue;

// States
private:
    /** The error signal at the previous step */
    float                                   previousStepError;

    /** The integrator */
    float                                   integrator;

public:
    /** Constructor */
    PIDGAM() {
        input                                         = NULL;
        output                                        = NULL;

        tStart                                        = NULL;
        tEnd                                          = NULL;
        tStartUsec                                    = NULL;
        tEndUsec                                      = NULL;

        currentTimeWindow                             =  0;
        numberOfTimeWindows                           = -1;
	IsPreviousControlValueAvailable               = False;
	previousControlValue                          = 0.0;

        Kp                                            = 0.0;
        Ki                                            = 0.0;
        Kd                                            = 0.0;

        Ts                                            = 0.0;

        antiwindupIsEnabled                           = False;
        antiwindupGain                                = 0.0;
        upperControlSaturation                        = 0.0;
        lowerControlSaturation                        = 0.0;

        fastDischargeEnabled                          = False;
        fastDischargeAbsoluteErrorScalingFactor       = 0.0;
        fastDischargeAbsoluteErrorMaximumContribution = 0.0;

        outputGain                                    = 0.0;

        controllerEnabled                             = True;
	
	absSlewRateLimitInAuPerSec                    = 0.0;

        Reset();
    }

    /** Destructor */
    ~PIDGAM(){
        CheckAndFreeDynamicMemory();
    }

    /** Free dynamic memory for tStart and tEnd arrays of each time window */
    inline void CheckAndFreeDynamicMemory(){
        if( tStart != NULL ){
	    free((void*&)tStart);
	    tStart = NULL;
	}
        if( tEnd != NULL ){
	    free((void*&)tEnd);
	    tEnd = NULL;
	}
        if( tStartUsec != NULL ){
	    free((void*&)tStartUsec);
	    tStartUsec = NULL;
	}
        if( tEndUsec != NULL ){
	    free((void*&)tEndUsec);
	    tEndUsec = NULL;
	}
    }

    /**
     * Resets the PIDGAM's states
     */
    inline void Reset() {
        previousStepError               = 0.0;
        integrator                      = 0.0;
	currentTimeWindow               =   0;
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


#endif /* PID_H_ */
