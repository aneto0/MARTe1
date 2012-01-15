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

#include "SequencerWaveform.h"

bool SequencerWaveform::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {
    bool res = True;

    if (!GCReferenceContainer::ObjectLoadSetup(cdbData, err)) {
        AssertErrorCondition(InitialisationError, "SequencerWaveform %s::ObjectLoadSetup: failed GCReferenceContainer constructor", Name());
        res = False;
    }
    CDBExtended cdb(cdbData);

    numberOfWaveforms = this->Size();

    if (numberOfWaveforms <= 0) {
        AssertErrorCondition(InitialisationError, "SequencerWaveform %s::ObjectLoadSetup: no waveforms specified", Name());
        return False;
    }


    // Malloc arrays
    if (waveforms != NULL) {
        delete[] waveforms;
        waveforms = NULL;
    }

    if (waveformUsecStartTimes != NULL) {
        free((void*&)waveformUsecStartTimes);
        waveformUsecStartTimes = NULL;
    }

    waveforms = new GCRTemplate<WaveformInterface>[numberOfWaveforms];
    if (waveforms == NULL) {
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: failed to allocate the waveforms array", Name());
        return False;
    }

    // +1 cause we want to end a far away end time
    waveformUsecStartTimes = (int32*)malloc(numberOfWaveforms * sizeof(int32) +1);
    if (waveformUsecStartTimes == NULL) {
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: failed to allocate the waveformUsecStartTimes array", Name());
        delete [] waveforms;
        waveforms = NULL;
        return False;
    }

    float* waveformStartTimes = (float*)malloc(numberOfWaveforms * sizeof(float));
    if (waveformStartTimes == NULL) {
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: failed to allocate the waveformStartTimes array", Name());
        CleanUp();
        return False;
    }


    // Read start time arrays
    int timeSize[1] = {0};
    int maxTimeSize = 1;
    if(!cdb->GetArrayDims(timeSize,maxTimeSize,"StartTimes")){
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: failed to obtain StartTimes array", Name());
        CleanUp();
        free((void*&)waveformStartTimes);
        waveformStartTimes = NULL;
        return False;
    }
    if (timeSize[0] != numberOfWaveforms) {
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: StartTimes array size is different than the number of waveforms present", Name());
        CleanUp();
        free((void*&)waveformStartTimes);
        waveformStartTimes = NULL;
        return False;
    }
    if(!cdb.ReadFloatArray(waveformStartTimes,timeSize,maxTimeSize,"StartTimes") ){
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: failed to read StartTimes array", Name());
        CleanUp();
        free((void*&)waveformStartTimes);
        waveformStartTimes = NULL;
        return False;
    }


    // Convert in usecs the start timed and check order
    int i;
    int32 lastWin = 0;
    if (waveformStartTimes[0] != 0.0) {
        AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: StartTimes doesn't start with 0", Name());
        res = False;
    }
    for (i=1; i<numberOfWaveforms; i++) {
        waveformUsecStartTimes[i] = (int32) (waveformStartTimes[i] * 1E6);
        if (waveformUsecStartTimes[i] <= lastWin) {
            AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: wrong order of start times, %d <= %d", Name(), waveformUsecStartTimes[i], lastWin);
            res = False;
        }
        lastWin = waveformUsecStartTimes[i];
    }
    waveformUsecStartTimes[numberOfWaveforms] = 0x7FFFFFFF;

    // Link the waveforms
    for (i=0; i<numberOfWaveforms; i++) {
        waveforms[i] = this->Find(i);
        if (!waveforms[i].IsValid()) {
            res = False;
            GCRTemplate<GCNamedObject> gcno = waveforms[i];
            if (gcno.IsValid()) {
                AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: %s is not of WaveformInterface type", Name(), gcno->Name());
            } else {
                AssertErrorCondition(InitialisationError,"SequencerWaveform %s::ObjectLoadSetup: found a not WaveformInterface nor GCNamedObject object", Name());
            }
        }
    }


    if (waveformStartTimes != NULL) free((void*&)waveformStartTimes);

    return res;

}



bool SequencerWaveform::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>SequencerWaveform %s</title></head>\n", Name());

    hStream.Printf("<body onload=\"resizeFrame()\">\n");

    hStream.Printf("<script type=\"text/javascript\">\n function resizeFrame() {\n");

    int i;
    for (i=0; i<numberOfWaveforms; i++) {
        GCRTemplate<GCNamedObject> gcno = waveforms[i];
        if(gcno.IsValid()) {
            hStream.Printf("f = document.getElementById('%s')\n", gcno->Name());
            hStream.Printf("f.style.height = f.contentWindow.document.body.scrollHeight + \"px\"\n");
        }
    }

    hStream.Printf("}\n </script>\n");

    hStream.Printf("<h1>SequencerWaveform %s</h1>", Name());

    for (i=0; i<numberOfWaveforms-1; i++) {
        hStream.Printf("<h2>From %d to %d use:</h2>\n", waveformUsecStartTimes[i], waveformUsecStartTimes[i+1]);
        if (waveforms[i].IsValid()){
            GCRTemplate<GCNamedObject> gcno = waveforms[i];
            if (gcno.IsValid()) {
                hStream.Printf("<iframe src =\"%s/\" width=\"100%%\" frameborder=\"0\" scrolling=\"no\" name=\"%s\" id=\"%s\"></iframe>\n", gcno->Name(), gcno->Name(), gcno->Name());
            }
        }
    }
    i = numberOfWaveforms-1;
    hStream.Printf("<h2>From %d to end use:</h2>\n", waveformUsecStartTimes[i]);
    if (waveforms[i].IsValid()){
        GCRTemplate<GCNamedObject> gcno = waveforms[i];
        if (gcno.IsValid()) {
            hStream.Printf("<iframe src =\"%s/\" width=\"100%%\" frameborder=\"0\" scrolling=\"no\" name=\"%s\" id=\"%s\"></iframe>\n", gcno->Name(), gcno->Name(), gcno->Name());
        }
    }


    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}



float SequencerWaveform::GetValue(int32 usecTime) {
    if (usecTime < 0.0) return 0.0;

    while (waveformUsecStartTimes[currentWaveformIndex+1] < usecTime ) {
        currentWaveformIndex++;
        if (currentWaveformIndex >= numberOfWaveforms) currentWaveformIndex = numberOfWaveforms - 1;
    }

    return waveforms[currentWaveformIndex]->GetValue(usecTime);
}



OBJECTLOADREGISTER(SequencerWaveform,"$Id$")
