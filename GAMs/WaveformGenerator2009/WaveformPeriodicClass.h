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

#if !defined (WAVEFORMPERIODICCLASS_H)
#define WAVEFORMPERIODICCLASS_H

#include "System.h"
#include "WaveformGenericClass.h"

class WaveformPeriodicClass: public WaveformGenericClass{

protected:

    /** Fixed frequency */
    float                               frequency;

    /** Frequency waveform */
    GCRTemplate<WaveformInterface>      frequencyWaveform;

    /** Fixed/Variable frequency flag */
    bool                                variableFrequency;


/*************************/
/*  Additional parameter */
/*************************/

    /** Dutycycle for periodic waveform */
    float                               dutyCycle;

    /** Phase for periodic waveform */
    float                               phase;

/*************************/
/*  Utility variables    */
/*************************/

    /** Local phase */
    float                               localPhase;

    /** Integrated frequency for sweeping option */
    float                               integratedFrequency;

    /** Last sample frequency */
    float                               oldFrequency;

    /** Last sample absolute time (Usec) */
    int32                               oldUsecTime;

    /** Actual period (Usec) */
    int32                               actPeriod;

    /** Time init of the last period (Usec) */
    int32                               startPeriod;

    /** Actual frequency */
    float                               actFrequency;

    /** If True the frequency is discretised as sequence of frequencies. Default = ON */
    bool                                discretiseFreq;

public:

    /** */
    WaveformPeriodicClass(){
        frequency           = 0.0;
        variableFrequency   = False;
        dutyCycle           = 50.0;
        phase               = 0.0;
        discretiseFreq      = True;
        Reset();
    }

    /** Copy constructor */
    WaveformPeriodicClass(const WaveformPeriodicClass &wave) : WaveformGenericClass(wave){
        this->frequency           = wave.frequency;
        this->variableFrequency   = wave.variableFrequency;
        this->dutyCycle           = wave.dutyCycle;
        this->phase               = wave.phase;
        this->integratedFrequency = wave.integratedFrequency;
        this->signalType          = wave.signalType;
        this->isIntSignal         = wave.isIntSignal;
        this->oldFrequency        = wave.oldFrequency;
        this->oldUsecTime         = wave.oldUsecTime;
        this->localPhase          = wave.localPhase;

        if( this->variableFrequency ) this->frequencyWaveform = wave.frequencyWaveform;
    }


    /** */
    ~WaveformPeriodicClass(){};

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    virtual void Reset(){
        actPeriod           = -1;
        startPeriod         = -1;
        actFrequency        = 0.0;
        integratedFrequency = 0.0;
        oldFrequency        = 0.0;
        oldUsecTime         = -1;
        localPhase          = 0.0;
    }

    /** */
    bool Execute(int32 actTimeUsec);

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
