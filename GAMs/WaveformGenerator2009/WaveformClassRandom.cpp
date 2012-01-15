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

#include "WaveformClassRandom.h"
#include "HRT.h"

///
bool WaveformClassRandom::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformGenericClass::ObjectLoadSetup(cdbData,NULL) ){
        AssertErrorCondition(InitialisationError,"WaveformClassRandom::Init error ObjectLoadSetup");
        return False;
    }

    CDBExtended cdb(cdbData);

    if(!cdb.ReadFloat(range[0], "MinVal", 0.0)) {
        AssertErrorCondition(Warning, "WaveformClassRandom::Init MinVal parameter not found, assuming %f", range[0]);
        return False;
    }
    if(!cdb.ReadFloat(range[1], "MaxVal", 1.0)) {
        AssertErrorCondition(Warning, "WaveformClassRandom::Init MinVal parameter not found, assuming %f", range[1]);
        return False;
    }

    // Seed the random number generator
#if defined(_VXWORKS) || defined(_WIN32)
    srand(HRT::HRTCounter());
#elif defined (_RTAI)
#else
    srandom(HRT::HRTCounter());
#endif

    return True;
}

///
float WaveformClassRandom::GetValue(int32 usecTime) {

#if defined(_VXWORKS)
    float output = range[0] + (range[1]-range[0])*random()*INV_RAND_MAX;
#else
    float output = range[0] + (range[1]-range[0])*rand()*INV_RAND_MAX;
#endif

    // if(WaveformGenericClass::Execute(usecTime)) {
    // 	output = output*gain+offsetValue;
    // }

    return output;
}


///
bool WaveformClassRandom::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassRandom %s</title></head>\n<body>", Name());

    WaveformGenericClass::ProcessHttpMessage(hStream);

    hStream.Printf("<h1>WaveformClassRandom %s</h1>", Name());

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassRandom,"$Id: WaveformClassRandom.cpp,v 1.4 2011/07/21 14:19:41 aneto Exp $")






