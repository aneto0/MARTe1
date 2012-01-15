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

#include "WaveformClassTime.h"


///
WaveformClassTime::WaveformClassTime(){
}

///
WaveformClassTime::~WaveformClassTime(){
}

///
bool WaveformClassTime::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformGenericClass::ObjectLoadSetup(cdbData,NULL) ){
        AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init error ObjectLoadSetup");
        return False;
    }

    GenerateDataPlot(tStart,tEnd);
    
    return True;
}

///
float WaveformClassTime::GetValue(int32 usecTime){

    if( WaveformGenericClass::Execute(usecTime) ){
        return offsetValue+gain*((float)usecTime)/1e6;
    }

    return 0.0;
}


///
bool WaveformClassTime::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassTime %s</title></head>\n<body>", Name());

    hStream.Printf("<h1>WaveformClassTime %s</h1>", Name());

    WaveformGenericClass::ProcessHttpMessage(hStream);

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassTime,"$Id: WaveformClassTime.cpp,v 1.2 2009/10/28 16:10:56 lzabeo Exp $")






