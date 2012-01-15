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

#if !defined (WAVEFORMGENERATOR_H)
#define WAVEFORMGENERATOR_H

#include "System.h"
#include "GAM.h"
#include "DDBOutputInterface.h"
#include "DDBInputInterface.h"
#include "WaveformInterface.h"

/** One needs to make sure that these events occur only once 
    and are not continuous 
*/
static uint32 EVENT_None      = 0xFFFFFFFF;
static uint32 EVENT_PrePulse  = GAMPrepulse;
static uint32 EVENT_PostPulse = GAMPostpulse;

/**

*/
OBJECT_DLL(WaveformGenerator)
class WaveformGenerator: public GAM{
    OBJECT_DLL_STUFF(WaveformGenerator)

private:

    /** DDB interface for time. */
    DDBInputInterface                   *ddbInputInterface;

    /** DDB interface. */
    DDBOutputInterface                  *ddbOutputInterface;

    /** List of Waveforms */
    GCRTemplate<WaveformInterface>      *waveformList;

    /** Output type list */
    bool                                *isIntOutputList;

    /** */
    uint32                               resetEvent;

public:

    /** */
    WaveformGenerator();

    /** */
    ~WaveformGenerator();

    /**  */
    bool Initialise(ConfigurationDataBase& cdbData);

    /**  */
    bool Execute(GAM_FunctionNumbers functionNumber);

    /**  */
    bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){return True;};

};

#endif
