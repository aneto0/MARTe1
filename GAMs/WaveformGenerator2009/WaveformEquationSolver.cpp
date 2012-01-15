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


#include "WaveformEquationSolver.h"
#include "LoadCDBObjectClass.h"
#include "FindSignalDummy.h"

///
WaveformEquationSolver::WaveformEquationSolver(){
}


///
WaveformEquationSolver::~WaveformEquationSolver(){
}


///
bool WaveformEquationSolver::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformGenericClass::ObjectLoadSetup(cdbData,NULL) ){
        AssertErrorCondition(InitialisationError,"WaveformEquationSolver::Init error ObjectLoadSetup");
        return False;
    }

    CDBExtended cdb(cdbData);

    // Read equation
    FString equation;
    if( !cdb.ReadFString(equation,"Equation") ){
        AssertErrorCondition(InitialisationError,"WaveformEquationSolver::Init error loading equation");
        return False;
    }

    if( !expressionTree.CreateExpTreePostfix(equation,dataTable) ){
        AssertErrorCondition(InitialisationError,"WaveformEquationSolver::Initialize  error creating expression tree");
        return False;
    }


    FindSignalDummy findSignaldummy;

    if( !dataTable.Init(findSignaldummy) ){
        AssertErrorCondition(InitialisationError,"WaveformEquationSolver::Initialize  error initialising data table");
        return False;
    }

    if( !expressionTree.Init(dataTable) ){
        AssertErrorCondition(InitialisationError,"WaveformEquationSolver::Initialize  error creating expression tree");
        return False;
    }

    int outputDimension = expressionTree.Evaluate();

    if( outputDimension != 1){
        AssertErrorCondition(InitialisationError,"WaveformEquationSolver::Initialize  error checking expression tree");
        return False;
    }

    GenerateDataPlot(tStart,tEnd);

    return True;
}


///
float WaveformEquationSolver::GetValue(int32 usecTime){

    float time = ((float)usecTime)/1e6;

    float result;

    expressionTree.Evaluate(&time,&result,ONLINE);

    return result;
}


OBJECTLOADREGISTER(WaveformEquationSolver,"$Id$")


