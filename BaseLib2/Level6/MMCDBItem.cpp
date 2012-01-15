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

#include "MMCDBItem.h"
#include "System.h"

OBJECTREGISTER(MMCDBItem,"$Id$")

static ClassStructure Pointer__CS__("*",sizeof(void *),0 ,NULL);


bool MMCDBISearchFilter::Test (LinkedListable *data){
    MMCDBItem *mmcdbi = dynamic_cast<MMCDBItem *>(data);
    if (mmcdbi == NULL) return False;
    return (name == mmcdbi->VariableName());
}

/** a class with a virtual table! */
class DummyVirtual{

public:
    /** */
    virtual ~DummyVirtual(){}
};




void MMCDBIPositionIterator::Do (LinkedListable *data){
    MMCDBItem *mmcdbi2 = dynamic_cast<MMCDBItem *>(data);
    if (mmcdbi2 == NULL) return ;
    if (mmcdbi == mmcdbi2){
        mmcdbi = NULL;
    } else {
        position++;
    }
}

/** copies all the dimensions that fit and collapses the last */
void MMCDBItem::CopyDimensions(int *destination, int maxDim,const int *source,int numberOfDimensions){

    int dimOffset = maxDim - numberOfDimensions;
    if (dimOffset > 0)dimOffset = 0;
    int i;
    for (i = (maxDim-1);i>(numberOfDimensions-1);i--){
        destination[i] = 0;
    }
    destination[0] = 1;
    for (i = (numberOfDimensions-1);i>=0;i--){
        int j = i + dimOffset;
        if (j >= 1)
            destination[j]  = source[i];
        else
            destination[0] *= source[i];
    }

}

bool MMCDBItem::CompareDimensions(const int *destination, int dSize,const int *source,int sSize){
    int dTemp[1] = { 1} ;
    int sTemp[1] = { 1} ;
    if (dSize == 0){
        destination = dTemp;
        dTemp[0] = 1;
        dSize = 1;
    }
    if (destination == NULL){
        destination = dTemp;
        dTemp[0] = dSize;
        dSize = 1;
    }
    if (sSize == 0){
        source = sTemp;
        sTemp[0] = 1;
        sSize = 1;
    }
    if (source == NULL){
        source = sTemp;
        sTemp[0] = sSize;
        sSize = 1;
    }
/*
    if ((destination != NULL) && (dSize >1)){
        int index = (dSize-1);
        while ((destination[index]==0)&& (index > 0)) {
            index--;
            dSize--;
        }
    }
*/
    int dCumulative = destination[0];
    int sCumulative = source[0];
    int i = (sSize-1);
    int j = (dSize);
    for (;(i >= 0)&&(j>=0);i--,j--){
        if ((j >= 1)&& (i >= 1))
            if (destination[j] != source[i]) return False;
        else
            if (j >=1) dCumulative *= destination[j];
        else
            if (i >=1) sCumulative *= source[j];
    }
    return (dCumulative == sCumulative);
}

MMCDBItem::MMCDBItem(){
    address         = NULL;
    arrayDimensions = NULL;
    classStructure  = NULL;
}

void MMCDBItem::CleanUp(){
    address        = NULL;
    if (arrayDimensions)   free((void *&)arrayDimensions);
    classStructure = NULL;
}


bool MMCDBItem::Init(
    void *                  address,
    const char *            className,
    const char *            variableName,
    int                     numberOfDimensions,
    int *                   dimensions){

    CleanUp();

    // do not accept bad pointer
    if (!MEMORYCheck(address,MTAM_Read | MTAM_Write)) return False;

    this->address      = address;
    this->className    = className;
    this->variableName = variableName;
    if ( (numberOfDimensions > 1) ||
         ((numberOfDimensions == 1) && (dimensions[0] > 1)) ){

        arrayDimensions         = (int *)malloc((numberOfDimensions+1)*sizeof(int));
        arrayDimensions[numberOfDimensions] = 0;
        CopyDimensions(arrayDimensions,numberOfDimensions,dimensions,numberOfDimensions);
    }

    if (IsPointer()) {
        classStructure = &Pointer__CS__;
    } else {
        classStructure  = NULL;
        ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(this->className.Buffer());
        if (ori != NULL){
            classStructure  = ori->structure;
        }
    }
    return True;
}




/** populate elements */
bool MMCDBItem::Populate(int index){

    UnPopulate();

    if (index >= 0){
        if (!IsArray()) return False;
        char name[32];
        sprintf(name,"%i",index);

        int elementSize = 4;
        if (!IsPointer()){
            if (!classStructure) return True;
            elementSize = classStructure->Size();
        }

        char *ptr = (char *)address;
        ptr += index * TotalSize(1) * elementSize;

        FString varName;
        varName.Printf("[%i]",index);

        MMCDBItem *mmcdbi = new MMCDBItem();
        if (mmcdbi->Init(
                ptr,
                className.Buffer(),
                varName.Buffer(),
                NumberOfArrayDimensions()-1,
                arrayDimensions+1)){

            elements.ListAdd(mmcdbi);
        } else delete mmcdbi;

        return True;
    }

    /** nop need */
    if (IsPointer() || IsArray()) return True;

    if (classStructure == NULL){
        AssertErrorCondition(Warning,"Populate: class %s structure unavailable ",className.Buffer());
        return True;
    }

    LinkedListHolder *listView = NULL;
    if (classStructure->flags & CSF_HasVirtual){

        DummyVirtual *dv = (DummyVirtual *)address;
        listView = dynamic_cast<LinkedListHolder *>(dv);

        if (listView){
            int index = 0;
            for (index = 0; index < listView->ListSize();index++){
                FString variableName;
                variableName.Printf("ListPeek(%i)",index);
                LinkedListable *p = listView->ListPeek(index);
                const char *typeIdClassName = typeid(*p).name();
                MMCDBItem *mmcdbi = new MMCDBItem();
                if (mmcdbi->Init(p,typeIdClassName+6,variableName.Buffer())){
                    elements.ListAdd(mmcdbi);
                } else delete mmcdbi;
            }
        }
    }

    if (classStructure->Members()){
        int index = 0;
        while(classStructure->Members()[index] != NULL){
            ClassStructureEntry *cse = classStructure->Members()[index];
            // deselect root and size for the linkedlistholders
            if ((listView == NULL) ||
                ((strcmp(cse->name,"llhRoot")!=0) &&
                 (strcmp(cse->name,"llhSize")!=0))){

                const char *modif = cse->modif;
                FString fullName;
                fullName = cse->name;
                FString fullType;
                fullType.Printf("%s%s",cse->type,modif);
                void *ptr = (((char *)address)+cse->pos);

                MMCDBItem *mmcdbi = new MMCDBItem();
                bool ret = mmcdbi->Init(
                                  ptr,
                                  fullType.Buffer(),
                                  fullName.Buffer(),
                                  cse->NumberOfDimensions(),
                                  cse->sizes);

                while(ret){
                    elements.ListAdd(mmcdbi);
                    if (mmcdbi->IsPointer()) {
                        fullName+="[0]";
                        fullType.SetSize(fullType.Size()-1);
                        modif++;
                        ptr = *((void **)ptr);
                        mmcdbi = new MMCDBItem();
                        ret = mmcdbi->Init(ptr,fullType.Buffer(),fullName.Buffer());

                    } else {
                        mmcdbi = NULL;
                        ret = False;
                    }
                }
                if (mmcdbi) delete mmcdbi;
            }
            index++;
        }
    }
    return True;
}

void MMCDBItem::Show(Streamable &s,const char *indent){
    s.Printf("%sAddress      = 0x%x\n",indent,address);
    s.Printf("%sClassName    = %s\n"  ,indent,className.Buffer());
    s.Printf("%sVariableName = %s\n"  ,indent,variableName.Buffer());

    FString newIndent;
    newIndent.Printf("%s    ",indent);
    if (NumberOfElements() > 0){
        int index;
        for (index = 0; index<NumberOfElements();index++){
            MMCDBItem *mmcdbi = GetElement(index);
            if (mmcdbi){
                s.Printf("Element[%i] = {\n",index);
                mmcdbi->Show(s,newIndent.Buffer());
                s.Printf("}\n",address);
            }
        }
    }
}




