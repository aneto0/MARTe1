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

/** 
 * @file
 * @brief Helper methods for the loading of matrices and vectors from a CDB
 */
#if !defined (LOADCDBOBJECTCLASS_H)
#define LOADCDBOBJECTCLASS_H

#include "System.h"
#include "Object.h"
#include "Matrix.h"
#include "ConfigurationDataBase.h"

extern "C" {

    /** */
    bool LoadDimension(ConfigurationDataBase &conf, const char *name, int &dimension, int *&sizeObject, Streamable &error);

    /** */
    bool LoadRTMatrixObject(ConfigurationDataBase &conf, const char *name, RTMatrixF &matrix, Streamable &error, int *sizeStrict = NULL);

    /** */
    bool LoadMatrixObject(ConfigurationDataBase &conf, const char *name, MatrixF &matrix, Streamable &error, int *sizeStrict = NULL);

    /** */
    bool LoadVectorObject(ConfigurationDataBase &conf, const char *name, void *&dataList, int &dimension, CDBTYPE CDBtype, Streamable &error, int sizeStrict = -1);

    /** */
    bool LoadSingleObject(ConfigurationDataBase &conf, const char *name, void *&dataList, CDBTYPE CDBtype, Streamable &error);

};

#endif


