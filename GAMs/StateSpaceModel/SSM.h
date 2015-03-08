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

#if !defined (SSM_H)
#define SSM_H

#include "System.h"
#include "GAM.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "SSMGenericModel.h"


class DataStateSpaceStruct{

public:

    /** */
    int         nData;

    /** */
    float       *data;

    /** */
    DataStateSpaceStruct(){
        nData  = 0;
        data   = NULL;
    }

    /** */
    ~DataStateSpaceStruct(){
        CleanUp();
    }

    /** */
    inline void CleanUp(){
        if( data != NULL ) free((void*&)data);
        data  = NULL;
        nData = 0;
    }

    /** */
    inline int GetSize(){ return nData; }

    /** */
    inline bool Allocate(int size){
        if( size <= 0 ){
            CStaticAssertErrorCondition(InitialisationError,"DataStateSpaceStruct::Allocate data size < 0 (%i)",size);
            return False;
        }
        nData = size;
        data = (float*)malloc(sizeof(float)*size);
        if( data == NULL ){
            CStaticAssertErrorCondition(InitialisationError,"DataStateSpaceStruct::Allocate error allocating %i memory",size);
            return False;
        }
        return True;
    }
};



OBJECT_DLL(SSM)
class SSM: public GAM {
    OBJECT_DLL_STUFF(SSM)

private:

//-------------------------- DDB input/output interfaces ---------------------

    /**  DDB input signals (read only) */
    DDBInputInterface                       *inputSignalDDB;

    /**  DDB output signals (write only) */
    DDBOutputInterface                      *outputSignalDDB;

    /**  DDB input time */
    DDBInputInterface                       *timeInput;

//-------------------------- Models container ---------------------

    /** Models container */
    GCReferenceContainer                    modelsContainer;

    /** Pointer to the list of State Space Models. */
    GCRTemplate<SSMGenericModel>            *modelsList;

    /** Number of State Space Models. */
    int32                                   numberOfModels;

    /** Vector where store the input data. */
    DataStateSpaceStruct                    inputData;

    /** Vector where store the output data. */
    DataStateSpaceStruct                    outputData;

//-------------------------- Models sequences parameters ---------------------

    /** Counter model. */
    int                                     counterModel;

    /** Actual time. */
    float                                   actualTime;

    /** */
    bool                                    isUsecTime;

    /** */
    bool                                    patchModeEnabled;

public:

    /** */
    SSM();

    /** */
    ~SSM();

    /**  */
    bool Initialise(ConfigurationDataBase& cdbData);

    /**  */
    bool Execute(GAM_FunctionNumbers functionNumber);

    /**  */
    bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){return True;};

public:

    /** Clean up all the class parameters. */
    void CleanUp();
};
#endif
