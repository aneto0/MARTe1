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
 * @brief A waveform defined by integer value points
 */
#ifndef __INTEGERWAVEFORM__
#define __INTEGERWAVEFORM__

#include "System.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"

/**
 * Structure holding data for a single element in an IntegerWaveform.
 */
struct IntegerWaveformData {
	int32 usecTime;
	float amplitude;
	float slope;
};

/**
 * A waveform which keeps its internal time in microseconds.
 * Note that all slopes are calculated at the moment of creation
 * (function ObjectLoadSetup), and that the waveform has to be
 * resetted (function Reset) between a pulse and the other.
 */
class IntegerWaveform {

private:
    /** Pointer to the current element in the IntegerWaveform */
    IntegerWaveformData* current;
    /** Pointer to the first element in the IntegerWaveform */
    IntegerWaveformData* waveform;

private:

    /** Minimum value */
    float                minimumValue;

    /** Maximum value */
    float                maximumValue;

public:
	/** Constructor */
    IntegerWaveform() {
    	current      = NULL;
    	waveform     = NULL;
        minimumValue = -1e16;
        maximumValue = +1e16;
    }

    /** Deconstructor */
    virtual ~IntegerWaveform() {
    	current = NULL;
    	if (waveform != NULL) free((void *&)waveform);
    }

    /**
    * Loads parameters from a CDB
    * @param cdbData the CDB
    * @return True if the initialisation went ok, False otherwise
    */
    bool ObjectLoadSetup(ConfigurationDataBase& cdbData) {
        if (waveform != NULL) {
            free((void *&)waveform);
            current = NULL;
        }

        CDBExtended cdb(cdbData);

        int timeSize[1] = {0};
        int amplSize[1] = {0};
        int maxTimeSize = 1;
        int maxAmplSize = 1;


        // Check times and amplitude sizes
        if(!cdb->GetArrayDims(timeSize,maxTimeSize,"UsecTimes")){
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to obtain UsecTimes array");
            return False;
        }
        if(!cdb->GetArrayDims(amplSize,maxAmplSize,"Amplitudes")){
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to obtain Amplitudes array");
            return False;
        }

        if(timeSize[0] != amplSize[0]) {
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: Times and Amplitudes arrays have different length.");
            return False;
        }

        if (timeSize[0] < 2) {
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: Times and Amplitudes arrays must have at least 2 elements.");
            return False;
        }

        // Load in memory data
        waveform = (IntegerWaveformData*)malloc(sizeof(IntegerWaveformData)*(timeSize[0]+2));

        if (waveform == NULL) {
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to malloc array");
            return False;
        }

        int32* usecTimes  = (int32*)malloc(sizeof(int32*)*(timeSize[0]));
        float* amplitudes = (float*)malloc(sizeof(float*)*(timeSize[0]));

        if ((usecTimes == NULL) || (amplitudes ==NULL )) {
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to malloc usectimes or amplitudes array");
            if (usecTimes != NULL)  free((void*&)usecTimes);
            if (amplitudes != NULL) free((void*&)amplitudes);
            return False;
        }

        if(!cdb.ReadInt32Array(usecTimes,timeSize,maxTimeSize,"UsecTimes") ){
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise error reading UsecTimes");
            if (usecTimes != NULL)  free((void*&)usecTimes);
            if (amplitudes != NULL) free((void*&)amplitudes);
            return False;
        }
        if(!cdb.ReadFloatArray(amplitudes,amplSize,maxAmplSize,"Amplitudes") ){
            CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise error reading Amplitudes");
            if (usecTimes != NULL)  free((void*&)usecTimes);
            if (amplitudes != NULL) free((void*&)amplitudes);
            return False;
        }

        minimumValue = +1e16;
        maximumValue = -1e16;
        int i = 0;
        for (i=0; i < timeSize[0]; i++) {
            if(amplitudes[i] > maximumValue) maximumValue = amplitudes[i];
            if(amplitudes[i] < minimumValue) minimumValue = amplitudes[i];
        }

        // Calculate slopes offline
        for (i=0; i < timeSize[0]; i++) {
            waveform[i+1].amplitude = amplitudes[i];
            waveform[i+1].usecTime  = usecTimes[i];
        }

        for (i=0; i < timeSize[0] - 1; i++) {
            float deltaT = (float)(usecTimes[i+1] - usecTimes[i]);
            waveform[i+1].slope = (amplitudes[i+1] - amplitudes[i]) / deltaT;
        }

        current = &waveform[0];
        if (usecTimes != NULL)  free((void*&)usecTimes);
        if (amplitudes != NULL) free((void*&)amplitudes);

        waveform[0].usecTime  = 0;
        waveform[0].slope     = 0;
        waveform[0].amplitude = waveform[1].amplitude;
        waveform[timeSize[0]+1].usecTime  = 0x7FFFFFFF;
        waveform[timeSize[0]+1].slope     = 0;
        waveform[timeSize[0]+1].amplitude = waveform[timeSize[0]].amplitude;

        return True;
    }

    /**
     * Returns the value of the waveform at a certain time.
     * @param usecTime Time in microseconds.
     *        Note that usecTime needs to be crescent: in order
     *        to obtain the value of the waveform in the past
     *        you must Reset it first.
     * @return The value of the waveform at the specified time.
     *         If usecTime is before or after the start or end time
     *         of the waveform, the first or last value is held.
     */
    inline float GetValue(int32 usecTime) {
    	if (current == NULL) return 0;
    	while ((current+1)->usecTime < usecTime) {
    		current++;
    	}
        float deltaT = (float)(usecTime - current->usecTime);
    	float value = current->amplitude + deltaT * current->slope;
    	return value;
    }

    /** Reset function. Resets the internal states and waveforms. To be called in the PREPULSE phase. */
    inline void Reset() {
    	current = waveform;
    }

    /**
     * Returns waveform's minimum value.
     */
    float Minimum(){
        return minimumValue;
    }

    /**
    * Returns waveform's maximum value.
    */
    float Maximum(){
        return maximumValue;
    }

};


#endif
