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
#include "ExpEvalGAM.h"
#include "CDBExtended.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "LoadCDBObjectClass.h"

///
ExpEvalGAM::ExpEvalGAM() {
    input          = NULL;
    output         = NULL;
    executableList = NULL;
}

///
ExpEvalGAM::~ExpEvalGAM(){
    if(input) {
        free((void*&)input);
    }
    if(output) {
        free((void*&)output);
    }
    if(numberOfInputSignals) {
        free((void*&)numberOfInputSignals);
    }
    for(int i = 0 ; i < Size() ; i++) {
        if(tableIndexes[i]) {
            free((void*&)tableIndexes[i]);
        }
    }
    if(tableIndexes) {
        free((void*&)tableIndexes);
    }
    if(executableList){
        delete []executableList;
    }
//    printf("ExpEvalGAM: Destructor ended\n");
}

///
bool ExpEvalGAM::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);
        
    if(Size() == 0) {
        AssertErrorCondition(InitialisationError,"ExpEvalGAM::Initialise(): no objects found");
        return False;
    }

//    printf("Size() = %d\n", Size());//DEBUG

    numberOfInputSignals = (uint32 *)malloc(Size()*sizeof(uint32));
    memset(numberOfInputSignals, 0, Size()*sizeof(uint32));

    /// Add input interfaces
    if((input = (DDBInputInterface **)malloc(Size()*sizeof(DDBInputInterface *))) == NULL) {
	printf("ExpEvalGAM: Initialise(): unable to allocate memory for input DDBInterface array\n");
	return False;
    }

    /// Add output interfaces
    if((output = (DDBOutputInterface **)malloc(Size()*sizeof(DDBOutputInterface *))) == NULL) {
	printf("ExpEvalGAM: Initialise(): unable to allocate memory for output DDBInterface array\n");
	return False;
    }
    for(int i = 0 ; i < Size() ; i++) {
	FString outputInterfaceName;
	outputInterfaceName = "";
	outputInterfaceName.Printf("ExpEvalObject%d", i);
	outputInterfaceName += "OutputInterface";
	output[i] = NULL;
	if(!AddOutputInterface(output[i], outputInterfaceName.Buffer())) {
	    AssertErrorCondition(InitialisationError, "ExpEvalGAM::Initialise(): failed to add output interface");
	    return False;
	}
    }

    /// Build table indexes matrix
    if((tableIndexes = (int32 **)malloc(Size()*sizeof(int32 *))) == NULL) {
	printf("ExpEvalGAM: Initialise(): unble to allocate memory for tableIndexes\n");
	return False;
    }
    
    executableList = new GCRTemplate<ExpEval>[Size()];
    if(executableList == NULL){
        AssertErrorCondition(InitialisationError, "ExpEvalGAM::Initialise: %s failed to allocate the executableList array with %d elements", Name(), Size());
        return False;
    }
    for(int j = 0 ; j < Size() ; j++) {
        GCRTemplate<ExpEval> gcr = Find(j);
        if(!gcr.IsValid()) {
            AssertErrorCondition(InitialisationError,"ExpEvalGAM::Initialise(): object %i is not ExpEval type", j);
            return False;
        }
        FString name = "+";
        name += gcr->Name();
    //	printf("Name() = %s\n", name.Buffer());
        if(!cdb->Move(name.Buffer())) { /// move to ExpEval object
            printf("Cannot move to %s\n", name.Buffer());
            return False;
        }
        
        if(cdb->Move("InputSignals")) {
            numberOfInputSignals[j] = cdb->NumberOfChildren();
    //	    printf("numberOfInputSignals[%d] = %d\n", j, numberOfInputSignals[j]);//DEBUG
            if((tableIndexes[j] = (int32 *)malloc(numberOfInputSignals[j]*sizeof(int32))) == NULL) {
            printf("ExpEvalGAM: Initialise(): unable to allocate memory for tableIndexes internal members\n");
            return False;
            }
            FString inputInterfaceName;
            inputInterfaceName = "";
            inputInterfaceName.Printf("ExpEvalObject%d", j);
            inputInterfaceName += "InputInterface";
            input[j] = NULL;
            if(!AddInputInterface(input[j], inputInterfaceName.Buffer())) {
            AssertErrorCondition(InitialisationError, "ExpEvalGAM::Initialise(): failed to add input interface");
            return False;
            }
            for(int i = 0 ; i < numberOfInputSignals[j] ; i++) {
            cdb->MoveToChildren(i);
            FString signalName, signalType;
            cdb.ReadFString(signalName, "SignalName", "");
            cdb.ReadFString(signalType, "SignalType", "");
            input[j]->AddSignal(signalName.Buffer(), signalType.Buffer());
            BasicTypeData none(BTDFloat,1);
            float fNone = 0;
            none.UpdateData(&fNone);
            tableIndexes[j][i] = gcr->eedt.CreateAndAddDataTableEntry(signalName, none);
            cdb->MoveToFather();
            }
            cdb->MoveToFather();
        }
        if(!cdb->Move("OutputSignal")){
            AssertErrorCondition(InitialisationError,"ExpEvalGAM::Initialise: %s did not specify OutputSignal entry", Name());
            return False;
        }
        if(cdb->NumberOfChildren() != 1) {
            AssertErrorCondition(InitialisationError, "ExpEvalGAM::Initialise: only one output signal allowed per ExpEval object");
            return False;
        }
        cdb->MoveToChildren(0);
        FString signalName, signalType;
        cdb.ReadFString(signalName, "SignalName", "");
        cdb.ReadFString(signalType, "SignalType", "");
        if(!output[j]->AddSignal(signalName.Buffer(), signalType.Buffer())) {
            AssertErrorCondition(InitialisationError, "ExpEvalGAM:Initialise(): Failed to add output signal");
            printf("failed to add output signal\n");
            return False;
        }
        
        cdb->MoveToFather();
        cdb->MoveToFather();
        cdb->MoveToFather();

    //	gcr->eeep.ShowEquation();//DEBUG
        gcr->ParseEquation();
        executableList[j] = gcr;
    }

    data.SetTypeAndLength(BTDFloat, 1);
    outBtd.SetTypeAndLength(BTDFloat,1);


    return True;
}

///
bool ExpEvalGAM::Execute(GAM_FunctionNumbers execFlag) {
    for(int j = 0 ; j < Size() ; j++) {
        float *inputData;
        float *outputData;
        if(numberOfInputSignals[j] > 0) {
            inputData  = (float *)input[j]->Buffer();
            input[j]->Read();
        }
        outputData = (float *)output[j]->Buffer();

        for(int i = 0 ; i < numberOfInputSignals[j] ; i++) {

            data.UpdateData(&inputData[i]);
            executableList[j]->eedt.UpdateDataTableEntryValue(tableIndexes[j][i], data);
        }

        outBtd.UpdateData(executableList[j]->Execute());

        outBtd.GetData(BTDFloat, &outputData[0]);

        output[j]->Write();
    }

    return True;
}

OBJECTLOADREGISTER(ExpEvalGAM,"$Id: ExpEvalGAM.cpp,v 1.4 2011/07/08 13:50:57 aneto Exp $")
