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

#include "WaveformClassEventSequencer.h"

///
bool WaveformClassEventSequencer::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {

    if( !WaveformGenericClass::ObjectLoadSetup(cdbData, err) ) {
        AssertErrorCondition(InitialisationError, "WaveformClassEventSequencer %s::ObjectLoadSetup: failed WaveformGenericClass ObjectLoadSetup", Name());
        return False;
    }

    CDBExtended cdb(cdbData);

    numberOfWaveforms = Size();

    if( numberOfWaveforms <= 0 ) {
        AssertErrorCondition(InitialisationError, "WaveformClassEventSequencer %s::ObjectLoadSetup: no waveforms specified", Name());
        return False;
    }

    waveforms = new GCRTemplate<WaveformInterface>[numberOfWaveforms];
    if( waveforms == NULL ){
        AssertErrorCondition(InitialisationError,"WaveformClassEventSequencer %s::ObjectLoadSetup: failed to allocate the waveforms array", Name());
        return False;
    }

    // Link the waveforms
    int i;
    for( i = 0; i < numberOfWaveforms; i++ ){
        waveforms[i] = Find(i);
        if( !waveforms[i].IsValid() ){
            GCRTemplate<GCNamedObject> gcno = waveforms[i];
            if( gcno.IsValid() ){
                AssertErrorCondition(InitialisationError,"WaveformClassEventSequencer %s::ObjectLoadSetup: %s is not of WaveformInterface type", Name(), gcno->Name());
            }else{
                AssertErrorCondition(InitialisationError,"WaveformClassEventSequencer %s::ObjectLoadSetup: found a not WaveformInterface nor GCNamedObject object (%i)", Name(),i);
            }
            return False;
        }
    }

    int32 numberOfEvents = 1;
    FString error;
    if(!LoadVectorObject(cdb, "EventList", (void*&)events, numberOfEvents, CDBTYPE_FString, error)) {
        AssertErrorCondition(Information, "WaveformClassEventSequencer %s::Initialise: EventList not found", Name());
        return False;
    } else {
        if(numberOfEvents != numberOfWaveforms) {
            AssertErrorCondition(InitialisationError, "WaveformClassEventSequencer %s::Initialise: numberOfEvents != numberOfWaveforms", Name());
            return False;
        } else {
            if((eventCode = (uint32 *)malloc(numberOfEvents*sizeof(uint32))) == NULL) {
                  AssertErrorCondition(InitialisationError, "WaveformClassEventSequencer %s::Initialise: unable to allocate memory for eventCode", Name());
                  return False;
            }
            for(int32 i = 0 ; i < numberOfEvents ; i++) {
                if(events[i] == "PrePulse") {
                    eventCode[i] = (uint32)GAMPrepulse;
                } else if(events[i] == "PostPulse") {
                    eventCode[i] = (uint32)GAMPostpulse;
                } else if(events[i] == "Online") {
                    eventCode[i] = (uint32)GAMOnline;
                } else if(events[i] == "Offline") {
                    eventCode[i] = (uint32)GAMOffline;
                } else {
                    AssertErrorCondition(Information, "WaveformClassEventSequencer %s::Initialise: Invalid %s event found", Name(), events[i].Buffer());
                    return False;
                }
            }
        }
    }

    // Check that there are no repeated events in the EventList
    for(int32 i = 0 ; i < numberOfWaveforms ; i++) {
        for(int32 j = 0 ; j < numberOfWaveforms ; j++) {
            if((j != i) && (eventCode[i] == eventCode[j])) {
                AssertErrorCondition(Information, "WaveformClassEventSequencer %s::Initialise: Repeated %s in EventList", Name(), events[i].Buffer());
                return False;
            }
        }
    }

    return True;
}

///
float WaveformClassEventSequencer::GetValue(int32 usecTime) {

    for(int32 i = 0 ; i < numberOfWaveforms ; i++) {
        if(currentState == eventCode[i]) {
            return waveforms[i]->GetValue(usecTime) * gain + offsetValue;
        }
    }
    
    return 0.0;
}

///
int32 WaveformClassEventSequencer::GetValueInt32(int32 usecTime) {

    for(int32 i = 0 ; i < numberOfWaveforms ; i++) {
        if(currentState == eventCode[i]) {
            return (int32)waveforms[i]->GetValue(usecTime) * gain + offsetValue;
        }
    }
    
    return 0.0;
}

///
bool WaveformClassEventSequencer::ProcessHttpMessage(HttpStream &hStream) {

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassEventSequencer %s</title></head>\n", Name());

    hStream.Printf("<body onload=\"resizeFrame()\">\n");
    hStream.Printf("<h1>WaveformClassEventSequencer %s</h1>", Name());

    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}

OBJECTLOADREGISTER(WaveformClassEventSequencer,"$Id: WaveformClassEventSequencer.cpp,v 1.6 2012/02/28 17:06:22 dalves Exp $")
