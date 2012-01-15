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

#include "CDBCInterface.h"
#include "ObjectRegistryDataBase.h"
#include "ErrorManagement.h"
#include "CDB.h"
#include "SXMemory.h"
#include "CDBDataTypes.h"
#include "LexicalAnalyzer.h"

ErrorReportingFunction globalErrorFunction = NULL;

static void AssembleErrorMessageFunction(const char *errorDescription,va_list argList,const char *errorHeader,...){
    FString msg;
    if (errorHeader != NULL){
        va_list argList2;
        va_start(argList2,errorHeader);
        msg.VPrintf(errorHeader,argList2);
        va_end(argList2);
    }

    msg.VPrintf(errorDescription,argList);
    msg+='\n';

    const char *errorCode = strstr(msg.Buffer(),"|E=");
    if (errorCode){
        errorCode += 3;
        int errorCodeNumeric;
        sscanf(errorCode,"%x",&errorCodeNumeric);
        if (errorCodeNumeric >= 0) return;
    }

    if (globalErrorFunction) globalErrorFunction(0,msg.Buffer());
}

AssembleErrorMessageFunctionType savedErrorFun = NULL;

void CDBCISetErrorFunction(ErrorReportingFunction errorFunction,int interceptAllErrors){
    if (errorFunction == NULL){
        globalErrorFunction = NULL;
        if (savedErrorFun){
            LSSetUserAssembleErrorMessageFunction(savedErrorFun);
            savedErrorFun = NULL;
        }
        return;
    }

    if (globalErrorFunction != NULL){
        globalErrorFunction(1,"SetErrorFunction:globalErrorFunction already defined ");
        return;
    }

    globalErrorFunction = errorFunction;

    if ((interceptAllErrors) && (savedErrorFun == NULL)){
        LSGetUserAssembleErrorMessageFunction(savedErrorFun);
        LSSetUserAssembleErrorMessageFunction(AssembleErrorMessageFunction);
    }
}

static const CDBTYPE *FindCDBTYPEFromCDBCDataType(CDBCDataType type){

    switch (type){
        case CDBC_float:{
            return &CDBTYPE_float;
        } break;
        case CDBC_double:{
            return &CDBTYPE_double;
        } break;
        case CDBC_int32:{
            return &CDBTYPE_int32;
        } break;
        case CDBC_uint32:{
            return &CDBTYPE_uint32;
        } break;
        case CDBC_char:{
            return &CDBTYPE_char;
        } break;
        case CDBC_String:{
            return &CDBTYPE_String;
        } break;
    };

    if (globalErrorFunction) globalErrorFunction(1,"FindCDBTYPEFromCDBCDataType:type %i is unknown ",type);
    return &CDBTYPE_NULL;
}


/** create an empty reference
    if cdbr == NULL just creates, otherwise it copies */
CDBReference    CDBCICreate(
                    CDBReference    cdbr
                ){

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;
    if (cdbrv != NULL){
        CDBVirtual *cdbv = cdbrv->Clone(CDBCM_CopyAddress);
        return  (CDBReference)cdbv;
    }

    Object *o = OBJObjectCreateByName("CDB");
    if (o == NULL) return 0;
    CDBVirtual *cdbv = dynamic_cast<CDBVirtual *>(o);
    if (cdbv == NULL) delete o;

    return cdbv;
}

/** create an empty reference */
void            CDBCIDestroy(
                    CDBReference   *cdbr
                ){
    if (cdbr== NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIDestroy:handle is NULL ");
        return ;
    }

    if (*cdbr != NULL){
        CDBVirtual *cdbrv = (CDBVirtual *)(*cdbr);
        delete cdbrv;
        *cdbr = NULL;
    }
}

/** cleans the current subtree */
int            CDBCIClean(
                    CDBReference    cdbr
                ){
    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;
    if (cdbrv != NULL){
        cdbrv->CleanUp(CDBAM_None);
        return 1;
    }
    return 0;
}

/** loads from a memory buffer into the current subtree
    return 1 on success 0 on failure */
int             CDBCILoad(
                    CDBReference    cdbr,
                    const char *    buffer
                ){

    if (cdbr == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCILoad:handle is NULL ");
        return 0;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;

    SXMemory sBuffer((char *)buffer,strlen(buffer));
    FString errors;

    if (!cdbrv->ReadFromStream(sBuffer,&errors)){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCILoad:failed");
        return 0;
    }
    return 1;
}

/** loads from a memory buffer into the current subtree
    return 1 on success 0 on failure */
int             CDBCISave(
                    CDBReference    cdbr,
                    char **         buffer
                ){

    if (cdbr == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCISave:handle is NULL ");
        return 0;
    }

    if (buffer == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCISave:buffer is NULL ");
        return 0;
    }

    if (*buffer != NULL){
        if (globalErrorFunction) globalErrorFunction(0,"CDBCISave:*buffer should be NULL (did you forget to free memory?)");
        *buffer = NULL;
//        return 0;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;

    FString sBuffer;
    FString errors;

    if (!cdbrv->WriteToStream(sBuffer,&errors)){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCISave:failed\n%s",errors.Buffer());
        return 0;
    }

    sBuffer.PutC(0);

    *buffer = (char *)malloc (sBuffer.Size()+1);
    memcpy(*buffer,sBuffer.Buffer(),sBuffer.Size()+1);

    return 1;

}

static int      CDBCIMove1(
                    CDBVirtual *    cdbrv,
                    const char *    command){

    FString buffer;
    buffer = command;
    FString token;
    buffer.Seek(0);
    char saveSep;
    while(buffer.GetToken(token,". \n\r\t:",&saveSep)){
        if ((token == "root") || (token == "Root")){
            if (!cdbrv->MoveToRoot()){
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:MoveToRoot failed");
                return 0;
            }
        } else
        if ((token == "up") || (token == "Up") || (token == "father") || (token == "Father")){
            if (!cdbrv->MoveToFather()){
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:MoveToFather failed");
                return 0;
            }
        } else
        if ((token == "child") || (token == "Child")){
            if (saveSep != ':'){
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:child must be followed by ':' and a number");
                return 0;
            }
            FString childNo;
            buffer.GetToken(childNo,". \n\r\t:",&saveSep);
            int childMumber = atoi(childNo.Buffer());
            if (!cdbrv->MoveToChildren(childMumber)){
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:MoveToChildren(%i) failed",childMumber);
                return 0;
            }
        } else
        if ((token == "brother") || (token == "Brother")){
            if (saveSep != ':'){
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:brother must be followed by ':' and a number");
                return 0;
            }
            FString childNo;
            buffer.GetToken(childNo,". \n\r\t:",&saveSep);
            int childMumber = atoi(childNo.Buffer());
            if (!cdbrv->MoveToBrother(childMumber)){
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:MoveToBrother(%i) failed",childMumber);
                return 0;
            }
        } else {
            if (!cdbrv->AddChildAndMove(token.Buffer())) {
                if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove::Move:AddChildAndMove(%s) failed",token.Buffer());
                return 0;
            }
        }
        token.SetSize(0);
    }
    return 1;
}


int             CDBCIMove(
                    CDBReference    cdbr,
                    const char *    command,
                    char **         location
                ){
    if (cdbr == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIMove:handle is NULL ");
        return 0;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;

    int ret = 1;
    if (command != NULL){
         ret = CDBCIMove1(cdbrv,command);
    }

    if(location == NULL) return 1;
    if(*location != NULL){
        if (globalErrorFunction) globalErrorFunction(0,"CDBCIMove:*location should be NULL (did you forget to free memory?)");
        *location = NULL;
//        return 0;
    }
    FString sxm;
    cdbrv->SubTreeName(sxm);
    *location = strdup(sxm.Buffer());

    return 1;
}

/** read a value in the specified format
    the data (float,double,int) is in the C standard order
    the data (char *) is stored as a single array separated by 0s
    arraySizes is a 0 terminated array of ints.
    It lists the array dimensions.
    *arrayValue,arraySizes are allocated by the library and
    MUST be deallocated with CDBCIFree */
int             CDBCIReadArray(
                    CDBReference    cdbr,
                    CDBCDataType    type,
                    void **         arrayValuesP,
                    int  **         arraySizesP
                ){

    if (cdbr == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray:handle is NULL ");
        return 0;
    }


    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;

    if (arrayValuesP == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray:arrayValues is NULL ");
        return 0;
    }

    if (arraySizesP == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray:arraySizes is NULL ");
        return 0;
    }
    if(*arrayValuesP != NULL){
        if (globalErrorFunction) globalErrorFunction(0,"CDBCIReadArray:*arrayValuesP should be NULL (did you forget to free memory?)");
        *arrayValuesP = NULL;
    }
    if(*arraySizesP != NULL){
        if (globalErrorFunction) globalErrorFunction(0,"CDBCIReadArray:*arraySizesP should be NULL (did you forget to free memory?)");
        *arraySizesP = NULL;
    }

    const  CDBTYPE *cdbType = FindCDBTYPEFromCDBCDataType(type);
    if (cdbType->size <= 0){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray:unknown type ");
        return 0;
    }

    int size[16];
    int maxDim = 16;
//    if (
//        (type == CDB_CDBStyle) ||
//        (type == CDB_CDBEval )
//    ) {
//        size[0] = 1;
//        maxDim = 1;
//    } else {
        if (!cdbrv->GetArrayDims(size,maxDim,"",CDBAIM_Strict)){
            if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray::ReadArray: failed reading array dimensions");
            return 0;
        }
//    }

    int allocSize= (maxDim+2)*sizeof(int);
    *arraySizesP = (int *)malloc(allocSize);
    if (*arraySizesP == NULL){
         if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray::arraySizes=malloc(%i): failed ",allocSize);
         return 0;
    }
    int  *arraySizes  = *arraySizesP;

    int totalSize= 1;
//    for (int i = 0;(i < maxDim) && (i < 1);i++){
    for (int i = 0;i < maxDim;i++){
        if (size[i] <= 0) size[i] = 1;
        arraySizes[i] = size[i];
        totalSize *= size[i];
    }
    arraySizes[maxDim]=0;

//    if (
//        (type == CDB_FString ) ||
//        (type == CDB_Content )
//    ) {
//        *arrayValuesP = (void *)(new FString[totalSize]);
//    } else
//    if (
//        (type == CDB_BString )
//    ) {
//        *arrayValuesP = (void *)(new BString[totalSize]);
//    } else
//    if (
//        (type == CDB_CDBStyle) ||
//        (type == CDB_CDBEval )
//    ) {
//        *arrayValuesP = (void *)(new FString);
//    } else {
        allocSize = totalSize * cdbType->size;
        *arrayValuesP = malloc(allocSize);
//    }

    if (*arrayValuesP == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray::malloc(%i): failed ",allocSize);
        free ((void *&)arraySizes);
        return 0;
    }
    memset(*arrayValuesP,0,allocSize);

    void *arrayValues = *arrayValuesP;

//printf("maxDim = %i\n",maxDim);
//    if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray::malloced(%i) #dim = %i ",allocSize,maxDim);

    if (!cdbrv->ReadArray (arrayValues,*cdbType,size,maxDim,"")){
         if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray::ReadArray: failed ");
         free (arrayValues);
         free ((void *&)arraySizes);
         return 0;
     }

     return 1;
}

/** write a value in the raw format
    the data (float,double,int) is in the C standard order
    the data (char *) is stored as a single array separated by 0s
    arraySizes is a 0 terminated array of ints.
    It lists the array dimensions. */
int             CDBCIWriteArray(
                    CDBReference    cdbr,
                    const char *    entryName,
                    CDBCDataType    type,
                    void *          arrayValues,
                    int  *          arraySizes
                ){

    if (entryName == NULL)
        entryName = "";

    if (cdbr== NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIWriteArray:handle is NULL ");
        return 0;
    }

//    if ((type == CDB_FString) ||
//        (type == CDB_BString)){
//        if (globalErrorFunction) globalErrorFunction(1,"CDBCIReadArray:type == CDB_(B)FString is not supported ");
//        return 0;
//    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;

    if (arrayValues == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIWriteArray:arrayValues is NULL ");
        return 0;
    }

    if (arraySizes == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIWriteArray:arraySizes is NULL ");
        return 0;
    }

    const CDBTYPE *cdbType = FindCDBTYPEFromCDBCDataType(type);
    if (cdbType->size <= 0){
        if (globalErrorFunction) globalErrorFunction(1,"CDBCIWriteArray:unknown type ");
        return 0;
    }

    int maxDim = 0;
    while (arraySizes[maxDim]>0)maxDim++;

    if (!cdbrv->WriteArray (arrayValues,*cdbType,arraySizes,maxDim,entryName)){
         if (globalErrorFunction) globalErrorFunction(1,"CDBCIWriteArray::WriteArray: failed ");
         return 0;
     }

     return 1;
}

int             CDBCIExist(
                    CDBReference    cdbr,
                    const char *    entryName
    ){

    if (cdbr== NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDCIExist: handle cdbr is NULL ");
        return 0;
    }

    if (entryName == NULL) {
        if (globalErrorFunction) globalErrorFunction(1,"CDCIExist: You must specify a valid entryName ");
        return 0;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;
    if(!cdbrv->Exists(entryName)) return 0;

    return 1;
}


int             CDBCINumberOfChildren(
                    CDBReference    cdbr
    ){

    if (cdbr== NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDCINumberOfChildren: handle cdbr is NULL ");
        return -1;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;
    return cdbrv->NumberOfChildren();
}


char            *CDBCINodeName(
                    CDBReference    cdbr,
		    int             full
    ){

    if (cdbr== NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDCINodeName: handle cdbr is NULL ");
        return NULL;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)cdbr;

    FString nodeName;
    if(full == 1){
	cdbrv->SubTreeName(nodeName);
    }else{
	cdbrv->NodeName(nodeName);
    }

    char *name = (char *)malloc(sizeof(char)*(strlen(nodeName.Buffer()) + 1));
    if(name == NULL){
        if (globalErrorFunction) globalErrorFunction(1,"CDCINodeName: Failed allocating memory to store nodeName ");
	return NULL;
    }
    
    strcpy(name, nodeName.Buffer());
    return name;
}


/**  to deallocate any of the allocated data */
void            CDBCIFree(
                    void *          data
                ){
    if (data != NULL) free(data);

}

/** Returns the API struct */
BL2cdbCI *GetCDBCI(){
    BL2cdbCI *bl2cdbCI = NULL;
    bl2cdbCI = (BL2cdbCI *)malloc(sizeof(BL2cdbCI));
    if(bl2cdbCI == NULL)
        return NULL;
    bl2cdbCI->CDBCICreate              = CDBCICreate;
    bl2cdbCI->CDBCIDestroy             = CDBCIDestroy;
    bl2cdbCI->CDBCIClean               = CDBCIClean;
    bl2cdbCI->CDBCILoad                = CDBCILoad;
    bl2cdbCI->CDBCISave                = CDBCISave;
    bl2cdbCI->CDBCIMove                = CDBCIMove;
    bl2cdbCI->CDBCIReadArray           = CDBCIReadArray;
    bl2cdbCI->CDBCIWriteArray          = CDBCIWriteArray;
    bl2cdbCI->CDBCIFree                = CDBCIFree;
    bl2cdbCI->CDBCIExist               = CDBCIExist;
    bl2cdbCI->CDBCINumberOfChildren    = CDBCINumberOfChildren;
    bl2cdbCI->CDBCINodeName            = CDBCINodeName;
    bl2cdbCI->CDBCISetErrorFunction    = CDBCISetErrorFunction;
    bl2cdbCI->CDBCISwitchNumberParsing = CDBCISwitchNumberParsing;
    return bl2cdbCI;
}

void CDBCISwitchNumberParsing(int on){
    LexicalAnalyzer::parseNumbers = on;
}

