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

#if !defined (WAVEFORMCLASSRANDOM_H)
#define WAVEFORMCLASSRANDOM_H

#include "System.h"
#include "WaveformGenericClass.h"

#ifndef RAND_MAX
    #define RAND_MAX  2147483647
#endif

const float INV_RAND_MAX = (float)(1.0/RAND_MAX);

OBJECT_DLL(WaveformClassRandom)
class WaveformClassRandom: public WaveformGenericClass {
    OBJECT_DLL_STUFF(WaveformClassRandom)

private:

    float range[2];

public:

    /** */
    WaveformClassRandom() {
	range[0] = 0.0;
	range[1] = 1.0;
    };

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    virtual float GetValue(int32 usecTime);

    /** */
    void Reset() {};

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
