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

#if !defined (WAVEFORMCLASSPOINTS_H)
#define WAVEFORMCLASSPOINTS_H

#include "System.h"
#include "WaveformPeriodicClass.h"


OBJECT_DLL(WaveformClassPoints)
class WaveformClassPoints: public WaveformPeriodicClass{
    OBJECT_DLL_STUFF(WaveformClassPoints)

private:

    /** */
    int32         *timeBase;

    /** */
    float         *values;

    /** */
    int           numberOfValues;

    /** */
    bool          closestEnable;

public:

    /** */
    WaveformClassPoints();

    /** */
    ~WaveformClassPoints();

    /** Copy constructor */
    WaveformClassPoints(const WaveformClassPoints &wave) : WaveformPeriodicClass(wave) {
        this->numberOfValues = wave.numberOfValues;
        this->closestEnable  = wave.closestEnable;
        if( wave.numberOfValues <= 0 || wave.timeBase == NULL || wave.values == NULL ){
            AssertErrorCondition(InitialisationError,"WaveformClassPoints %s::Copy constructor: input data incorrect", Name());
            return;
        }

        this->timeBase = (int32*)malloc(sizeof(int32)*wave.numberOfValues);
        this->values   = (float*)malloc(sizeof(float)*wave.numberOfValues);
        if( this->timeBase == NULL || this->values == NULL ){
            AssertErrorCondition(InitialisationError,"WaveformClassPoints %s::Copy constructor: error allocating memory", Name());
            return;
        }
        int i;
        for( i = 0; i < wave.numberOfValues; i++ ){
            this->timeBase[i] = wave.timeBase[i];
            this->values[i]   = wave.values[i];
        }
    }

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    virtual float GetValue(int32 usecTime);

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);

private:

    /** */
    bool CheckTimeBase(float *tBase, int nValues);
};

#endif
