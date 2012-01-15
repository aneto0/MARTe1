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

#include "WaveformGenericClass.h"
#include "GlobalObjectDataBase.h"


///
bool WaveformGenericClass::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    // A waveform could be a collection of many waveforms
    if( !GCReferenceContainer::ObjectLoadSetup(cdbData,err) ){
        AssertErrorCondition(InitialisationError, "WaveformGenericClass %s::ObjectLoadSetup: failed GCReferenceContainer constructor", Name());
        return False;
    }

    CDBExtended cdb(cdbData);

    // Common initialisation valid for each derived waveform

    offsetValue = 0.0;
    if( !IsWaveformType("Offset",cdb,offsetWaveform,offsetValue,variableOffset) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformGenericClass::Init error loading Offset");
        return False;
    }

    gain = 1.0;
    if( !IsWaveformType("Gain",cdb,gainWaveform,gain,variableGain) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformGenericClass::Init error loading Gain");
        return False;
    }

    if( !cdb.ReadFloat(tStart,"Tstart") ){
        CStaticAssertErrorCondition(Information,"WaveformGenericClass::Init Tstart not initialised. Default = 0");
        tStart = 0.0;
    }
    if( !cdb.ReadFloat(tEnd,"Tend") ){
        CStaticAssertErrorCondition(Information,"WaveformGenericClass::Init Tend not initialised. Default = 100");
        tEnd = 100.0;
    }

    triggerON = False;
    int i;

    for( i = 0; i < Size(); i++ ){
        triggerObj = Find(i);
        if( triggerObj.IsValid() ){
            triggerON = True;
            CStaticAssertErrorCondition(Information,"WaveformGenericClass::Init Trigger object found %s",triggerObj->Name());
            break;
        }
    }

    // Convert time to microseconds
    tStartUsec = (int32)(tStart*1000000);
    tStartUsecLocal = tStartUsec;
    tEndUsec   = (int32)(tEnd*1000000);

    // Type of output
    cdb.ReadFString(signalType, "SignalType", "float");
    if( strcmp(signalType.Buffer(),"float") == 0 ) isIntSignal = False;
    else                                           isIntSignal = True;

    return True;
}


///
bool WaveformGenericClass::Execute(int32 actTimeUsec){

    if( actTimeUsec < tStartUsec || actTimeUsec > tEndUsec ) return False;

    if( triggerON && !simMode ){
        if( !triggerObj->IsTriggered() ){
            triggerObj->Update(actTimeUsec);
            if( triggerObj->IsTriggered() ) tStartUsecLocal = actTimeUsec;
            else                            return False;
        }else{
            triggerObj->Update(actTimeUsec);
            if( !triggerObj->IsTriggered() ) return False;
        }
    }

    if( variableOffset ){
        offsetValue = offsetWaveform->GetValue(actTimeUsec);
    }

    if( variableGain ){
        gain = gainWaveform->GetValue(actTimeUsec);
    }

    return True;
}


///
bool WaveformGenericClass::IsWaveformType(FString entry, CDBExtended &cdb, GCRTemplate<WaveformInterface> &waveform, float &value, bool &isWaveformType){

    waveform = this->Find(entry.Buffer());
    if( waveform.IsValid() ){
        AssertErrorCondition(Information,"WaveformGenericClass %s::ObjectLoadSetup: is waveform type", Name());
        value = 0.0;
        isWaveformType = True;
    }else{
        if( cdb->Exists(entry.Buffer()) ){
            FString entryValue;
            if( !cdb.ReadFString(entryValue,entry.Buffer()) ){
                AssertErrorCondition(InitialisationError,"WaveformGenericClass %s::ObjectLoadSetup: error reading %s", Name(),entry.Buffer());
                return False;
            }
            if( IsNumber(entryValue.Buffer(),value) ){
                AssertErrorCondition(Information,"WaveformGenericClass %s::ObjectLoadSetup: is value", Name());
                isWaveformType = False;
            }else{
                // Is waveform name from a pool of waveforms
                GCReference wave = GetGlobalObjectDataBase()->Find(entryValue.Buffer(),GCFT_Recurse);
                if( !wave.IsValid() ){
                    AssertErrorCondition(InitialisationError, "WaveformGenericClass::ObjectLoadSetup: %s: Could not find module in Object Containers", Name());
                    return False;
                }
                waveform = wave;
                if( !waveform.IsValid() ){
                    AssertErrorCondition(InitialisationError, "WaveformGenericClass::ObjectLoadSetup: %s: is not WaveformInterface", Name());
                    return False;
                }
                isWaveformType = True;
            }
        }else{
            AssertErrorCondition(Information,"WaveformGenericClass %s::ObjectLoadSetup: entry %s not defined", Name(),entry.Buffer());
            isWaveformType = False;
        }
    }
    return True;
}


bool WaveformGenericClass::ProcessHttpMessage(HttpStream &hStream) {

    hStream.Printf("<br>");
    hStream.Printf("<table border=\"2\">");
    hStream.Printf("<tr><th>Parameters</th>");

    if( variableGain ){
        GCRTemplate<GCNamedObject> gcN = gainWaveform;
        if( gcN.IsValid() ){
            hStream.Printf("<tr><th>Gain</th><td><a href=\"%s\">%s</a></td></tr>",gcN->Name(),gcN->Name());
        }else{
            hStream.Printf("<tr><th>Gain</th><td>unknown</td></tr>");
        }
    }else{
        hStream.Printf("<tr><th>Gain</th><td>%e</td></tr>",gain);
    }

    if( variableOffset ){
        GCRTemplate<GCNamedObject> gcN = offsetWaveform;
        if( gcN.IsValid() ){
            hStream.Printf("<tr><th>Offset</th><td><a href=\"%s\">%s</a></td></tr>",gcN->Name(),gcN->Name());
        }else{
            hStream.Printf("<tr><th>Offset</th><td>unknown</td></tr>");
        }
    }else{
        hStream.Printf("<tr><th>Offset</th><td>%e</td></tr>",offsetValue);
    }
    hStream.Printf("</table> ");

    return True;
}
