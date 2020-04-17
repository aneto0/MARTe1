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
 * @brief Definition of the waveforms.
 */
#ifndef WAVEFORMINTERFACE_H_
#define WAVEFORMINTERFACE_H_

#include "System.h"
#include "GAM.h"

class WaveformInterface {
public:
    virtual float         GetValue(int32 usecTime)  = 0;

    virtual int32         GetValueInt32(int32 usecTime) {
        return (int32)GetValue(usecTime);
    };

    inline virtual double GetValueD(int32 usecTime) {
        return (double)GetValue(usecTime);
    }

    inline virtual float  GetValue64(int64 usecTime) {
        return GetValue((int32)usecTime);
    }

    inline virtual double GetValueD64(int64 usecTime) {
        return (double)GetValue((int32)usecTime);
    }

    inline virtual void SetState(GAM_FunctionNumbers state) {}

    virtual void  Reset()                           = 0;
};


#endif /* WAVEFORMINTERFACE_H_ */
