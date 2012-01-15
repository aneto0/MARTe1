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

#define CDB_INTERNAL
#include "CDB.h"

#include "CDBObjectNode.h"
#include "ObjectRegistryDataBase.h"

OBJECTREGISTER (CDBObjectNode,CDBObjectNodeVersion)



bool CDBObjectNode::ReadArray(const char *configName,void *array,const CDBTYPE &valueType,const int *size,int nDim){
    if (nDim < 0) return False;

    CDBDataType cdbdt = valueType.dataType;
    if (configName == "Address"){
        if (cdbdt == CDB_Pointer){
            intptr *di = (intptr *)array;
            di[0] = (intptr)objectAddress;
        } else {
            AssertErrorCondition(ParametersError,"ReadEntry:unknown data type %i",valueType.dataType.Value());
            return False;
        }
    }


    if (configName == "ClassType"){
        if (cdbdt == CDB_FString){
            FString *ds = (FString *)array;
            ds[0] = classType;
        } else
        if (cdbdt == CDB_BString){
            BString *ds = (BString *)array;
            ds[0] = classType;
        } else
        if (cdbdt == CDB_String){
            char **ds = (char **)array;
            ds[0] = strdup(classType);
        } else {
            AssertErrorCondition(ParametersError,"ReadEntry:unknown data type %i",valueType.dataType.Value());
            return False;
        }
    }

    return CDBGroupNode::ReadArray(configName,array,valueType,size,nDim);
}

static inline void WriteIndent(Streamable *s,int level){
    uint32 size = 16;
    const char *buffer = "                ";
    while (level > 0) {
        size = 16;
        if (size > (uint32)level) size = level;
        s->Write(buffer,size);
        level-=size;
    }
}

bool CDBObjectNode::ReadContent(
    void *          value,
    const CDBTYPE & valueType,
    int             size,
    va_list         argList){

    if (value == NULL){
        AssertErrorCondition(ParametersError,"ReadContent::value=NULL");
        return False;
    }

    if ((valueType.dataType != CDB_CDBEval) &&
        (valueType.dataType != CDB_CDBStyle)){
            AssertErrorCondition(ParametersError,"ReadContent:unknown or unsupported data type %i",valueType.dataType.Value());
            return False;
    }

    uint32          indentChars = va_arg(argList,int);
    uint32          maxElements = va_arg(argList,int);
    CDBWriteMode    mode        = (CDBWriteMode)va_arg(argList,int);
    CDBWriteMode    wmode       = (CDBWriteMode)(mode & CDBWM_Modes);

//    Streamable **ds = (Streamable **)value;
//    Streamable *stream = ds[0];
    Streamable *stream = (Streamable *)value;
    if (stream == NULL) return False;

    if (!IsRootNode()){

        if ((!(mode & (CDBWM_NoIndent | CDBWM_NameJoin)))
           &&  (wmode == CDBWM_Tree)) {
            WriteIndent(stream,indentChars);
        }
        mode = mode & CDBWM_NameNoJoin;

        uint32 size = strlen(Name());
        stream->Write(Name(),size);
        size = 4;
        stream->Write(" = (",size);
        size = strlen(classType);
        stream->Write(classType,size);
        size = 2;
        stream->Write("){",size);
        if (wmode == CDBWM_Tree) {
            stream->PutC('\n');
        } else
        if (wmode == CDBWM_Comma) {
            stream->PutC(',');
        }

        indentChars += 4;
    }

    if (subTree.ListSize() > 0){
        CDBNode *node = (CDBNode *)subTree.List();
        while (node != NULL){
            node->ReadContent(value,valueType,size,indentChars,maxElements,mode);
            node = (CDBNode *)node->Next();
        }
    } else {
        if ((!(mode & (CDBWM_NoIndent | CDBWM_NameJoin)))
           &&  (wmode == CDBWM_Tree)) {
            WriteIndent(stream,indentChars);
        }
        stream->Printf(" EmptyNode=Empty",Name(),size);
        if (wmode == CDBWM_Tree) {
            stream->PutC('\n');
        } else
        if (wmode == CDBWM_Comma) {
            stream->PutC(',');
        }
    }


    if (!IsRootNode()){
        indentChars -= 4;

        WriteIndent(stream,indentChars);
        stream->PutC('}');
        if (wmode == CDBWM_Tree) {
            stream->PutC('\n');
        } else
        if (wmode == CDBWM_Comma) {
            stream->PutC(',');
        }
    }

    return True;

};
