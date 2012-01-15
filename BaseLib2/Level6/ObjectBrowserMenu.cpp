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

#include "ObjectBrowserMenu.h"
#include "ClassStructureEntry.h"
#include "ObjectRegistryDataBase.h"
#include "RTMatrix.h"

void OBMSetup(ObjectBrowserMenu &obm,const char *className,const char *name,void *data){
    if (obm.className) free ((void *&)obm.className);
    if (className)  obm.className = strdup(className);
    if (obm.name) free ((void *&)obm.name);
    if (name)       obm.name = strdup(name);
    obm.data = data;
    if (obm.className){
        FString title;
        title.Printf("%s ",obm.name);
        obm.SetTitle(title.Buffer());
    }
}

typedef void (*TypeListFn)(void *data,Streamable &out);

struct TypeList{
public:
    const char *type;
    const char *modif;
    int  size;
    TypeListFn display;
};

void int_TypeListFn(void *data,Streamable &out){
    int *p = (int *)data;
    out.Printf("%i",p[0]);
}
void long_TypeListFn(void *data,Streamable &out){
    long *p = (long *)data;
    out.Printf("%i",*p);
}
void longlong_TypeListFn(void *data,Streamable &out){
    long *p = (long *)data;
    out.Printf("0x%x%x",p[0],p[1]);
}
void short_TypeListFn(void *data,Streamable &out){
    short *p = (short *)data;
    out.Printf("%i",*p);
}
void char_TypeListFn(void *data,Streamable &out){
    char *p = (char *)data;
    out.Printf("%i",*p);
}
void string_TypeListFn(void *data,Streamable &out){
    char **p = (char **)data;
    out.Printf("%s",*p);
}
void FString_TypeListFn(void *data,Streamable &out){
    FString *p = (FString *)data;
    out.Printf("%s",p->Buffer());
}
void double_TypeListFn(void *data,Streamable &out){
    double *p = (double *)data;
    out.Printf("%e",*p);
}
void float_TypeListFn(void *data,Streamable &out){
    float *p = (float *)data;
    out.Printf("%e",*p);
}
void void_TypeListFn(void *data,Streamable &out){
    int*p = (int*)data;
    out.Printf("0x%p",*p);
}
static TypeList typeList[]={
    { "void"            ,"*",sizeof(void *        ),void_TypeListFn},
    { "char"            ,"*",sizeof(char *        ),string_TypeListFn},
    { "float"           ,"" ,sizeof(float         ),float_TypeListFn},
    { "double"          ,"" ,sizeof(double        ),double_TypeListFn},
    { "FString"         ,"" ,sizeof(FString       ),FString_TypeListFn},
    { "char"            ,"" ,sizeof(char          ),char_TypeListFn},
    { "unsigned char"   ,"" ,sizeof(unsigned char ),char_TypeListFn},
    { "int8"            ,"" ,sizeof(int8          ),char_TypeListFn},
    { "uint8"           ,"" ,sizeof(uint8         ),char_TypeListFn},
    { "int16"           ,"" ,sizeof(int16         ),short_TypeListFn},
    { "uint16"          ,"" ,sizeof(uint16        ),short_TypeListFn},
    { "short"           ,"" ,sizeof(short         ),short_TypeListFn},
    { "unsigned short"  ,"" ,sizeof(unsigned short),short_TypeListFn},
    { "int"             ,"" ,sizeof(int           ),int_TypeListFn},
    { "unsigned int"    ,"" ,sizeof(unsigned int  ),int_TypeListFn},
    { "int32"           ,"" ,sizeof(int32         ),long_TypeListFn},
    { "uint32"          ,"" ,sizeof(uint32        ),long_TypeListFn},
    { "long"            ,"" ,sizeof(long          ),long_TypeListFn},
    { "unsigned long"   ,"" ,sizeof(unsigned long ),long_TypeListFn},
    { "int64"           ,"" ,sizeof(int64         ),longlong_TypeListFn},
    { "uint64"          ,"" ,sizeof(uint64        ),longlong_TypeListFn},
    { ""                ,"" ,0,NULL}
};

bool OBMBrowseVectors(Streamable &in,Streamable &out,char *data,const char *className,const char *modif,const char *name,int size0,int size1,int size2);

class simple{
public:
    virtual ~simple(){}
};


// True if immediately displayable
bool OBMEvaluateType(Streamable &in,Streamable &out,const char *type,const char *modif,int size0,int size1,int size2,const char *name,char *data,bool visit){
    if (size0 != 0){
        if (visit) return OBMBrowseVectors(in,out,data,type,modif,name,size0,size1,size2);
        out.Printf("%36s %25s ",type,name);
        int i = 3;
        if (size0>0) out.Printf("[%i]",size0);
        if (size1>0) out.Printf("[%i]",size1);
        if (size2>0) out.Printf("[%i]",size2);
        out.Printf("\n");
        return True;
    }
    int ix = 0;
    while (typeList[ix].type[0] != 0){
        if ((strcmp(type,typeList[ix].type)==0) && (strcmp(modif,typeList[ix].modif)==0)){
            out.Printf("%34s%2s %25s ",type,modif,name);
            typeList[ix].display(data,out);
            out.Printf("\n");
            return False;
        }
        ix++;
    }
    // treat unknown types
    if ((modif[0] == '*') || (modif[0] == '&')){
        char **p = (char **)data;
        if (*p == NULL){
            out.Printf("%34s%2s %25s NULL\n",type,modif,name);
            return True;
        }

//#if (defined(_VXWORKS) || defined(_SOLARIS) || defined(_LINUX))

//#else
        // check for virtual
        ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(type);
        // only if a class * not a class **...
        if ((modif[1]==0) &&(ori != NULL)&&(ori->structure != NULL)) {
            if (ori->structure->flags & CSF_HasVirtual){
                simple *s = (simple *)(*p);
                const char *className = typeid(*s).name();
                // skip class or struct;
                if (strncmp(className,"class ",6)==0) className+=6;
                if (strncmp(className,"struct ",7)==0) className+=7;

                if (visit) return OBMBrowseVectors(in,out,*p,(char *)className,modif+1,name,-1,0,0);
                else out.Printf("(%32s)%2s %25s %p\n",(char *)className,modif,name,*p);
                return True;

            }
        }
//#endif
        if (visit) return OBMBrowseVectors(in,out,*p,type,modif+1,name,-1,0,0);
        else out.Printf("%34s%2s %25s %p\n",type,modif,name,*p);
        return True;
    }
    if (strcmp(type,"RTMatrix")==0){
        RTMatrixF *rt = (RTMatrixF *)data;
        if (visit) return OBMBrowseVectors(in,out,(char *)rt->Data(),"float","",name,rt->NColumns(),rt->NRows(),0);
        else out.Printf("%36s %25s\n",type,name);
        return True;
    }

    if (visit) return OBMBrowse(in,out,data,type,name);
    else out.Printf("%36s %25s \n",type,name);
    return True;
}


bool OBMBrowse(Streamable &in,Streamable &out,char *data,const char *className,const char *name){

    ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(className);
    if ((ori == NULL)||(ori->structure == NULL)) {
        out.Printf(" structure of class %s unknown\n",className);
        return False;
    }
    ClassStructure *cs = ori->structure;

    int nOfElements = 0;
    ClassStructureEntry **cse = cs->Members();
    if (cse != NULL){
        while(cse[nOfElements]!= NULL) nOfElements++;
    }

    char buffer[128];
    uint32 size = sizeof(buffer);

    int position = 0;
    while(1){
        out.Printf(
        "###############################################################################\n"
        "##     %s (%s at %p)                           \n"
        "###############################################################################\n"
        ,name,className,data);

        int index = 0;
        char c = 'A';
        while((cse[index+position]!=NULL)&&(index<16)){
            ClassStructureEntry *csep = cse[index+position];
            FString tempor;
            bool ret = OBMEvaluateType(in,tempor,csep->type,csep->modif,csep->sizes[0],csep->sizes[1],csep->sizes[2],csep->name,data+csep->pos,False);
            if (ret){
                out.Printf("[%c] %s",c,tempor.Buffer());
            } else {
                out.Printf("  %s",tempor.Buffer());
            }
            c++;
            index++;
        }

        out.Printf("###############################################################################\n");
        out.Printf("0: EXIT");
        if ((nOfElements-position)>16) out.Printf(" >: UP");
        if (position>0) out.Printf("<: DOWN");
        out.Printf("\n");

        size = sizeof(buffer);
        if (!in.StreamInterface::GetLine(buffer,size,False)) SleepMsec(100);
        else
        if (strlen(buffer)>0){
            char command = toupper(buffer[0]);
            if (command == '0') {
                return True;
            } else
            if (command == '>') {
                position += 8;
                if ((nOfElements-position)<16) position = nOfElements - 16;
                if (position<0) position = 0;
            } else
            if (command == '<') {
                position -= 8;
                if (position<0) position = 0;
            } else {
                int index = command - 'A' + position;
                if ((index >= 0) && (index < nOfElements)){
                    ClassStructureEntry *csep = cse[index];
                    OBMEvaluateType(in,out,csep->type,csep->modif,csep->sizes[0],csep->sizes[1],csep->sizes[2],csep->name,data+csep->pos,True);
                }
            }
        }
    }
}

bool OBMBrowseVectors(Streamable &in,Streamable &out,char *data,const char *className,const char *modif,const char *name,int size0,int size1,int size2){
    if (size0 == 0) return OBMBrowse(in,out,data,className,name);

    char buffer[128];
    uint32 size = sizeof(buffer);

    // True if type belongs to basic types (int .. float...)
    bool isBasicType = False;
    int dataSize = 0;
    // if a pointer...
    if (modif[0] != 0) dataSize = sizeof(void *);
    else {
        int ix = 0;
        while ((typeList[ix].type[0] != 0)&&(dataSize == 0)){
            if ((strcmp(className,typeList[ix].type)==0) && (strcmp(modif,typeList[ix].modif)==0)){
                dataSize = typeList[ix].size;
        isBasicType = True;
            }
            ix++;
        }

        if (dataSize == 0 ){

            ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(className);
            if ((ori == NULL)||(ori->structure == NULL)) {
                out.Printf(" structure of class %s unknown\n",className);
                return False;
            }
            ClassStructure *cs = ori->structure;
            dataSize = cs->Size();
        }
    }

    int position = 0;
    while(1){
        FString fullName;
        fullName.Printf("%s",name);
        if (size2>0) fullName.Printf("[%i]",size1);
        if (size1>0) fullName.Printf("[%i]",size2);

        out.Printf(
        "###############################################################################\n"
        "##     %s (%s at %p)                           \n"
        "###############################################################################\n"
        ,fullName.Buffer(),className,data);

        int index = 0;
        char c = 'A';

        while((index<16) && (((index+position) < size0) || (size0 == -1))){
            FString tempor;
            FString fullName;
            fullName.Printf("%s",name);
            fullName.Printf("[=%i]",index+position);
            bool ret = OBMEvaluateType(in,tempor,className,modif,size1,size2,0,fullName.Buffer(),data+(index+position)*dataSize,False);
            if (ret){
                out.Printf("[%c] %s",c,tempor.Buffer());
            } else {
                out.Printf("0x%p  %s",data+(index+position)*dataSize,tempor.Buffer());
            }

            c++;
            index++;
        }

        out.Printf("###############################################################################\n");
        out.Printf("0: EXIT");

        if (((size0-position)>16)||(size0==-1)) out.Printf(" >: UP");
        if (position>0) out.Printf("<: DOWN");
        out.Printf("\n");

        size = sizeof(buffer);
        if (!in.StreamInterface::GetLine(buffer,size,False)) SleepMsec(100);
        else
        if (strlen(buffer)>0){
            char command = toupper(buffer[0]);
            if (command == '0') {
                return True;
            } else
            if (command == '>') {
                position += 8;
                if (((size0-position)<16)&&(size0 != -1)) position = size0 - 16;
                if (position<0) position = 0;
            } else
            if (command == '<') {
                position -= 8;
                if (position<0) position = 0;
            } else {
                int index = command - 'A' + position;
                if ((index >= 0) && ((index < size0)||(size0==-1))){
                    FString fullName;
                    fullName.Printf("%s",name);
                    fullName.Printf("[%i]",index+position);
            int dataOffset = (index+position)*dataSize;
            // if float pippo[10][4] --> data = data + dataSize*4*index
            if(isBasicType){
            if(size2==0){
                if(size1 == 0){
                dataOffset *= size0;
                }else{
                dataOffset *= size1;
                }
            }else{
                dataOffset *= size2*size1;
            }
            }
                    OBMEvaluateType(in,out,className,modif,size1,size2,0,fullName.Buffer(),data+dataOffset,True);

                }
            }
        }
    }
}

