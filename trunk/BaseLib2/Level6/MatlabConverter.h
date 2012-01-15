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
 * @brief Writing of matlab binary data
 */
#if !defined (MATLABCONVERTER_H)
#define MATLABCONVERTER_H

#include "System.h"
#include "Streamable.h"
#include "Matrix.h"

#define ASCII  0
#define BINARY 1

class MatlabConverter;
extern "C"{
    bool MCSaveF(MatlabConverter &mc, float value, const char* variableName);
    bool MCSaveD(MatlabConverter &mc, double value, const char* variableName);
    bool MCSaveMF(MatlabConverter &mc, MatrixF &mat, const char* variableName);
    bool MCSaveMD(MatlabConverter &mc, MatrixD &mat, const char* variableName);
    bool MCSaveMI(MatlabConverter &mc, MatrixI &mat, const char* variableName);
    bool MCSaveS(MatlabConverter &mc, const char** listOfStrings, int nStrings, const char* variableName);
}

class MatlabConverter{
    friend bool MCSaveF(MatlabConverter &mc, float value, const char* variableName);
    friend bool MCSaveD(MatlabConverter &mc, double value, const char* variableName);
    friend bool MCSaveMF(MatlabConverter &mc, MatrixF &mat, const char* variableName);
    friend bool MCSaveMD(MatlabConverter &mc, MatrixD &mat, const char* variableName);
    friend bool MCSaveMI(MatlabConverter &mc, MatrixI &mat, const char* variableName);
    friend bool MCSaveS(MatlabConverter &mc, const char** listOfStrings, int nStrings, const char* variableName);

private:

    /** */
    Streamable  *output;

    /** */
    char        headerTextField[116];



private:

    /** */
    void WriteHeader();

    /** */
    void WriteDataBlockType(int dataType, int dataDimension, int options);

    /** */
    void WriteDimensions(int *dime, int nDimensions);

    /** */
    void WriteVariableName(const char* varName);

    /** */
    void WriteChar(char data);

    /** */
    void WriteShort(short data);

    /** */
    void WriteSingle(int data);

    /** */
    void WriteDouble(double data);

    /** */
    void WriteString(const char *str, uint32 dim, bool padding = False);

    /** */
    void AddPadding(int padding);

    /** */
    int CalculateVariableNameSize(const char* nameVar, int bytesSize = 8);



public:

    /** */
    MatlabConverter(Streamable* out) {
        memset(headerTextField,512,sizeof(char)*116);
        output = out;
        WriteHeader();
    }

    /** */
    int CalculateDimension(MatrixF &mat, const char* variableName);

    /** */
    int CalculateDimension(MatrixD &mat, const char* variableName);

    /** */
    int CalculateDimension(MatrixI &mat, const char* variableName);

    /** */
    int CalculateDimension(const char** listOfStrings, int nStrings, const char* variableName);

    /** */
    int CalculateDimensionFloat(const char* variableName);

    /** */
    int CalculateDimensionDouble(const char* variableName);

    /** */
    bool Save(float value, const char* variableName){
        return MCSaveF(*this, value, variableName);
    }

    /** */
    bool Save(double value, const char* variableName){
        return MCSaveD(*this, value, variableName);
    }

    /** */
    bool Save(MatrixF &mat, const char* variableName){
        return MCSaveMF(*this, mat, variableName);
    }

    /** */
    bool Save(MatrixD &mat, const char* variableName){
        return MCSaveMD(*this, mat, variableName);
    }

    /** */
    bool Save(MatrixI &mat, const char* variableName){
        return MCSaveMI(*this, mat, variableName);
    }

    /** */
    bool Save(const char** listOfStrings, int nStrings, const char* variableName){
        return MCSaveS(*this, listOfStrings, nStrings, variableName);
    }

    /** */
    bool InitStructure(const char* nameStruct, const char** listField, int nFields, int totalDimension);

    /** */
    bool InitCell(const char* nameCell, int nFields, int totalDimension);

};

#endif

