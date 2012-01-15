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

#include "CDBGroupNode.h"
#include "ObjectRegistryDataBase.h"
#include "CDBObjectNode.h"

OBJECTLOADREGISTER (CDBGroupNode,CDBGroupNodeVersion)

int CDBGroupNode::SubTreeSize(bool leafOnly) {
    int size = 1;
    // skip current node !
    if (leafOnly) size = 0;

    CDBNode *cdbn = Children(0);
    while (cdbn){
        size += cdbn->SubTreeSize(leafOnly);
        cdbn = (CDBNode *)cdbn->Next();
    }
    return size;
}

CDBNode *CDBGroupNode::Children(int childNumber) {
    if (childNumber < 0){
        CDBNode *cdbn = this;
        while(childNumber++ < 0){
            cdbn = cdbn->Father();
        }
        return cdbn;
    }

    return (CDBNode *)subTree.ListPeek(childNumber);
}


CDBNode * CDBGroupNode::Children(
                                    const char *    childName,
                                    CDBNMode        functionMode,
                                    SortFilterFn *  sortFn,
                                    const char *    containerClassName,
                                    bool            caseSensitive
                                    ){

    if (childName == NULL) return NULL;

    bool followLink =  (functionMode & CDBN_FollowLink);
    bool createOnly = ((functionMode & CDBN_ModeMask) == CDBN_CreateOnly);
    bool create     = createOnly;
    create = create | ((functionMode & CDBN_ModeMask) == CDBN_SearchAndCreate);

    CDBNode *cdbn = (CDBNode *)subTree.List();
    while (cdbn != NULL){
        if(caseSensitive){
            if (strcmp(cdbn->Name(),childName)==0){
                // error the node aready exists!
                if (createOnly) return NULL;
                return cdbn;
            }
        }
        else{
            if (strcasecmp(cdbn->Name(),childName)==0){
                    // error the node aready exists!
                if (createOnly) return NULL;
                return cdbn;
            }
        }
        if (cdbn->IsLinkNode() && followLink){
            FString path;
            if (cdbn->ReadContent(&path,CDBTYPE_FString,1,0,-1,0)){
                FString fullPath;
                fullPath.Printf("RootNode.%s",path.Buffer());
                CDBNode *linkedNode = this->Find(fullPath.Buffer());
                if (linkedNode){
                    CDBNode *cdbnl = linkedNode->Children(childName);
                    if (cdbnl) {
                        // error the node aready exists!
                        if (createOnly) return NULL;
                        return cdbnl;
                    }
                }
            }
        }

        cdbn = cdbn->Next();
    }

    CDBNode *child = NULL;
    if (create){
        if ((containerClassName != NULL) && (containerClassName[0] != 0)){
            Object *obj = OBJObjectCreateByName(containerClassName);
            if (obj == NULL) {
                AssertErrorCondition(FatalError,"Cannot create object of class %s",containerClassName);
            } else {
                child = dynamic_cast<CDBNode *>(obj);
                if (child == NULL) {
                    child = new CDBObjectNode(childName,obj,containerClassName);
/*
                    AssertErrorCondition(Warning,"Cannot convert object of class %s to CDBGroupNode",containerClassName);
                    CDBGroupNode *group = new CDBGroupNode(childName);
                    child = group;

                    CDBStringDataNode *address = new CDBStringDataNode("Address");
                    char addressText[16];
                    sprintf(addressText,"0x%08x",obj);
                    address->WriteContent(&addressText,CDBTYPE_String,1);

                    CDBStringDataNode *type    = new CDBStringDataNode("ClassType");
                    address->WriteContent(&containerClassName,CDBTYPE_String,1);

                    address->SetFather(group);
                    if (sortFn) group->subTree.ListInsert(address,sortFn);
                    else        group->subTree.ListAdd(address);

                    type->SetFather(group);
                    if (sortFn) group->subTree.ListInsert(type,sortFn);
                    else        group->subTree.ListAdd(type);
*/
                } else {
                    child->Init(childName);
                }
            }
        } else {
            int creationMode = functionMode & CDBN_CreateMask;
            switch (creationMode){
                case CDBN_CreateGroupNode:{
                    child = new CDBGroupNode(childName);
                } break;
                case CDBN_CreateStringNode:{
                    child = new CDBStringDataNode(childName);
                } break;
                case CDBN_CreateLinkNode:{
                    child = new CDBLinkNode(childName);
                } break;
            }
        }
        if (child){
            child->SetFather(this);
            if (sortFn) subTree.ListInsert(child,sortFn);
            else        subTree.ListAdd(child);
        }
    }

    return child;
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

bool CDBGroupNode::ReadContent(
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
            BString temp;
            AssertErrorCondition(ParametersError,"ReadContent:unknown or unsupported data type %s", BTConvertToString(valueType.dataType, temp));
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

    int nOfChildrens = subTree.ListSize();
/*
    if (!IsRootNode()){

        if ((!(mode & (CDBWM_NoIndent | CDBWM_NameJoin)))
           &&  (wmode == CDBWM_Tree)) {
            WriteIndent(stream,indentChars);
        }
        mode = mode & CDBWM_NameNoJoin;

        uint32 size = strlen(Name());
        stream->Write(Name(),size);
        size = 4;
        stream->Write(" = {",size);
        if (wmode == CDBWM_Tree) {
            stream->PutC('\n');
        } else
        if (wmode == CDBWM_Comma) {
            stream->PutC(',');
        }

        indentChars += 4;
    }
*/
    if (nOfChildrens >= 1){
        CDBNode *node = (CDBNode *)subTree.List();
        while (node != NULL){

            if (node->IsGroupNode()){

                if ((!(mode & (CDBWM_NoIndent | CDBWM_NameJoin)))
                   &&  (wmode == CDBWM_Tree)) {
                    WriteIndent(stream,indentChars);
                }
                mode = mode & CDBWM_NameNoJoin;

                uint32 size = strlen(node->Name());
                stream->Write(node->Name(),size);
                size = 2;
                stream->Write(" =",size);
		if (mode & CDBWM_AllmanStyle) {
                    stream->PutC('\n');
                    WriteIndent(stream,indentChars);
                    stream->PutC('{');
                } else {
                    stream->PutC(' ');
                    stream->PutC('{');
                }

                if (wmode == CDBWM_Tree) {
                    stream->PutC('\n');
                } else
                if (wmode == CDBWM_Comma) {
                    stream->PutC(',');
                }

                node->ReadContent(value,valueType,size,indentChars + 4,maxElements,mode);
                node = (CDBNode *)node->Next();

                WriteIndent(stream,indentChars);
                stream->PutC('}');
                if (wmode == CDBWM_Tree) {
                    stream->PutC('\n');
                } else
                if (wmode == CDBWM_Comma) {
                    stream->PutC(',');
                }
            } else {
                node->ReadContent(value,valueType,size,indentChars,maxElements,mode);
                node = (CDBNode *)node->Next();
            }
        }
    } else { //0 children
        stream->Printf("EmptyNode=Empty");
        if (wmode == CDBWM_Tree) {
            stream->PutC('\n');
        } else
        if (wmode == CDBWM_Comma) {
            stream->PutC(',');
        }
    }

/*
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
*/
    return True;

};

static inline bool IsSeparator(char c){
    if (c == 0) return True;
    const char *s = CDBSeparators;
    while(s[0] != 0){
        if (s[0] == c) return True;
        s++;
    }
    return False;
}
static inline const char *ExtractSegment(const char *name,FString &segment){
    if (name == NULL) return NULL;
    segment = "";
    const char *p = name;
    while(!IsSeparator(p[0])){
        segment += p[0];
        p++;
    }
    while((p[0]!=0) && IsSeparator(p[0]))p++;
    return p;
}

bool CDBGroupNode::WriteArray(const char *configName,const void *array,const CDBTYPE &valueType,const int *size,int nDim,CDBNMode functionMode,SortFilterFn *sortFn){
    CDBNode *node = this;

    // extract a segment name and reduce the path
    FString segment;
    if (configName != NULL)
        configName = ExtractSegment(configName,segment);

    // if there is still stuff in ConfigName it means that we have a segnment as well
    // we can clearly recurse at this stage since we do not need to distinguish between
    // vector and array
    if (configName[0] != 0){
        node = node->Find(segment.Buffer(),CDBN_SearchAndCreate | CDBN_CreateGroupNode,sortFn);
        if (node == NULL) return False;
        return node->WriteArray(configName,array,valueType,size,nDim,functionMode,sortFn);
    } else
    if (segment.Size() > 0){
        if ((size == NULL) || (nDim < 2)){
            node = node->Find(segment.Buffer(),CDBN_SearchAndCreate | functionMode,sortFn,valueType.containerClassName);
            if (node == NULL) return False;
            return node->WriteArray("",array,valueType,size,nDim,functionMode,sortFn);
        } else {
            // it was an array, therefore move another indirection level
            node = node->Find(segment.Buffer(),CDBN_SearchAndCreate | CDBN_CreateGroupNode,sortFn,valueType.containerClassName);
            if (node == NULL) return False;
            return node->WriteArray("",array,valueType,size,nDim,functionMode,sortFn);
        }
    }

    char *arrayC = (char *)array;
    int i;
    if (nDim < 0) return False;

    if (size != NULL){
        // reduce salient dimensions
//      while((size[0]<=1) && (nDim>0)) { size++; nDim--;}

        // reduce dimensions leaving at least
//      while((nDim>2) && (size[nDim-1]<=1) ) nDim--;

        // remove zeros
        for (i = 0;i < nDim; i++){
            if (size[i] < 1){
                AssertErrorCondition(Warning,"WriteArray: size contains elements less than 1");
                return False;
            }
        }
    }

    // distinguish between vector and array
    if ((size == NULL) || (nDim < 2)){

        int nElements = nDim;
        if (size != NULL) nElements = size[0];
        if (nElements == 0) nElements = 1;

        return node->WriteContent(array,valueType,nElements);
    }

    int stride = valueType.size;
    for (i = 1;i < nDim;i++) stride *= size[i];
    bool ret = True;

    for (i = 0;i < size[0];i++){
        char indexName[16];
        sprintf(indexName,"%i",i);
        ret = ret && node->WriteArray(indexName,arrayC+stride*i,valueType,size+1,nDim-1,functionMode,sortFn);
    }

    return ret;
}

// must perform recursive calls of virtual ReadArray to allow implementing custom array handling

bool CDBGroupNode::ReadArray(const char *configName,void *array,const CDBTYPE &valueType,const int *size,int nDim, bool caseSensitive){
    CDBNode *node = this;

    // extract a segment name and reduce the path
    FString segment;
    if (configName != NULL)
        configName = ExtractSegment(configName,segment);

    if (segment.Size() > 0){
        node = node->Find(segment.Buffer(),CDBN_SearchOnly | CDBN_FollowLink, NULL, NULL, caseSensitive);
        if (node == NULL) return False;
        return node->ReadArray(configName,array,valueType,size,nDim,caseSensitive);
    }

    if (nDim < 0) return False;
    if ((size == NULL) || (nDim < 2)){

        int nElements = nDim;
        if (size != NULL) nElements = size[0];
        if (nElements == 0) nElements = 1;
        return node->ReadContent(array,valueType,nElements,0,-1,0);
    }

    int stride = valueType.size;
    int i;
    for (i = 1;i<nDim;i++){
        stride *= size[i];
    }

    bool ret = True;
    char *arrayC = (char *)array;
    for (i = 0;i < size[0];i++){
        char indexName[16];
        sprintf(indexName,"%i",i);
        if (!node->ReadArray(indexName,arrayC+stride*i,valueType,size+1,nDim-1,caseSensitive)){
            AssertErrorCondition(ParametersError,"ReadArray:Row %i missing in matrix %s",i,node->Name());
            ret = False;
        }
    }
    return ret;
}
