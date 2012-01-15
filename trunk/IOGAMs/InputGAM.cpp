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
#include "InputGAM.h"
#include "GlobalObjectDataBase.h"

#include "CDBExtended.h"
#include "HRT.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "CDBBrowserMenu.h"

const int32 InputGAM::MAX32BitsValue = 2.147e9;

InputGAM::~InputGAM(){
    if(inputModule.IsValid())     inputModule->SetInputBoardInUse(False);
    if(cal0                != NULL)free((void *&)cal0);
    if(oldCal0             != NULL)free((void *&)oldCal0);
    if(cal1                != NULL)free((void *&)cal1);
    if(needsCalibration    != NULL)free((void *&)needsCalibration);
    if(noOffsetAdjustment  != NULL)free((void *&)noOffsetAdjustment);
    if(offsetCompensation  != NULL)free((void *&)offsetCompensation);
    if(physicalOffsetValue != NULL)free((void *&)physicalOffsetValue);    
}

bool InputGAM::ReadConfigurationFromCDB(CDBExtended &cdb){
    int32  temp                      = 0;    
    cdb.ReadInt32(temp, "ObjectSaveSetup", 0);
    bool   fromObjectSaveSetup       = (temp == 1);
 
    if(!cdb->Move("Signals")){
        return False;
    }
    float *cal0Pointer               = cal0;
    float *oldCal0Pointer            = oldCal0;
    float *cal1Pointer               = cal1;
    float *defaultOffPointer         = physicalOffsetValue;
    bool  *needsCalibrationPointer   = needsCalibration;
    int32 *noOffsetAdjustmentPointer = noOffsetAdjustment;

    int32 nOfEntries                            =  output->NumberOfEntries();
    const DDBSignalDescriptor *signalDescriptor =  output->SignalsList();

    
    for(int entry = 0; entry < nOfEntries; entry++){
        cdb->MoveToChildren(entry);

        int32 signalSize    = signalDescriptor->SignalSize();


        ////////////////////////////////////////
        // Initialise the calibration vectors //
        ////////////////////////////////////////

        for(int j = 0; j < signalSize; j++){
            cal0Pointer[j]                                  = 0.0;
            oldCal0Pointer[j]                               = 0.0;
            cal1Pointer[j]                                  = 1.0;
            needsCalibrationPointer[j]                      = False;
            noOffsetAdjustmentPointer[j]                    = 0;
        }

        if(cdb->Exists("Cal0")){
            
            int size[1]               = {signalSize};
            int maxDim                = 1;
            if(signalSize == 1)maxDim = 0;

            // Read Cal0 and Cal1
            if(!cdb.ReadFloatArray(cal0Pointer,size,maxDim,"Cal0")){
                AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s: Failed Reading Cal0 Array for signal %s",Name(),signalDescriptor->SignalName());
                return False;
            }
      
            if(!cdb.ReadFloatArray(oldCal0Pointer,size,maxDim,"OldCal0")){
                if(!cdb.ReadFloatArray(oldCal0Pointer,size,maxDim,"Cal0")){
                    AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s: Failed Reading OldCal0 Array for signal %s",Name(),signalDescriptor->SignalName());
                    return False;
                }
            }
           

            if(!cdb.ReadFloatArray(cal1Pointer,size,maxDim,"Cal1")){
                AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s: Failed Reading Cal1 Array for signal %s",Name(),signalDescriptor->SignalName());
                return False;
            }
            
            for(int j = 0; j < signalSize; j++){
                needsCalibrationPointer[j] = True;
            }

            if(automaticOffsetCompensation) cdb.ReadFloatArray(defaultOffPointer,size,maxDim,"PhysicalOffset");
          
            cdb.ReadInt32Array(noOffsetAdjustmentPointer, size, maxDim, "NoOffsetAdjustment");
        }

        cal0Pointer                                      += signalSize;
        oldCal0Pointer                                   += signalSize;
        cal1Pointer                                      += signalSize;
        needsCalibrationPointer                          += signalSize;
        noOffsetAdjustmentPointer                        += signalSize;
        if(automaticOffsetCompensation)defaultOffPointer += signalSize;
        
        signalDescriptor = signalDescriptor->Next();
        cdb->MoveToFather();
    }

    return cdb->MoveToFather();    
}

bool InputGAM::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);

    ConfigurationDataBase savedCDB;
    GetGAMPersistentCDB(savedCDB);
    CDBExtended cdbSave(savedCDB);

    FString boardName;
    if(!cdb.ReadFString(boardName,"BoardName")){
        AssertErrorCondition(InitialisationError, "InputGAM::Initialise: %s does not specify a BoardName", Name());
        return False;
    }

    GCReference module = GetGlobalObjectDataBase()->Find(boardName.Buffer(),GCFT_Recurse);
    if(!module.IsValid()){
        AssertErrorCondition(InitialisationError, "InputGAM::Initialise: %s: Could not find module %s in Object Containers", Name(),boardName.Buffer());
        return False;
    }

    inputModule = module;
    if(!inputModule.IsValid()){
        AssertErrorCondition(InitialisationError, "InputGAM::Initialise: %s: %s is not of GenericAcqModule Type", Name(),boardName.Buffer());
        return False;
    }
    
    if(!InitialiseTimeInformation(cdb)){
        AssertErrorCondition(InitialisationError, "InputGAM::Initialise: %s: Failed reading Time Informations ", Name());        
        return False;
    }
    
    //////////////////////////////////
    // Add Signal List to Interface //
    //////////////////////////////////
    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: %s failed to add output interface OutputInterface",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    // Get the word size of the packet and compare it with the module packet size
    numberOfInputs = output->BufferWordSize();

    if(numberOfInputs != inputModule->NumberOfInputs()){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s specifies %d signal while the module %s only accepts %d ",Name(),numberOfInputs,boardName.Buffer(),inputModule->NumberOfInputs());
        return False;
    }

    //////////////////////////////////
    // Read the Calibration Factors //
    //////////////////////////////////

    if(cal0               != NULL)free((void *&)cal0);
    if(oldCal0            != NULL)free((void *&)oldCal0);
    if(cal1               != NULL)free((void *&)cal1);
    if(needsCalibration   != NULL)free((void *&)needsCalibration);
    if(noOffsetAdjustment != NULL)free((void *&)noOffsetAdjustment);

    cal0 = (float *)malloc(numberOfInputs*sizeof(float));
    if(cal0 == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (cal0) of size %d ",Name(),numberOfInputs*sizeof(float));
        return False;
    }

    oldCal0 = (float *)malloc(numberOfInputs*sizeof(float));
    if(oldCal0 == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (oldCal0) of size %d ",Name(),numberOfInputs*sizeof(float));
        return False;
    }

    cal1 = (float *)malloc(numberOfInputs*sizeof(float));
    if(cal1 == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (cal1) of size %d ",Name(),numberOfInputs*sizeof(float));
        return False;
    }

    needsCalibration = (bool *)malloc(numberOfInputs*sizeof(bool));
    if(needsCalibration == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (needsCalibration) of size %d ",Name(),numberOfInputs*sizeof(bool));
        return False;
    }

    noOffsetAdjustment = (int32 *)malloc(numberOfInputs*sizeof(int32));
    if(noOffsetAdjustment == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for Calibration vector (noOffsetAdjustment) of size %d ",Name(),numberOfInputs*sizeof(int32));
        return False;
    }
   
    if(offsetCompensation != NULL)free((void *&)offsetCompensation);
    if(physicalOffsetValue != NULL)free((void *&)physicalOffsetValue);
    
    offsetCompensation = (float *)malloc(numberOfInputs*sizeof(float));
    if(offsetCompensation == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for automatic offset compensation (offsetCompensation) of size %d ",Name(),numberOfInputs*sizeof(float));
        return False;
    }
        
    physicalOffsetValue = (float *)malloc(numberOfInputs*sizeof(float));
    if(physicalOffsetValue == NULL){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s : Failed to allocate space for automatic offset compensation (physicalOffsetValue) of size %d ",Name(),numberOfInputs*sizeof(float));
        return False;
    }

    memset(physicalOffsetValue,0,numberOfInputs*sizeof(float));
    memset(offsetCompensation,0,numberOfInputs*sizeof(float));

    FString enableAutoCompensation;
    cdb.ReadFString(enableAutoCompensation,"EnableOffsetCompensation",    "No");
    if(enableAutoCompensation == "No")      automaticOffsetCompensation = False;
    else                                    automaticOffsetCompensation = True;
    
    if(automaticOffsetCompensation){
        if(!cdb.ReadFloat(percentageCorrection,"MaximumPercentageCorrection")){
            AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s :MaximumPercentageCorrection has not been specified ",Name());
            return False;
        }
        if(!cdb.ReadInt32(offsetCompensationNumberOfCycles ,"OffsetCompensationNumberOfCycles")){
            AssertErrorCondition(InitialisationError,"InputGAM::Initialise: GAM %s :OffsetCompensationNumberOfCycles has not been specified ",Name());
            return False;
        }
    }
    

    if(automaticOffsetCompensation){
        if(!ReadConfigurationFromCDB(cdbSave)){
            if(!ReadConfigurationFromCDB(cdb)){
                return False;
            }
        }
    }else{
        if(!ReadConfigurationFromCDB(cdb)){
            return False;
        }
        cdbSave->CleanUp();
        UpdateGAMPersistentCDB(cdbSave);
    }

    if(!inputModule->SetInputBoardInUse()){
        AssertErrorCondition(InitialisationError,"InputGAM::Initialise: %s: Module %s is already being used as input module by another GAM", Name(),inputModule->Name());
        return False;
    }

    performOffsetCheck   = False;
    calibrationRequested = False;
    startUpCycleNumber   = 0;
    
    AssertErrorCondition(Warning,"InputGAM::Initialise: %s: Module %s Has been Successfully Loaded", Name(),inputModule->Name());

    return True;
}


bool InputGAM::InitialiseTimeInformation(ConfigurationDataBase& cdbData){

    
    CDBExtended cdb(cdbData);
    
    //////////////////////////
    // Add Time Base Signal //
    //////////////////////////

    if(!AddIOInterface(usecTime,"UsecTimeInterface",DDB_ReadMode)){
        AssertErrorCondition(InitialisationError,"InputGAM::InitialiseTimeInformation: %s failed to add input interface IOInterface",Name());
        return False;
    }

    FString timeBase;
    if(!cdb.ReadFString(timeBase,"UsecTimeSignalName")){
        AssertErrorCondition(InitialisationError, "InputGAM::InitialiseTimeInformation: %s does not specify a UsecTimeSignalName", Name());
        return False;
    }

    FString timeBaseType;
    if(!cdb.ReadFString(timeBaseType,"TimeSignalType","int32")){
        AssertErrorCondition(Warning, "InputGAM::InitialiseTimeInformation: %s does not specify a TimeSignalType. Assuming int32", Name());
    }

    if(!usecTime->AddSignal(timeBase.Buffer(), timeBaseType.Buffer())){
        AssertErrorCondition(InitialisationError,"InputGAM::InitialiseTimeInformation: %s failed to add input Signal %s to interface InputInterface",Name(),timeBase.Buffer());
        return False;
    }
    
    return True;
}
 
int32 InputGAM::ReadTime(){
    usecTime->Read();
    int32 time   = *((int32 *)usecTime->Buffer());
    return time;
}


bool InputGAM::Execute(GAM_FunctionNumbers functionNumber){

    ///////////////////////////////////////////////////////////
    // If it is a time module do not read time from the DDB  //
    ///////////////////////////////////////////////////////////
    int32 time = ReadTime();

    currentExecutionState = functionNumber;

    ///////////////////////////
    // Implement Acquisition //
    ///////////////////////////
    float *floatBuffer = (float *)output->Buffer();
    int   *intBuffer   = (int *)  output->Buffer();

    if(calibrationRequested){
        functionNumber = GAMStartUp;
    }

    switch(functionNumber){
        case GAMStartUp:{
            if(inputModule->GetData(time,output->Buffer()) == -1){
                AssertErrorCondition(FatalError,"InputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), inputModule->Name());
                return False;
            }

            if(automaticOffsetCompensation == False) return True;

            startUpCycleNumber++;

            for(int sig = 0; sig < numberOfInputs; sig++){
                if(needsCalibration[sig]) {
                    floatBuffer[sig]         = (intBuffer[sig]*cal1[sig] + cal0[sig]);
                    offsetCompensation[sig] +=  floatBuffer[sig] - cal0[sig]; 
                }
            }
            performOffsetCheck = True;
            if(startUpCycleNumber == offsetCompensationNumberOfCycles){
                calibrationRequested = False;
            }
        }break;
        case GAMPrepulse:{
            // Reset the counter on the ATCA if in soft Trigger mode
            inputModule->PulseStart();
            if(inputModule->GetData(time, output->Buffer()) == -1){
                AssertErrorCondition(FatalError,"TimeInputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), inputModule->Name());
                return False;
            }
            for(int sig = 0; sig < numberOfInputs; sig++) 
                if(needsCalibration[sig]) {
                    floatBuffer[sig] = (intBuffer[sig]*cal1[sig] + cal0[sig]);
                }
        }break;    
        case GAMOffline:{
            if(performOffsetCheck){
                for(int sig = 0; sig < numberOfInputs; sig++){ 
                    if((needsCalibration[sig]) && (startUpCycleNumber != 0)){ 
                        if(noOffsetAdjustment[sig] == 1){
                            continue;
                        }
                        offsetCompensation[sig] = offsetCompensation[sig]/startUpCycleNumber;
                        cal0[sig] = physicalOffsetValue[sig] - offsetCompensation[sig];
                        float offsetPercent  = fabs((cal0[sig] - oldCal0[sig])/(cal1[sig]*MAX32BitsValue));
                        if(offsetPercent     > percentageCorrection){
                            AssertErrorCondition(Warning, "InputGAM::Execute:: Module %s: Offset NOT corrected %f >allowed %f for driver %s, signal %d, oldcal0 = %e, newcal0=%e",Name(), offsetPercent , percentageCorrection, inputModule->Name(), sig, oldCal0[sig], cal0[sig]);
                            cal0[sig] = oldCal0[sig];
                        }
                    }
                }
                startUpCycleNumber = 0;
                ConfigurationDataBase cdb;
                ObjectSaveSetup(cdb, NULL);
                UpdateGAMPersistentCDB(cdb);
                performOffsetCheck = False;
            }

            if(inputModule->GetData(time,output->Buffer()) == -1){
                AssertErrorCondition(FatalError,"InputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), inputModule->Name());
                return False;
            }

            for(int sig = 0; sig < numberOfInputs; sig++) {
                if(needsCalibration[sig]) {
                    floatBuffer[sig] = (intBuffer[sig]*cal1[sig] + cal0[sig]);
                }
            }
        }break;
        case GAMPostpulse:
        case GAMOnline:{
            if(inputModule->GetData(time,output->Buffer()) == -1){
                AssertErrorCondition(FatalError,"TimeInputGAM::Execute:: Module %s GetData Failed for driver %s",Name(), inputModule->Name());
                return False;
            }
            for(int sig = 0; sig < numberOfInputs; sig++) {
                if(needsCalibration[sig]) {
                    floatBuffer[sig] = (intBuffer[sig]*cal1[sig] + cal0[sig]);
                }
            }
        }break;
    };

    output->Write();
    return True;
}

bool InputGAM::ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){

    // Dump Interface to CDB
    CDBExtended cdb(info);

    FString moduleName;
    moduleName = Name();

    cdb.WriteString(ClassName(),"Class");
    cdb.WriteString(inputModule->Name(),"BoardName");
    if(usecTime != NULL) cdb.WriteString(usecTime->SignalsList()->SignalName(),"UsecTimeSignalName");

    // Save Interface
    // Add Calibration Factors if needed
    cdb->AddChildAndMove("Signals");

    int32 nOfEntries                            =  output->NumberOfEntries();
    const DDBSignalDescriptor *signalDescriptor =  output->SignalsList();

    float *cal0Pointer               = cal0;
    float *cal1Pointer               = cal1;
    float *oldCal0Pointer            = oldCal0;
    float *defaultOffPointer         = physicalOffsetValue;
    bool  *needsCalibrationPointer   = needsCalibration;
    int32 *noOffsetAdjustmentPointer = noOffsetAdjustment;

    for(int entry = 0; entry < nOfEntries; entry++){
        cdb->AddChildAndMove(signalDescriptor->SignalName());

        int32 signalSize    = signalDescriptor->SignalSize();

        cdb.WriteString(signalDescriptor->SignalName(), "SignalName");
        // Just check the first entry in the signal
        if(needsCalibrationPointer[0]){

            int size[1]               = {signalSize};
            int maxDim                = 1;
            if(signalSize == 1)maxDim = 0;

            cdb.WriteFloatArray(cal0Pointer,size,maxDim,"Cal0");
            cdb.WriteFloatArray(oldCal0Pointer,size,maxDim,"OldCal0");
            cdb.WriteFloatArray(cal1Pointer,size,maxDim,"Cal1");
            cdb.WriteFloatArray(defaultOffPointer,size,maxDim,"PhysicalOffset");
            cdb.WriteInt32Array(noOffsetAdjustmentPointer,size,maxDim,"NoOffsetAdjustment");
        }

        cal0Pointer               += signalSize;
        oldCal0Pointer            += signalSize;
        cal1Pointer               += signalSize;
        noOffsetAdjustmentPointer += signalSize;
        needsCalibrationPointer   += signalSize;
        defaultOffPointer         += signalSize;

        signalDescriptor = signalDescriptor->Next();
        cdb->MoveToFather();
    }


    cdb->MoveToFather();
    return True;
}

bool InputGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
   
    if (!envelope.IsValid()) {
        AssertErrorCondition(FatalError,"ProcessMessage: envelope is not valid!!");
        return False;
    }

    GCRTemplate<Message> gcrtm = envelope->GetMessage();

    if (!gcrtm.IsValid()) {
        AssertErrorCondition(FatalError,"ProcessMessage: (Message)gcrtm is not valid!!");
        return False;
    }

    const char *requestedAction = gcrtm->Content();
    AssertErrorCondition(Information, "ProcessMessage: processing message %s from [%s]",requestedAction, envelope->Sender());
    if(strcmp(requestedAction, "INITIALISE") == 0){
        if(currentExecutionState == GAMOnline){
            AssertErrorCondition(Warning, "ProcessMessage: Cannot calibrate while online %s from %s",requestedAction, envelope->Sender());
            return False;
        }
        else{
            if(automaticOffsetCompensation){
                calibrationRequested = True;
            }else{
                AssertErrorCondition(Warning, "Offset compensation is switched off. Ignoring message request from %s", envelope->Sender());
            }
        }
    }
    else{
        AssertErrorCondition(Warning, "ProcessMessage: Ignoring action requested %s from %s",requestedAction, envelope->Sender());
    }

    return True;
}

bool InputGAM::ProcessHttpMessage(HttpStream &hStream){
    hStream.Printf("<form><button type=\"submit\" name=\"Calibrate\" value=\"1\">Calibrate</button></form>\n");
    FString calibrationReq;
    calibrationReq.SetSize(0);
    if (hStream.Switch("InputCommands.Calibrate")){
        hStream.Seek(0);
        hStream.GetToken(calibrationReq, "");
        hStream.Switch((uint32)0);
    }

    FString showCalib;
    showCalib.SetSize(0);
    if (hStream.Switch("InputCommands.ShowCalib")){
        hStream.Seek(0);
        hStream.GetToken(showCalib, "");
        hStream.Switch((uint32)0);
    }

    if(showCalib == "1"){
        hStream.Printf("<form><button type=\"submit\" name=\"ShowCalib\" value=\"0\">Hide calibration</button></form>\n");    
    }
    else{
        hStream.Printf("<form><button type=\"submit\" name=\"ShowCalib\" value=\"1\">Show calibration</button></form>\n");
    }
    if(showCalib == "1"){
        hStream.Printf("<table border=\"1\">\n");
        hStream.Printf("<tr><th>Signal Name</th><th>Original cal0</th><th>Calibrated cal0</th><th>cal1</th><th>cal0 correction</th></tr>\n");

        int32 nOfEntries                            =  output->NumberOfEntries();
        const DDBSignalDescriptor *signalDescriptor =  output->SignalsList();

        for(int entry = 0; entry < nOfEntries; entry++){
            float offsetPercent  = fabs((cal0[entry] - oldCal0[entry])/(cal1[entry]*MAX32BitsValue));
            FString offsetColor = "#00FF00";    
            if(offsetPercent > percentageCorrection){
                offsetColor = "#FF0000";
            }
            
            hStream.Printf("<tr><td>%s</td><td>%e</td><td>%e</td><td>%e</td><td><font color=\"%s\">%e</font></td></tr>\n", signalDescriptor->SignalName(), oldCal0[entry], cal0[entry], cal1[entry], offsetColor.Buffer(), offsetPercent);
            signalDescriptor = signalDescriptor->Next();
        }
        hStream.Printf("</table>\n");
    }

    if(calibrationReq == "1"){
        if(currentExecutionState == GAMOnline){
            hStream.Printf("<font color=\"#FF0000\"><h2>Cannot calibrate while online</h2></font>");
        }
        else{
            if(automaticOffsetCompensation){
                calibrationRequested = True;
            }else{
                hStream.Printf("<font color=\"#FF0000\"><h2>Offset compensation is switched off</h2></font>");                
            }
        }
    }    

    return inputModule->ProcessHttpMessage(hStream);
}

OBJECTLOADREGISTER(InputGAM,"$Id$")
