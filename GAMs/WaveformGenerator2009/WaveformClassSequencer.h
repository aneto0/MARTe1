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

#ifndef WAVEFORMCLASSEQUENCER_H_
#define WAVEFORMCLASSSEQUENCER_H_

#include "System.h"
#include "WaveformGenericClass.h"
#include "Matrix.h"

OBJECT_DLL(WaveformClassSequencer)
class WaveformClassSequencer: public WaveformGenericClass{
OBJECT_DLL_STUFF(WaveformClassSequencer)

private:

    /** */
    int32                                 numberOfWaveforms;

    /** */
    MatrixI                               timeWindowsUsecTime;

    /** */
    GCRTemplate<WaveformInterface>*       waveforms;

    /** */
    int32                                 currentWaveformIndex;

public:

    /** */
    WaveformClassSequencer() {
        numberOfWaveforms         = 0;
	currentWaveformIndex      = 0;
        waveforms                 = NULL;
    }

    /** */
    ~WaveformClassSequencer() {
        CleanUp();
    }

    /** Copy constructor */
    WaveformClassSequencer(const WaveformClassSequencer &wave) : WaveformGenericClass(wave) {
        this->numberOfWaveforms    = wave.numberOfWaveforms;
        this->timeWindowsUsecTime  = wave.timeWindowsUsecTime;
	this->currentWaveformIndex = wave.currentWaveformIndex;

        if( wave.waveforms != NULL && wave.numberOfWaveforms > 0 ) {
            this->waveforms = new GCRTemplate<WaveformInterface>[numberOfWaveforms];
            if( this->waveforms == NULL ){
                AssertErrorCondition(InitialisationError,"WaveformClassSequencer %s::Copy constructor: failed to allocate the waveforms array", Name());
                return;
            }
            int i;
            for( i = 0; i < wave.numberOfWaveforms; i++ ){
                this->waveforms[i] = wave.waveforms[i];
            }
        }
    }


    /** */
    void Reset() {
        int i;
        for( i = 0; i < numberOfWaveforms; i++ ){
            waveforms[i]->Reset();
        }
        currentWaveformIndex = 0;
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

    /** */
    virtual float GetValue(int32 usecTime);

private:

    /** */
    void CleanUp() {
        if( waveforms != NULL ) delete [] waveforms;
        waveforms = NULL;
    }

};


#endif /* SEQUENCERWAVEFORM_H_ */
