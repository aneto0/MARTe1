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
 * @brief Sine waveform
 */
#ifndef SINEWAVEFORM_H_
#define SINEWAVEFORM_H_

#include "System.h"
#include "WaveformInterface.h"
#include "HttpInterface.h"
#include "GCReferenceContainer.h"
#include "GCRTemplate.h"

#define PI 3.1415926535897931

OBJECT_DLL(SineWaveform)
class SineWaveform: public WaveformInterface, public GCReferenceContainer, public HttpInterface {
OBJECT_DLL_STUFF(SineWaveform)
// Parameters
private:
    float*                                     sineWaveform;

    float                                      samplingTime;

    float                                      frequency;

    float                                      previousFrequency;

    float                                      phase;

    float                                      initialPhase;

    float                                      amplitude;

    float                                      bias;

    GCRTemplate<WaveformInterface>             amplitudeWaveform;

    GCRTemplate<WaveformInterface>             frequencyWaveform;

    GCRTemplate<WaveformInterface>             biasWaveform;

private:

    float Phase(int32 usecTime);

public:
    SineWaveform() {
        sineWaveform               = NULL;

	samplingTime               = 0.0;
        frequency                  = 0.0;
	previousFrequency          = 0.0;
	phase                      = 0.0;
        initialPhase               = 0.0;
        amplitude                  = 0.0;
        bias                       = 0.0;
    }

    ~SineWaveform() {
        if (sineWaveform != NULL) free((void*&)sineWaveform);
    }

    virtual void Reset() {
        phase = initialPhase;
	previousFrequency = 0.0;
	if (amplitudeWaveform.IsValid()) amplitudeWaveform->Reset();
	if (biasWaveform.IsValid())      biasWaveform->Reset();
	if (frequencyWaveform.IsValid()) frequencyWaveform->Reset();
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

    virtual float GetValue(int32 usecTime);
};

#endif /* SINEWAVEFORM_H_ */
