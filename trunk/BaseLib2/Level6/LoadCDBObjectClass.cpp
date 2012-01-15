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

#include "LoadCDBObjectClass.h"



///
bool LoadDimension(ConfigurationDataBase &info, const char *name, int &dimension, int *&sizeObject, Streamable &error){

    CDBExtended cdb(info);

    if( !cdb->Exists(name) ){
        error.Printf("LoadDimension: %s object not found",name);
        return False;
    }

    dimension = 5;

    if( sizeObject == NULL ){
        sizeObject = (int*)malloc(sizeof(int)*5);
    }

    // Load dimension
    if( !cdb->GetArrayDims(sizeObject, dimension, name) ){
        error.Printf("LoadMatrixObect: error loading dimension of %s object",name);
        return False;
    }

    return True;
}

///
bool LoadRTMatrixObject(ConfigurationDataBase &info, const char *name, RTMatrixF &matrix, Streamable &error, int *sizeStrict){

    CDBExtended cdb(info);

    int maxDim = 5;
    int size[5];

    // Check if exist
    if( !cdb->Exists(name) ){
        error.Printf("LoadMatrixObect: %s not found",name);
//        matrix.ReSize(0,0);
        return True;
    }

    // Load dimension
    if( !cdb->GetArrayDims(size, maxDim, name) ){
        error.Printf("LoadMatrixObect: %s error loading dimension",name);
        return False;
    }

    if( maxDim > 2 ){
        error.Printf("LoadMatrixObect: dimension not correct (>2)");
        return False;
    }

    // Single vector that has to be as row vector
    int matR,matZ;
    if( size[1] == 0 ){
        matR = 1;
        matZ = size[0];
    } else {
        matR = size[0];
        matZ = size[1];
    }


    if( sizeStrict != NULL ){
        if( (sizeStrict[0] != -1 && sizeStrict[0] != matR) ||
            (sizeStrict[1] != -1 && sizeStrict[1] != matZ)  ){
            error.Printf("LoadMatrixObect: incompatibility with imposed dimensions");
            return False;
        }
    }


//    matrix.ReSize(matR,matZ);
    matrix.Allocate(matR,matZ);

    if( !cdb.ReadFloatArray(matrix.data,size,maxDim,name) ){
        error.Printf("LoadMatrixObect: error reading %s",name);
        return False;
    }

    return true;
}

///
bool LoadMatrixObject(ConfigurationDataBase &info, const char *name, MatrixF &matrix, Streamable &error, int *sizeStrict){
    return LoadRTMatrixObject(info,name,(RTMatrixF&)matrix,error,sizeStrict);
}


///
bool LoadVectorObject(ConfigurationDataBase &info, const char *name, void *&dataList, int &dimension, CDBTYPE CDBtype, Streamable &error, int sizeStrict){

    CDBExtended cdb(info);

    int maxDim = 5;
    int size[5];

    dimension = 0;
    if( dataList != NULL ){
        error.Printf("LoadVectorObject: data not empty");
        return False;
    }

    // Check if exist
    if( !cdb->Exists(name) && sizeStrict == -1 ){
        error.Printf("LoadVectorObject: %s not found",name);
        return True;
    }else if( !cdb->Exists(name) && sizeStrict == 1 ){
         error.Printf("LoadVectorObject: %s not found and doesn't match the input",name);
         return False;
    }

    // Load dimension
    if( !cdb->GetArrayDims(size, maxDim, name) ){
        error.Printf("LoadVectorObject: %s error loading dimension",name);
        return False;
    }

    if( maxDim > 1 ){
        error.Printf("LoadVectorObject: dimension not correct (>1)");
        return False;
    }

    if( sizeStrict != -1 && size[0] != sizeStrict ){
        error.Printf("LoadVectorObject: incompatibility with imposed diemension");
        return False;
    }

    if( size[0] == 0 ){
        return True;
    }

    dimension = size[0];

    if( CDBtype.dataType == CDBTYPE_FString.dataType ){
        dataList = (FString*)new FString[size[0]];
    }else{
        dataList = malloc( CDBtype.size * size[0] );
    }


    if( dataList == NULL ){
        error.Printf("LoadVectorObject: error allocationg memory");
        return False;
    }

    return cdb->ReadArray(dataList,CDBtype,size,maxDim,name);
}


///
bool LoadSingleObject(ConfigurationDataBase &info, const char *name, void*&dataList, CDBTYPE CDBtype, Streamable &error){

    CDBExtended cdb(info);

    int maxDim = 5;
    int size[5];

    // Check if exist
    if( !cdb->Exists(name) ){
        error.Printf("LoadSingleObject: %s not found",name);
        return False;
    }

    // Load dimension
    if( !cdb->GetArrayDims(size, maxDim, name) ){
        error.Printf("LoadSingleObject: %s error loading dimension",name);
        return False;
    }

    if( maxDim != 0 ){
        error.Printf("LoadSingleObject: dimension not correct (>1)");
        return False;
    }

    if( size[0] == 0 ){
        return True;
    }


    return cdb->ReadArray(&dataList,CDBtype,NULL,0,name);
}



