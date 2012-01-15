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

#if !defined (WAVEFORMCLASSOPT_H)
#define WAVEFORMCLASSOPT_H

#include "System.h"
#include "WaveformGenericClass.h"

#define NOTDEFINED      0

#define SUM             0x001
#define SUB             0x002
#define PROD            0x003
#define RATIO           0x004

#define ABS             0x101   // absolute
#define SQRT            0x102   // square root
#define INV             0x103   // inverted sign


OBJECT_DLL(WaveformClassOpt)
class WaveformClassOpt: public WaveformGenericClass{
    OBJECT_DLL_STUFF(WaveformClassOpt)

private:

    /** */
    GCRTemplate<WaveformInterface>      wave1;

    /** */
    GCRTemplate<WaveformInterface>      wave2;

    /** */
    int                                 optType;

public:

    /** */
    WaveformClassOpt();

    /** */
    ~WaveformClassOpt();

    /** Copy constructor */
    WaveformClassOpt(const WaveformClassOpt &wave) : WaveformGenericClass(wave) {
        this->optType = wave.optType;

        switch( this->optType ){
            case SUM:
            case SUB:
            case PROD:
            case RATIO:{
                this->wave1 = wave.wave1;
                this->wave2 = wave.wave2;
            }break;

            case ABS:
            case SQRT:
            case INV:{
                this->wave1 = wave.wave1;
            }break;
        }
    }

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    virtual float GetValue(int32 usecTime);

    /** */
    void Reset(){};

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
