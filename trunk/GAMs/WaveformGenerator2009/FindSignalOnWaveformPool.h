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
#if !defined (FINDSIGNALONWAVEFORMPOOL_H)
#define FINDSIGNALONWAVEFORMPOOL_H

#include "System.h"
#include "FindSignalOnSource.h"
#include "WaveformInterface.h"

class FindSignalOnWaveformPool: public FindSignalOnSource{

    GCReferenceContainer    *gcrc;

public:

    /**  */
    FindSignalOnWaveformPool(GCReferenceContainer *gcrc){
        this->gcrc = gcrc;
    }

    /**  */
    FindSignalOnWaveformPool(){
    };

    /**  */
    int Find(char *name, int &dimension){

        dimension = 0;
        GCRTemplate<WaveformInterface> gcW = gcrc->Find(name);
        if( gcW.IsValid() ){
            dimension = 1;
        }

        return 0;
    };

};

#endif







