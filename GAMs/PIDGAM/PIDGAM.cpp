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

#include "PIDGAM.h"
#include "PIDGAMInputStructure.h"
#include "PIDGAMOutputStructure.h"
#include "CDBExtended.h"

#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"

#include "LoadCDBObjectClass.h"

#define TSTARTDEFAULT    0.0
#define TENDDEFAULT    100.0

bool PIDGAM::Initialise(ConfigurationDataBase& cdbData) {

    CDBExtended cdb(cdbData);


    ////////////////////////////////////////////////////
    //                Add interfaces to DDB           //
    ////////////////////////////////////////////////////
    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"PIDGAM::Initialise: %s failed to add input interface",Name());
        return False;
    }

    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"PIDGAM::Initialise: %s failed to add output interface",Name());
        return False;
    }


    ////////////////////////////////////////////////////
    //                Add input signals               //
    ////////////////////////////////////////////////////
    if(!cdb->Move("InputSignals")){
        AssertErrorCondition(InitialisationError,"PIDGAM::Initialise: %s did not specify InputSignals entry",Name());
        return False;
    }

    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"PIDGAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }

    cdb->MoveToFather();




    ////////////////////////////////////////////////////
    //                Add output signals              //
    ////////////////////////////////////////////////////
    if(!cdb->Move("OutputSignals")){
        AssertErrorCondition(InitialisationError,"PIDGAM::Initialise: %s did not specify OutputSignals entry",Name());
        return False;
    }

    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"PIDGAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        return False;
    }

    cdb->MoveToFather();


    ////////////////////////////////////////////////////
    //               Controller enabled               //
    ////////////////////////////////////////////////////
    {
        FString tmp;
        controllerEnabled = True;
        if(cdb.ReadFString(tmp, "ControllerOn")) {
	    if (tmp == "OFF") {
	        controllerEnabled = False;
                AssertErrorCondition(Warning,"PIDGAM::Initialise: %s PID is NOT enabled",Name());
	    }
        }
    }



    ////////////////////////////////////////////////////
    //            Time interval for control           //
    ////////////////////////////////////////////////////
    {
        CheckAndFreeDynamicMemory();

        int tStartLength = 1;
        int tEndLength   = 1;
	FString error;
        if(!LoadVectorObject(cdb, "TStart", (void*&)tStart, tStartLength, CDBTYPE_float, error)) {
            AssertErrorCondition(Information,"PIDGAM %s::Initialise: TStart entry not found, assuming %f secs", Name(), TSTARTDEFAULT);
	    tStart[0] = TSTARTDEFAULT;
	    tStartLength = 1;
        }
        if(!LoadVectorObject(cdb, "TEnd", (void*&)tEnd, tEndLength, CDBTYPE_float, error)) {
            AssertErrorCondition(Information,"PIDGAM %s::Initialise: TEnd entry not found, assuming %f secs", Name(), TENDDEFAULT);
	    tEnd[0] = TENDDEFAULT;
	    tEndLength = 1;
        }

	// Check if tStart and tEnd array lengths are the same, if not issue an error
	if(tStartLength != tEndLength) {
            AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: Time windows mismatch -> (tStartLength = %d) != (tEndLength = %d)", Name(), tStartLength, tEndLength);
	    return False;
	} else {
	    numberOfTimeWindows = tStartLength;
	}

	// Check if time windows are consistent in terms of tStart and tEnd
	for( int i = 0 ; i < numberOfTimeWindows ; i++ ){
	    if( tStart[i] >= tEnd[i] ){
	        AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: For time window %d the start time (%f) is larger than the end time (%f)", Name(), i, tStart[i], tEnd[i]);
                return False;
	    }
            if( i < numberOfTimeWindows - 1 ){
	        if( tStart[i+1] < tEnd[i] ){
	            AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: tStart for time window %d is smaller than tEnd for time window %d", Name(), i+1, i);
                    return False;
	        }
            }
	}
	AssertErrorCondition(Information, "PIDGAM %s::Initialise: %d time windows correctly setup", Name(), numberOfTimeWindows);

        // Allocate memory for tStartUsec and tEndUsec
        if((tStartUsec = (uint32 *)malloc(tStartLength*sizeof(uint32))) == NULL) {
            AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: unable to allocate memory for tStartUsec", Name());
            return False;
        }
        if((tEndUsec = (uint32 *)malloc(tStartLength*sizeof(uint32))) == NULL) {
            AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: unable to allocate memory for tEndUsec", Name());
            return False;
        }
        
        // Initialise window's microsecond start and end time
        for(int32 i = 0 ; i < tStartLength ; i++) {
            tStartUsec[i] = (uint32)(1e6*tStart[i]);
            tEndUsec[i]   = (uint32)(1e6*tEnd[i]);
        }
    }
    
    ////////////////////////////////////////////////////
    //             Saturation and antiwindup          //
    ////////////////////////////////////////////////////
    {
        // Antiwindup gain
        if(cdb.ReadFloat(antiwindupGain, "AntiwindupGain")) {
            antiwindupIsEnabled = True;
        } else {
            antiwindupIsEnabled = False;
        }

        // Saturation levels
        if(!cdb.ReadFloat(upperControlSaturation,"UpperControlSaturation") && antiwindupIsEnabled){
            AssertErrorCondition(InitialisationError,"PIDGAM %s::Initialise: UpperControlSaturation entry demanded by antiwindup feature not found", Name());
            return False;
        }
        if(!cdb.ReadFloat(lowerControlSaturation,"LowerControlSaturation") && antiwindupIsEnabled){
            AssertErrorCondition(InitialisationError,"PIDGAM %s::Initialise: LowerControlSaturation entry demanded by antiwindup feature not found", Name());
            return False;
        }
    }

    ////////////////////////////////////////////////////
    //                  Fast discharge                //
    ////////////////////////////////////////////////////
    {
        fastDischargeEnabled = False;

        if (cdb->Move("FastDischarge")) {
            if(!cdb.ReadFloat(fastDischargeAbsoluteErrorMaximumContribution, "FastDischargeAbsoluteErrorMaximumContribution")){
                AssertErrorCondition(InitialisationError,"PIDGAM %s::Initialise: FastDischarge enabled but no FastDischargeAbsoluteErrorMaximumContribution specified", Name());
                return False;
            }
            if(!cdb.ReadFloat(fastDischargeAbsoluteErrorScalingFactor, "FastDischargeAbsoluteErrorScalingFactor")){
                AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: FastDischarge enabled but no FastDischargeAbsoluteErrorScalingFactor specified", Name());
                return False;
            }
            fastDischargeEnabled = True;
	    /** Make both quantities positive no matter what */
	    fastDischargeAbsoluteErrorMaximumContribution = fabs(fastDischargeAbsoluteErrorMaximumContribution);
	    fastDischargeAbsoluteErrorScalingFactor = fabs(fastDischargeAbsoluteErrorScalingFactor);
            cdb->MoveToFather();
        } else {
            AssertErrorCondition(Information,"PIDGAM %s::Initialise: FastDischarge block not specified, turning off FastDischarge action", Name());
            fastDischargeEnabled = False;
        }
    }

    ////////////////////////////////////////////////////
    //             PID gains and parameters           //
    ////////////////////////////////////////////////////
    {
        if(!cdb.ReadFloat(Kp, "Kp")){
            AssertErrorCondition(InitialisationError, "PIDGAM %s::Initialise: Kp entry not found", Name());
            return False;
        }
        cdb.ReadFloat(Ki, "Ki", 0.0);
        cdb.ReadFloat(Kd, "Kd", 0.0);
    }
    
    // Load Ts
    if(!cdb.ReadFloat(Ts, "SamplingTime")) {
        AssertErrorCondition(InitialisationError,"PIDGAM %s::Initialise: SamplingTime entry not found", Name());
        return False;
    }
    
    // Load the output gain if specified
    cdb.ReadFloat(outputGain, "OutputGain", 1.0);
    
    // Load slew-rate limit if specified
    cdb.ReadFloat(absSlewRateLimitInAuPerSec, "AbsSlewRateLimitInAuPerSec", 0.0);
    absSlewRateLimitInAuPerSec = fabs(absSlewRateLimitInAuPerSec);

    return True;
}

bool PIDGAM::Execute(GAM_FunctionNumbers functionNumber) {

    // Get input and output data pointers
    PIDGAMInputStructure  *inputData  = (PIDGAMInputStructure  *)input->Buffer();
    PIDGAMOutputStructure *outputData = (PIDGAMOutputStructure *)output->Buffer();

    // Set all the outputs to zero
    outputData->controlSignal       = 0.0;
    outputData->error               = 0.0;
    outputData->feedback            = 0.0;
    outputData->integratorState     = 0.0;
    outputData->fastDischargeAction = 0.0;
    outputData->antiwindupAction    = 0.0;

    switch(functionNumber) {
        case GAMPrepulse: {
            // Reset everything
            Reset();
        }
        default: {
            if(controllerEnabled) {
                input->Read();
                
                if(inputData->usecTime >= tStartUsec[currentTimeWindow] && inputData->usecTime <= tEndUsec[currentTimeWindow]) {
                    //float error = inputData->errorSignal;
                    float error            = inputData->reference - inputData->measurement;
                    float controlValue     = 0.0;
                    float dischargeAction  = 0.0;
                    float antiwindupAction = 0.0;
                    float feedback         = 0.0;
                    
                    // Calculate discharge action if enabled
                    if (fastDischargeEnabled) {
		        if((integrator*error) < 0) {
			    float absoluteErrorContribution = error*fastDischargeAbsoluteErrorScalingFactor;
			    if (fabs(absoluteErrorContribution) > fastDischargeAbsoluteErrorMaximumContribution) {
			        absoluteErrorContribution = absoluteErrorContribution*fastDischargeAbsoluteErrorMaximumContribution/fabs(absoluteErrorContribution);
			    }
			    dischargeAction = absoluteErrorContribution;
			} else {
			    dischargeAction = 0.0;
			}
                    }
                    
                    // Calculate the PID
                    controlValue  = Kp * error + integrator + Kd * (error - previousStepError)/Ts;
                    
                    // Calculate antiwindup signal
                    if(antiwindupIsEnabled) {
                        float saturatedControlValue = controlValue;
                        
                        if(saturatedControlValue > upperControlSaturation) saturatedControlValue = upperControlSaturation;
                        if(saturatedControlValue < lowerControlSaturation) saturatedControlValue = lowerControlSaturation;
                        
                        // Update the integrator
                        antiwindupAction = antiwindupGain * (controlValue - saturatedControlValue);
                        
                        controlValue = saturatedControlValue;
                    } else {
		        antiwindupAction = 0.0;
		    }
                    
                    // Update the integrator result for the next cycle
                    integrator += Ts * (Ki * error + antiwindupAction + dischargeAction);
                    
                    // Update error
                    previousStepError = error;
                    
                    // Update the controlValue
                    controlValue *= outputGain;
                    feedback = controlValue;
                    
                    // Add the feedforward action
                    controlValue += inputData->feedforward;
                    
		    // Perform slew-rate limit correction
		    if(absSlewRateLimitInAuPerSec != 0.0) {
                        if(IsPreviousControlValueAvailable) {
                            if((controlValue-previousControlValue) > (absSlewRateLimitInAuPerSec*Ts)) {
                                controlValue = previousControlValue + absSlewRateLimitInAuPerSec*Ts;
                            } else if((controlValue-previousControlValue) < (-absSlewRateLimitInAuPerSec*Ts)) {
                                controlValue = previousControlValue - absSlewRateLimitInAuPerSec*Ts;
                            }
                        }
		    }
                    
                    // Apply saturations
                    if(controlValue > upperControlSaturation) {
                        controlValue = upperControlSaturation;
                    } else if(controlValue < lowerControlSaturation) {
                        controlValue = lowerControlSaturation;
                    }
                    
		    // Update previousControlValue
		    previousControlValue = controlValue;
		    IsPreviousControlValueAvailable = True;
                    
                    // Write signals to DDB
                    outputData->controlSignal       = controlValue;
                    outputData->feedback            = feedback;
                    outputData->error               = error;
                    outputData->fastDischargeAction = dischargeAction;
                    outputData->antiwindupAction    = antiwindupAction;
                    outputData->integratorState     = integrator;
                } else if(inputData->usecTime > tEndUsec[currentTimeWindow]) {
                    if(currentTimeWindow < numberOfTimeWindows-1) {
		        IsPreviousControlValueAvailable = False;
			previousControlValue = 0.0;
                        currentTimeWindow++;
                    }
                } else {
                    IsPreviousControlValueAvailable = False;
		    previousControlValue = 0.0;
                }
            } // end if controllerEnabled
        }
    }
    
    // Update the data output buffer
    output->Write();

    return True;
}





bool PIDGAM::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>PID %s</title></head><body>\n", Name());

    hStream.Printf("<h1>PID %s</h1>", Name());
    if (!controllerEnabled) hStream.Printf("<br><p style=\"color: 'red'\">CONTROLLER DISABLED</p><br>\n");

    hStream.Printf("<p>Kp: %f</p>\n", Kp);
    hStream.Printf("<p>Ki: %f</p>\n", Ki);
    hStream.Printf("<p>Kd: %f</p>\n", Kd);

    hStream.Printf("<p>Sampling time: %f</p>\n", Ts);

    hStream.Printf("<p>PID output gain: %f</p>\n", outputGain);

    if (antiwindupIsEnabled) {
        hStream.Printf("<h2>Antiwindup parameters</h2>\n");
        hStream.Printf("<p>Upper saturation: %f</p>\n", upperControlSaturation);
        hStream.Printf("<p>Lower saturation: %f</p>\n", lowerControlSaturation);
        hStream.Printf("<p>Antiwindup gain: %f</p>\n", antiwindupGain);
    }

    if(fastDischargeEnabled) {
        hStream.Printf("<h2>Fast discharge parameters</h2>\n");
        hStream.Printf("<p>Fast discharge gain: %f</p>\n", fastDischargeAbsoluteErrorScalingFactor);
        hStream.Printf("<p>Maximum contribution of absolute error: %f</p>\n", fastDischargeAbsoluteErrorMaximumContribution);
    }

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}
OBJECTLOADREGISTER(PIDGAM,"$Id: PIDGAM.cpp,v 1.18 2011/12/02 13:57:47 dalves Exp $")
