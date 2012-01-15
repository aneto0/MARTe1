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
 * @brief Definition of the sawtooth waveform
 */
#ifndef SAWWAVE_H_
#define SAWWAVE_H_

#include "CDBExtended.h"
#include "Waveform.h"
#include "WaveformInterface.h"
#include "FString.h"
#include "GCNamedObject.h"
#include "HttpInterface.h"

OBJECT_DLL(SawtoothWaveform)
class SawtoothWaveform: public WaveformInterface, public GCNamedObject, public HttpInterface {
OBJECT_DLL_STUFF(SawtoothWaveform)

// Parameters
private:
    /** The waveform used to emulate the sawtooth one */
    GCRTemplate<Waveform>           waveform;
    /** The period of the sawtooth */
    int32              period;
    /** The amplitude of the sawtooth */
    float              amplitude;
    /** The rounding factor */
    int32              rounding;

// States
private:
    /** Last reset time */
    int32              lastReset;

public:
    /** Constructor */
    SawtoothWaveform() : waveform(GCFT_Create) {
        period          = 0;
        amplitude       = 0.0;
        rounding        = 0;
        lastReset       = 0;
    }

    /** Resets the GCNSawtoothWaveform */
    virtual void Reset() {
        waveform->Reset();
        lastReset       = 0;
    }

    /**
    * Loads object's parameters from a CDB
    * @param cdbData the CDB to load configuration from
    * @param err the StreamInterface to write errors to (not used)
    * @return True if initialised correctly, False otherwise
    */
    bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err);

    /**
     * Get the current value of the waveform
     * @param usecTime current time in microseconds
     * @return the value of the waveform
     */
    inline float GetValue(int32 usecTime) {
        int32 relativeTime = usecTime - lastReset;

        if ( relativeTime >= period) {
            lastReset = usecTime;
            relativeTime = 0;
            waveform->Reset();
        }
        return waveform->GetValue(relativeTime);
    }

    /**
    * Builds a webpage for the current object
    * @param hStream the HttpStream to write to
    * @return True
    */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif /* SAWWAVE_H_ */
