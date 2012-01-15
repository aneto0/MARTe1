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

#include "WaveformClassSine.h"

///
WaveformClassSine::WaveformClassSine(){
    sweepingSet = True;
}

///
WaveformClassSine::~WaveformClassSine(){
}

///
bool WaveformClassSine::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformPeriodicClass::ObjectLoadSetup(cdbData,NULL) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassSine::Init error ObjectLoadSetup");
        return False;
    }

    CDBExtended cdb(cdbData);

    FString sweep;
    cdb.ReadFString(sweep,"Sweeping","ON");

    sweepingSet = True;

    if( strcmp(sweep.Buffer(),"ON")       == 0 ) sweepingSet = True;
    else if( strcmp(sweep.Buffer(),"OFF") == 0 ) sweepingSet = False;

    CStaticAssertErrorCondition(Information,"WaveformClassSine::Init Sweeping option = %s",sweep.Buffer());

    // 2pi is multiplied during run-time
    phase /= 360.0;

    GenerateDataPlot(tStart,tEnd);

    return True;
}

///
float WaveformClassSine::GetValue(int32 actTimeUsec){

    if( WaveformPeriodicClass::Execute(actTimeUsec) ){
        if( sweepingSet ) return sin(PI2*integratedFrequency)*gain+offsetValue;
        else              return sin(PI2*localPhase)*gain+offsetValue;
    }

    return 0.0;
}

///
bool WaveformClassSine::ProcessHttpMessage(HttpStream &hStream) {

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassSine %s</title></head>\n", Name());

    hStream.Printf("<h1>WaveformClassSine %s</h1>", Name());

    WaveformPeriodicClass::ProcessHttpMessage(hStream);

    ProcessGraphHttpMessage(hStream);

    hStream.Printf("</body></html>");

    hStream.WriteReplyHeader(True);

    return True;
}


OBJECTLOADREGISTER(WaveformClassSine,"$Id$")

