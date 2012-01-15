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

#include "CDBVirtual.h"
#include "CDB.h"
#include "CDBCore.h"
#include "CDBStringDataNode.h"
#include "CDBLinkNode.h"
#include "ObjectRegistryDataBase.h"

OBJECTLOADREGISTER(CDB,"$Id: CDB.cpp,v 1.23 2011/10/20 18:25:22 aneto Exp $")

int32 CDBC_SortFilterFn(LinkedListable *data1,LinkedListable *data2){
    CDBNode *cfg1 = (CDBNode *)data1;
    CDBNode *cfg2 = (CDBNode *)data2;
    if (cfg1==NULL) return 1;
    if (cfg2==NULL) return -1;
    const char *n1 = cfg1->Name();
    const char *n2 = cfg2->Name();
    if (n1 == NULL) return 1;
    if (n2 == NULL) return -1;
    if (n1[0]=='*') return 1;
    if (n2[0]=='*') return -1;

    return(strcmp(n1,n2));
}

inline bool isNumber(char x){
    return ((x>='0') && (x <='9'));
}


int32 CDBC_SortNumFilterFn(LinkedListable *data1,LinkedListable *data2){
    CDBNode *cfg1 = (CDBNode *)data1;
    CDBNode *cfg2 = (CDBNode *)data2;
    if(cfg1==NULL) return 1;
    if(cfg2==NULL) return -1;
    const char *n1 = cfg1->Name();
    const char *n2 = cfg2->Name();
    if (n1 == NULL) return 1;
    if (n2 == NULL) return -1;
    if (n1[0]=='*') return 1;
    if (n2[0]=='*') return -1;


    int ix1 = 0;
    int ix2 = 0;

    while ((n1[ix1] != 0) && (n2[ix2] != 0)){
        if (isNumber(n1[ix1]) && isNumber(n2[ix2])){
            int a1 = atoi(n1+ix1);
            int a2 = atoi(n2+ix2);
            if (a1>a2) return 1;
            if (a2>a1) return -1;
            while(isNumber(n1[ix1])) ix1++;
            while(isNumber(n2[ix2])) ix2++;
        }
        if (n1[ix1] > n2[ix2]) return 1;
        if (n2[ix1] > n1[ix2]) return -1;
        if (n1[ix1] != 0) ix1++;
        if (n2[ix2] != 0) ix2++;
    }
    if (n1[ix1] == n2[ix2]) return 0;
    if (n1[ix1] == 0) return 1;
    if (n2[ix2] == 0) return -1;
    return 0;
}



const char *CDBSeparators= ".";

bool CDB::GetArrayDims(int *size,int &maxDim,const char *configName,CDBArrayIndexingMode cdbaim, bool caseSensitive){

    if (!Lock()) return False;
    bool ret = node().GetArrayDims(size,maxDim,configName,cdbaim,caseSensitive);

    UnLock();
    return ret;
}

CDB::CDB(CDB *base,CDBCreationMode cdbcm){
    parserReportEnabled = False;

    if (base == NULL){
        core = new CDBCore();
    } else {
        core = base->GetCore();
    }
    if (!Lock()) return;

    coreRef.MoveReference(core,CDBNR_None);

    if ((cdbcm & CDBCM_CopyAddress) && (base != NULL))   node = *base->GetNode();
    else                node = coreRef;

    UnLock();
}

CDB::~CDB(){
    if (!Lock()) return;
    node.MoveReference(NULLCDBNODE,CDBNR_None);

    coreRef.MoveReference(NULLCDBNODE,CDBNR_None);

    if (core->NumberOfReferences()<=0){

        core->DeleteSubTree();

        delete core;
    } else {
        UnLock();
    }
    core = NULL;
}

bool CDB::SubTreeName(Streamable &name,const char *sep){
    if (!Lock()) return False;
    node().SubTreeName(name,sep);
    UnLock();
    return True;
}


bool CDB::NodeName(BString &name){
    if (!Lock()) return False;
    name = node().Name();
    UnLock();
    return True;
}

bool CDB::NodeType(BString &name){
    if (!Lock()) return False;
    name = node().ClassName();
    UnLock();
    return True;
}



void CDB::CleanUp(CDBAddressMode cdbam){
    if (!Lock()) return;
    if (cdbam & CDBAM_FromRoot) node = coreRef;
    node().DeleteSubTree();
    UnLock();
}


bool CDB::Move(const char *subTreeName){
    if (!Lock()) return False;
    bool ret = node.Move(subTreeName);
    UnLock();
    return ret;
}

bool CDB::AddChildAndMove(const char *subTreeName,SortFilterFn *sorter){
    if (!Lock()) return False;
    bool ret = node.Move(subTreeName,CDBN_SearchAndCreate | CDBN_CreateGroupNode,sorter);
    UnLock();
    return ret;
}

bool CDB::MoveToChildren(int childNumber){
    if (!Lock()) return False;
    bool ret = node.MoveToChildren(childNumber);
    UnLock();
    return ret;
}

int CDB::NumberOfChildren(){
    if (!Lock()) return False;
    int ret = node().NumberOfChildren();
    UnLock();
    return ret;
}

bool CDB::MoveToBrother(int steps){
    if (!Lock()) return False;
    bool ret = node.MoveToBrother(steps);
    UnLock();
    return ret;
}

bool CDB::MoveToFather(int steps){
    bool ret = True;
    if (!Lock()) return False;
    if (steps > 0 ) ret = node.MoveToFather(steps);
    else
    if (steps == -1 ) ret = node.Move("RootNode");
    else ret = False;
    UnLock();
    return ret;
}


bool CDB::Lock(){
    return core->Lock();
}

void CDB::UnLock(){
    core->UnLock();
}



bool CDB::ReadArray (void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName, bool caseSensitive){
    if (!Lock()) return False;
    bool ret = node().ReadArray(configName,array,valueType,size,nDim,caseSensitive);

    UnLock();
    return ret;
}

bool CDB::WriteArray(const void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName,SortFilterFn *sorter){
    if (!Lock()) return False;

    bool ret = node().WriteArray(configName,array,valueType,size,nDim,CDBN_CreateStringNode,sorter);

    UnLock();
    return ret;
}



bool CDB::Delete(const char *configName){
    if (configName == NULL) return False;

    if (!Lock()) return False;
    CDBNodeRef cdbr;
    cdbr = node;
    bool ret = cdbr.Move(configName);


    if (ret) {
        if (cdbr().IsDataNode()){
            ret = cdbr.DeleteAndMoveToFather();
        } else {
            cdbr().DeleteSubTree();
            ret = True;
        }
    }

    UnLock();
    return ret;
}

bool CDB::Exists(const char *configName){
    if (configName == NULL) return False;

    if (!Lock()) return False;
    CDBNodeRef cdbr;
    cdbr = node;
    bool ret = cdbr.Move(configName);
    UnLock();
    return ret;
}

bool CDB::FindSubTree(const char *configName,CDBAddressMode cdbam){
    if (configName == NULL) return False;

    if (!Lock()) return False;

    bool ret = node.FindSubTreeAndMove(configName,cdbam);

    UnLock();
    return ret;
}

int CDB::Size(CDBAddressMode cdbam){
    if (!Lock()) return False;

    bool subTreeOnly = (cdbam & CDBAM_SubTreeOnly);
    bool leafsOnly   = (cdbam & CDBAM_LeafsOnly);

    if (node == coreRef) subTreeOnly = False;

    int ret = 0;
    if (subTreeOnly){
        ret = node().SubTreeSize(leafsOnly);
    } else {
        ret = coreRef().SubTreeSize(leafsOnly);
    }
    // remove root node
    if (!leafsOnly) ret = ret-1;

    UnLock();
    return ret;
}

int CDB::TreePosition(CDBAddressMode cdbam){
    if (!Lock()) return False;
    bool leafsOnly   = (cdbam & CDBAM_LeafsOnly);

    int ret = 0;
    ret = node().TreePosition(leafsOnly);

    UnLock();
    return ret;
}

bool CDB::TreeMove(int index,CDBAddressMode cdbam){

    if (!Lock()) return False;

    bool ret = 0;
    ret = node.TreeMove(index,cdbam);

    UnLock();
    return ret;
}

static int linkCounter = 0;

bool CDB::Link(const char *linkFrom,const char *linkTo,SortFilterFn *sorter){
    if (linkTo   == NULL) return False;
    if (linkFrom == NULL) return False;

    FString name;
    if (strlen(linkFrom) > 0){
        name.Printf("%s.link%i",linkFrom,linkCounter++);
    } else {
        name.Printf("link%i",linkCounter++);
    }

    if (!Lock()) return False;
    bool ret = node().WriteArray(name.Buffer(),&linkTo,CDBTYPE_String,NULL,1,CDBN_CreateLinkNode,sorter);
    UnLock();
    return ret;
}



//#################################################################################
//#################################################################################
//##                                                                             ##
//##        Stream I/O                                                           ##
//##                                                                             ##
//#################################################################################
//#################################################################################

bool CDB::ReadFromStream(StreamInterface &stream,StreamInterface *err,SortFilterFn *sorter){
    if (!Lock()) return False;
    bool ret = node().ReadFromStream(stream,err,parserReportEnabled,sorter);
    UnLock();
    return ret;
}

static inline void CDC_WriteIndent(Streamable &s,uint32 indentLevel){
    uint32 i;
    for (i=0;i<indentLevel;i++) s.Printf("    ");
}


bool CDB::WriteToStream(StreamInterface &stream,StreamInterface *err,CDBWriteMode mode){

    if (!Lock()) return False;

    Streamable *s= dynamic_cast<Streamable *>(&stream);
    if(s == NULL){
        UnLock();
        return False;
    }
    bool ret = node().ReadContent(s,CDBTYPE_CDBStyle,1,0,-1,mode);

    UnLock();
    return ret;
}


typedef bool(*CDBSRWReadFunction)(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err);

typedef bool(*CDBSRWWriteFunction)(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err);

struct TypeList{
public:
    const char *type;
    const char *modif;
    int  size;
    CDBSRWReadFunction  readFun;
    CDBSRWWriteFunction writeFun;
};

static bool CDBSRWReadNull(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    if (err) err->Printf("Cannot read %s of type %s\n",name,type);
    return False;
}

static bool CDBSRWReadString2(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){

    if (size2 > 1){
        if (err) err->Printf("CDBSRWReadString2:array of char[%i,%i] not supported (%s,%s)\n",size1,size2,address,name);
        return False;
    }

    FString value;
    bool ret = cdb.ReadArray(&value,CDBTYPE_FString,NULL,0,name);
    if (!ret){
        if (err) err->Printf("%s not found\n",name);
        return False;
    }

    int valueSize = value.Size();
    if (valueSize > (size1-1)){
        if (err) err->Printf("string %s too big : %i\n",name,valueSize);
        return False;
    }

    strncpy(address,value.Buffer(),valueSize);
    address[valueSize]=0;

    return True;

}

static bool CDBSRWReadString1(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    if ((size1 != 1)||(size2 !=1)){
        if (err) err->Printf("array of strings not supported\n",name);
        return False;
    }
    FString value;
    bool ret = cdb.ReadArray(&value,CDBTYPE_FString,NULL,0,name);
    if (!ret){
        if (err) err->Printf("%s not found\n",name);
        return False;
    }
    FString *s = (FString *)address;
    *s = value;
    return True;
}

static bool CDBSRWReadNumber(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    int sizes[2]={size1,size2};
    int dim=0;
    if (size1>1)dim=1; else size1 = 1;
    if (size2>1)dim=2; else size2 = 1;

    if (strcmp(type,"float")==0)        return cdb.ReadArray(address,CDBTYPE_float ,sizes,dim,name);
    if (strcmp(type,"double")==0)       return cdb.ReadArray(address,CDBTYPE_double,sizes,dim,name);
    if (strcmp(type,"int32")==0)        return cdb.ReadArray(address,CDBTYPE_int32 ,sizes,dim,name);
    if (strcmp(type,"uint32")==0)       return cdb.ReadArray(address,CDBTYPE_uint32,sizes,dim,name);
    if (strcmp(type,"unsigned int")==0) return cdb.ReadArray(address,CDBTYPE_uint32,sizes,dim,name);
    if (strcmp(type,"int")==0)          return cdb.ReadArray(address,CDBTYPE_int32 ,sizes,dim,name);
    if (strcmp(type,"unsigned long")==0)return cdb.ReadArray(address,CDBTYPE_uint32,sizes,dim,name);
    if (strcmp(type,"long")==0)         return cdb.ReadArray(address,CDBTYPE_int32 ,sizes,dim,name);
    return False;
}

static bool CDBSRWWriteNull(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    if (err) err->Printf("Cannot write %s of type %s\n",name,type);
    return False;
}
static bool CDBSRWWriteString1(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    if ((size1 != 1)||(size2 !=1)){
        if (err) err->Printf("array of strings not supported\n",name);
        return False;
    }
    FString *s = (FString *)address;
    const char *string = s->Buffer();
    cdb.WriteArray(&string,CDBTYPE_String,NULL,0,name);
    return True;
}
static bool CDBSRWWriteString2(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    if ((size2 > 1)){
        if (err) err->Printf("CDBSRWWriteString2: array of char[%i,%i] not supported (%s,%s)\n",size1,size2,address,name);
        return False;
    }
    FString value;
    if (value.Size()> (size1-1)){
        if (err) err->Printf("string %s too big : %i\n",name,value.Size());
        return False;
    }
    cdb.WriteArray(&address,CDBTYPE_String,NULL,0,name);
    return True;
}

static bool CDBSRWWriteNumber(CDB &cdb,char *address,int size1,int size2,const char *type,const char *name,Streamable *err){
    int sizes[2]={size1,size2};
    int dim=0;
    if (size1>1)dim=1; else size1 = 1;
    if (size2>1)dim=2; else size2 = 1;
    if (strcmp(type,"float")==0)            return cdb.WriteArray(address,CDBTYPE_float ,sizes,dim,name);
    if (strcmp(type,"double")==0)           return cdb.WriteArray(address,CDBTYPE_double,sizes,dim,name);
    if (strcmp(type,"int32")==0)            return cdb.WriteArray(address,CDBTYPE_int32 ,sizes,dim,name);
    if (strcmp(type,"uint32")==0)           return cdb.WriteArray(address,CDBTYPE_uint32,sizes,dim,name);
    if (strcmp(type,"unsigned int")==0)     return cdb.WriteArray(address,CDBTYPE_uint32,sizes,dim,name);
    if (strcmp(type,"int")==0)              return cdb.WriteArray(address,CDBTYPE_int32 ,sizes,dim,name);
    if (strcmp(type,"unsigned long")==0)    return cdb.WriteArray(address,CDBTYPE_uint32,sizes,dim,name);
    if (strcmp(type,"long")==0)             return cdb.WriteArray(address,CDBTYPE_int32 ,sizes,dim,name);
    return False;
}

static TypeList typeList[]={
    { "void"                ,"*",sizeof(void *            ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "char"                ,"*",sizeof(char *            ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "float"               ,"" ,sizeof(float             ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "double"              ,"" ,sizeof(double            ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "FString"             ,"" ,sizeof(FString           ),CDBSRWReadString1,CDBSRWWriteString1},
    { "char"                ,"" ,sizeof(char              ),CDBSRWReadString2,CDBSRWWriteString2},
    { "unsigned char"       ,"" ,sizeof(unsigned char     ),CDBSRWReadString2,CDBSRWWriteString2},
    { "int8"                ,"" ,sizeof(int8              ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "uint8"               ,"" ,sizeof(uint8             ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "int16"               ,"" ,sizeof(int16             ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "uint16"              ,"" ,sizeof(uint16            ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "short"               ,"" ,sizeof(short             ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "unsigned short"      ,"" ,sizeof(unsigned short    ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "int"                 ,"" ,sizeof(int               ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "unsigned int"        ,"" ,sizeof(unsigned int      ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "int32"               ,"" ,sizeof(int32             ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "uint32"              ,"" ,sizeof(uint32            ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "long"                ,"" ,sizeof(long              ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "long int"            ,"" ,sizeof(long int          ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "unsigned long"       ,"" ,sizeof(unsigned long     ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "unsigned long int"   ,"" ,sizeof(unsigned long int ),CDBSRWReadNumber ,CDBSRWWriteNumber },
    { "int64"               ,"" ,sizeof(int64             ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { "uint64"              ,"" ,sizeof(uint64            ),CDBSRWReadNull   ,CDBSRWWriteNull   },
    { ""                    ,"" ,0                         ,CDBSRWReadNull   ,CDBSRWWriteNull   }
};

bool CDB::WriteStructureEntry_private(ClassStructureEntry *cse,char *address,Streamable *err){
    int ix=0;
    while(typeList[ix].size>0){
        if ((strcmp(cse->type,typeList[ix].type)==0) && ((strcmp(cse->modif,typeList[ix].modif)==0))){
            return typeList[ix].writeFun(*this,address+cse->pos,cse->sizes[0],cse->sizes[1],cse->type,cse->name,err);
        }
        ix++;
    }
    bool ret=True;

    int size1 = cse->sizes[0];
    int size2 = cse->sizes[1];
    if((size1>1)||(size2>1)){
        if (size2>1){
            int i,j;
            int size = cse->size/(size1*size2);
            for (i =0;i<size1;i++){
                for (j =0;j<size2;j++){
                    FString sdbName;
                    sdbName.Printf("%s.%i.%i",cse->name,i,j);

                    AddChildAndMove(sdbName.Buffer());
                    if (ret)
                        ret = ret && WriteStructure(cse->type,address+cse->pos+size*(i*size2+j),NULL,err);
                    MoveToFather(3);
                }
            }
        } else {
            int i;
            int size = cse->size/size1;
            for (i =0;i<size1;i++){
                FString sdbName;
                sdbName.Printf("%s.%i",cse->name,i);

                AddChildAndMove(sdbName.Buffer());
                if (ret)
                    ret = ret && WriteStructure(cse->type,address+cse->pos+size*i,NULL,err);
                MoveToFather(2);
            }
        }
    } else {

        AddChildAndMove(cse->name);
        if (ret)
            ret = ret && WriteStructure(cse->type,address+cse->pos,NULL,err);
        MoveToFather(1);
    }

    return ret;
}


bool CDB::WriteStructure(const char *className,char *address,const char *variableName,Streamable *err){
    if ((className==NULL) || (address == NULL) || (className[0] == 0)) return False;
    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(className);
    if (ori==NULL){
        if (err) err->Printf("Can't find class %s\n",className);
        return False;
    }
    ClassStructure *cs = ori->structure;
    if (cs == NULL){
        if (err) err->Printf("Class %s has no structure information\n",className);
        return False;
    }

    ClassStructureEntry **members = cs->Members();
    if (members == NULL) {
        if (err) err->Printf("I am dealing badly with a BasicType called %s\n",className);
        return False;
    }

    bool ret = True;
    int ix=0;
    ClassStructureEntry *cse = members[ix];
    while(cse!=NULL){
        ret = ret && WriteStructureEntry_private(cse,address,err);
        ix++;
        cse = members[ix];
    }
    return ret;
}


bool CDB::ReadStructureEntry_private(ClassStructureEntry *cse,char *address,Streamable *err){
    int ix=0;
    while(typeList[ix].size>0){
        if ( (strcmp(cse->type,typeList[ix].type  )==0) &&
             (strcmp(cse->modif,typeList[ix].modif)==0)   ){
            return typeList[ix].readFun(*this,address+cse->pos,cse->sizes[0],cse->sizes[1],cse->type,cse->name,err);
        }
        ix++;
    }
    bool ret = True;

    int size1 = cse->sizes[0];
    int size2 = cse->sizes[1];
    if ((size1>1)||(size2>1)){
        if (size2>1){
            int i,j;
            int size = cse->size/(size1*size2);
            for (i =0;i<size1;i++){
                for (j =0;j<size2;j++){
                    FString sdbName;
                    sdbName.Printf("%s.%i.%i",cse->name,i,j);
                    AddChildAndMove(sdbName.Buffer());
                    if (ret)
                        ret = ret && ReadStructure(cse->type,address+cse->pos+size*(i*size2+j),err);
                    MoveToFather(3);
                }
            }
        } else {
            int i;
            int size = cse->size/size1;
            for (i =0;i<size1;i++){
                FString sdbName;
                sdbName.Printf("%s.%i",cse->name,i);

                AddChildAndMove(sdbName.Buffer());
                if (ret)
                    ret = ret && ReadStructure(cse->type,address+cse->pos+size*i,err);
                MoveToFather(2);

            }
        }
    } else {
        AddChildAndMove(cse->name);
        if (ret)
            ret = ret && ReadStructure(cse->type,address+cse->pos,err);
        MoveToFather(1);
    }
    return ret;
}

bool CDB::ReadStructure(const char *className,char *address,Streamable *err){
    if ((className==NULL) || (address == NULL) || (className[0] == 0)) return False;
    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(className);
    if (ori==NULL){
        if (err) err->Printf("Can't find class %s\n",className);
        return False;
    }
    ClassStructure *cs = ori->structure;
    if (cs == NULL){
        if (err) err->Printf("Class %s has no structure information\n",className);
        return False;
    }

    ClassStructureEntry **members = cs->Members();
    if (members == NULL) {
        if (err) err->Printf("I am dealing badly with a BasicType called %s\n",className);
        return False;
    }

    bool ret = True;
    int ix=0;
    ClassStructureEntry *cse = members[ix];
    while(cse!=NULL){
        ret = ret && ReadStructureEntry_private(cse,address,err);
        ix++;
        cse = members[ix];
    }
    return ret;
}

bool CDB::LoadFromEnvironment(char **env){
#if (defined(_VXWORKS) || defined(_SOLARIS) || defined(_RTAI) || defined(_MACOSX))
#else
    char **p = env;
    if (p == NULL) p = environ;
    while (*p != NULL){
        char *s = *p;
        uint32 index = 0;
        while((s[index] != '=')&&(s[index] != 0)) index++;
        if (s[index] == '='){
            s[index] = 0;
            WriteArray(&s[index+1],CDBTYPE_String,NULL,0,s);
            s[index] = '=';
        }
        p++;
    }
#endif
    return True;
}


void CDB::EnableParserReports(bool flag){
    parserReportEnabled = flag;
}

#if 0
/** The only function exported */
CDBVirtual *CreateCDB(CDBVirtual *base,SortFilterFn *sorter,CDBCreationMode cdbcm){
    CDB *baseCDB = NULL;
    if (base != NULL){
        baseCDB = dynamic_cast<CDB *>(base);
        if (baseCDB == NULL){
            CStaticAssertErrorCondition(FatalError,"CreateCDB(base,...): base is not a pointer to a valid CDB");
            return NULL;
        }
    }
    return new CDB(baseCDB,sorter,cdbcm);
}
#endif


