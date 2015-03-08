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
 * $Id: $
 *
**/

#include "SSM.h"
#include "LoadCDBObjectClass.h"

///
SSM::SSM(){
    numberOfModels  = 0;
    modelsList      = NULL;
    counterModel    = 0;
    actualTime      = 0;
    inputSignalDDB  = NULL;
    outputSignalDDB = NULL;
    timeInput       = NULL;
    isUsecTime      = False;
}


///
SSM::~SSM(){
    CleanUp();
}


///
void SSM::CleanUp(){
    if( modelsList != NULL ) delete[] modelsList;
    modelsList     = NULL;
    numberOfModels = 0;
    inputData.CleanUp();
    outputData.CleanUp();
    counterModel   = 0;
    actualTime     = 0;
    isUsecTime      = False;
}


///
bool SSM::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);

    CleanUp();

    // ************************************************************************
    // Read output signals patch mode
    // ************************************************************************
    if(!cdb->Exists("PatchOutputSignals"))
    {
        this->patchModeEnabled = True;
        AssertErrorCondition(Information, "SSM::Initialise Patch mode not specified, using patch mode");
    }
    else
    {
        FString patchMode;
        cdb.ReadFString(patchMode, "PatchOutputSignals");
        if((patchMode == "True") ||
           (patchMode == "Yes")  ||
           (patchMode == "T")    ||
           (patchMode == "Y"))
        {
            this->patchModeEnabled = True;
            AssertErrorCondition(Information, "SSM::Initialise Signal patch mode enabled");
        }
        else if((patchMode == "False") ||
                (patchMode == "No")    ||
                (patchMode == "F")     ||
                (patchMode == "N"))
        {
            this->patchModeEnabled = False;
            AssertErrorCondition(Information, "SSM::Initialise Signal patch mode disabled, using write mode");
        }
        else
        {
            this->patchModeEnabled = True;
            AssertErrorCondition(Information, "SSM::Initialise Invalid patch mode specified, using patch mode");
        }
    }

//---------------------- Load time signal ---------------------------------------------

    if( !AddInputInterface(timeInput,"InputInterface") ){
        AssertErrorCondition(InitialisationError,"SSM::Failed to add timeInput interface InputInterface");
        return False;
    }

    FString timeSignalName;
    if( cdb->Exists("SecTimeSignalName") ){
        if( !cdb.ReadFString(timeSignalName,"SecTimeSignalName") ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize error loading SecTimeSignalName");
            return False;
        }
        if( !timeInput->AddSignal(timeSignalName.Buffer(),"float") ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize Cannot request read Signal %s from ddb",timeSignalName.Buffer());
            return False;
        }
        isUsecTime = False;
    }else if( cdb->Exists("USecTimeSignalName") ){
        if( !cdb.ReadFString(timeSignalName,"USecTimeSignalName") ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize error loading USecTimeSignalName");
            return False;
        }
        if( !timeInput->AddSignal(timeSignalName.Buffer(),"int32") ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize Cannot request read Signal %s from ddb",timeSignalName.Buffer());
            return False;
        }
        isUsecTime = True;
    }

//---------------------- Init InputMap ---------------------------------------------

    inputData.nData = 0;

    if( cdb->Exists("InputMap") ){

        cdb->Move("InputMap");

        // Add input interface
        if( !AddInputInterface(inputSignalDDB,"InputInterface") ){
            AssertErrorCondition(InitialisationError,"SSM::Failed to add input interface InputInterface");
            return False;
        }

        if( !inputSignalDDB->ObjectLoadSetup(cdb,NULL) ){
            AssertErrorCondition(InitialisationError,"SSM::Init error ObjectLoadSetup InputMap");
            return False;
        }

        cdb->MoveToFather();

        //To support arrays as well
        int32 numberOfSignalsWithArrays = 0;
        const DDBSignalDescriptor *inSignalDescriptor = inputSignalDDB->SignalsList();
        while( inSignalDescriptor != NULL ){
            numberOfSignalsWithArrays += inSignalDescriptor->SignalSize();
            inSignalDescriptor = inSignalDescriptor->Next();
        }
        if( !inputData.Allocate(numberOfSignalsWithArrays) ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize error allocating %i memory input",inputData.nData);
            return False;
        }
    }


//---------------------- Init OutputMap ---------------------------------------------

    outputData.nData = 0;

    if( cdb->Exists("OutputMap") ){

 	cdb->Move("OutputMap");

        // Add output interface
        if(this->patchModeEnabled)
        {
            // Add output interface in patch mode
            if(!AddOutputInterface(outputSignalDDB,"OutputInterface", DDB_PatchMode))
            {
                AssertErrorCondition(InitialisationError,"SSM::Failed to add output interface OutputInterface");
                return False;
            }
        }
        else
        {
            // Add output interface in write mode
            if(!AddOutputInterface(outputSignalDDB,"OutputInterface", DDB_WriteMode))
            {
                AssertErrorCondition(InitialisationError,"SSM::Failed to add output interface OutputInterface");
                return False;
            }
        }

        if( !outputSignalDDB->ObjectLoadSetup(cdb,NULL) ){
            AssertErrorCondition(InitialisationError,"SSM::Init error ObjectLoadSetup OutputMap");
            return False;
        }

        cdb->MoveToFather();

        int32 numberOfSignalsWithArrays = 0;
        const DDBSignalDescriptor *outSignalDescriptor = outputSignalDDB->SignalsList();
        while( outSignalDescriptor != NULL ){
            numberOfSignalsWithArrays += outSignalDescriptor->SignalSize();
            outSignalDescriptor = outSignalDescriptor->Next();
        }
        if( !outputData.Allocate(numberOfSignalsWithArrays) ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize error allocating %i memory output",outputData.nData);
            return False;
        }
    }

//---------------------- Init Models ---------------------------------------------

    if( cdb->Exists("Models") ){

        cdb->Move("Models");

        if( !modelsContainer.ObjectLoadSetup(cdb,NULL) ){
            AssertErrorCondition(InitialisationError,"SSM::Initialise Failed ObjectLoadSetup");
            return False;
        }

        numberOfModels = modelsContainer.Size();

        modelsList = new GCRTemplate<SSMGenericModel>[numberOfModels];

        if( modelsList == NULL ){
            AssertErrorCondition(InitialisationError,"SSM::Initialize error allocating %i for the models",numberOfModels);
            return False;
        }

        int i;
        for( i = 0; i < numberOfModels; i++ ){
            cdb->MoveToChildren(i);
            GCRTemplate<SSMGenericModel> gcR = modelsContainer.Find(i);
            if( gcR.IsValid() ){
                modelsList[i] = modelsContainer.Find(i);
            }else{
                AssertErrorCondition(InitialisationError,"SSM::Initialize: error identifing object %i",i);
                return False;
            }

            if( !modelsList[i]->LoadModel(cdb,*this,inputData.GetSize(),outputData.GetSize()) ){
                AssertErrorCondition(InitialisationError,"SSM::Initialize error loading model %i",i);
                return False;
            }
            cdb->MoveToFather();
        }

        cdb->MoveToFather();
    }else{
        AssertErrorCondition(InitialisationError,"SSM::Initialize no models inserted in the cfg");
        return False;
    }

    return True;
}


///
bool SSM::Execute(GAM_FunctionNumbers functionNumber){

    switch( functionNumber ){

        case GAMPrepulse : {
            counterModel = 0;
        }break;

        case GAMOffline:
        case GAMOnline:{

            timeInput->Read();
            if( isUsecTime ) actualTime = (float)(*((int32*)timeInput->Buffer()))*1.0e-6;
            else             actualTime = *((float*)timeInput->Buffer());

            inputSignalDDB->Read();

            /// Pointer to the input data
            float *pointerInput = ((float*)inputSignalDDB->Buffer());
            int i;
            // Copy input to a local memory.
            float *pIn  = pointerInput;
            float *pCo = inputData.data;
            for( i = 0; i < inputData.GetSize(); i++ ){
                *pCo = *pIn;
                pIn++;
                pCo++;
            }

            /// Pointer to the output
            float *pointerOutput =  (float*)outputSignalDDB->Buffer();

            if( counterModel == 0 && actualTime >= modelsList[0]->TimeStart() ){
                /// Init model
                modelsList[0]->Execute(inputData.data,pointerOutput,INITEXECUTE);
                counterModel++;
            }

            if( counterModel != 0 ){

                /// Execute model
                modelsList[counterModel-1]->Execute(inputData.data,pointerOutput,EXECUTE);

                /// Save ouputs on DDB
                outputSignalDDB->Write();

                if( counterModel < numberOfModels ){
                    if( actualTime >= modelsList[counterModel]->TimeStart() ){

                        /// Init model
                        modelsList[counterModel]->Execute(inputData.data,pointerOutput,INITEXECUTE);

                        counterModel++;
                    }
                }
            }

        }break;

        case GAMSafety:{
            return True;
        }
    }

    return True;
}


OBJECTLOADREGISTER(SSM,"$Id")

