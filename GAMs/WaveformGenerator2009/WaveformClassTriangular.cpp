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

#include "WaveformClassTriangular.h"

///
WaveformClassTriangular::WaveformClassTriangular(){
}

///
WaveformClassTriangular::~WaveformClassTriangular(){
}

///
float WaveformClassTriangular::GetValue(int32 usecTime){

    if( WaveformPeriodicClass::Execute(usecTime) ){

        if( actFrequency == 0 ){
            CStaticAssertErrorCondition(Warning,"WaveformClassTriangular::Init run-time error. Frequency = 0");
            return 0.0;
        }

        if( localPhase < dutyCycle ) return offsetValue + gain * localPhase / dutyCycle;
        else                         return offsetValue + gain * (localPhase-1.0) / (dutyCycle-1.0);
    }

    return 0.0;
}


///
bool WaveformClassTriangular::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformPeriodicClass::ObjectLoadSetup(cdbData,NULL) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassTriangular::Init error ObjectLoadSetup");
        return False;
    }

    if( !variableFrequency && frequency == 0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassSquare::Init Frequency = 0");
        return False;
    }

    CDBExtended cdb(cdbData);

    if( dutyCycle <= 0.0 || dutyCycle >= 100.0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassTriangular::Init error value DutyCycle ]0,100[");
        return False;
    }
    dutyCycle /= 100.0;

    if( phase < 0.0 || phase >= 100.0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassTriangular::Init error value Phase [0,100[");
        return False;
    }
    phase /= 100.0;

    GenerateDataPlot(tStart,tEnd);

    return True;
}

///
bool WaveformClassTriangular::ProcessHttpMessage(HttpStream &hStream){

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassTriangular %s</title></head>\n", Name());

    hStream.Printf("<h1>WaveformClassTriangular %s</h1>", Name());

    WaveformPeriodicClass::ProcessHttpMessage(hStream);

    ProcessGraphHttpMessage(hStream);

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassTriangular,"$Id$")
