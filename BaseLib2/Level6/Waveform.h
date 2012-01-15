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
 * @brief Base implemention of the waveform.
 *
 * Provides the basic functionality for any waveform, like the CDB loading and 
 * getting the value at a given time.
 */
#ifndef __WAVEFORM__
#define __WAVEFORM__

#include "System.h"
#include "CDBExtended.h"
#include "HttpInterface.h"
#include "GCNamedObject.h"
#include "WaveformInterface.h"
#include "SVGGraph.h"


/**
 * Structure holding data for a single element in an IntegerWaveform.
 */
struct WaveformData {
	int32 usecTime;
	float amplitude;
	float slope;
};

/**
 * A waveform which keeps its internal time in microseconds but loads data from
 * CDB in seconds.
 * Note that all slopes are calculated at the moment of creation
 * (function ObjectLoadSetup), and that the waveform has to be
 * resetted (function Reset) between a pulse and the other.
 */
OBJECT_DLL(Waveform)
class Waveform: public WaveformInterface, public GCNamedObject, public HttpInterface {
OBJECT_DLL_STUFF(Waveform)

private:
    /** Index of the current element in the IntegerWaveform */
    int32         current;
    /** Pointer to the first element in the IntegerWaveform */
    WaveformData* waveform;
    /** Rounding factor for conversion between seconds and microseconds */
    int32         rounding;
    /** Number of time windows */
    int32         numberOfWindows;

private:
    /** Minimum value */
    float                minimumValue;
    /** Maximum value */
    float                maximumValue;

private:
    /** The graph of the waveform */
    SVGGraph             svgGraph;


public:
	/** Constructor */
    Waveform() {
    	current         = 0;
    	waveform        = NULL;
        minimumValue    = -1e16;
        maximumValue    = +1e16;
        rounding        = 0;
        numberOfWindows = 0;
    }

    /** Deconstructor */
    virtual ~Waveform() {
    	if (waveform != NULL) free((void *&)waveform);
    }

    /**
    * Loads parameters from a CDB
    * @param cdbData the CDB
    * @return True if the initialisation went ok, False otherwise
    */
    virtual bool ObjectLoadSetup(ConfigurationDataBase& cdbData, StreamInterface *err);

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
    virtual float GetValue(int32 usecTime);

    /** Reset function. Resets the internal states and waveforms. To be called in the PREPULSE phase. */
    inline virtual void Reset() {
    	current = 0;
    }

    virtual bool ProcessHttpMessage(HttpStream &hStream);

    void HTMLInfo(HttpStream &hStream);

    inline WaveformData* GetWaveformDataPointer() {
        return waveform;
    }

    inline int32 NumberOfWindows() {
        return numberOfWindows;
    }

};


#endif
