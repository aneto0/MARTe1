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

/**
 * @file 
 * @brief Memory mapped CDB Item 
 */
#include "LinkedListHolder.h"
#include "LinkedListable.h"
#include "Object.h"
#include "FString.h"
#include "CDBVirtual.h"
#include "ObjectRegistryDataBase.h"
#include "Iterators.h"
#include "StackHolder.h"
#include "StaticStackHolder.h"


#if !defined (MEMORYMAP_CDB_ITEM_H)
#define MEMORYMAP_CDB_ITEM_H

class MMCDBItem;

/** serch a member of a given name */
class MMCDBISearchFilter: public SearchFilter{

    /** the name to search for */
    FString name;
public:

    /** */
    MMCDBISearchFilter(const char *name){
        this->name = name;
    }

    virtual ~MMCDBISearchFilter(){}

    /** the function that performs the search on a set of data */
    virtual bool Test (LinkedListable *data);

};

/** find the node position */
class MMCDBIPositionIterator: public Iterator{
    /** */
    int position;

    /** the object to search for, NULL after found! */
    MMCDBItem *mmcdbi;
public:
    /** */
    MMCDBIPositionIterator(MMCDBItem *mmcdbi){
        this->mmcdbi = mmcdbi;
        position = 0;
    }

    /** the function that performs the search on a set of data */
    virtual void Do (LinkedListable *data);

    /** -1  means not found*/
    int Position(){
        if (mmcdbi != NULL) return -1;
        return position;
    }
};



/** Used by MMCDB as element of a path. It is a contianer of elements of the same type */
OBJECT_DLL(MMCDBItem)
class MMCDBItem: public LinkedListable,public Object{
    OBJECT_DLL_STUFF(MMCDBItem)

    friend class MMCDB;

private: // loaded by Init()
    /** the address of the variable */
    void *                  address;

    /** the class name of the variable */
    FString                 className;

    /** the name of the variable */
    FString                 variableName;

    /** NULL or 0 terminated list of dimensions */
    int *                   arrayDimensions;

    /** the class structure associated to the class */
    const ClassStructure *  classStructure;

private: // loaded by Populate()
    /** list of MMCDBItem */
    LinkedListHolder        elements;

private:

    /** copies all the dimensions that fit and collapses the last */
    static void CopyDimensions(
        int *                   destination,
        int                     maxDim,
        const int *             source,
        int                     numberOfDimensions);

    /** dimension must be the same or comaptible */
    static bool CompareDimensions(
        const int *             destination,
        int                     maxDim,
        const int *             source,
        int                     sSize);

    /** deallocates memory */
    void CleanUp();
public:
    /** */
    MMCDBItem();

    /** */
    virtual ~MMCDBItem(){
        CleanUp();
    }

    /** the last three parameters are only necessary to handle packed structures */
    bool Init(
        void *                  address,
        const char *            className,
        const char *            variableName,
        int                     numberOfDimensions=0,
        int *                   dimensions=NULL);

    /** in case of a structure creates virtaul view of the structure
        in case of an arry does nothing
        if (index > 0) then it will populate one array row.   */
    bool Populate(int index = -1);

    /** de-populate elements */
    bool UnPopulate(){
        elements.CleanUp();
        return True;
    }

public:

    /** whether this is a pointer*/
    bool IsPointer() {
        if (className == "const char*" ) return False;
        if (className == "char*" ) return False;
        int pos = className.Size()-1;
        char c = className.Buffer()[pos];
        return (c=='*') || (c=='&');
    }

    /** */
    bool IsArray() {
        return (arrayDimensions != NULL);
    }

    /** */
    int NumberOfElements(){
        if (IsArray()) return arrayDimensions[0];
        return elements.ListSize();
    }

    /** returns also fake entries for pointers */
    MMCDBItem * GetElement(int index){
        if (IsArray()){
            Populate(index);
            char name[32];
            sprintf(name,"[%i]",index);
            MMCDBISearchFilter mmcdbisf(name);
            return dynamic_cast<MMCDBItem *>(elements.ListSearch(&mmcdbisf));
        }
        return dynamic_cast<MMCDBItem *>(elements.ListPeek(index));
    }

    /** returns also fake entries for pointers */
    MMCDBItem * GetElement(const char *name){
        if (name == NULL) return NULL;
        int nameLen = strlen(name);

        if (IsArray() && (name[0] == '[') && (name[nameLen-1] == ']')){

            int index = atoi(name+1);
            return GetElement(index);
        }
        MMCDBISearchFilter mmcdbisf(name);
        return dynamic_cast<MMCDBItem *>(elements.ListSearch(&mmcdbisf));
    }

    /** -1 for not found*/
    int GetElementPosition(MMCDBItem *mmcdbi ){
        if (mmcdbi == NULL) return -1;
        MMCDBIPositionIterator mmcdbipi(mmcdbi);
        elements.ListIterate(&mmcdbipi);
        return mmcdbipi.Position();
    }

    /** */
    void Show(Streamable &s,const char *indent);

    /** */
    const char *ClassName() const{
        return className.Buffer();
    }

    /** */
    const char *VariableName() const{
        return variableName.Buffer();
    }

    /** */
    void *Address() const{
        return address;
    }

    /** cannot be broken down into smaller parts or we don't need to */
    bool IsFinal() {
        if (className == "FString" ) return True;
        return (NumberOfElements() == 0);
    }

    /** */
    int NumberOfArrayDimensions(){
        if (arrayDimensions == NULL) return 0;
        int ix = 0;
        while (arrayDimensions[ix] > 0) ix++;
        return ix;
    }

    /** */
    int *GetArrayDimensions(){
        return arrayDimensions;
    }

    /** total number of elements of array */
    int TotalSize(int fromDim=0){
        if (arrayDimensions == NULL) return 1;
        int size = 1;
        int ix = fromDim;
        while (arrayDimensions[ix] > 0){
            size *= arrayDimensions[ix];
            ix++;
        }
        return size;
    }

    /** for elementary types whether this is a float or double*/
    bool IsFloat(){
        if (className == "float") return True;
        if (className == "double") return True;
        return False;
    }

    /** in bytes */
    int ElementSize(){
        if (classStructure) return classStructure->Size();
        return -1;
    }

};



#endif


