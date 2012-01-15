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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <api.h>
#include <ertti.h>
#include <winsock.h>
#include <math.h>

struct uint64{
     unsigned int data[2];
};

struct int64{
      int data[2];
};

#define volatile

#include "temp/temp.pp"

class ClassInfoRecord;
ClassInfoRecord *listedClasses = NULL;
ClassInfoRecord *FindClass(char *name);

char *strdup(char *s){
    if (s==NULL) return NULL;
    int sz = strlen(s);
    char *ns = (char *)malloc(sz+1);
    strcpy(ns,s);
    return ns;
}


const int CMR_Const         = 0x1;

const int CMR_Static        = 0x2;

const int CM_HasVirtual     = 0x1;


class ClassMemberRecord{
public:
    //
    ClassMemberRecord *next;
    //
    char *name;
    //
    char *nameCorrected;
    //
    int sizes[4];
    //
    int flags;
    //
    char *type;
    //
    char *modif;
    //
    ClassMemberRecord(char *name,char *type,char *modif,int flags,int size0,int size1,int size2,int size3){
        this->flags = flags;
        this->name = strdup(name);
        this->type = strdup(type);
        this->modif = strdup(modif);
        nameCorrected = strdup(name);
        char *c = nameCorrected;
        while(*c != 0) {
            if (*c==':') *c='_';
            c++;
        }

        sizes[0] = size0;
        sizes[1] = size1;
        sizes[2] = size2;
        sizes[3] = size3;
        next = NULL;
    }
};

class ClassInfoRecord{
public:
    //
    ClassInfoRecord *next;
    //
    char *className;
    // avoid ::
    char *classNameCorrected;
    //
    ClassMemberRecord *list;
    //
    int flags;
    //
    ClassInfoRecord(char *name){
        flags = 0;
        className = strdup(name);
        classNameCorrected = strdup(name);
        char *c = classNameCorrected;
        while(*c != 0) {
            // Replace '::' in the class Scope, '<' and '>' in the Template definition with '_'
            if ((*c==':')||(*c=='<')||(*c=='>')) *c='_';
            c++;
        }
        list = NULL;
    }
    //
    void AddMember(char *name,char *type,char *modif,int flags,int size0,int size1,int size2,int size3){
        ClassMemberRecord *cmr= new ClassMemberRecord(name,type,modif,flags,size0,size1,size2,size3);
        if (list == NULL){
            list = cmr;
            return;
        }
        ClassMemberRecord *p = list;
        while(p->next != NULL){
            p = p->next;
        }
        p->next = cmr;
    }

    void Print(){
        ClassMemberRecord *p = list;
        int no = 0;
        while(p!=NULL){
            char flagsString[64];
            sprintf(flagsString,"0 ");
            if (p->flags & CMR_Const)  strcat(flagsString,"| CSE_Const");
            if (p->flags & CMR_Static) strcat(flagsString,"| CSE_Static");
            printf("static ClassStructureEntry %s_%s_CSE_EL(\"%s\",\"%s\",%i,%i,%i,%i,%s",classNameCorrected,p->name,p->type,p->modif,p->sizes[0],p->sizes[1],p->sizes[2],p->sizes[3],flagsString);
            if ((p->flags & CMR_Const) || (p->flags & CMR_Static))
                printf(",\"%s\",msizeof(%s,%s),(int)&%s::%s);\n",p->name,className,p->name,className,p->name);
            else
                printf(",\"%s\",msizeof(%s,%s),indexof(%s,%s));\n",p->name,className,p->name,className,p->name);

            no++;
            p = p->next;
        }
        // skip strange things that hace no members!!
        if (no == 0) return;

        printf("static ClassStructureEntry * %s__CSE__[] = {\n",classNameCorrected);
        p = list;
        while(p!=NULL){
            printf("    &%s_%s_CSE_EL",classNameCorrected,p->name);
            printf(",\n");
            p = p->next;
        }
        printf("    NULL\n");
        printf("};\n");

        char flagsString[64];
        sprintf(flagsString,"0 ");
        if (flags & CM_HasVirtual)  strcat(flagsString,"| CSF_HasVirtual");
        printf("ClassStructure %s__CS__(\"%s\",sizeof(%s),%s,%s__CSE__);\n",classNameCorrected,className,className,flagsString,classNameCorrected);
        printf("STRUCTREGISTER(\"%s\",%s__CS__)\n",className,classNameCorrected);

    }
};

ClassInfoRecord dummy("known");

ClassInfoRecord *FindClass(char *name){
    if ( (strcmp(name,"float")==0)         ||
         (strcmp(name,"unsigned int")  ==0)||
         (strcmp(name,"int")  ==0)         ||
         (strcmp(name,"short") ==0)         ||
         (strcmp(name,"unsigned short") ==0)||
         (strcmp(name,"long") ==0)         ||
         (strcmp(name,"uint64") ==0)    ||
         (strcmp(name,"int64") ==0)    ||
         (strcmp(name,"unsigned long") ==0)||
         (strcmp(name,"char") ==0)         ||
         (strcmp(name,"void") ==0)         ||
         (strcmp(name,"unsigned char") ==0)||
         (strcmp(name,"double")==0)) return &dummy;
    if (listedClasses == NULL) return NULL;
    ClassInfoRecord *p =listedClasses;
    while(p!= NULL){
        if (strcmp(p->className,name)==0) return p;
        p = p->next;
    }
    return NULL;
}

ClassInfoRecord *AddClass(char *name){
    if (FindClass(name) != NULL) return NULL;
    ClassInfoRecord *p = new ClassInfoRecord(name);
    p->next = listedClasses;
    listedClasses = p;
}


void BrowseClassComponents (char *className,ClassInfoRecord *oldClass=NULL);

void BrowseClassComponents (char *className,ClassInfoRecord *oldClass){

    // skip directories!
    char *c = className;
    while(*c!=0){
        if ((*c=='/') || (*c=='\\')) className = c+1;
        c++;
    }
    G__DataMemberInfo        datainfo(className);
    ClassInfoRecord *newClass = oldClass;
    if (newClass == NULL) newClass = AddClass(className);
    if (newClass != NULL){
        G__ClassInfo classinfo(className);
        G__BaseClassInfo baseinfo(classinfo);
        while(baseinfo.Next()) {
            BrowseClassComponents (baseinfo.Name(),newClass);
        }

        bool mainFlags = 0;
        while(datainfo.Next()) {
            G__TypeInfo *typeinfo=datainfo.Type();

            if (strcmp("G__virtualinfo",datainfo.Name())==0 ){
                mainFlags |= CM_HasVirtual;
            }
            if ((strcmp(datainfo.Name(),"G__virtualinfo" )!=0 ) &&
                (strcmp(typeinfo->Name(),"unknown")!=0)){
                int flags = 0;
                if (datainfo.Property()&G__BIT_ISSTATIC)   flags |= CMR_Static;
                if ((datainfo.Property()&G__BIT_ISCONSTANT) &&
                    (!(datainfo.Property()&G__BIT_ISPOINTER)) ) flags |= CMR_Const;


 //            if((~(datainfo.Property()&G__BIT_ISSTATIC)) &&
 //               (~(datainfo.Property()&G__BIT_ISCONSTANT)) &&
 //               (strcmp(datainfo.Name(),"G__virtualinfo" )!=0 ) &&
 //               (strcmp(typeinfo->Name(),"unknown")!=0)){

                int sizes[4]= {0,0,0,0};
                int i;
                int nDims = datainfo.ArrayDim();
                int offset = (nDims-4);
                if (offset<0) offset = 0;
                for(i = (nDims-1);i>=0;i--){
                    if (i<offset)   sizes[0]  *= datainfo.MaxIndex(i);
                    else            sizes[i-offset]  = datainfo.MaxIndex(i);
                }

                char *typeName = strdup(typeinfo->Name());
                char *p = typeName;
                while((*p != 0) && (*p != '*') && (*p != '&')) p++;
                char *modifName = strdup(p);
                *p = 0;
//printf("%s %s %s %x\n",datainfo.Name(),typeName,modifName,datainfo.Property());

                newClass->AddMember(datainfo.Name(),typeName,modifName,flags,sizes[0],sizes[1],sizes[2],sizes[3]);
                BrowseClassComponents(typeName);
            }

        }
        newClass->flags |= mainFlags;

    }
}

void CreateDefinitionFile (char *className){
    printf ("#define protected public\n");
    printf ("#define private public\n");
    printf ("#include \"%s.h\"\n",className);
    printf ("#include \"ObjectRegistryItem.h\"\n");
    printf ("#include \"ClassStructure.h\"\n");
    printf ("#include \"ObjectMacros.h\"\n");

    ClassInfoRecord *p = listedClasses;
    while(p!=NULL){
        p->Print();
        p = p->next;
    }
    printf("ClassStructure * %s_sinfo[] = {\n",className);
    ClassInfoRecord *p = listedClasses;
    while(p!=NULL){
        int no = 0;
        ClassMemberRecord *q = p->list;
        while(q!=NULL){
            no++;
            q = q->next;
        }
        if (no != 0) printf("    &%s__CS__,\n",p->classNameCorrected);
        p = p->next;
    }
    printf("    NULL\n");
    printf("};\n");

}

int main(int argc,char **argv){

    if (argc!=2){
        printf("syntax: test className\n");
        return -1;
    }

    BrowseClassComponents(argv[1]);
    CreateDefinitionFile (argv[1]);
    return 0;
}

