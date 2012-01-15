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

#include "ConstantWaveform.h"

bool ConstantWaveform::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {

    bool res = True;

    if (!GCNamedObject::ObjectLoadSetup(cdbData, err)) {
        AssertErrorCondition(InitialisationError, "ConstantWaveform %s::ObjectLoadSetup: failed GCReferenceContainer constructor", Name());
        res = False;
    }
    CDBExtended cdb(cdbData);

    if (!cdb.ReadFloat(value,"Value")) {
        AssertErrorCondition(InitialisationError,"ConstantWaveform %s::ObjectLoadSetup: did not specify Value", Name());
        res = False;
    }

    return res;
}



bool ConstantWaveform::ProcessHttpMessage(HttpStream &hStream) {

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>ConstantWaveform %s</title></head><body>\n", Name());

    hStream.Printf("<h1>ConstantWaveform %s</h1>", Name());

    hStream.Printf("<p>Value: %f</p>\n", value);
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}


OBJECTLOADREGISTER(ConstantWaveform,"$Id$")
