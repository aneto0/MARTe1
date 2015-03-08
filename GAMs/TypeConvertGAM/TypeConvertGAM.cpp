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
 * $Id: WebStatisticGAM.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
#include "TypeConvertGAM.h"
#include "CDBExtended.h"

/// This method runs at GAM initialisation
bool TypeConvertGAM::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    //Add interfaces to DDB
    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s failed to add input interface",Name());
        return False;
    }

    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s failed to add output interface",Name());
        return False;
    }
    //Add input signals 
    if(!cdb->Move("InputSignals")){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s did not specify InputSignals entry",Name());
        return False;
    }
    numberOfSignals = cdb->NumberOfChildren();
    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }

    FString *inputTypes = new FString[input->NumberOfEntries()];
    if(inputTypes == NULL){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup could not allocate inputTypes ",Name());
        return False;
    }

    const DDBSignalDescriptor *inSignalDescriptor = input->SignalsList();
    for(int entry = 0; entry < input->NumberOfEntries(); entry++){
        cdb->MoveToChildren(entry);
        FString signalType;
        if(!cdb.ReadFString(signalType, "SignalType")){
            AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup SignalType was not specified for signal %s", Name(), inSignalDescriptor->SignalName());
            delete[] inputTypes;
            return False;
        }
        inputTypes[entry] = signalType;
        inSignalDescriptor->Next();
        cdb->MoveToFather();
    }
    cdb->MoveToFather();
    //Add output signals
    if(!cdb->Move("OutputSignals")){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s did not specify OutputSignals entry",Name());
        delete[] inputTypes;
        return False;
    }
    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        delete[] inputTypes;
        return False;
    }
    if(cdb->NumberOfChildren() != numberOfSignals){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup the number of input signals [%d] is different from the number of output signals [%d]", Name(), numberOfSignals, cdb->NumberOfChildren());
        delete[] inputTypes;
        return False;
    }

    if(convertType != NULL) delete[] convertType;
    convertType = new ConvertType[numberOfSignals];
    if(convertType == NULL){
        AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup Failed to create convertType array ", Name());
        delete[] inputTypes;
        return False;
    }

    //Read all convert types
    const DDBSignalDescriptor *signalDescriptor = output->SignalsList();
    for(int entry = 0; entry < output->NumberOfEntries(); entry++){
        cdb->MoveToChildren(entry);
        FString signalType;
        if(!cdb.ReadFString(signalType, "SignalType")){
            AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup SignalType was not specified for signal %s", Name(), signalDescriptor->SignalName());
            delete[] inputTypes;
            return False;
        }

        convertType[entry] = TranslateConvertType(inputTypes[entry], signalType);
        if(convertType[entry] == Unknown){
            AssertErrorCondition(InitialisationError,"TypeConvertGAM::Initialise: %s ObjectLoadSetup Convert from %s to %s for signal %s is not supported", Name(), inputTypes[entry].Buffer(), signalType.Buffer(), signalDescriptor->SignalName());
            delete[] inputTypes;
            return False;
        }
        signalDescriptor->Next();
        cdb->MoveToFather();
    }

    cdb->MoveToFather();

    delete[] inputTypes;
    return True;
}

/// Called in every control loop
bool TypeConvertGAM::Execute(GAM_FunctionNumbers execFlag) {
    input->Read();
    char *inputBuffer  = (char *)input->Buffer();
    char *outputBuffer = (char *)output->Buffer();
    switch(execFlag) {
        case GAMPrepulse:
        break;

        case GAMOffline:
        case GAMOnline:
            int signal    = 0;
            int inPtrPos  = 0;
            int outPtrPos = 0;
            for(signal=0; signal<numberOfSignals; signal++){
                inputBuffer  += inPtrPos;
                outputBuffer += outPtrPos;
                switch(convertType[signal]){
                    case Int32ToInt32:
                        *(int32 *)outputBuffer = *(int32 *)inputBuffer;
                        inPtrPos  += sizeof(int32);
                        outPtrPos += sizeof(int32);
                        break;
                    case Int32ToUint32:
                        *(uint32 *)outputBuffer = *(int32 *)inputBuffer;
                        inPtrPos  += sizeof(int32);
                        outPtrPos += sizeof(uint32);
                        break;
                    case Int32ToFloat:
                        *(float *)outputBuffer = *(int32 *)inputBuffer;
                        inPtrPos  += sizeof(int32);
                        outPtrPos += sizeof(float);
                        break;
                    case FloatToInt32:
                        *(int32 *)outputBuffer = (int32)(*(float *)inputBuffer);
                        inPtrPos  += sizeof(float);
                        outPtrPos += sizeof(int32);
                        break;
                    case Int32ToInt64:
                        *(int64 *)outputBuffer = *(int32 *)inputBuffer;
                        inPtrPos  += sizeof(int32);
                        outPtrPos += sizeof(int64);
                        break;
                    case Int64ToInt32:
                        *(int32 *)outputBuffer = *(int64 *)inputBuffer;
                        inPtrPos  += sizeof(int64);
                        outPtrPos += sizeof(int32);
                        break;
                    case FloatToInt64:
                        *(int64 *)outputBuffer = *(float *)inputBuffer;
                        inPtrPos  += sizeof(float);
                        outPtrPos += sizeof(int64);
                        break;
                    case Int64ToFloat:
                        *(float *)outputBuffer = *(int64 *)inputBuffer;
                        inPtrPos  += sizeof(int64);
                        outPtrPos += sizeof(float);
                        break;
                    case Int32ToDouble:
                        *(double *)outputBuffer = *(int32 *)inputBuffer;
                        inPtrPos  += sizeof(int32);
                        outPtrPos += sizeof(double);
                        break;
                    case DoubleToInt32:
                        *(int32 *)outputBuffer = (int32)(*(double *)inputBuffer);
                        inPtrPos  += sizeof(double);
                        outPtrPos += sizeof(int32);
                        break;
                    case Int64ToDouble:
                        *(double *)outputBuffer = *(int64 *)inputBuffer;
                        inPtrPos  += sizeof(int64);
                        outPtrPos += sizeof(double);
                        break;
                    case DoubleToInt64:
                        *(int64 *)outputBuffer = (int64)(*(double *)inputBuffer);
                        inPtrPos  += sizeof(double);
                        outPtrPos += sizeof(int64);
                        break;
                    case DoubleToFloat:
                        *(float *)outputBuffer = *(double *)inputBuffer;
                        inPtrPos  += sizeof(double);
                        outPtrPos += sizeof(float);
                        break;
                    case FloatToDouble:
                        *(double *)outputBuffer = *(float *)inputBuffer;
                        inPtrPos  += sizeof(float);
                        outPtrPos += sizeof(double);
                        break;
                }
            }
            output->Write();
	    break;
    }

    return True;
}
OBJECTLOADREGISTER(TypeConvertGAM, "$Id: TypeConvertGAM.cpp,v 1.2 2011/03/11 10:16:01 ppcc_dev Exp $")
