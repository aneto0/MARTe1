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

#include "MMCDB.h"
#include "BasicTypes.h"

OBJECTLOADREGISTER(MMCDB,"$Id: MMCDB.cpp,v 1.7 2011/10/20 18:25:44 aneto Exp $")


/** get the last path position*/
MMCDBItem * MMCDB::CurrentPosition(){
    MMCDBItem *mmcdbi;
    if (path.StackDepth() <= 0) return NULL;
    if (path.StackTop((intptr *)&mmcdbi)) return mmcdbi;
    return NULL;
}

MMCDBItem * MMCDB::StartPosition(){
    MMCDBItem *mmcdbi;
    int stackDepth = path.StackDepth();
    if (stackDepth <= 0) return NULL;
    if (path.StackPeek((intptr *)&mmcdbi,stackDepth-1)) return mmcdbi;
    return NULL;
}


/** remove the last path position
    deallocation is performed only if this is the last left */
void MMCDB::RemovePathElement(){
    MMCDBItem *mmcdbi;
    if (path.StackPop((intptr *)&mmcdbi)){
        if (path.StackDepth() == 0){
            if (mmcdbi) delete mmcdbi;
        }
    }
}

/** remove all the path elements until the one specified
    maxStep <= -2 means all
    maxStep == -1 means leave father
    maxStep > 0 means remove so many elements
*/
void MMCDB::UnWind(MMCDBItem *keep,int maxStep){
    if (maxStep == 0) return;
    if (maxStep == -1){
        if (path.StackDepth() == 1) return;
    }
    MMCDBItem *mmcdbi = CurrentPosition();
    while ((mmcdbi != keep) && (mmcdbi != NULL)){
        mmcdbi->UnPopulate();
        RemovePathElement();
        mmcdbi = CurrentPosition();
        if (maxStep > 0) maxStep--;
        if (maxStep == 0) return;
        if (maxStep == -1){
            if (path.StackDepth() == 1) return;
        }
    }
}


/** either copies or references a memory structure,
    at address @param address and of type @param className
    CDB transforms the memory, MMCDB just references it  */
bool MMCDB::WriteStructure(const char *className,char *address,const char *variableName,Streamable *err){
    UnWind();
    MMCDBItem *mmcdbi = new MMCDBItem();
    if (mmcdbi->Init(address,className,variableName)){
        path.StackPush((intptr *)&mmcdbi);
        mmcdbi->Populate();
        return True;
    } else {
        delete mmcdbi;
    }
    return False;
}

/** creates a new reference to a database, or if that is not possible it creates a copy
    @param cdbcm = CDBCM_CopyAddress ensures that the new object points at the same location  */
CDBVirtual *MMCDB::Clone(CDBCreationMode cdbcm){
    MMCDB * mmcdb= new MMCDB;
    MMCDBItem *mmcdbi = StartPosition();
    if (mmcdbi){
        mmcdb->WriteStructure(mmcdbi->ClassName(),(char *)mmcdbi->Address(),mmcdbi->VariableName());

        FString position;
        if (cdbcm & CDBCM_CopyAddress){
            if (SubTreeName(position)){
                mmcdb->Move(position.Buffer());
            }
        }
    }
    return mmcdb;
}

/** finds the overall path leading to the current node */
bool MMCDB::SubTreeName(Streamable &name,const char *sep){
    MMCDBItem *mmcdbi;
    FString pathS;
    for (int i = (path.StackDepth()-2);i>=0;i--){
        if (path.StackPeek((intptr *)&mmcdbi,i)){
            if (pathS.Size()>0){
                FString temp;
                temp = pathS;
                pathS = mmcdbi->VariableName();
                pathS += ".";
                pathS += temp;
            } else {
                pathS = mmcdbi->VariableName();
            }
        }
    }
    uint32 size = pathS.Size();
    name.Write(pathS.Buffer(),size);
    return True;
}

/** the name of the current node */
bool MMCDB::NodeName(BString &name){
    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi){
        name  = mmcdbi->VariableName();
        return True;
    }
    return False;
}

/** the type of the current node */
bool MMCDB::NodeType(BString &name){
    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi){
        name  = mmcdbi->ClassName();
        return True;
    }
    return False;
}


/** how many branches from this node. Negative number implies that the location is a leaf */
int  MMCDB::NumberOfChildren(){
    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi == NULL) return 0;
    if (mmcdbi->IsPointer()) return -1;
    if (mmcdbi->IsFinal()) return -1;

    return mmcdbi->NumberOfElements();
}

/** move to the specified location. The movement is relative to the current location */
bool MMCDB::Move(const char *subTreeName){
    MMCDBItem *oldMmcdbi = CurrentPosition();
    if (oldMmcdbi == NULL) return False;
    char buffer[512];

    while (Streamable::GetCStringToken(subTreeName,&buffer[0],".",sizeof(buffer)-1)){
        MMCDBItem *mmcdbi = CurrentPosition();
        if (mmcdbi == NULL){
            AssertErrorCondition(FatalError,"Move: CurrentPosition() invalid!!");
            return False;
        }
        if (strcmp(buffer,"UpNode")==0){
            UnWind(NULL,1);
        } else
        if (strcmp(buffer,"RootNode")==0){
            UnWind(NULL,-1);
        } else {
            mmcdbi = mmcdbi->GetElement(buffer);

            // unwind all!
            if (mmcdbi == NULL) {
                AssertErrorCondition(Warning,"Move: element %s not found",buffer);
                UnWind(oldMmcdbi);
                return False;
            }

            mmcdbi->Populate();
            path.StackPush((intptr *)&mmcdbi);
        }
    }
    return True;
}

/** negative number move up. >=0 chooses the subtree, 0 is the first of the children */
bool MMCDB::MoveToChildren(int childNumber){
    if (childNumber < 0) return MoveToFather(-childNumber);
    // no data
    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi == NULL) return False;

    mmcdbi = mmcdbi->GetElement(childNumber);
    if (mmcdbi == NULL) return False;
    mmcdbi->Populate();
    path.StackPush((intptr *)&mmcdbi);

    return True;
}

/**  0 means remain where you are, > 0 brothers on the right < 0 on the left */
bool MMCDB::MoveToBrother(int steps){
    if (steps == 0) return True;

    // no father!
    if (path.StackDepth() <= 1) return False;

    // no data
    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi == NULL) return False;

    // get father
    MMCDBItem *mmcdbi1;
    if (!path.StackPeek((intptr *)&mmcdbi1,1))return False;
    if (mmcdbi1 == NULL) return False;

    // find position
    int position = mmcdbi1->GetElementPosition(mmcdbi );
    if (position < 0) return False;

    // new position
    int newPosition = position + steps;
    if (newPosition < 0) return False;
    if (newPosition >= mmcdbi1->NumberOfElements()) return False;

    // change things
    UnWind(NULL,1);
    return MoveToChildren(newPosition);
}

/** -1 = Root, one level up for each positive */
bool MMCDB::MoveToFather(int steps){
    if (steps == 0) return True;
    if (steps < 0 ) return False;

    // no father!
    if (path.StackDepth() == 0) return False;
    if (path.StackDepth() == 1) return True;

    while ((path.StackDepth() >= 1) && (steps>0)){
        UnWind(NULL,1);
        steps--;
    }
    return True;
}

/** whether a certain entry exists */
bool MMCDB::Exists(const char *configName){
    CDBVirtual *copy = this->Clone(CDBCM_CopyAddress);
    bool ret = copy->Move(configName);
    delete copy;
    return ret;
}

bool MMCDB::GetArrayDims(int *size,int &maxDim,const char *configName,CDBArrayIndexingMode cdbaim, bool caseSensitive){
    if (size) {
        for (int i=0;i<maxDim;i++) size[i] = 0;
        maxDim = 0;
    }
    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi == NULL) return False;

    MMCDBItem::CopyDimensions(size,maxDim,mmcdbi->GetArrayDimensions(),mmcdbi->NumberOfArrayDimensions());


    return True;
}

/** if size == NULL it treats the input as a monodimensional array of size nDim
    if size does not agree with the actual dimensions of the matrix the routine will fail   */
bool MMCDB::ReadArray (
    void *          array,
    const CDBTYPE & valueType,
    const int *     size,
    int             nDim,
    const char *    configName,
    bool            caseSensitive){

    if ((configName != NULL) && (configName[0] != 0)){
        CDBVirtual *copy = Clone(CDBCM_CopyAddress);
        if (!copy->Move(configName)) return False;
        bool ret = copy->ReadArray (array,valueType,size,nDim,NULL,caseSensitive);
        delete copy;
        return ret;
    }

    bool ret = True;
    if (array == NULL){
        AssertErrorCondition(ParametersError,"ReadArray:value=NULL");
        return False;
    }

    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi == NULL) return False;

    int totalSize = mmcdbi->TotalSize();

    int fillSize = 0;
    if (
        ((valueType.dataType != CDB_CDBStyle) &&
         (valueType.dataType != CDB_CDBEval)
        )){
        if (size == NULL){
            if (totalSize != 1){
                AssertErrorCondition(ParametersError,"ReadArray:array sizes incompatible");
                return False;
            }

        } else {
            if (!MMCDBItem::CompareDimensions(mmcdbi->GetArrayDimensions(),mmcdbi->NumberOfArrayDimensions(),size,nDim)){
                AssertErrorCondition(ParametersError,"ReadArray:array sizes incompatible");
                return False;
            }
        }
    }



    CDBDataType cdbdt = valueType.dataType;
    if (cdbdt == CDB_double){
        AssertErrorCondition(ParametersError,"CDB_double ReadArray:Not Implemented");
        return False;
    } else
    if (cdbdt == CDB_float){
        AssertErrorCondition(ParametersError,"CDB_float ReadArray:Not Implemented");
        return False;
    } else
    if (cdbdt == CDB_int32){
        AssertErrorCondition(ParametersError,"CDB_int32 ReadArray:Not Implemented");
        return False;
    } else
    if (cdbdt == CDB_uint32){
        AssertErrorCondition(ParametersError,"CDB_uint32 ReadArray:Not Implemented");
        return False;
    } else
    if (cdbdt == CDB_char){
        AssertErrorCondition(ParametersError,"ReadArray:CDB_char Not Implemented");
        return False;
    } else
    if ((cdbdt == CDB_Content) ||
        (cdbdt == CDB_FString)){
        FString *ds = (FString *)array;
        if (mmcdbi->IsPointer()){
            int **address = (int **)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i] = "";
                ds[i].Printf("0x%x",address[i]);
            }
        } else
        if (strcmp(mmcdbi->ClassName(),"FString")==0){
            FString *fs = (FString *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i] = fs[i];
            }
        } else
        if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==8)){
            int64 *ip = (int64 *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i]= "";
                ds[i].Printf("%Li",ip[i]);
            }
        } else
        if ((strcmp(mmcdbi->ClassName(),"int")==0) ||
            (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==4))){
            intptr *ip = (intptr *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i]= "";
                ds[i].Printf("%i",ip[i]);
            }
        } else
        if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==2)){
            int16 *ip = (int16 *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i]= "";
                ds[i].Printf("%i",ip[i]);
            }
        } else
        if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==1)){
            int8 *ip = (int8 *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i]= "";
                ds[i].Printf("%i",ip[i]);
            }
        } else
        if (mmcdbi->IsFloat() && (mmcdbi->ElementSize()==4)){
            float *fp = (float *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i]= "";
                ds[i].Printf("%f",fp[i]);
            }
        } else
        if (mmcdbi->IsFloat() && (mmcdbi->ElementSize()==8)){
            double *fp = (double *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i]= "";
                ds[i].Printf("%f",fp[i]);
            }
        } else
        if ((strcmp(mmcdbi->ClassName(),"char*")==0) ||
            (strcmp(mmcdbi->ClassName(),"const char*")==0)){
            char **cpp = (char **)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i] = cpp[i];
            }
        } else
        {
            AssertErrorCondition(ParametersError,"ReadArray:%s Not Implemented",mmcdbi->ClassName());
        }
    } else
    if (cdbdt == CDB_String){
        char **ds = (char **)array;
        if (mmcdbi->IsPointer()){
            int **address = (int **)mmcdbi->Address();
            char buffer[256];
            for (int i = 0;i < totalSize;i++) {
                buffer[0] =0;
                sprintf(buffer,"0x%x",address[i]);
                ds[i] = strdup(buffer);
            }
        } else
        if (strcmp(mmcdbi->ClassName(),"FString")==0){
            FString *fs = (FString *)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i] = strdup(fs[i].Buffer());
            }
        } else
        if ((strcmp(mmcdbi->ClassName(),"char*")==0) ||
            (strcmp(mmcdbi->ClassName(),"const char*")==0)){
            char **cpp = (char **)mmcdbi->Address();
            for (int i = 0;i < totalSize;i++) {
                ds[i] = strdup(cpp[i]);
            }
        } else
        {
            AssertErrorCondition(ParametersError,"ReadArray: %s Not Implemented",mmcdbi->ClassName());
        }
    } else
    if ((cdbdt == CDB_CDBEval) ||
        (cdbdt == CDB_CDBStyle)){
        AssertErrorCondition(ParametersError,"ReadArray:CDB_CDBEval Not Implemented");
        return False;
    } else
    if (cdbdt == CDB_Interpret){
        AssertErrorCondition(ParametersError,"ReadArray:unsupported data type CDB_Interpret");
        return False;
    } else {
        AssertErrorCondition(ParametersError,"ReadArray:unknown data type %i",valueType.dataType.Value());
        return False;
    }
    return ret;
}

/** if size == NULL it treats the input as a monodimensional array of size nDim
    if size does not agree with the actual dimensions of the matrix the routine will fail   */
bool MMCDB::WriteArray (
    const void *    array,
    const CDBTYPE & valueType,
    const int *     size,
    int             nDim,
    const char *    configName,
    SortFilterFn *  sorter){


    if ((configName != NULL) && (configName[0] != 0)){
        CDBVirtual *copy = Clone(CDBCM_CopyAddress);
        if (!copy->Move(configName)) return False;
        bool ret = copy->WriteArray (array,valueType,size,nDim,NULL,NULL);
        delete copy;
        return ret;
    }

    bool ret = True;
    if (array == NULL){
        AssertErrorCondition(ParametersError,"WriteArray:value=NULL");
        return False;
    }

    MMCDBItem *mmcdbi = CurrentPosition();
    if (mmcdbi == NULL) return False;

    int totalSize = mmcdbi->TotalSize();


    int fillSize = 0;
    if (
        ((valueType.dataType != CDB_CDBStyle) &&
         (valueType.dataType != CDB_CDBEval)
        )){
        if (size == NULL){
            if (totalSize != 1){
                AssertErrorCondition(ParametersError,"WriteArray:array sizes incompatible");
                return False;
            }

        } else {
            if (!MMCDBItem::CompareDimensions(mmcdbi->GetArrayDimensions(),mmcdbi->NumberOfArrayDimensions(),size,nDim)){
                AssertErrorCondition(ParametersError,"WriteArray:array sizes incompatible");
                return False;
            }
        }
    }


    CDBDataType cdbdt = valueType.dataType;
    if (cdbdt == CDB_double){
            AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            return False;
        } else
        if (cdbdt == CDB_float){
            AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            return False;
        } else
        if (cdbdt == CDB_int32){
            AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            return False;
        } else
        if (cdbdt == CDB_uint32){
            AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            return False;
        } else
        if (cdbdt == CDB_char){
            AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            return False;
        } else
        if ((cdbdt == CDB_Content) ||
            (cdbdt == CDB_FString)){
            FString *ds = (FString *)array;
            if (mmcdbi->IsPointer()){
                AssertErrorCondition(ParametersError,"WriteArray:Cannot change pointers");
                return False;
            } else
            if (strcmp(mmcdbi->ClassName(),"FString")==0){
                FString *fs = (FString *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    fs[i] = ds[i];
                }
            } else
            if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==8)){
                int64 *ip = (int64 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i].Buffer();
                    ip[i] = StringToInt64(p);
                }
            } else
            if ((strcmp(mmcdbi->ClassName(),"int")==0) ||
                (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==4))){
                int32 *ip = (int32 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i].Buffer();
                    ip[i] = StringToInt32(p);
                }
            } else
            if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==2)){
                int16 *ip = (int16 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i].Buffer();
                    ip[i] = StringToInt16(p);
                }
            } else
            if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==1)){
                int8 *ip = (int8 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i].Buffer();
                    ip[i] = StringToInt8(p);
                }
            } else
            if (mmcdbi->IsFloat() && (mmcdbi->ElementSize()==4)){
                float *fp = (float *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    fp[i] = atof(ds[i].Buffer());
                }
            } else
            if (mmcdbi->IsFloat() && (mmcdbi->ElementSize()==8)){
                double *fp = (double *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    fp[i] = atof(ds[i].Buffer());
                }
            } else

            {
                AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            }
        } else
        if (cdbdt == CDB_String){
            char **ds = (char **)array;
            if (mmcdbi->IsPointer()){
                AssertErrorCondition(ParametersError,"WriteArray:Cannot change pointers");
                return False;
            } else
            if (strcmp(mmcdbi->ClassName(),"FString")==0){
                FString *fs = (FString *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    fs[i] = ds[i];
                }
            } else
            if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==8)){
                int64 *ip = (int64 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i];
                    ip[i] = StringToInt64(p);
                }
            } else
            if ((strcmp(mmcdbi->ClassName(),"int")==0) ||
                (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==4))){
                int32 *ip = (int32 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i];
                    ip[i] = StringToInt32(p);
                }
            } else
            if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==2)){
                int16 *ip = (int16 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i];
                    ip[i] = StringToInt16(p);
                }
            } else
            if (!mmcdbi->IsFloat() && (mmcdbi->ElementSize()==1)){
                int8 *ip = (int8 *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    const char *p = ds[i];
                    ip[i] = StringToInt8(p);
                }
            } else
            if (mmcdbi->IsFloat() && (mmcdbi->ElementSize()==4)){
                float *fp = (float *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    fp[i] = atof(ds[i]);
                }
            } else
            if (mmcdbi->IsFloat() && (mmcdbi->ElementSize()==8)){
                double *fp = (double *)mmcdbi->Address();
                for (int i = 0;i < totalSize;i++) {
                    fp[i] = atof(ds[i]);
                }
            } else
            {
                AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            }
        } else
        if ((cdbdt == CDB_CDBEval) ||
            (cdbdt == CDB_CDBStyle)){
            AssertErrorCondition(ParametersError,"WriteArray:Not Implemented");
            return False;
        } else
        if (cdbdt == CDB_Interpret){
            AssertErrorCondition(ParametersError,"WriteArray:unsupported data type CDB_Interpret");
            return False;
        } else {
            AssertErrorCondition(ParametersError,"WriteArray:unknown data type %i",valueType.dataType.Value());
            return False;
        }
    return ret;
}
