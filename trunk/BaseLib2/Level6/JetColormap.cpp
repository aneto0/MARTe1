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

#include "JetColormap.h"

bool JetColormap::BuildJetColorMap() {
    if(nColors <= 0) {
        CStaticAssertErrorCondition(InitialisationError, "EventGAM::Colormap: unable to build color map");
	return False;
    }
    for(int32 i = 0 ; i < nColors ; i++) {
        if(i <= nColors/3.0) {
	    cMap[i].red   = 0;
	    cMap[i].green = i*256*3/nColors;
	    cMap[i].blue  = 256;
	} else if((i > nColors/3.0) && (i <= 2.0*nColors/3.0)) {	      
	    cMap[i].red   = (int32)((int32)floor(i-nColors/3.0)*3*256/nColors);
	    cMap[i].green = 256;
	    cMap[i].blue  = (int32)(256-(int32)floor(i-nColors/3.0)*3*256/nColors);
	} else {
	    cMap[i].red   = 256;
	    cMap[i].green = (int32)(256-(int32)floor(i-2.0*nColors/3.0)*3*256/nColors);
	    cMap[i].blue  = 0;
	}
    }
    return True;
}
