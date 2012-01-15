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

#include "WaveformClassSequencer.h"
#include "Matrix.h"
#include "LoadCDBObjectClass.h"

///
bool WaveformClassSequencer::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface * err) {

    if( !GCReferenceContainer::ObjectLoadSetup(cdbData, err) ) {
        AssertErrorCondition(InitialisationError, "WaveformClassSequencer %s::ObjectLoadSetup: failed GCReferenceContainer constructor", Name());
        return False;
    }

    CDBExtended cdb(cdbData);

    numberOfWaveforms = Size();

    if( numberOfWaveforms <= 0 ) {
        AssertErrorCondition(InitialisationError, "WaveformClassSequencer %s::ObjectLoadSetup: no waveforms specified", Name());
        return False;
    }

    waveforms = new GCRTemplate<WaveformInterface>[numberOfWaveforms];
    if( waveforms == NULL ){
        AssertErrorCondition(InitialisationError,"WaveformClassSequencer %s::ObjectLoadSetup: failed to allocate the waveforms array", Name());
        return False;
    }

    // Link the waveforms
    int i;
    for( i = 0; i < numberOfWaveforms; i++ ){
        waveforms[i] = Find(i);
        if( !waveforms[i].IsValid() ){
            GCRTemplate<GCNamedObject> gcno = waveforms[i];
            if( gcno.IsValid() ){
                AssertErrorCondition(InitialisationError,"WaveformClassSequencer %s::ObjectLoadSetup: %s is not of WaveformInterface type", Name(), gcno->Name());
            }else{
                AssertErrorCondition(InitialisationError,"WaveformClassSequencer %s::ObjectLoadSetup: found a not WaveformInterface nor GCNamedObject object (%i)", Name(),i);
            }
            return False;
        }
    }

    FString error;
    MatrixF timeWindows;
    if( !LoadMatrixObject(cdb,"TimeWindows",timeWindows,error) ){
        AssertErrorCondition(InitialisationError,"WaveformClassSequencer::Initialize error loading Tstep");
        return False;
    }

    if( timeWindows.NColumns() != 2 ){
        AssertErrorCondition(InitialisationError,"WaveformClassSequencer::Initialize error loading TimeWindows");
        return False;
    }
    if( timeWindows.NRows() != numberOfWaveforms ){
        AssertErrorCondition(InitialisationError,"WaveformClassSequencer::Initialize number of waveforms different from number of time windows");
        return False;
    }

    // Microseconds time table
    timeWindowsUsecTime.ReSize(timeWindows.NRows(),timeWindows.NColumns());

    // Check time windows consistency
    float *pointer   = timeWindows.data;
    int *pointerUsec = timeWindowsUsecTime.data;

    for( i = 0; i < numberOfWaveforms; i++ ){
        if( pointer[2*i] < 0.0 || pointer[2*i+1] < 0.0 ){
            AssertErrorCondition(InitialisationError,"WaveformClassSequencer::Initialize error on time window (<0)");
            return False;
        }
        if( pointer[2*i] > pointer[2*i+1] ){
            AssertErrorCondition(InitialisationError,"WaveformClassSequencer::Initialize error on time window (%i)",i);
            return False;
        }
        pointerUsec[2*i]   = (int32) (pointer[2*i]   * 1E6);
        pointerUsec[2*i+1] = (int32) (pointer[2*i+1] * 1E6);
    }

    for( i = 0; i < numberOfWaveforms-1; i++ )
        if( pointer[2*i+1] > pointer[2*(i+1)] ){
            AssertErrorCondition(InitialisationError,"WaveformClassSequencer::Initialize sequence tiem windows incorrect");
            return False;
        }

#if !defined(_VX5100) // As a precaution because this is messing the PRFA controller online system
    GenerateDataPlot(timeWindows[0][0],timeWindows[timeWindows.NRows()-1][timeWindows.NColumns()-1]);
#endif

    return True;
}

///
float WaveformClassSequencer::GetValue(int32 usecTime) {

//     if( !WaveformGenericClass::Execute(usecTime) ) return 0.0;

//     if( usecTime < 0.0 ) return 0.0;

//     if( currentWaveformIndex == numberOfWaveforms ) return 0.0;

//     if( usecTime >= timeWindowsUsecTime[currentWaveformIndex][0] ){
//         if( usecTime < timeWindowsUsecTime[currentWaveformIndex][1] ){
//             return waveforms[currentWaveformIndex]->GetValue(usecTime) * gain + offsetValue;
//         }else{
//             currentWaveformIndex++;
//             if( currentWaveformIndex == numberOfWaveforms ) return 0.0;
//             if( usecTime >= timeWindowsUsecTime[currentWaveformIndex][0] ){
//                 if( usecTime < timeWindowsUsecTime[currentWaveformIndex][1] ){
//                     return waveforms[currentWaveformIndex]->GetValue(usecTime) * gain + offsetValue;
//                 }
//             }
//         }
//     }

    if( !WaveformGenericClass::Execute(usecTime) ) return 0.0;

    if( usecTime < 0.0 ) return 0.0;

    int32 currentWaveformIndex = 0;

    while(currentWaveformIndex < numberOfWaveforms) {
      if( usecTime >= timeWindowsUsecTime[currentWaveformIndex][0] ){
        if( usecTime < timeWindowsUsecTime[currentWaveformIndex][1] ){
	  return (waveforms[currentWaveformIndex]->GetValue(usecTime) * gain + offsetValue);
        } else {
	  currentWaveformIndex++;
	}
      }
    }
    
    return 0.0;
}

///
bool WaveformClassSequencer::ProcessHttpMessage(HttpStream &hStream) {

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassSequencer %s</title></head>\n", Name());

    hStream.Printf("<body onload=\"resizeFrame()\">\n");
    hStream.Printf("<h1>WaveformClassSequencer %s</h1>", Name());


    hStream.Printf("<table border=\"1\">");
    hStream.Printf("<tr><th>From</th><th>To</th><th>Waveform</th></tr>");
    int i;
    for( i = 0; i < numberOfWaveforms; i++ ){
        if( waveforms[i].IsValid() ){
            GCRTemplate<GCNamedObject> gcno = waveforms[i];
            if( gcno.IsValid() ){
                hStream.Printf("<tr><td>%f</td><td>%f</td><td><a href=\"%s\">%s</a></td></tr>",(float)timeWindowsUsecTime[i][0]/1000000,(float)timeWindowsUsecTime[i][1]/1000000,gcno->Name(),gcno->Name());
            }
        }
    }
    hStream.Printf("</table> ");


    httpJScope.ProcessGraphHttpMessageCreate(hStream);
    for( i = 0; i < numberOfWaveforms; i++ ){
        GCRTemplate<WaveformGenericClass> wgc = waveforms[i];
        httpJScope.ProcessGraphHttpMessageAddSignal(hStream,wgc->GetXData(),wgc->GetYData(),wgc->Name());
    }
    httpJScope.ProcessGraphHttpMessageShow(hStream);


    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}


OBJECTLOADREGISTER(WaveformClassSequencer,"$Id$")
