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

#include "SSMGenericModel.h"
#include "CDBExtended.h"
#include "LoadCDBObjectClass.h"

///
SSMGenericModel::SSMGenericModel(){
    state          = NULL;
    oldState       = NULL;
    stateDDB       = NULL;
    stateDim       = 0;
    saveStateOnDDB = False;
}

///
SSMGenericModel::~SSMGenericModel(){
    CleanUp();
}

///
void SSMGenericModel::CleanUp(){
    if( state    != NULL ) free((void*&)state);
    if( oldState != NULL ) free((void*&)oldState);
    state          = NULL;
    oldState       = NULL;
    stateDim       = 0;
    saveStateOnDDB = False;
    stateFromState.CleanUp();
    stateFromInput.CleanUp();
    outputFromState.CleanUp();
    outputFromInput.CleanUp();
    initialInput.CleanUp();
    initialOutput.CleanUp();
    stateInitGuess.CleanUp();
    offsetState.CleanUp();
    tstart = -1.0;
}

///
bool SSMGenericModel::LoadModel(ConfigurationDataBase &cdb, GAM &gam, int inputDim, int outputDim){


    CDBExtended cdblocal(cdb);

    CStaticAssertErrorCondition(Information,"SSMGenericModel::LoadModel input(%d) state(%d) ouptut(%d)",inputDim,stateDim,outputDim);

    CleanUp();

    if( !cdblocal.ReadFloat(tstart,"Tstart") ){
        CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error reading Tstart");
        return False;
    }

    /// Load StateMap
    stateDim = 0;

    if( cdblocal->Exists("StateMap") ){

        cdblocal->Move("StateMap");

        stateDDB = new DDBOutputInterface(gam.Name(),"StateMap",DDB_WriteMode);

        if( !gam.AddOutputInterface(stateDDB,"OutputState") ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error adding output state interface");
            return False;
        }

        if( !stateDDB->ObjectLoadSetup(cdblocal,NULL) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error ObjectLoadSetp StateMap");
            return False;
        }
        cdblocal->MoveToFather();

        stateDim = stateDDB->NumberOfEntries();

        saveStateOnDDB = True;
    }

    if( cdblocal->Exists("StateFromState") ){
        if( !stateFromState.Load(cdblocal,"StateFromState") ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading StateFromState");
            return False;
        }
        if( stateFromState.NRows() != stateFromState.NColumns() ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel StateFromState is not square");
            return False;
        }
        if( stateDim == 0 ) stateDim = stateFromState.NRows();
        else if( stateFromState.NRows() != stateDim ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel incompatibility in State-State dimensions");
            return False;
        }
    }

    if( cdblocal->Exists("StateFromInput") ){
        if( !stateFromInput.Load(cdblocal,"StateFromInput") ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading StateFromInput");
            return False;
        }
        if( stateDim == 0 ) stateDim = stateFromInput.NRows();
        if( (stateFromInput.NRows() != stateDim) || (stateFromInput.NColumns() != inputDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel incompatibility in State-Input dimensions");
            return False;
        }
    }

    if( cdblocal->Exists("OutputFromState") ){
        if( !outputFromState.Load(cdblocal,"OutputFromState") ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModelClass::LoadModel error loading OutputFromState");
            return False;
        }
        if( stateDim == 0 ) stateDim = outputFromState.NRows();
        if( (outputFromState.NRows() != outputDim) || (outputFromState.NColumns() != stateDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel incompatibility in Output-State dimensions");
            return False;
        }
    }

    if( cdblocal->Exists("OutputFromInput") ){
        if( !outputFromInput.Load(cdblocal,"OutputFromInput") ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading OutputFromInput");
            return False;
        }
        if( (outputFromInput.NRows() != outputDim) || (outputFromInput.NColumns() != inputDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel incompatibility in Output-Input dimensions");
            return False;
        }
    }


    if( cdblocal->Exists("InitialInput") ){
        if( !initialInput.LoadStartPoint(cdblocal,gam,"InitialInput",inputDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading InitialInput");
            return False;
        }
    }

    if( cdblocal->Exists("InitialOutput") ){
        if( !initialOutput.LoadStartPoint(cdblocal,gam,"InitialOutput",outputDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading InitialOutput");
            return False;
        }
    }

    if( cdblocal->Exists("StateInitGuess") ){
        if( !stateInitGuess.LoadStartPoint(cdblocal,gam,"StateInitGuess",stateDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading StateInitGuess");
            return False;
        }
    }

    if( cdblocal->Exists("OffsetState") ){
        if( !offsetState.LoadStartPoint(cdblocal,gam,"OffsetState",stateDim) ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error loading OffsetState");
            return False;
        }
    }

    if( stateDim != 0 ){
        if( !saveStateOnDDB ){
            state = (float*)malloc(sizeof(float)*stateDim);
            if( state == NULL ){
                CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error allocating %i memory state",stateDim);
                return False;
            }
        }

        oldState = (float*)malloc(sizeof(float)*stateDim);
        if( oldState == NULL ){
            CStaticAssertErrorCondition(InitialisationError,"SSMGenericModel::LoadModel error allocating %i memory old state",stateDim);
            return False;
        }
    }

    return True;
}


bool SSMGenericModel::Execute(float *input, float *output, int execFlag){
    if( execFlag == INITEXECUTE ){

        if( !initialInput.Catch()   ){
            CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute initialInput model %s",name.Buffer());
            return False;
        }
        if( !initialOutput.Catch()  ){
            CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute initialOutput model %s",name.Buffer());
            return False;
        }
        if( !stateInitGuess.Catch() ){
            CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute stateInitGuess model %s",name.Buffer());
            return False;
        }
        if( !offsetState.Catch()    ){
            CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute offsetState model %s",name.Buffer());
            return False;
        }

        for( int i = 0; i < stateDim; i++ ) oldState[i] = 0.0;
        if( !stateInitGuess.Update(oldState,COPY) ) return False;

    }else if( execFlag == EXECUTE ){

        // Different inputs (optional)
        if( !initialInput.Update(input,SUB)  )return False;

        if( saveStateOnDDB ) state = (float*)stateDDB->Buffer();

        /// Update status
        if( stateFromState.NRows() != 0 ){
            if( !stateFromState.Product(oldState,state) ){
                CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute run-time error State/State");
                return False;
            }
        }

        if( stateFromInput.NRows() != 0 ){
            if( !stateFromInput.Product_Acc(input,state) ){
                CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute run-time error State/Input");
                return False;
            }
        }

        // Offset on states (optional)
        if( !offsetState.Update(state,ADD) ) return False;

        /// Update outputs
        if( outputFromState.NRows() != 0 ){
            if( !outputFromState.Product(oldState,output) ){
                CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute run-time error Output/State");
                return False;
            }
        }

        if( outputFromInput.NRows() != 0 ){
            if( !outputFromInput.Product_Acc(input,output) ){
                CStaticAssertErrorCondition(FatalError,"SSMGenericModel::Execute run-time error Output/Input");
                return False;
            }
        }

        // Offset on outputs (optional)
        if( !initialOutput.Update(output,ADD) )return False;

        // Save new state
        for( int i = 0; i < stateDim; i++ ) oldState[i] = state[i];

    }

    // Save states on DDB (optional)
    if( saveStateOnDDB ) stateDDB->Write();

    return True;
}

OBJECTLOADREGISTER(SSMGenericModel,"$Id")
