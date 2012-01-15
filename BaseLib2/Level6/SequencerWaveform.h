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
 * @brief Waveform which is a collection of other waveforms.
 */
#ifndef SEQUENCERWAVEFORM_H_
#define SEQUENCERWAVEFORM_H_

#include "System.h"
#include "WaveformInterface.h"
#include "HttpInterface.h"
#include "GCReferenceContainer.h"
#include "GCRTemplate.h"
#include "CDBExtended.h"


OBJECT_DLL(SequencerWaveform)
class SequencerWaveform: public WaveformInterface, public GCReferenceContainer, public HttpInterface {
OBJECT_DLL_STUFF(SequencerWaveform)
// Parameters
private:
    int32                                 numberOfWaveforms;

    int32*                                waveformUsecStartTimes;

    GCRTemplate<WaveformInterface>*       waveforms;

// States
private:
    int32                                 currentWaveformIndex;

private:
    void CleanUp() {
        if (waveforms != NULL) {
            delete [] waveforms;
            waveforms = NULL;
        }

        if (waveformUsecStartTimes) {
            free((void*&)waveformUsecStartTimes);
            waveformUsecStartTimes = NULL;
        }
    }

public:
    SequencerWaveform() {
        numberOfWaveforms         = 0;
        currentWaveformIndex      = 0;
        waveformUsecStartTimes    = NULL;
        waveforms                 = NULL;
    }

    ~SequencerWaveform() {
        CleanUp();
    }

    virtual void Reset() {
        int i;
        for (i=0; i<numberOfWaveforms; i++) {
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

    virtual float GetValue(int32 usecTime);
};


#endif /* SEQUENCERWAVEFORM_H_ */
