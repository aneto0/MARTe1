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

#include "IntegralWaveform.h"

bool IntegralWaveform::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {
    bool res = True;

    if (!GCReferenceContainer::ObjectLoadSetup(cdbData, err)) {
        AssertErrorCondition(InitialisationError, "IntegralWaveform %s::ObjectLoadSetup: failed GCReferenceContainer constructor", Name());
        res = False;
    }
    CDBExtended cdb(cdbData);

    if (!cdb.ReadFloat(samplingTime,"SamplingTime")) {
        AssertErrorCondition(InitialisationError,"IntegralWaveform %s::ObjectLoadSetup: did not specify SamplingTime", Name());
        res = False;
    }

    int i;
    for (i=0; i<this->Size(); i++) {
        waveform = this->Find(i);
        if (waveform.IsValid()) break;
    }
    if (!waveform.IsValid()) {
        AssertErrorCondition(InitialisationError,"IntegralWaveform %s::ObjectLoadSetup: could not find a waveform to integrate", Name());
        res = False;
    } else {
        GCRTemplate<GCNamedObject> gcno = waveform;
        if (gcno.IsValid()) AssertErrorCondition(Information,"IntegralWaveform %s::ObjectLoadSetup: using waveform %s", Name(), gcno->Name());
        Reset();
    }


    return res;
}



bool IntegralWaveform::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>IntegralWaveform %s</title></head><body onload=\"resizeFrame()\">\n", Name());

    hStream.Printf("<script type=\"text/javascript\">\n function resizeFrame() {\n");
    hStream.Printf("f = document.getElementById('%s_WAVE')\n", this->Name());
    hStream.Printf("f.style.height = f.contentWindow.document.body.scrollHeight + \"px\"\n");
    hStream.Printf("}\n </script>\n");

    hStream.Printf("<h1>IntegralWaveform %s</h1>", Name());

    hStream.Printf("<p>SamplingTime: %f</p>\n", samplingTime);
    hStream.Printf("<p>Current integral value: %f</p>\n", integral);

    GCRTemplate<GCNamedObject> gcno = waveform;

    if ( (gcno.IsValid()) && (waveform.IsValid())) {
        hStream.Printf("<p>Integrating waveform %s</p>\n", gcno->Name());
        hStream.Printf("<iframe src =\"%s\" width=\"100%%\"  frameborder=\"0\" scrolling=\"no\" name=\"%s_WAVE\" id=\"%s_WAVE\"></iframe>\n", gcno->Name(), this->Name(), this->Name());
    }

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}





OBJECTLOADREGISTER(IntegralWaveform,"$Id: IntegralWaveform.cpp,v 1.3 2009/08/11 12:48:25 rvitelli Exp $")
