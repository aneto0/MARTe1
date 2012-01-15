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

#include "WaveformClassIncrement.h"
#include "HRT.h"

///
bool WaveformClassIncrement::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformGenericClass::ObjectLoadSetup(cdbData,NULL) ){
        AssertErrorCondition(InitialisationError,"WaveformClassIncrement::Init error ObjectLoadSetup");
        return False;
    }

    CDBExtended cdb(cdbData);

    if(!cdb.ReadDouble(startValue, "StartValue", 0.0)) {
        AssertErrorCondition(Warning, "WaveformClassIncrement::Init StartValue parameter not found, assuming %f", startValue);
    }
    if(!cdb.ReadDouble(increment, "Increment", 0.0)) {
        AssertErrorCondition(Warning, "WaveformClassIncrement::Init Increment parameter not found, assuming %f", increment);
    }

    Reset();

    return True;
}

///
float WaveformClassIncrement::GetValue(int32 usecTime) {

    value += increment;

    return (float)value;
}

///
int32 WaveformClassIncrement::GetValueInt32(int32 usecTime) {

    value += increment;

    return (int32)value;
}

///
bool WaveformClassIncrement::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassIncrement %s</title></head>\n<body>", Name());

    WaveformGenericClass::ProcessHttpMessage(hStream);

    hStream.Printf("<h1>WaveformClassIncrement %s</h1>", Name());

    hStream.Printf("<table>");
    hStream.Printf("<tr>");
    hStream.Printf("<th>Start value</th>");
    hStream.Printf("<th>Increment</th>");
    hStream.Printf("<th>Current value</th>");
    hStream.Printf("</tr>");
    hStream.Printf("<tr>");
    hStream.Printf("<td>%f</td>", startValue);
    hStream.Printf("<td>%f</td>", increment);
    hStream.Printf("<td>%f</td>", value);
    hStream.Printf("</tr>");
    hStream.Printf("</table>");

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassIncrement,"$Id$")
