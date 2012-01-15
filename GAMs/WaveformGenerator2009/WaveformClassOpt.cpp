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

#include "WaveformClassOpt.h"


///
WaveformClassOpt::WaveformClassOpt(){
    optType = NOTDEFINED;
}

///
WaveformClassOpt::~WaveformClassOpt(){
}

///
bool WaveformClassOpt::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformGenericClass::ObjectLoadSetup(cdbData,NULL) ){
        AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init error ObjectLoadSetup");
        return False;
    }

    CDBExtended cdb(cdbData);

    FString error;

    FString opt;
    if( !cdb.ReadFString(opt,"Operator") ){
        AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init Operator not defined");
        return False;
    }

         if( strcmp(opt.Buffer(),"ADD"   ) == 0 ) optType = SUM;
    else if( strcmp(opt.Buffer(),"SUB"   ) == 0 ) optType = SUB;
    else if( strcmp(opt.Buffer(),"PROD"  ) == 0 ) optType = PROD;
    else if( strcmp(opt.Buffer(),"RATIO" ) == 0 ) optType = RATIO;
    else if( strcmp(opt.Buffer(),"ABS"   ) == 0 ) optType = ABS;
    else if( strcmp(opt.Buffer(),"SQRT"  ) == 0 ) optType = SQRT;
    else if( strcmp(opt.Buffer(),"INV"   ) == 0 ) optType = INV;
    else{
        AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init Operator unknown");
        return False;
    }

    if( Size() == 0 || Size() > 2 ){
        AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init number of waveform != 2");
        return False;
    }

    if( (Size() == 2) && (optType & 0x100) ){
        AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init number of waveform = 2 and unary operator");
        return False;
    }

    wave1 = Find(0);
    if( !wave1.IsValid() ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassOpt::Init Wave1 is not WaveformInterface type");
        return False;
    }

    if( Size() == 2 ){
        wave2 = Find(1);
        if( !wave2.IsValid() ){
            AssertErrorCondition(InitialisationError,"WaveformClassOpt::Init Wave2 is not WaveformInterface type");
            return False;
        }
    }

    GenerateDataPlot(tStart,tEnd);

    return True;
}

///
float WaveformClassOpt::GetValue(int32 usecTime){

    float output = 0.0;

    if( WaveformGenericClass::Execute(usecTime) ){

        switch( optType ){
            case SUM:  output = wave1->GetValue(usecTime)+wave2->GetValue(usecTime); break;
            case SUB:  output = wave1->GetValue(usecTime)-wave2->GetValue(usecTime); break;
            case PROD: output = wave1->GetValue(usecTime)*wave2->GetValue(usecTime); break;
            case RATIO:{
                float wave2Value = wave2->GetValue(usecTime);
                if( wave2Value != 0.0 ) output = wave1->GetValue(usecTime)/wave2Value;
                else{
                    AssertErrorCondition(Warning,"WaveformClassOpt::GetValue Operator Ratio: Wave2 is = 0; run-time warning");
                    return 0.0;
                }
            }break;
            case ABS:  output = fabs(wave1->GetValue(usecTime)); break;
            case SQRT:{
                float waveValue = wave1->GetValue(usecTime);
                if( waveValue >= 0 ) output = sqrt(waveValue);
                else{
                    AssertErrorCondition(Warning,"WaveformClassOpt::GetValue Operator SQRT: Wave is < 0; run-time warning");
                    return 0.0;
                }
            }break;
            case INV:  output = -wave1->GetValue(usecTime); break;
        }
	output = output*gain+offsetValue;
    }

    return output;
}


///
bool WaveformClassOpt::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassOpt %s</title></head>\n<body>", Name());

    WaveformGenericClass::ProcessHttpMessage(hStream);

    switch( optType ){
        case SUM:   hStream.Printf("<p>Waveform operator SUM </p>");break;
        case SUB:   hStream.Printf("<p>Waveform operator SUM </p>");break;
        case PROD:  hStream.Printf("<p>Waveform operator SUM </p>");break;
        case RATIO: hStream.Printf("<p>Waveform operator SUM </p>");break;
        case ABS:   hStream.Printf("<p>Waveform operator SUM </p>");break;
        case SQRT:  hStream.Printf("<p>Waveform operator SUM </p>");break;
        case INV:   hStream.Printf("<p>Waveform operator SUM </p>");break;
    }

    hStream.Printf("<h1>WaveformClassOpt %s</h1>", Name());

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassOpt,"$Id$")






