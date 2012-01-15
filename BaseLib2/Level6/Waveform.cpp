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

#include "Waveform.h"


bool Waveform::ObjectLoadSetup(ConfigurationDataBase& cdbData, StreamInterface *err) {
    if (!GCNamedObject::ObjectLoadSetup(cdbData, err)) {
        AssertErrorCondition(InitialisationError, "Waveform %s::ObjectLoadSetup: failed GCNamedObject constructor", Name());
        return False;
    }

    if (waveform != NULL) {
        free((void *&)waveform);
        current = NULL;
    }

    CDBExtended cdb(cdbData);

    int timeSize[1] = {0};
    int amplSize[1] = {0};
    int maxTimeSize = 1;
    int maxAmplSize = 1;


    // Load rounding factor
    if (!cdb.ReadInt32(rounding,"Rounding")) {
        CStaticAssertErrorCondition(InitialisationError,"Wavform::Initialise: did not specify Rounding");
        return False;
    }

    // Check times and amplitude sizes
    if(!cdb->GetArrayDims(timeSize,maxTimeSize,"Times")){
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to obtain Times array");
        return False;
    }
    if(!cdb->GetArrayDims(amplSize,maxAmplSize,"Amplitudes")){
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to obtain Amplitudes array");
        return False;
    }

    if(timeSize[0] != amplSize[0]) {
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: Times and Amplitudes arrays have different length.");
        return False;
    }

    if (timeSize[0] < 2) {
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: Times and Amplitudes arrays must have at least 2 elements.");
        return False;
    }

    // Load in memory data
    waveform = (WaveformData*)malloc(sizeof(WaveformData)*(timeSize[0]+2));

    if (waveform == NULL) {
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to malloc array");
        return False;
    }

    float* times  = (float*)malloc(sizeof(float*)*(timeSize[0]));
    float* amplitudes = (float*)malloc(sizeof(float*)*(timeSize[0]));

    if ((times == NULL) || (amplitudes ==NULL )) {
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise: failed to malloc times or amplitudes array");
        if (times != NULL)  free((void*&)times);
        if (amplitudes != NULL) free((void*&)amplitudes);
        return False;
    }

    if(!cdb.ReadFloatArray(times,timeSize,maxTimeSize,"Times") ){
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise error reading Times");
        if (times != NULL)  free((void*&)times);
        if (amplitudes != NULL) free((void*&)amplitudes);
        return False;
    }
    if(!cdb.ReadFloatArray(amplitudes,amplSize,maxAmplSize,"Amplitudes") ){
        CStaticAssertErrorCondition(InitialisationError,"Waveform::Initialise error reading Amplitudes");
        if (times != NULL)  free((void*&)times);
        if (amplitudes != NULL) free((void*&)amplitudes);
        return False;
    }

    minimumValue = +1e16;
    maximumValue = -1e16;
    int i = 0;
    for (i=0; i < timeSize[0]; i++) {
        if(amplitudes[i] > maximumValue) maximumValue = amplitudes[i];
        if(amplitudes[i] < minimumValue) minimumValue = amplitudes[i];
    }

    // Calculate slopes offline
    for (i=0; i < timeSize[0]; i++) {
        waveform[i+1].amplitude = amplitudes[i];

        int32 temp = (int32)(times[i] * 1E6);
        temp += rounding/2;
        temp = temp/rounding;
        temp = temp*rounding;

        waveform[i+1].usecTime  = temp;
        times[i] = temp;
    }

    for (i=0; i < timeSize[0] - 1; i++) {
        float deltaT = times[i+1] - times[i];
        waveform[i+1].slope = (amplitudes[i+1] - amplitudes[i]) / deltaT;
    }

    current = 0;
    if (times != NULL)  free((void*&)times);
    if (amplitudes != NULL) free((void*&)amplitudes);

    waveform[0].usecTime              = 0;
    waveform[0].slope                 = 0;
    waveform[0].amplitude             = waveform[1].amplitude;

    // Slope at the last point MUST be initialised!!!!
    waveform[timeSize[0]].slope       = 0;

    waveform[timeSize[0]+1].usecTime  = 0x7FFFFFFF;
    waveform[timeSize[0]+1].slope     = 0;
    waveform[timeSize[0]+1].amplitude = waveform[timeSize[0]].amplitude;

    numberOfWindows = timeSize[0]+2;


    // Pre-generate the graph
    for (i=1; i < numberOfWindows-2; i++) {
        svgGraph.AddLine(((float)waveform[i].usecTime)/1E6, waveform[i].amplitude, ((float)waveform[i+1].usecTime)/1E6, waveform[i+1].amplitude,SVGLinePlain,SVGBlue,1);
    }
    svgGraph.gridDensity = 5.0;
    svgGraph.grid = True;

    return True;
}


float Waveform::GetValue(int32 usecTime) {
    while (waveform[current+1].usecTime < usecTime) {
        current++;
        if (current >= numberOfWindows) current = numberOfWindows - 1;
    }
    float deltaT = (float)(usecTime - waveform[current].usecTime);
    float value = waveform[current].amplitude + deltaT * waveform[current].slope;
    return value;
}


void Waveform::HTMLInfo(HttpStream &hStream){
    hStream.Printf("<table border=\"1\"><tr><th>Time</th><th>Amplitude</th></tr>\n");
    int i = 0;
    for (i=1; i<numberOfWindows-1; i++) {
        hStream.Printf("<tr>");
        hStream.Printf("<td>%.3e</td><td>%f</td></tr>\n", (float)waveform[i].usecTime/1E6, waveform[i].amplitude);
    }
    hStream.Printf("</table>");
    hStream.Printf("<br>\n");
}


bool Waveform::ProcessHttpMessage(HttpStream &hStream) {
    //hStream.keepAlive = False;
    //hStream.WriteReplyHeader(False);

    if (hStream.Switch("InputCommands.GRAPH")) { // Output image
        hStream.Switch((uint32)0);
        hStream.SSPrintf("OutputHttpOtions.Content-Type","image/svg+xml");
        svgGraph.Draw(hStream);
    } else { // Normal behaviour
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");

        hStream.Printf("<html><head><title>Waveform %s</title></head><body>\n", Name());

        hStream.Printf("<h1>Waveform %s</h1>", Name());

        HTMLInfo(hStream);

        hStream.Printf("<embed src=\"?GRAPH\" width=\"100%%\" height\"100%%\" type=\"image/svg+xml\">\n");
        hStream.Printf("<br>\n");

        hStream.Printf("</body></html>");
    }
    hStream.WriteReplyHeader(True);

    return True;
}


OBJECTLOADREGISTER(Waveform,"$Id: Waveform.cpp,v 1.4 2009/10/16 13:49:23 rvitelli Exp $")
