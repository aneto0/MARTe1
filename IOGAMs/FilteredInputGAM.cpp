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
#include "FilteredInputGAM.h"
#include "CDBExtended.h"
#include "DDBSignalDescriptor.h"
#include "DDBOutputInterface.h"

bool FilteredInputGAM::Initialise(ConfigurationDataBase& cdbData){
    
    CDBExtended cdb(cdbData);    
    
    if(!InputGAM::Initialise(cdb)){
        AssertErrorCondition(InitialisationError, "FilteredInputGAM::Initialise: %s: InputGAM::Initialize failed", Name());
        return False;
    }
    
    int size[2] = {0, 0};
    int dim     = 1;
    if(!cdb->GetArrayDims(size,dim,"FilterCoefficients")){
        AssertErrorCondition(InitialisationError, "FilteredInputGAM::Initialise: %s: Failed retrieving FilterCoefficients dimensions", Name());
        return False;
    }
    
    downSampling = size[0];
    if(downSampling > inputModule->NumberOfBuffers()){
        AssertErrorCondition(InitialisationError, "FilteredInputGAM::Initialise: %s: Filter dimensions [%d] is larger than the buffer size [%d] of the driver %s", Name(), downSampling ,inputModule->NumberOfBuffers() ,inputModule->Name());
        return False;
    }

    if(filterCoefficients != NULL) free((void *&)filterCoefficients);
    filterCoefficients = (float *)malloc(downSampling*sizeof(float)); 
    if(filterCoefficients == NULL){
        AssertErrorCondition(InitialisationError, "FilteredInputGAM::Initialise: %s: Failed allocating memory for Filter Coefficient [%d]", Name(),downSampling);
        return False;
    }
    
    if(!cdb.ReadFloatArray(filterCoefficients,size,dim,"FilterCoefficients")){
        AssertErrorCondition(InitialisationError, "FilteredInputGAM::Initialise: %s: Failed reading Filter Coefficients", Name());
        return False;
    }
    
    
    // Use the needCalibration vector to find out if the signal is an integer
    int32 nOfEntries                            =  output->NumberOfEntries();
    const DDBSignalDescriptor *signalDescriptor =  output->SignalsList();
    bool  *needsCalibrationPointer              = needsCalibration;

    for(int entry = 0; entry < nOfEntries; entry++){
        int32 signalSize    = signalDescriptor->SignalSize();

        ////////////////////////////////////////
        // Initialise the calibration vectors //
        ////////////////////////////////////////
        for(int j = 0; j < signalSize; j++){
            needsCalibrationPointer[j]                      = False;
        }

        if(signalDescriptor->SignalTypeCode() != BTDFloat ){
            for(int j = 0; j < signalSize; j++){
                needsCalibrationPointer[j] = True;
            }
        }

        needsCalibrationPointer                          += signalSize;
        
        signalDescriptor = signalDescriptor->Next();
    }
    
    
    if(buffer != NULL)free((void *&)buffer);
    buffer     = (float *)malloc(sizeof(float)*output->BufferWordSize());
    if(buffer == NULL){
        AssertErrorCondition(InitialisationError, "FilteredInputGAM::Initialise: %s: Failed allocating memory for buffer of size [%d]", Name(),sizeof(float)*output->BufferWordSize());
        return False;
    }
    
    return True;
}


bool FilteredInputGAM::Execute(GAM_FunctionNumbers functionNumber){
    ///////////////////////////////////////////////////////////
    // If it is a time module do not read time from the DDB  //
    ///////////////////////////////////////////////////////////
    int32 time = ReadTime();

    ///////////////////////////
    // Implement Acquisition //
    ///////////////////////////
    float *floatBuffer = (float *)output->Buffer();
    int   *intBuffer   = (int *)  output->Buffer();

    if(functionNumber == GAMPrepulse) {
            // Reset the counter on the ATCA if in soft Trigger mode
            inputModule->PulseStart();
    }
    // Reset the output
    int sig;
    for(sig = 0; sig < output->BufferWordSize(); sig++){
        floatBuffer[sig] = 0.0; 
    }
    
    // Copy the data in the buffer and peform filtering activities 
    for(int32 i = 0; i < downSampling; i++ ){
        if(inputModule->GetData(time,(int32 *)buffer,-i) == -1){
            AssertErrorCondition(FatalError,"InputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), inputModule->Name());
            return False;
        }
        
        for(sig = 0; sig < output->BufferWordSize(); sig++){
            floatBuffer[sig] += buffer[sig]*filterCoefficients[i]; 
        }
    }
    
    // Copy last value of integer entries in the DDB
    for(sig = 0; sig < output->BufferWordSize(); sig++){
        int32 *intB = (int32 *)buffer;
        if(inputModule->GetData(time,(int32 *)buffer,0) == -1){
            AssertErrorCondition(FatalError,"InputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), inputModule->Name());
            return False;
        }
        if(needsCalibration[sig])  intBuffer[sig] = intB[sig];
    }

    output->Write();
    return True;
}

OBJECTLOADREGISTER(FilteredInputGAM,"$Id: FilteredInputGAM.cpp,v 1.4 2011/02/10 14:06:00 dalves Exp $")
