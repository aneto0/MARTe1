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

#if !defined (SSMOPTIMISED_MATRIX_H)
#define SSMOPTIMISED_MATRIX_H

#include "System.h"
#include "Matrix.h"
#include "ConfigurationDataBase.h"

#define NOTINIT         -1
#define EMPTY            0
#define IDENTITY         1
#define DIAGONAL         2
#define WHOLE            3

class SSMOptimisedMatrix {

public:

    /** Matrix. */
    MatrixF     mat;

    /** Type of the matrix EMPTY, IDENTITY, DIAGONAL or WHOLE. */
    int         type;

    /** Real number of rows. */
    int         numberOfRows;

    /** Real number of columns. */
    int         numberOfColumns;

public:

    /** */
    SSMOptimisedMatrix();

    /** */
    ~SSMOptimisedMatrix();

    /** Initialised matrix by an input vector. */
    bool Init(int nRow, int nColumn, float *data);

    /** Initialised matrix by CDB. */
    bool Load(ConfigurationDataBase &cdb, char *entryName);

    /** Product output = Matrix x input. */
    inline bool Product(float *input, float *output){
        return Product_Private(input,output,False);
    }

    /** Product output = output + Matrix x input. */
    inline bool Product_Acc(float *input, float *output){
        return Product_Private(input,output,True);
    }

    /** Return the number of the rows. */
    inline int NRows(){ return numberOfRows; }

    /** Return the number of the columns. */
    inline int NColumns(){ return numberOfColumns; }

    /** Clean up the class parameters. */
    inline void CleanUp(){
        mat.ReSize(0,0);
        type = NOTINIT;
    }

private:

    /** Private product function. */
    bool Product_Private(float *input, float *output, bool accumulate);

    /** Analisys of the matrix type. */
    void AnalisedMatrix();
};

#endif
