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
#ifndef WAVEFORMCLASSEVENTEQUENCER_H_
#define WAVEFORMCLASSEVENTEQUENCER_H_

#include "System.h"
#include "WaveformGenericClass.h"
#include "LoadCDBObjectClass.h"

OBJECT_DLL(WaveformClassEventSequencer)
class WaveformClassEventSequencer : public WaveformGenericClass {
OBJECT_DLL_STUFF(WaveformClassEventSequencer)

private:

    /** */
    int32                                 numberOfWaveforms;

    /** */
    FString                              *events;

    /** */
    GCRTemplate<WaveformInterface>       *waveforms;

    /** */
    uint32                               *eventCode;

public:

    /** */
    WaveformClassEventSequencer() {
        numberOfWaveforms         = 0;
        waveforms                 = NULL;
        events                    = NULL;
        eventCode                 = NULL;
    }

    /** */
    ~WaveformClassEventSequencer() {
        CleanUp();
    }

    /** Copy constructor */
    WaveformClassEventSequencer(const WaveformClassEventSequencer &wave) : WaveformGenericClass(wave) {
        this->numberOfWaveforms    = wave.numberOfWaveforms;
        this->events               = wave.events;
        this->eventCode            = wave.eventCode;

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

    /** */
    virtual int32 GetValueInt32(int32 usecTime);

private:

    /** */
    void CleanUp() {
        if( waveforms != NULL ) delete [] waveforms;
        waveforms = NULL;
        if( events != NULL ) delete [] events;
        events = NULL;
        if( eventCode != NULL ) delete [] eventCode;
        eventCode = NULL;
    }

};


#endif /* EVENTSEQUENCERWAVEFORM_H_ */
