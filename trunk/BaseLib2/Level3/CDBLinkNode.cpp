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


#include "CDBLinkNode.h"

OBJECTREGISTER(CDBLinkNode,CDBLinkNodeVersion)

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

bool CDBLinkNode::ReadContent(void *value,const CDBTYPE &valueType,int size,va_list argList){

    bool ret = True;
    if (value == NULL){
        AssertErrorCondition(ParametersError,"Read:value=NULL");
        return False;
    }

    CDBDataType cdbdt = valueType.dataType;
    if ((cdbdt == CDB_Content) ||
        (cdbdt == CDB_FString)){
        FString *ds = (FString *)value;
        ret = ReadString(ds,size);
    } else
    if (cdbdt == CDB_BString){
        BString *ds = (BString *)value;
        ret = ReadString(ds,size);
    } else
    if (cdbdt == CDB_String){
        char **ds = (char **)value;
        ret = ReadString(ds,size);
    } else
    if ((cdbdt == CDB_CDBEval) ||
        (cdbdt == CDB_CDBStyle)){

        uint32          indentChars = va_arg(argList,int);
        uint32          maxElements = va_arg(argList,int);
        CDBWriteMode    mode        = (CDBWriteMode)va_arg(argList,int);
        CDBWriteMode    wmode       = (CDBWriteMode)(mode & CDBWM_Modes);

//            Streamable **ds = (Streamable **)value;
//            Streamable *stream = ds[0];
        Streamable *stream = (Streamable *)value;
        if (stream == NULL) return False;

        if ((!(mode & CDBWM_NoIndent)) && (wmode == CDBWM_Tree)) {
            WriteIndent(stream,indentChars);
        }
        stream->PutC('*');
        ret = ReadFormatted(stream,indentChars,maxElements);
        if (wmode == CDBWM_Tree){
            stream->Printf("\n");
        } else
        if (wmode == CDBWM_Comma) {
            stream->Printf(",");
        }

    } else {
        AssertErrorCondition(ParametersError,"ReadEntry:unknown data type %i",valueType.dataType.Value());
        return False;
    }
    return ret;
}

