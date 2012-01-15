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

#include "WaveformClassSquare.h"

///
WaveformClassSquare::WaveformClassSquare(){
    simmEnable = False;
}


///
float WaveformClassSquare::GetValue(int32 usecTime){

    if( WaveformPeriodicClass::Execute(usecTime) ){

        if( actFrequency == 0 ){
            CStaticAssertErrorCondition(Warning,"WaveformClassSquare::Init run-time error. Frequency = 0");
            return 0.0;
        }

        if( simmEnable ){
            if( localPhase > dutyCycle ) return -gain+offsetValue;
            else                         return  gain+offsetValue;
        }else{
            if( localPhase > dutyCycle ) return offsetValue;
            else                         return gain+offsetValue;
        }
    }

    return 0.0;
}

///
bool WaveformClassSquare::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformPeriodicClass::ObjectLoadSetup(cdbData,NULL) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassSquare::Init error ObjectLoadSetup");
        return False;
    }

    if( !variableFrequency && frequency == 0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassSquare::Init Frequency = 0");
        return False;
    }

    CDBExtended cdb(cdbData);

    if( dutyCycle <= 0.0 || dutyCycle >= 100.0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassSquare::Init error value DutyCycle ]0,100[");
        return False;
    }
    dutyCycle /= 100.0;

    if( phase < 0.0 || phase >= 100.0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassSquare::Init error value Phase [0,100[");
        return False;
    }
    phase /= 100.0;

    FString simmEntry;
    cdb.ReadFString(simmEntry,"Simmetric","Off");
    if( strcmp(simmEntry.Buffer(),"On") == 0 ) simmEnable = True;
    else                                       simmEnable = False;

    GenerateDataPlot(tStart,tEnd);

    return True;
}

///
bool WaveformClassSquare::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassSquare %s</title></head>\n<body>", Name());

    hStream.Printf("<h1>WaveformClassSquare %s</h1>", Name());

    WaveformPeriodicClass::ProcessHttpMessage(hStream);

    if( simmEnable ) hStream.Printf("<p>Simmetric = On</p>");
    else             hStream.Printf("<p>Simmetric = Off</p>");


    ProcessGraphHttpMessage(hStream);

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassSquare,"$Id: WaveformClassSquare.cpp,v 1.3 2009/10/28 16:10:56 lzabeo Exp $")

