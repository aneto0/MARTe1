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

#if !defined (SSMGENERICMODEL_H)
#define SSMGENERICMODEL_H

#include "System.h"
#include "SSMOptimisedMatrix.h"
#include "SSMInputStateOutput.h"
#include "DDBOutputInterface.h"

#define  INITEXECUTE    1
#define  EXECUTE        2

OBJECT_DLL(SSMGenericModel)
class SSMGenericModel: public GCNamedObject{
OBJECT_DLL_STUFF(SSMGenericModel)

private:

    /** Optional state on DDB. */
    DDBOutputInterface          *stateDDB;

    /** Flag to save the state on DDB. */
    bool                        saveStateOnDDB;

    /** SSM name. */
    FString                     name;

    /**
        State space matrices defining the model

        x(t+1) = Fx(t) + Gu(t)
        y(t)   = Hx(t) + Du(t)

    */

    /** Matrix F. */
    SSMOptimisedMatrix          stateFromState;

    /** Matrix G. */
    SSMOptimisedMatrix          stateFromInput;

    /** Matrix H. */
    SSMOptimisedMatrix          outputFromState;

    /** Matrix D. */
    SSMOptimisedMatrix          outputFromInput;

    /** Initial input. */
    SSMInputStateOutput         initialInput;

    /** Initial output. */
    SSMInputStateOutput         initialOutput;

    /** Initial state. */
    SSMInputStateOutput         stateInitGuess;

    /** Offset state. */
    SSMInputStateOutput         offsetState;

    /** Vector to store the actual state. */
    float                       *state;

    /** Vector to store the old state. */
    float                       *oldState;

    /** Number of status. */
    int                         stateDim;

    /** Start time. */
    float                       tstart;

public:

    /** */
    SSMGenericModel();

    /** */
    ~SSMGenericModel();

    /** Load model from CDB. */
    bool LoadModel(ConfigurationDataBase &cdb, GAM &gam, int inputDim, int outputDim);

    /** Step execution. */
    bool Execute(float *input, float *output, int execFlag);

    /** Return start time. */
    float TimeStart(){ return tstart; }

private:
    /** */
    void CleanUp();
};
#endif
