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

#if !defined (WAVEFORMCLASSINCREMENT_H)
#define WAVEFORMCLASSINCREMENT_H

#include "System.h"
#include "WaveformGenericClass.h"

OBJECT_DLL(WaveformClassIncrement)
class WaveformClassIncrement: public WaveformGenericClass {
    OBJECT_DLL_STUFF(WaveformClassIncrement)

private:

    double increment;

    double value;

    double startValue;

public:

    /** */
    WaveformClassIncrement() {
        startValue = 0.0;
        value      = startValue;
        increment  = 0.0;
    };

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    virtual float GetValue(int32 usecTime);

    /** */
    virtual int32 GetValueInt32(int32 usecTime);

    /** */
    void Reset() {
        value = startValue;
    };

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
