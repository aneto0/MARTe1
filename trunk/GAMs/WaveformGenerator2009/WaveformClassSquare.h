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

#if !defined (WAVEFORMCLASSSQUARE_H)
#define WAVEFORMCLASSSQUARE_H

#include "System.h"
#include "WaveformPeriodicClass.h"

OBJECT_DLL(WaveformClassSquare)
class WaveformClassSquare: public WaveformPeriodicClass{
    OBJECT_DLL_STUFF(WaveformClassSquare)

private:

    /** Simmetric waveform */
    bool    simmEnable;

public:

    /** */
    WaveformClassSquare();

    /** */
    ~WaveformClassSquare(){};

    /** Copy constructor */
    WaveformClassSquare(const WaveformClassSquare &wave) : WaveformPeriodicClass(wave) {
        this->simmEnable = wave.simmEnable;
    }

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    virtual float GetValue(int32 usecTime);

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
