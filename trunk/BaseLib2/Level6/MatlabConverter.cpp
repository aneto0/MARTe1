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

#include "MatlabConverter.h"

// Char dimension (1 byte)
#define     ZERO            0
#define     FDEFAULT        0
#define     FLOGICAL        2
#define     FGLOBAL         4
#define     FCOMPLEX        8

#define     mxCELL_CLASS    1
#define     mxSTRUCT_CLASS  2
#define     mxOBJECT_CLASS  3
#define     mxCHAR_CLASS    4
#define     mxSPARSE_CLASS  5
#define     mxDOUBLE_CLASS  6
#define     mxSINGLE_CLASS  7
#define     mxINT8_CLASS    8
#define     mxUINT8_CLASS   9
#define     mxINT16_CLASS   10
#define     mxUINT16_CLASS  11
#define     mxINT32_CLASS   12
#define     mxUINT32_CLASS  13

// Short dimension (2 bytes)
#define     miINT8          1
#define     miUINT8         2
#define     miINT16         3
#define     miUINT16        4
#define     miINT32         5
#define     miUINT32        6
#define     miSINGLE        7
#define     RESERVED0       8
#define     miDOUBLE        9
#define     RESERVED1      10
#define     RESERVED2      11
#define     miINT64        12
#define     miUINT64       13
#define     miMATRIX       14
#define     miCOMPRESSED   15
#define     miUTF8         16
#define     miUTF16        17
#define     miUTF32        18


#define     SUBSYSTEMDATAOFFSET     0       // To be used as double (8 bytes)

#define     VERSION                 256     // 0x0100
#define     ENDIANITY               19785   // MI chars



///
int MatlabConverter::CalculateDimensionFloat(const char* variableName){
   return 16 + 8 + 4*2 + CalculateVariableNameSize(variableName) + 16;
}

///
int MatlabConverter::CalculateDimensionDouble(const char* variableName){
   return 16 + 8 + 4*2 + CalculateVariableNameSize(variableName) + 16;
}

///
int MatlabConverter::CalculateDimension(MatrixF &mat, const char* variableName){
    int tDim = mat.NRows()*mat.NColumns();

    int dimMtx = tDim;
    if( (tDim % 2) != 0 ) dimMtx++;

    return 16 + 8 + 4*2 + CalculateVariableNameSize(variableName) + 8 + 4*dimMtx;
}


int MatlabConverter::CalculateDimension(MatrixI &mat, const char* variableName){
    int tDim = mat.NRows()*mat.NColumns();

    int dimMtx = tDim;
    if( (tDim % 2) != 0 ) dimMtx++;

    return 16 + 8 + 4*2 + CalculateVariableNameSize(variableName) + 8 + 4*dimMtx;
}



///
int MatlabConverter::CalculateDimension(MatrixD &mat, const char* variableName){
    return 16 + 8 + 4*2 + CalculateVariableNameSize(variableName) + 8 + 8*mat.NRows()*mat.NColumns();
}


///
bool MatlabConverter::InitStructure(const char* nameStruct, const char** listField, int nFields, int totalDimension){

    int tot = 32 + CalculateVariableNameSize(nameStruct) + 16 + 32*nFields + totalDimension + nFields * 8;

    WriteDataBlockType(mxSTRUCT_CLASS,tot,FDEFAULT);

    int dime[2];
    dime[0] = 1;
    dime[1] = 1;

    WriteDimensions(dime,2);

    WriteVariableName(nameStruct);

    WriteShort(miINT32);
    WriteShort(4);
    WriteSingle(32);

    WriteSingle(miINT8);
    WriteSingle(nFields*32);

    int i;
    for( i = 0; i < nFields; i++ ){
        WriteString(listField[i],strlen(listField[i])+1);
        AddPadding(32-strlen(listField[i])-1);
    }

    return True;

}

///
bool MatlabConverter::InitCell(const char* nameCell, int nFields, int totalDimension){

    int tot = 32 + CalculateVariableNameSize(nameCell) + totalDimension + nFields * 8;

    WriteDataBlockType(mxCELL_CLASS,tot,FDEFAULT);

    int dime[2];
    dime[0] = 1;
    dime[1] = nFields;

    WriteDimensions(dime,2);

    WriteVariableName(nameCell);

    return True;

}

///
int MatlabConverter::CalculateDimension(const char** listOfStrings, int nStrings, const char* variableName){

    int sizeRef = strlen(listOfStrings[0]);

    int tDim = sizeRef*nStrings*2;

    int padding = 0;
    if( (tDim % 8) != 0 ) padding = 8-tDim % 8;

    return 16 + 8 + 4*2 + CalculateVariableNameSize(variableName) + 8 + tDim + padding;
}


///
int MatlabConverter::CalculateVariableNameSize(const char* nameVar, int bytesSize){

    int32 leng = strlen(nameVar);

    if( leng <= 4 ) return 8;

    int dim = 8 + leng;

    if( (leng % 8) != 0 ) dim += 8 - leng % 8;

    return dim;
}


///
bool MCSaveF(MatlabConverter &mc, float value, const char* variableName){

    mc.WriteDataBlockType(mxSINGLE_CLASS,mc.CalculateDimensionFloat(variableName),FDEFAULT);

    int dime[2];
    dime[0] = 1;
    dime[1] = 1;

    mc.WriteDimensions(dime,2);

    mc.WriteVariableName(variableName);

    mc.WriteSingle(miSINGLE);
    mc.WriteSingle(4);

    mc.WriteSingle(*(int*&)value);
    mc.WriteSingle(0);

    return True;

}

///
bool MCSaveD(MatlabConverter &mc, double value, const char* variableName){
    mc.WriteDataBlockType(mxDOUBLE_CLASS,mc.CalculateDimensionDouble(variableName),FDEFAULT);

    int dime[2];
    dime[0] = 1;
    dime[1] = 1;

    mc.WriteDimensions(dime,2);

    mc.WriteVariableName(variableName);

    mc.WriteSingle(miDOUBLE);
    mc.WriteSingle(8);

    mc.WriteSingle(value);

    return True;
}


///
bool MCSaveMF(MatlabConverter &mc, MatrixF &mat, const char* variableName){
    MatrixD matrix(mat.NRows(),mat.NColumns());
    int i;
    for( i = 0; i < mat.NRows()*mat.NColumns(); i++ )
        matrix.data[i] = mat.data[i];

    return mc.Save(matrix,variableName);
}


/** */
bool MCSaveMD(MatlabConverter &mc, MatrixD &mat, const char* variableName){

    int tDim = mat.NRows()*mat.NColumns();

    mc.WriteDataBlockType(mxDOUBLE_CLASS,mc.CalculateDimension(mat,variableName),FDEFAULT);

    int dime[2];
    dime[0] = mat.NRows();
    dime[1] = mat.NColumns();

    mc.WriteDimensions(dime,2);

    mc.WriteVariableName(variableName);

    mc.WriteSingle(miDOUBLE);
    mc.WriteSingle(tDim*8);

    int i,j;
    for( i = 0; i < tDim; i++ ) mc.WriteDouble(((double*)mat.data)[i]);

    return True;

}

bool MCSaveMI(MatlabConverter &mc, MatrixI &mat, const char* variableName){

    MatrixD matrix(mat.NRows(),mat.NColumns());
    int i;
    for( i = 0; i < mat.NRows()*mat.NColumns(); i++ )
        matrix.data[i] = mat.data[i];

    return mc.Save(matrix,variableName);

}


/** */
bool MCSaveS(MatlabConverter &mc, const char** listOfStrings, int nStrings, const char* variableName){

    if( listOfStrings == NULL || nStrings == 0 ) return False;

    int i;
    int sizeRef = strlen(listOfStrings[0]);
    for( i = 0; i < nStrings; i++ ) if( strlen(listOfStrings[i]) != sizeRef ) return False;

    int tDim = sizeRef*nStrings*2;
    int padding = 0;
    if( (tDim % 8) != 0 ) padding = 8-tDim % 8;

    mc.WriteDataBlockType(mxCHAR_CLASS,mc.CalculateDimension(listOfStrings,nStrings,variableName),FDEFAULT);

    int dime[2];
    dime[0] = nStrings;
    dime[1] = sizeRef+1;

    mc.WriteDimensions(dime,2);

    mc.WriteVariableName(variableName);

    mc.WriteSingle(miUINT16);
    mc.WriteSingle(tDim);

    int j;
    for( j = 0; j < sizeRef; j++ ){
        for( i = 0; i < nStrings; i++ ){
            mc.WriteShort((short)listOfStrings[i][j]);
        }
    }

    mc.AddPadding(padding);

    return True;
}


///
void MatlabConverter::WriteDimensions(int *dime, int nDimensions){

    WriteSingle(miINT32);
    WriteSingle(nDimensions*4);
    int i;
    for( i = 0; i < nDimensions; i++ ) WriteSingle(dime[i]);
    if( (nDimensions % 2) != 0 ) WriteSingle(0);
}

///
void MatlabConverter::WriteDataBlockType(int dataType, int dataDimension, int options){

    WriteSingle(miMATRIX);
    WriteSingle(dataDimension);

    WriteSingle(miUINT32);
    WriteSingle(8);

    WriteSingle(options*16+dataType);
    WriteSingle(0);
}

///
void MatlabConverter::WriteVariableName(const char* varName){

    int leng = strlen(varName);

    if( leng == 0 ){
        WriteSingle(miINT8);
        WriteSingle(0);
    }else if( leng <= 4 ){
        WriteShort(miINT8);
        WriteShort(leng);
        WriteString(varName,leng);
        int padding = leng % 4;
        if( padding != 0 ) AddPadding(4-padding);
    }else{
        WriteSingle(miINT8);
        WriteSingle(leng);
        WriteString(varName,leng,True);
    }

}

///
void MatlabConverter::WriteChar(char data){
    uint32 dim = 1;
    char value[1];
    value[0] = data;
    output->Write(value,dim);
}

///
void MatlabConverter::WriteShort(short data){
    uint32 dim = 2;
    short value[1];
    value[0] = (short)data;
    output->Write(value,dim);
}

///
void MatlabConverter::WriteSingle(int data){
    uint32 dim = 4;
    int value[1];
    value[0] = data;
    output->Write(value,dim);
}

///
void MatlabConverter::WriteDouble(double data){
    uint32 dim = 8;
    double value[1];
    value[0] = data;
    output->Write(value,dim);
}

///
void MatlabConverter::WriteString(const char *str, uint32 dim, bool padding){
    output->Write(str,dim);
    if( padding && ( (dim % 8) != 0 ) ) AddPadding(8-dim % 8);
}

///
void MatlabConverter::AddPadding(int padding){
    int i;
    uint32 dim = 1;
    char zero = ' ';

    for( i = 0; i < padding; i++ ) output->Write(&zero,dim);
}

///
void MatlabConverter::WriteHeader(){
    sprintf(headerTextField,"%s","MATLAB 5.0 MAT-file");

    WriteString(headerTextField,116);

    WriteDouble(SUBSYSTEMDATAOFFSET);
    WriteShort(VERSION);
    WriteShort(ENDIANITY);

}



