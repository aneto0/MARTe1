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

#include "WaveformPeriodicClass.h"

/** */
bool WaveformPeriodicClass::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    // A waveform could be a collection of many waveforms
    if( !WaveformGenericClass::ObjectLoadSetup(cdbData,err) ){
        AssertErrorCondition(InitialisationError, "WaveformPeriodicClass %s::ObjectLoadSetup: failed WaveformGenericClass constructor", Name());
        return False;
    }

    CDBExtended cdb(cdbData);

    frequency = 0.0;
    if( !IsWaveformType("Frequency",cdb,frequencyWaveform,frequency,variableFrequency) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformPeriodicClass::Init error loading Frequency");
        return False;
    }

    FString freqType;
    cdb.ReadFString(freqType,"DiscretisedFrequencyWaveform","ON");

    discretiseFreq = True;
    if( strcmp(freqType.Buffer(),"OFF") == 0 ) discretiseFreq = False;


    // Phase is different between waveforms ([0,2pi] or [0 100])
    cdb.ReadFloat(phase,"Phase",0.0);

    // DutyCycle is used by not all the waveforms [0 100]
    cdb.ReadFloat(dutyCycle,"DutyCycle",50.0);

    Reset();

    return True;
}


///
bool WaveformPeriodicClass::Execute(int32 actTimeUsec){

    // Check if inside the time window
    if( !WaveformGenericClass::Execute(actTimeUsec) ){
        Reset();
        return False;
    }

    // Check if the function has been already called (multiple links avoided)
    if( oldUsecTime == actTimeUsec ) return True;

    // Recover the frequency in case of waveform frequency
    if( variableFrequency ){
        frequency = frequencyWaveform->GetValue(actTimeUsec);
    }

    {   // Calculate the integral of the phase for sweeping option

        if( oldUsecTime == -1 ){
            integratedFrequency  = frequency*(actTimeUsec-tStartUsecLocal)/100000.0-phase;
        }else{
            integratedFrequency += (frequency+oldFrequency)*(actTimeUsec-oldUsecTime)*0.0000005;
        }

        // Update data for sweeping option
        oldFrequency = frequency;
        oldUsecTime  = actTimeUsec;
    }


    // Absolute time normalised to the time window
    int32 localTime = actTimeUsec-tStartUsecLocal;


    if( frequency == 0 ){
        actFrequency = 0.0;
        actPeriod    = 0;
        startPeriod  = localTime;
        localPhase   = 0.0;
        return True;
    }

    if( actPeriod == -1 ){ // First setting
        actFrequency = frequency;
        actPeriod    = (int32)(1000000.0/frequency);
        startPeriod  = localTime-phase*actPeriod; // Add the phase on the first period
    }else{
        if( discretiseFreq ){
            if( (localTime-startPeriod) > actPeriod ){ // Update frequency
                startPeriod += actPeriod;
                if( frequency != actFrequency ){
                    actFrequency  = frequency;
                    actPeriod     = (int32)(1000000.0/frequency);
                }
            }
            localPhase = (float)(localTime-startPeriod)*actFrequency/1000000.0;
        }else{
            actFrequency = frequency;
            float dummy = (localTime*frequency/1000000)-phase;
            localPhase = dummy-floor(dummy);
        }
    }

    return True;
}


///
bool WaveformPeriodicClass::ProcessHttpMessage(HttpStream &hStream) {

    WaveformGenericClass::ProcessHttpMessage(hStream);

    if( variableFrequency || frequency != 0.0 ){
        hStream.Printf("<br>");
        hStream.Printf("<table border=\"1\">");
        hStream.Printf("<tr><th>Periodic parameters</th></tr>");
        if( variableFrequency ){
            GCRTemplate<GCNamedObject> gcN = frequencyWaveform;
            if( gcN.IsValid() ){
                hStream.Printf("<tr><th>Frequency</th><td><a href=\"%s\">%s</a></td></tr>",gcN->Name(),gcN->Name());
            }else{
                hStream.Printf("<tr><th>Frequency</th><td>unknown</td></tr>");
            }
        }else{
            hStream.Printf("<tr><th>Frequency</th><td>%e</td></tr>",frequency);
        }
        hStream.Printf("<tr><th>Phase</th><td>%e</td></tr>",phase);
        hStream.Printf("<tr><th>DutyCycle</th><td>%e</td></tr>",dutyCycle);
        hStream.Printf("</table> ");
    }

    return True;
}
