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

#include "SineWaveform.h"
#include "FastMath.h"

bool SineWaveform::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {

    bool res = True;

    if (!GCReferenceContainer::ObjectLoadSetup(cdbData, err)) {
        AssertErrorCondition(InitialisationError, "SineWaveform %s::ObjectLoadSetup: failed GCReferenceContainer constructor", Name());
        res = False;
    }
    CDBExtended cdb(cdbData);

    int32 samplingUsecTime;
    if (!cdb.ReadInt32(samplingUsecTime,"SamplingUsecTime")) {
        AssertErrorCondition(InitialisationError,"SineWaveform %s::ObjectLoadSetup: did not specify SamplingTime", Name());
        res = False;
    }
    samplingTime = (float)(samplingUsecTime)/1000000.0;

    if (!cdb.ReadFloat(initialPhase,"InitialPhase")) {
        AssertErrorCondition(Warning,"SineWaveform %s::ObjectLoadSetup: did not specify InitialPhase, assuming 0", Name());
        initialPhase = 0.0;
    }
    phase = initialPhase;

    {
        frequencyWaveform = this->Find("FrequencyWaveform");
        if (frequencyWaveform.IsValid()) {
            AssertErrorCondition(Information,"SineWaveform %s::ObjectLoadSetup: using FrequencyWaveform", Name());
        } else {
            if (!cdb.ReadFloat(frequency,"Frequency")) {
                AssertErrorCondition(FatalError,"SineWaveform %s::ObjectLoadSetup: did not specify Frequency or FrequencyWaveform", Name());
                res = False;
            }
        }
    }

    {
        amplitudeWaveform = this->Find("AmplitudeWaveform");
        if (amplitudeWaveform.IsValid()) {
            AssertErrorCondition(Information,"SineWaveform %s::ObjectLoadSetup: using AmplitudeWaveform", Name());
        } else {
            if (!cdb.ReadFloat(amplitude,"Amplitude")) {
                AssertErrorCondition(FatalError,"SineWaveform %s::ObjectLoadSetup: did not specify Amplitude or AmplitudeWaveform", Name());
                res = False;
            }
        }
    }


    {
        biasWaveform = this->Find("BiasWaveform");
        if (amplitudeWaveform.IsValid()) {
            AssertErrorCondition(Information,"SineWaveform %s::ObjectLoadSetup: using BiasWaveform", Name());
        } else {
            if (!cdb.ReadFloat(bias,"Bias")) {
                AssertErrorCondition(Warning,"SineWaveform %s::ObjectLoadSetup: did not specify Bias or BiasWaveform, assuming 0.0", Name());
                bias = 0.0;
            }
        }
    }

    return res;
}


bool SineWaveform::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>SineWaveform %s</title></head><body onload=\"resizeFrame()\">\n", Name());

    hStream.Printf("<script type=\"text/javascript\">\n function resizeFrame() {\n");
    hStream.Printf("f = document.getElementById('%s_AMPL')\n", this->Name());
    hStream.Printf("f.style.height = f.contentWindow.document.body.scrollHeight + \"px\"\n");
    hStream.Printf("f = document.getElementById('%s_FREQ')\n", this->Name());
    hStream.Printf("f.style.height = f.contentWindow.document.body.scrollHeight + \"px\"\n");
    hStream.Printf("f = document.getElementById('%s_BIAS')\n", this->Name());
    hStream.Printf("f.style.height = f.contentWindow.document.body.scrollHeight + \"px\"\n");
    hStream.Printf("}\n </script>\n");


    hStream.Printf("<h1>SineWaveform %s</h1>", Name());

    if (amplitudeWaveform.IsValid()){
        hStream.Printf("<iframe src =\"AmplitudeWaveform/\" width=\"100%%\" frameborder=\"0\" scrolling=\"no\"  name=\"%s_AMPL\" id=\"%s_AMPL\"></iframe>\n", this->Name(), this->Name());
    } else {
        hStream.Printf("<p>Amplitude: %f</p>\n", amplitude);
    }

    if (frequencyWaveform.IsValid()){
        hStream.Printf("<iframe src=\"FrequencyWaveform/\" width=\"100%%\" frameborder=\"0\" scrolling=\"no\"  name=\"%s_FREQ\" id=\"%s_FREQ\"></iframe>\n", this->Name(), this->Name());
    } else {
        hStream.Printf("<p>Frequency: %f</p>\n", frequency);
    }

    if (biasWaveform.IsValid()){
        hStream.Printf("<iframe src =\"BiasWaveform/\" width=\"100%%\" frameborder=\"0\" scrolling=\"no\"  name=\"%s_BIAS\" id=\"%s_BIAS\"></iframe>\n", this->Name(), this->Name());
    } else {
        hStream.Printf("<p>Bias: %f</p>\n", bias);
    }

    hStream.Printf("<p>InitialPhase: %f</p>\n", initialPhase);

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

float SineWaveform::GetValue(int32 usecTime) {
    if (usecTime < 0) return 0;

    if (amplitudeWaveform.IsValid()) amplitude = amplitudeWaveform->GetValue(usecTime);
    if (biasWaveform.IsValid())      bias      = biasWaveform->GetValue(usecTime);

    return (amplitude * FastSin(Phase(usecTime)) + bias);
    //return (amplitude * sin(Phase(usecTime)) + bias);
}

float SineWaveform::Phase(int32 usecTime) {
    float time = usecTime / 1E6;

    if (frequencyWaveform.IsValid()) {
        frequency = frequencyWaveform->GetValue(usecTime);
	// Trapezoid rule
	phase    += PI*samplingTime*(frequency+previousFrequency);

	previousFrequency = frequency;
    } else {
        phase = 2.0*PI*frequency*time;
    }

    return phase;
}


OBJECTLOADREGISTER(SineWaveform,"$Id: SineWaveform.cpp,v 1.6 2010/02/26 14:18:08 dalves Exp $")
