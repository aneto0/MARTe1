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

#include "StatisticGAM.h"
#include "ConfigurationDataBase.h"
#include "FString.h"
#include "CDBExtended.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "DDBSignalDescriptor.h"


// Initialise the module
bool StatisticGAM::Initialise(ConfigurationDataBase& cdbData){

    CDBExtended cdb(cdbData);
    FString verb;

    cdb.ReadFString(verb,"Verbose","False");
    if(verb=="True") {
    	verbose = True;
	cdb.ReadInt32(frequencyOfVerbose,"FrequencyOfVerbose",1);
    }else{
    	verbose = False;
    }

    /////////////////
    // Add signals //
    /////////////////

    if(!AddInputInterface(inputData,"StatisticSignalList")){
        AssertErrorCondition(InitialisationError,"StatisticGAM::Initialise: %s failed to add input interface InputInterface StatisticSignalList",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"StatisticGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }
	
    int32 nOfSignals = cdb->NumberOfChildren();
	
    if(!inputData->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"StatisticGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),inputData->InterfaceName());
        return False;
    }

    cdb->MoveToFather();
    
    if(!AddOutputInterface(statistics,"Statistics")){
        AssertErrorCondition(InitialisationError,"StatisticGAM::Initialise: %s failed to add input interface OutputInterface Statistics",Name());
        return False;
    }

    const DDBSignalDescriptor *descriptor = inputData->SignalsList();
    uint32 signalCounter = 0;
    for(int i = 0; ((i < nOfSignals)&&(descriptor != NULL)); i++){
        FString signalName;
        signalName         = descriptor->SignalName();
        uint32 signalSize  = descriptor->SignalSize();
        // If it is an array!
        if(signalSize > 1){
            for(int j = 0; j < signalSize; j++){
                FString  newName;
                newName.Printf("%s[%d]",signalName.Buffer(),j);
                FString temp;
                temp.Printf("%sMean",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                temp.SetSize(0);
                temp.Printf("%sVariance",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                temp.SetSize(0);
                temp.Printf("%sMinimum",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                temp.SetSize(0);
                temp.Printf("%sMaximum",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                // Increment offset
                signalCounter++;
            }
        }else{
            FString  newName;
            newName.Printf("%s",signalName.Buffer());
            FString temp;
            temp.Printf("%sMean",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            temp.SetSize(0);
            temp.Printf("%sVariance",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            temp.SetSize(0);
            temp.Printf("%sMinimum",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            temp.SetSize(0);
            temp.Printf("%sMaximum",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            // Increment offset
            signalCounter++;
        }
        descriptor = descriptor->Next();
    }

    if(signalCounter == 0) {
        AssertErrorCondition(InitialisationError,"StatisticGAM::Initialise: %s: No Signal has been specified", Name());	
        return False;
    }

    numberOfSignals = signalCounter;
    statInfo = new StatSignalInfo[numberOfSignals];
    if(statInfo == NULL){
        AssertErrorCondition(InitialisationError,"StatisticGAM::Initialise: %s: Failed allocating memory for %d StatSignalInfo structures", Name(), signalCounter);	
        return False;
    }

    AssertErrorCondition(Information,"StatisticGAM::Initialise: %s: Correctly Initialized", Name());	
	
    return True;	
}

/** Execute the module functionalities */
bool StatisticGAM::Execute(GAM_FunctionNumbers functionNumber){

    inputData->Read();	

    switch(functionNumber){
    case GAMOffline:{
        static int offCounter = 0;
	if (((offCounter++ % frequencyOfVerbose) == 0) && (verbose == True)) {
		AssertErrorCondition(Warning, "StatisticGAM: sampleNo=%d", offCounter);
	}
    }break;
    case GAMOnline:{
        static int counter = 0;
	if (counter++ < 100) return True;
        const float *inputBuffer = (const float *)inputData->Buffer();
	if (((sampleNumber++ % frequencyOfVerbose) == 0) && (verbose == True)) {
		AssertErrorCondition(Warning, "StatisticGAM: sampleNo=%d signal=%f", sampleNumber,*inputBuffer);
	}
        for(int i = 0; i < numberOfSignals; i++){
            statInfo[i].Update(inputBuffer[i]);
        }
    }break;

    case GAMPrepulse:{
        sampleNumber = 0;
        for(int i = 0; i < numberOfSignals; i++) statInfo[i].Reset();
    }break;

    case GAMPostpulse:{
        const DDBSignalDescriptor *descriptor = inputData->SignalsList();
        int counter = 0;
        while(descriptor != NULL){
            FString signalName;
            signalName         = descriptor->SignalName();
            uint32 signalSize  = descriptor->SignalSize();
            if(signalSize > 1){
                for(int j=0; j < signalSize; j++){
                    FString output;
                    output.Printf("StatisticGAM::GAMPostpulse: Signal %s[%i]: Mean: %e Variance: %e Max: %e Min: %e",signalName.Buffer(),j,statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].Max(), statInfo[counter].Min());	
                    AssertErrorCondition(Information,"%s",output.Buffer());						
                    counter++;
                }
            }else{
                FString output;
                output.Printf("StatisticGAM::GAMPostpulse: Signal[%d] %s: Mean: %e Variance: %e Max: %e Min: %e",sampleNumber, signalName.Buffer(),statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].Max(), statInfo[counter].Min());	
                AssertErrorCondition(Information,"%s",output.Buffer());						
                counter++;
            }
            descriptor = descriptor->Next();
        }
     }break;
    };
	
    return True;
}




OBJECTLOADREGISTER(StatisticGAM,"$Id$")
