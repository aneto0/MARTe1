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

#include "SawtoothWaveform.h"

bool SawtoothWaveform::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {
    if (!GCNamedObject::ObjectLoadSetup(cdbData, err)) {
        AssertErrorCondition(InitialisationError, "SawtoothWaveform %s::ObjectLoadSetup: failed GCNamedObject constructor", Name());
        return False;
    }

    CDBExtended cdb(cdbData);

    // Load parameters
    float tempPeriod;
    if (!cdb.ReadFloat(tempPeriod,"Period")) {
        AssertErrorCondition(InitialisationError,"SawtoothWaveform %s::ObjectLoadSetup: did not specify Period", Name());
        return False;
    }


    period = (int32)(tempPeriod* 1E6);

    if (!cdb.ReadFloat(amplitude,"Amplitude")) {
        AssertErrorCondition(InitialisationError,"SawtoothWaveform %s::ObjectLoadSetup: did not specify Amplitude", Name());
        return False;
    }

    if (!cdb.ReadInt32(rounding,"Rounding")) {
        AssertErrorCondition(InitialisationError,"SawtoothWaveform %s::ObjectLoadSetup: did not specify Rounding", Name());
        return False;
    }

    FString cdbWave;
    cdbWave.SetSize(0);

    cdbWave.Printf("Times = {0 %f %f}\n", tempPeriod/2, tempPeriod);
    cdbWave.Printf("Amplitudes = {%f %f %f}\n", -amplitude, amplitude, -amplitude);
    cdbWave.Printf("Rounding = %d\n", rounding);

    cdbWave.Seek(0);

    CDBExtended wv;
    wv->ReadFromStream(cdbWave,NULL,NULL);

    if (!waveform->ObjectLoadSetup(wv, err)) {
        AssertErrorCondition(InitialisationError,"SawtoothWaveform %s::ObjectLoadSetup: error initialising ditherWaveform", Name());
        return False;
    }

    return True;
}

bool SawtoothWaveform::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>SawtoothWaveform %s</title></head><body>\n", Name());

    hStream.Printf("<h1>SawtoothWaveform %s</h1>", Name());

    hStream.Printf("<p>Period: %.3e</p>\n", (float)period/1E6);
    hStream.Printf("<p>Amplitude: %.3e</p>\n", amplitude);

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}




OBJECTLOADREGISTER(SawtoothWaveform,"$Id: SawtoothWaveform.cpp,v 1.2 2009/10/16 13:48:59 rvitelli Exp $")
