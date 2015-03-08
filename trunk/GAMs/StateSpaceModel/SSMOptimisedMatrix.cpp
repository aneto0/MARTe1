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

#include "SSMOptimisedMatrix.h"

///
SSMOptimisedMatrix::SSMOptimisedMatrix(){
    type = NOTINIT;
}

///
SSMOptimisedMatrix::~SSMOptimisedMatrix(){
}


///
bool SSMOptimisedMatrix::Init(int nRow, int nColumn, float *data){

    if( nRow == 0 || nColumn == 0 ){
        CStaticAssertErrorCondition(InitialisationError,"SSMOptimisedMatrix::Init error on the matrix dimensions [%d,%d]",nRow,nColumn);
        return False;
    }

    if( data == NULL ){
        CStaticAssertErrorCondition(InitialisationError,"SSMOptimisedMatrix::Init error on the data input pointer == NULL");
        return False;
    }

    mat.ReSize(nRow,nColumn);

    for( int i = 0; i < nRow*nColumn; i++ ) mat.data[i] = data[i];

    numberOfRows    = mat.NRows();
    numberOfColumns = mat.NColumns();

    AnalisedMatrix();

    return True;
}

///
bool SSMOptimisedMatrix::Load(ConfigurationDataBase &cdb, char *entryName){

    CDBExtended cdblocal(cdb);

    int dimensions[2] = {0,0};
    int maxDim        = 2;

    if( !cdblocal->GetArrayDims(dimensions,maxDim,entryName) ){
        CStaticAssertErrorCondition(InitialisationError,"SSMOptimisedMatrix::Load failed for entry %s",entryName);
        return False;
    }

    // Either a Matrix or a scalar
    if( maxDim > 2 ){
        CStaticAssertErrorCondition(InitialisationError,"SSMOptimisedMatrix::Load %s must be a matrix",entryName);
        return False;
    }

    if( maxDim == 0 ){
        mat.ReSize(1,1);
    }else if( maxDim == 1 ){
        mat.ReSize(1,dimensions[0]);
    }else{
        mat.ReSize(dimensions[0],dimensions[1]);
    }

    if( !cdblocal.ReadFloatArray(mat.Data(),dimensions,maxDim,entryName) ){
        CStaticAssertErrorCondition(InitialisationError,"SSMOptimisedMatrix::Load failed for entry %s",entryName);
        return False;
    }

    numberOfRows    = mat.NRows();
    numberOfColumns = mat.NColumns();

    AnalisedMatrix();

    return True;
}


///
bool SSMOptimisedMatrix::Product_Private(float *input, float *output, bool accumulate){

    switch( type ){

        case WHOLE:{
            if( accumulate ) RTMVProductAcc_U(mat,input,output);
            else             RTMVProduct_U(mat,input,output);
        }break;

        case NOTINIT:{
            CStaticAssertErrorCondition(InitialisationError,"SSMOptimisedMatrix::Product_Private matrix has been not initialised");
            return False;
        }break;

        case EMPTY:{
            if( !accumulate ) for( int i = 0; i < mat.NRows(); i++ ) output[i] = 0.0;
        }break;

        case IDENTITY:{
            if( accumulate ) for( int i = 0; i < mat.NColumns(); i++ ) output[i] += input[i];
            else             for( int i = 0; i < mat.NColumns(); i++ ) output[i]  = input[i];
        }break;

        case DIAGONAL:{
            if( accumulate ) for( int i = 0; i < mat.NColumns(); i++ ) output[i] += input[i]*mat.data[i];
            else             for( int i = 0; i < mat.NColumns(); i++ ) output[i]  = input[i]*mat.data[i];
        }break;

        default:{
            CStaticAssertErrorCondition(FatalError,"SSMOptimisedMatrix::Product_Private run-time error; type of matrix unknown");
            return False;
        }
    }

    return True;
}

///
void SSMOptimisedMatrix::AnalisedMatrix(){

    int i,j;

    bool isIdentity = True;
    bool isDiagonal = True;
    bool isNull     = True;

    if( mat.NRows() != mat.NColumns() ){
        type       = WHOLE;
        isIdentity = False;
        isDiagonal = False;
        for( i = 0; i < mat.NRows()*mat.NColumns(); i++ ){
            if( mat.data[i] != 0 ){
                isNull = False;
                break;
            }
        }
    }else{
        for( i = 0; i < mat.NRows(); i++ ){
            for( j = 0; j < mat.NColumns(); j++ ){
                if( mat[i][j] != 0 ) isNull = False;
                if( j != i ){
                    if( mat[i][j] != 0 ){
                        isIdentity = False;
                        isDiagonal = False;
                        break;
                    }
                }else{
                    if( mat[i][j] != 1 ){
                        isIdentity = False;
                    }
                }
            }
        }
    }


    if( (isIdentity || isDiagonal) && !isNull ){

    int dim = mat.NRows();

        MatrixF dummyMat(1,dim);
        for( i = 0; i < dim; i++ ) dummyMat.data[i] = mat[i][i];

        mat.ReSize(1,dim);
        for( i = 0; i < dim; i++ ) mat.data[i] = dummyMat.data[i];

        if( isIdentity ) type = IDENTITY;
        else             type = DIAGONAL;

    }else if( isNull ){

        // If null the matrix is not modify in order to store the dimensions
        // The Product function will do nothing
        type = EMPTY;

    }else type = WHOLE;

}
