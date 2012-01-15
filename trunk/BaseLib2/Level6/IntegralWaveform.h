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

/**
 * @file
 * @brief A waveform which integrates the defined points
 */
#ifndef INTEGRALWAVEFORM_H_
#define INTEGRALWAVEFORM_H_

#include "System.h"
#include "WaveformInterface.h"
#include "HttpInterface.h"
#include "GCReferenceContainer.h"
#include "GCRTemplate.h"
#include "CDBExtended.h"

OBJECT_DLL(IntegralWaveform)
class IntegralWaveform : public WaveformInterface, public GCReferenceContainer, public HttpInterface {
OBJECT_DLL_STUFF(IntegralWaveform)

// Parameters
private:
    float                                       samplingTime;
    GCRTemplate<WaveformInterface>              waveform;

// States
private:
    float                                       integral;
    float                                       previousSample;

public:
    IntegralWaveform() {
        Reset();
    }

    virtual void Reset() {
        integral       = 0.0;
	previousSample = 0.0;
        waveform->Reset();
    }

    /**
    * Loads parameters from a CDB
    * @param cdbData the CDB
    * @return True if the initialisation went ok, False otherwise
    */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err);


    /**
    * Builds a webpage for the current object
    * @param hStream the HttpStream to write to
    * @return True
    */
    virtual bool ProcessHttpMessage(HttpStream &hStream);

    virtual float GetValue(int32 usecTime) {
        float sample = waveform->GetValue(usecTime);

	// Trapezoidal rule
        integral    += samplingTime*(sample+previousSample)*0.5;
	
        previousSample = sample;

        return integral;
    }
};


#endif /* INTEGRALWAVEFORM_H_ */
