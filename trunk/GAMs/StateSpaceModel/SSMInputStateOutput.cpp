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

#include "SSMInputStateOutput.h"
#include "LoadCDBObjectClass.h"

///
SSMInputStateOutput::SSMInputStateOutput(){
    signalValues    = NULL;
    signalTypes     = NULL;
    numberOfSignals = 0;
    enable          = False;
    allValues       = True;
    ddbInputSignals = NULL;
}

///
SSMInputStateOutput::~SSMInputStateOutput(){
    CleanUp();
}

///
void SSMInputStateOutput::CleanUp(){
    if( signalValues != NULL ) free((void*&)signalValues);
    if( signalTypes  != NULL ) free((void*&)signalTypes);
    numberOfSignals = 0;
    enable          = False;
    allValues       = True;
}



///
bool SSMInputStateOutput::LoadStartPoint(ConfigurationDataBase &cdbData, GAM &gam, char *entry, int dimension){

    CDBExtended cdb(cdbData);

    CleanUp();

    if( !cdb->Exists(entry) ){
        enable = False;
        return true;
    }

    ddbInputSignals = new DDBInputInterface(gam.Name(),"InputStateOutput",DDB_ReadMode);

    if (ddbInputSignals == NULL) {
        CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint: error allocating memeory");
        return False;
    }


    FString error;

    signalValues = (float*)malloc(sizeof(float)*dimension);
    signalTypes  = (int*)  malloc(sizeof(int)  *dimension);

    FString *signalsName = NULL;
    int nSignals;

    if( !LoadVectorObject(cdb,entry,(void*&)signalsName,nSignals,CDBTYPE_FString,error) ){
        CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint error loading %s ",entry);
        if( signalsName != NULL ) delete[] signalsName;
        return False;
    }

    if( nSignals == 0 ){
        CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint number of signal == 0");
        if( signalsName != NULL ) delete[] signalsName;
        return False;
    }

    int counter = 0;
    int numberOfDDBSignals = 0;
    for( int jj = 0; jj < nSignals; jj++ ){

        if( counter >= dimension ){
            CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint error in the %s dimension",entry);
            return False;
        }

        if( IsNumber(signalsName[jj],signalValues[counter]) ){

            signalTypes[counter] = 0;
            counter++;

        }else{

            int dimSgn = ddbInputSignals->NumberOfEntries();

            if( !ddbInputSignals->AddSignal(signalsName[jj].Buffer(),"float") ){
                CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint Cannot request write Signal %s from ddb",signalsName[jj].Buffer());
                if( signalsName != NULL ) delete[] signalsName;
                return False;
            }

            dimSgn -= ddbInputSignals->NumberOfEntries(); // Is previous - last so it is negative in principle

            if( (counter-dimSgn) > dimension ){
                CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint error in the %s dimension",entry);
                if( signalsName != NULL ) delete[] signalsName;
                return False;
            }

            int i;
            for( i = 0; i < dimSgn; i++ ){
                signalValues[counter] = 0.0;
                signalTypes[counter]  = 1;
                counter++;
            }
            allValues = False;
            numberOfDDBSignals++;
        }
    }

    if( signalsName != NULL ) delete[] signalsName;

    if (numberOfDDBSignals == 0) {
        if (ddbInputSignals != NULL) {
            delete ddbInputSignals;
        }
    } else {
        if( ddbInputSignals != NULL ) {
            if(!gam.AddInputInterface(ddbInputSignals,NULL)){
                CStaticAssertErrorCondition(InitialisationError,"SSMInputStateOutput::LoadStartPoint: AddInputInterface Failed");
                return False;
            }
        }
    }

    numberOfSignals = dimension;

    enable = True;

    return True;

}

///
bool SSMInputStateOutput::Catch(){

    if( enable ){

        float *pointer = NULL;

        if( !allValues ){

            ddbInputSignals->Read();

            pointer = (float*)ddbInputSignals->Buffer();

            if( pointer == NULL ){
                CStaticAssertErrorCondition(FatalError,"SSMInputStateOutput::Catch run-time error pointer to NULL");
                return False;
            }
        }

        int counter = 0;
        int i;
        for( i = 0; i < numberOfSignals; i++ ){
            if( signalTypes[i] == 1 ){
                signalValues[i] = pointer[counter];
                counter++;
            }
        }
    }

    return True;
}


///
bool SSMInputStateOutput::Update(float *dataIO, int typeOp){

    if( enable ){

        if( dataIO == NULL ){
            CStaticAssertErrorCondition(FatalError,"SSMInputStateOutput::Update run-time error pointer to NULL");
            return False;
        }

        switch(typeOp){

            case ADD:{
                for( int i = 0; i < numberOfSignals; i++ ) dataIO[i] += signalValues[i];
            }break;

            case SUB:{
                for( int i = 0; i < numberOfSignals; i++ ) dataIO[i] -= signalValues[i];
            }break;

            case COPY:{
                for( int i = 0; i < numberOfSignals; i++ ) dataIO[i] = signalValues[i];
            }break;

            case MUL:{
                for( int i = 0; i < numberOfSignals; i++ ) dataIO[i] *= signalValues[i];
            }break;
        }
    }

    return True;
}


///
int SSMInputStateOutput::IsVector(FString &signalName){

    FString dummySignal;
    FString dummyDim;

    signalName.GetToken(dummySignal,"[");
    signalName.GetToken(dummyDim,"]");
    dummySignal.Seek(0);

    signalName = "";
    signalName.Printf("%s",dummySignal.Buffer());

    int dim = atoi(dummyDim.Buffer());

    if( dim == 0 ) return 1;

    return dim;
}


///
bool SSMInputStateOutput::IsNumber(FString entry, float &value){

    char * pEnd;
    value = (float) strtod((char*)entry.Buffer(),&pEnd);

    if( *pEnd == NULL ) return True;
    else return False;

}
