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
 *@file
 * The implementation of a hierarchical database.
 *
 * It works similarly to a filesystem having directory nodes (branches) and file nodes (leaves).
 * Each leaf node is a container of a sequence of characters, both ASCII and not ascii.
 * CDB is infact a reference to an instance of the database. Copying CDBs does not duplicate
 * the database but increases a reference counter. Operation to the same database by different
 * threads is safe. 
 */
#if !defined(CDB_H)
#define CDB_H

#include "System.h"
#include "Iterators.h"
#include "FString.h"
#include "CDBNodeRef.h"
#include "Object.h"
#include "CDBVirtual.h"
#include "CDBExtended.h"

class CDB;
class CDBCore;
class CDB;

/** a string whose characters are separators */
extern const char *CDBSeparators;

OBJECT_DLL(CDB)

class CDB: public CDBVirtual{
    OBJECT_DLL_STUFF(CDB)

protected:
    /** the database core. It is the actual database */
    CDBCore                   *core;

    /** Pointer to the root node */
    CDBNodeRef                 coreRef;

    /** Pointer to the current node. It can point to branches or leaves */
    CDBNodeRef                 node;

    /** Whether parsing reports faults */
    bool                       parserReportEnabled;

    /** */
    virtual CDBCore *           GetCore()
    {
        return core;
    }

    /** */
    virtual CDBNodeRef *        GetNode()
    {
        return &node;
    }

public:

    /**
        Constructor of a CDB. If base is specified it creates a new reference to an existing database
        @param base        If NULL creates a new database. If not NULL it creates a reference to an existing one
        @param cdbcm       Use CDBCM_CopyAddress to copy the address from the reference.
                           In case of two or more flags, after oring one must cast back to the enum type  */
    CDB(CDB *base=NULL,CDBCreationMode cdbcm = CDBCM_None);

//##################################################################/
//##                                                              ##
//##            Primary   functions:   all virtual                ##
//##                                                              ##
//##################################################################/

    /** creates a new reference to a database, or if that is not possible it creates a copy
        @param cdbcm = CDBCM_CopyAddress ensures that the new object points at the same location  */
    CDBVirtual *Clone(CDBCreationMode cdbcm){
        return new CDB(this,cdbcm);
    }

    /** will destroy database if instance count goes to 0 */
    virtual ~CDB();

    /** */
    virtual bool CopyFrom(CDBVirtual *cdbv){
        // nothing to copy
        if (cdbv == NULL) return True;

        bool ret = True;
        int i;
        for (i = 0;i < cdbv->NumberOfChildren();i++){
            if (cdbv->MoveToChildren(i)){
                FString name;
                ret = ret && cdbv->NodeName(name);
                if ((ret) && (cdbv->NumberOfChildren() > 0)){
                    if (AddChildAndMove(name.Buffer())){
                        ret = ret && CopyFrom(cdbv);
                        ret = ret && MoveToFather();
                    } else ret = False;
                } else {
                    FString *value;
                    int arrayDimension = 1;
                    int arraySize[1];
                    if(!cdbv->GetArrayDims(arraySize, arrayDimension, "")){
                        return False;
                    }
                    if(arraySize[0] < 1){
                        return False;
                    }
                    value = new FString[arraySize[0]];
                    if(value == NULL){
                        return False;
                    }
                    ret = ret && cdbv->ReadArray(value,CDBTYPE_FString,(const int *)arraySize,arrayDimension,"");
                    if (ret) {
//                        const char *p = value.Buffer();
		      ret = ret && WriteArray(value,CDBTYPE_FString,(const int *)arraySize,arrayDimension,name.Buffer());
                    }
                    delete []value;
                }
                ret = ret && cdbv->MoveToFather();
            } else ret = False;
        }
        return ret;
    }

    /**
        remove all the content from the database
        unless some other database instance exists and points to a subtree
        one can reques to delete only the current subtree.
    */
    virtual void CleanUp(CDBAddressMode cdbam = CDBAM_FromRoot);

    /** locks the main database for exclusive access: use if a group of transactions should be atomic */
    virtual bool Lock();

    /** unlocks the main database: rememver unlocking a locked database after use */
    virtual void UnLock();

    /** finds the overall path leading to the current node
        it resizes the stream name to 0 abd then adds the name  */
    virtual bool SubTreeName(Streamable &name,const char *sep = ".");

    /** the name of the current node */
    virtual bool NodeName(BString &name);

    /** the type of the current node */
    virtual bool NodeType(BString &name);

    /** the function can be used to create a new subtree */
    virtual bool AddChildAndMove(const char *subTreeName,SortFilterFn *sorter=NULL);

    /** move to the specified location. The movement is relative to the current location */
    virtual bool Move(const char *subTreeName);

    /** negative number move up. >=0 chooses the subtree, 0 is the first of the children */
    virtual bool MoveToChildren(int childNumber=0);

    /** how many branches from this node. Negative number implies that the location is a leaf*/
    virtual int  NumberOfChildren();

    /**   0 means remain where you are, > 0 brothers on the right < 0 on the left */
    virtual bool MoveToBrother(int steps = 1);

    /** -1 = Root, one level up for each positive */
    virtual bool MoveToFather(int steps = 1);

    /** from node search on the right of the tree for the subtree identified by the string name.
        On success nodes points to the node containing the subtree or leaf
        Will not follow links.
        @param cdbam  CDBAM_SkipCurrent   search in the current subtree excluding current node
                      any other flag are NOT SUPPORTED  */
    virtual bool FindSubTree(const char *configName,CDBAddressMode cdbam = CDBAM_None);

    /** The total number of nodes.
        @param cdbam CDBAM_SubTreeOnly allows to measure the current subtree
                     CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int  Size(CDBAddressMode cdbam = CDBAM_SubTreeOnly | CDBAM_LeafsOnly);

    /** in the left to right bottom to top order the absolute location of a node
        @param cdbam CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int  TreePosition(CDBAddressMode cdbam = CDBAM_LeafsOnly);

    /** move to a location within the whole (sub)tree.
        The nodes are numbered from left to right and from subnode to supernode
        @param cdbam if it contains CDBAM_LeafsOnly   it will not account the group nodes
                     if it contains CDBAM_SubTreeOnly addresses only the subtree
                     if it contains CDBAM_Relative    it is a step from current location
        If the node does not exist returns False and remains in the start position   */
    virtual bool TreeMove(int index,CDBAddressMode cdbam = CDBAM_LeafsOnly);

    /** if cdbaim is CDBAIM_Rigid, it expects all the indexes to be found form 0 to n-1 and the same number in all subtrees. */
    virtual bool GetArrayDims(int *size,int &maxDim,const char *configName,CDBArrayIndexingMode cdbaim = CDBAIM_Flexible, bool caseSensitive = True);

    /** if size == NULL it treats the input as a monodimensional array of size nDim
        if size does not agree with the actual dimensions of the matrix the routine will fail   */
    virtual bool ReadArray (void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName, bool caseSensitive=True);

    /** if size == NULL it treats the input as a monodimensional array of size nDim */
    virtual bool WriteArray(const void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName,SortFilterFn *sorter=NULL);

    /** remove a leaf or a subtree (position is relative!).
        A leaf is removed nad then the pointer is moved to the father
        A specified node's children are deleted, but not the node.
        The pointer still points at the node
        to delete a link use the linkTo as the leaf name
        to delete a subtree simply specify the group node  */
    virtual bool Delete(const char *configName);

    /** whether a certain entry exists */
    virtual bool Exists(const char *configName);

    /** linkFrom is the path where the link is created from */
    virtual bool Link(const char *linkFrom,const char *linkTo,SortFilterFn *sorter=NULL);


//##################################################################/
//##                                                              ##
//##            Complex load/save functions                       ##
//##                                                              ##
//##################################################################/

    /** Reads a database from a stream */
    virtual bool ReadFromStream(StreamInterface &stream,StreamInterface *err=NULL,SortFilterFn *sorter=NULL);

    /** enable reports of parser during ReadFromStream into err */
    virtual void EnableParserReports(bool flag);

    /** writes the database to stream without any ordering */
    virtual bool WriteToStream(StreamInterface &stream,StreamInterface *err=NULL,CDBWriteMode mode=CDBWM_Tree);

    /** load from environment or from any NULL terminated list of chars */
    virtual bool LoadFromEnvironment(char **env);

    /** from CDB to memory */
    virtual bool ReadStructure (const char *className,char *address,Streamable *err=NULL);

    /** either copies or references a memory structure,
        at address @param address and of type @param className
        CDB transforms the memory, MMCDB just references it  */
    virtual bool WriteStructure(const char *className,char *address,const char *variableName=NULL,Streamable *err=NULL);

//##################################################################/
//##                                                              ##
//##            private functions                                 ##
//##                                                              ##
//##################################################################/

private:
    /* */
    void Init(CDB *base=NULL,const char *subTreeName=NULL,SortFilterFn *sorter=NULL,bool copyLocation=False);

    /* */
    bool ReadStructureEntry_private(ClassStructureEntry *cse,char *address,Streamable *err);

    /* */
    bool WriteStructureEntry_private(ClassStructureEntry *cse,char *address,Streamable *err);

};

#endif



