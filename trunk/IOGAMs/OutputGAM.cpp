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


#include "OutputGAM.h"
#include "GlobalObjectDataBase.h"

#include "CDBExtended.h"
#include "HRT.h"
#include "DDBInputInterface.h"
#include "FastMath.h"

OutputGAM::~OutputGAM(){
    if(outputModule.IsValid())  outputModule->SetOutputBoardInUse(False);
    if(cal0             != NULL)free((void *&)cal0);
    if(cal1             != NULL)free((void *&)cal1);
    if(maxOutputValue   != NULL)free((void *&)maxOutputValue);
    if(minOutputValue   != NULL)free((void *&)minOutputValue);
    if(needsCalibration != NULL)free((void *&)needsCalibration);
}

bool OutputGAM::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);

    FString boardName;
    if(!cdb.ReadFString(boardName,"BoardName")){
        AssertErrorCondition(InitialisationError, "OutputGAM::Initialise: %s does not specify a BoardName", Name());
        return False;
    }

    GCReference module = GetGlobalObjectDataBase()->Find(boardName.Buffer(),GCFT_Recurse);
    if(!module.IsValid()){
        AssertErrorCondition(InitialisationError, "OutputGAM::Initialise: %s: Could not find module %s in Object Containers", Name(),boardName.Buffer());
        return False;
    }

    outputModule = module;
    if(!outputModule.IsValid()){
        AssertErrorCondition(InitialisationError, "OutputGAM::Initialise: %s: %s is not of GenericAcqModule Type", Name(),boardName.Buffer());
        return False;
    }

    //////////////////////////
    // Add Time Base Signal //
    //////////////////////////

    FString timeBase;
    if(!cdb.ReadFString(timeBase,"UsecTimeSignalName")){
        AssertErrorCondition(InitialisationError, "OutputGAM::Initialise: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }

    FString timeBaseType;
    if(!cdb.ReadFString(timeBaseType,"TimeSignalType","int32")){
        AssertErrorCondition(Warning, "OutputGAM::Initialise: %s does not specify a TimeSignalType> Assuming int32", Name());
    }

    writeInPrepulse = False;
    FString writeInPrepulseTxt;
    if(!cdb.ReadFString(writeInPrepulseTxt,"WriteToBoardInPrepulse","")){
        AssertErrorCondition(Warning, "OutputGAM::Initialise: %s does not specify a WriteToBoardInPrepulse > Assuming False", Name());
    }
    if(writeInPrepulseTxt == "true" || writeInPrepulseTxt == "True"){
        writeInPrepulse = True;
    }

    if(!AddInputInterface(usecTime,"UsecTimeInterface")){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s failed to add input interface UsecTimeInterface",Name());
        return False;
    }

    if(!usecTime->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s failed to add input Signal %s to interface InputInterface",Name(),timeBase.Buffer());
        return False;
    }

    FString enableSignalName;
    if(!cdb.ReadFString(enableSignalName,"EnableSignalName", "")){
        AssertErrorCondition(Information, "OutputGAM::Initialise: %s does not specify an EnableSignalName", Name());
    }

    FString enableSignalType;
    if(!cdb.ReadFString(enableSignalType,"EnableSignalType","int32")){
        AssertErrorCondition(Information, "OutputGAM::Initialise: %s does not specify a EnableSignalType. Assuming int32", Name());
    }
    
    if(enableSignalName.Size() > 0){
        if(!AddInputInterface(enableSignalInterface,"EnableSignalInterface")){
            AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s failed to add input interface EnableSignalInterface",Name());
            return False;
        }

        if(!enableSignalInterface->AddSignal(enableSignalName.Buffer(), enableSignalType.Buffer())){
            AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s failed to add input Signal %s to interface EnableSignalInterface",Name(),enableSignalName.Buffer());
            return False;
        }
    }

    //////////////////////////////////
    // Add Signal List to Interface //
    //////////////////////////////////

    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s failed to add output interface OutputInterface",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    // Get the word size of the packet and compare it with the module packet size
    dataWordSize = input->BufferWordSize();

    if(dataWordSize != outputModule->NumberOfOutputs()){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s specifies %d signal while the module %s only accepts %d ",Name(),dataWordSize,boardName.Buffer(),outputModule->NumberOfOutputs());
        return False;
    }

    //////////////////////////////////
    // Read the Calibration Factors //
    //////////////////////////////////

    if(cal0             != NULL)free((void *&)cal0);
    if(cal1             != NULL)free((void *&)cal1);
    if(needsCalibration != NULL)free((void *&)needsCalibration);
    if(maxOutputValue   != NULL)free((void *&)maxOutputValue);
    if(minOutputValue   != NULL)free((void *&)minOutputValue);

    cal0 = (float *)malloc(dataWordSize*sizeof(float));
    if(cal0 == NULL){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (cal0) of size %d ",Name(),dataWordSize*sizeof(float));
        return False;
    }

    cal1 = (float *)malloc(dataWordSize*sizeof(float));
    if(cal1 == NULL){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (cal1) of size %d ",Name(),dataWordSize*sizeof(float));
        return False;
    }

    maxOutputValue = (float *)malloc(dataWordSize*sizeof(float));
    if(maxOutputValue == NULL){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s : Failed to allocate space for Maximum Output Value of size %d ",Name(),dataWordSize*sizeof(float));
        return False;
    }

    minOutputValue = (float *)malloc(dataWordSize*sizeof(float));
    if(minOutputValue == NULL){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s : Failed to allocate space for Minimum Output Value of size %d ",Name(),dataWordSize*sizeof(float));
        return False;
    }

    needsCalibration = (bool *)malloc(dataWordSize*sizeof(bool));
    if(needsCalibration == NULL){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (needsCalibration) of size %d ",Name(),dataWordSize*sizeof(bool));
        return False;
    }

    cdb->Move("Signals");

    float *cal0Pointer             = cal0;
    float *cal1Pointer             = cal1;
    float *maxPointer              = maxOutputValue;
    float *minPointer              = minOutputValue;
    bool  *needsCalibrationPointer = needsCalibration;

    int32 nOfEntries                            =  input->NumberOfEntries();
    const DDBSignalDescriptor *signalDescriptor =  input->SignalsList();

    for(int entry = 0; entry < nOfEntries; entry++){
        cdb->MoveToChildren(entry);

        int32 signalSize    = signalDescriptor->SignalSize();

        ////////////////////////////////////////
        // Initialise the calibration vectors //
        ////////////////////////////////////////

        for(int j = 0; j < signalSize; j++){
            cal0Pointer[j]                 = 0.0;
            cal1Pointer[j]                 = 1.0;
            needsCalibrationPointer[j]     = False;
            minPointer[j]                  = -10.0;
            maxPointer[j]                  =  10.0;
        }

        if(cdb->Exists("Cal0")){

            int size[1]               = {signalSize};
            int maxDim                = 1;
            if(signalSize == 1)maxDim = 0;

            // Read Cal0 and Cal1
            if(!cdb.ReadFloatArray(cal0Pointer,size,maxDim,"Cal0")){
                AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s: Failed Reading Cal0 Array for signal %s",Name(),signalDescriptor->SignalName());
                return False;
            }


            if(!cdb.ReadFloatArray(cal1Pointer,size,maxDim,"Cal1")){
                AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s: Failed Reading Cal1 Array for signal %s",Name(),signalDescriptor->SignalName());
                return False;
            }

            for(int j = 0; j < signalSize; j++){
                needsCalibrationPointer[j] = True;
                // Store the 1/cal1 in the array to avoid doing division in real time
                if(cal1Pointer[j] != 0.0)cal1Pointer[j] = 1/cal1Pointer[j];
            }

            if(!cdb.ReadFloatArray(maxPointer,size,maxDim,"MaxOutputValue")){
                AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s: Failed Reading MaxOutputValue Array for signal %s",Name(),signalDescriptor->SignalName());
                return False;
            }


            if(!cdb.ReadFloatArray(minPointer,size,maxDim,"MinOutputValue")){
                AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: GAM %s: Failed Reading MinOutputValue Array for signal %s",Name(),signalDescriptor->SignalName());
                return False;
            }

        }

        cal0Pointer             += signalSize;
        cal1Pointer             += signalSize;
        needsCalibrationPointer += signalSize;
        maxPointer              += signalSize;
        minPointer              += signalSize;

        signalDescriptor = signalDescriptor->Next();
        cdb->MoveToFather();
    }

    cdb->MoveToFather();

    if(!outputModule->SetOutputBoardInUse()){
        AssertErrorCondition(InitialisationError,"OutputGAM::Initialise: %s: Module %s is already being used as output module by another GAM", Name(),outputModule->Name());
        return False;
    }


    return True;
}



bool OutputGAM::Execute(GAM_FunctionNumbers functionNumber){

    usecTime->Read();
    int32 time   = *((int32 *)usecTime->Buffer());

    if(functionNumber == GAMPrepulse){
        outputModule->PulseStart();
        if(!writeInPrepulse){
            return True;
        }
    }

    if(enableSignalInterface != NULL){
        enableSignalInterface->Read();
        int32 enable = *(int32 *)enableSignalInterface->Buffer();
        if(enable == 0){
            return True;
        }
    }


    ///////////////////////////
    // Implement Acquisition //
    ///////////////////////////

    input->Read();
    
    ////////////////////////
    // Apply Calibrations //
    ////////////////////////

    float *floatBuffer = (float *)input->Buffer();
    int   *intBuffer   = (int *)  input->Buffer();

    for(int sig = 0; sig < dataWordSize; sig++){
        if(needsCalibration[sig]){
            float output = floatBuffer[sig];
            if(output > maxOutputValue[sig]) output = maxOutputValue[sig];
            if(output < minOutputValue[sig]) output = minOutputValue[sig];
            float temp     = (output - cal0[sig])*cal1[sig];
            int   tempInt  = FastFloat2Int(temp);
            intBuffer[sig] = tempInt;
        }
    }
    if(!outputModule->WriteData(time,input->Buffer())){
        AssertErrorCondition(FatalError,"OutputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), outputModule->Name());
        return False;
    }

    return True;
}


bool OutputGAM::ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){

    // Dump Interface to CDB
    CDBExtended cdb(info);

    FString moduleName;
    moduleName = Name();

    cdb.WriteString(ClassName(),"Class");
    cdb.WriteString(outputModule->Name(),"BoardName");
    cdb.WriteString(usecTime->SignalsList()->SignalName(),"UsecTimeSignalName");

    // Save Interface
    input->ObjectSaveSetup(cdb,NULL);

    // Add Calibration Factors if needed
    cdb->Move(input->InterfaceName());

    int32 nOfEntries                            =  input->NumberOfEntries();
    const DDBSignalDescriptor *signalDescriptor =  input->SignalsList();

    float *tempCal1                = (float *)malloc(dataWordSize*sizeof(float));
    if(tempCal1 == NULL){
        AssertErrorCondition(FatalError,"OutputGAM::ObjectSaveSetup:: Module %s: Failed allocating memory of size %d words",Name(), dataWordSize);
        return False;
    }

    for(int i = 0; i < dataWordSize; i++){
        if(cal1[i] != 0.0)tempCal1[i] = 1.0/cal1[i];
        else              tempCal1[i] = 1.0;
    }

    float *cal0Pointer             = cal0;
    float *cal1Pointer             = tempCal1;
    bool  *needsCalibrationPointer = needsCalibration;

    for(int entry = 0; entry < nOfEntries; entry++){

        cdb->MoveToChildren(entry);

        int32 signalSize    = signalDescriptor->SignalSize();

        // Just check the first entry in the signal
        if(needsCalibrationPointer[0]){

            int size[1]               = {signalSize};
            int maxDim                = 1;
            if(signalSize == 1)maxDim = 0;

            cdb.WriteFloatArray(cal0Pointer,size,maxDim,"Cal0");
            cdb.WriteFloatArray(cal1Pointer,size,maxDim,"Cal1");
        }

        cal0Pointer             += signalSize;
        cal1Pointer             += signalSize;
        needsCalibrationPointer += signalSize;

        signalDescriptor = signalDescriptor->Next();
        cdb->MoveToFather();
    }


    cdb->MoveToFather();
    free((void *&)tempCal1);
    return True;
}


OBJECTLOADREGISTER(OutputGAM,"$Id$")
