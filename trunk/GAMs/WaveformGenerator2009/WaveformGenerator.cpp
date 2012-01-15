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

#include "WaveformGenerator.h"
#include "CDBExtended.h"
#include "GCNamedObject.h"
#include "WaveformGenericClass.h"

///
WaveformGenerator::WaveformGenerator(){
    ddbInputInterface  = NULL;
    ddbOutputInterface = NULL;
    waveformList       = NULL;
    isIntOutputList    = NULL;
}


///
WaveformGenerator::~WaveformGenerator(){
    if( waveformList    != NULL ) delete[] waveformList;
    if( isIntOutputList != NULL ) free((void*&)isIntOutputList);
}

///
bool WaveformGenerator::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);

    if( !AddInputInterface(ddbInputInterface,"InputInterface") ){
        AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize: Failed adding InputInterface");
        return False;
    }

    FString usecTime;
    if( !cdb.ReadFString(usecTime,"UsecTime") ){
        AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize error reading UsecTime");
        return False;
    }

    FString usecTimeType;
    if( !cdb.ReadFString(usecTimeType,"UsecTimeType", "int32") ){
        AssertErrorCondition(Warning,"WaveformGenerator::Initialize UsecTimeType not specified, assuming int32");
    }


    FString resetOnEventString;
    if(!cdb.ReadFString(resetOnEventString, "ResetOnEvent")) {
        resetEvent = EVENT_None;
    } else {
        if(resetOnEventString == "PrePulse") {
            resetEvent = GAMPrepulse;
        } else if(resetOnEventString == "PostPulse") {
            resetEvent = GAMPostpulse;
        } else {
            AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize error: Unkown ResetOnEvent value");
            return False;
        }
    }

    if( !ddbInputInterface->AddSignal(usecTime.Buffer(), usecTimeType.Buffer()) ){
        AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize error adding UsecTime to ddb interface");
        return False;
    }


    if( !AddOutputInterface(ddbOutputInterface,"OutputInterface") ){
        AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize: Failed adding output interface");
        return False;
    }

    if( Size() == 0 ){
        AssertErrorCondition(FatalError,"WaveformGenerator::Initialize no waveform have been initialised");
        return False;
    }

    waveformList = new GCRTemplate<WaveformInterface>[Size()];
    if( waveformList == NULL ){
        AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize: error allocating memory for the object list");
        return False;
    }

    isIntOutputList = (bool*)malloc(sizeof(bool)*Size());
    if( isIntOutputList == NULL ){
        AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize: error allocating memory for the object list");
        return False;
    }

    int i;
    // Initialise objects
    for( i = 0; i < Size(); i++ ){

        GCRTemplate<WaveformInterface> gcr = Find(i);
        if( gcr.IsValid() ){
            waveformList[i] = gcr;
        }else{
            AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize: object %i is not WaveformInterface type",i);
            return False;
        }

        FString waveformName;
        GCRTemplate<GCNamedObject> gcRN = Find(i);
        if( gcRN.IsValid() ){
            waveformName = gcRN->Name();
        }else{
            waveformName.Printf("WaveformOutput%i",i);
        }

        FString outputType = "float";
        isIntOutputList[i] = False;
        GCRTemplate<WaveformGenericClass> gcGC = Find(i);
        if( gcGC.IsValid() ){
            if( gcGC->IsInt() ){
//                outputType = "int";
                isIntOutputList[i] = True;
            }
            outputType = gcGC->GetSignalType();
        }

        if( !ddbOutputInterface->AddSignal(waveformName.Buffer(),outputType.Buffer()) ){
            AssertErrorCondition(InitialisationError,"WaveformGenerator::Initialize Cannot request read Signal %s from ddb",waveformName.Buffer());
            return False;
        }
    }

    // Reset all the waveforms
    for(i = 0 ; i < Size() ; i++) {
        waveformList[i]->Reset();
    }

    AssertErrorCondition(Information,"WaveformGenerator::Initialize initialised %i waveforms",Size());

    return True;
}


///
bool WaveformGenerator::Execute(GAM_FunctionNumbers functionNumber){

    int i;
    if(functionNumber == resetEvent) {
        for(i = 0 ; i < Size() ; i++) {
            waveformList[i]->Reset();
        }
    }
    
    ddbInputInterface->Read();
    int32 *inputData = (int32*)(ddbInputInterface->Buffer());
    int32 usecTime = inputData[0];
    
    float *output = (float *)ddbOutputInterface->Buffer();
    
    for(i = 0 ; i < Size() ; i++) {
        if(isIntOutputList[i]) {
            *((int32*)output) = (int32)(waveformList[i]->GetValueInt32(usecTime));
        } else {
            *output = waveformList[i]->GetValue(usecTime);
        }
        output++;
    }
    ddbOutputInterface->Write();
    
    return True;
}

OBJECTLOADREGISTER(WaveformGenerator,"$Id: WaveformGenerator.cpp,v 1.12 2011/08/10 16:09:13 ppcc_dev Exp $")
