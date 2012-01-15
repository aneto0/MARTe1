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
 * The interface to a hierarchical database.
 *
 * It works similarly to a filesystem having directory nodes (branches) and file nodes (leaves).
 * Each leaf node is a container of a sequence of characters, both ASCII and not ascii.
 * CDB is infact a reference to an instance of the database. Copying CDBs does not duplicate
 * the database but increases a reference counter. Operation to the same database by different
 * threads is safe. 
 * Use CreateCDB to create an instance of CDB
 */
#if !defined(CONFIGURATION_DATABASE_VIRTUAL)
#define CONFIGURATION_DATABASE_VIRTUAL

#include "System.h"
#include "BString.h"
#include "CDBTypes.h"
#include "Iterators.h"
#include "GCNamedObject.h"


class Streamable;

extern "C" {

    /** standard sorter for database */
    int32 CDBC_SortFilterFn(LinkedListable *data1,LinkedListable *data2);

    /** special sorter for database that knows about the xxxxxnnn syntax */
    int32 CDBC_SortNumFilterFn(LinkedListable *data1,LinkedListable *data2);
}


class CDBVirtual: public GCNamedObject{
protected:

#if !defined(_CINT)


public:

    /** will destroy database if instance count goes to 0 */
    virtual ~CDBVirtual(){};

    /** creates a new reference to a database, or if that is not possible it creates a copy
        @param cdbcm = CDBCM_CopyAddress ensures that the new object points at the same location  */
    virtual CDBVirtual *Clone(CDBCreationMode cdbcm)=0;

    /** remove all the content from the database
        unless some other database instance exists and points to a subtree
        one can reques to delete only the current subtree.    */
    virtual void CleanUp(CDBAddressMode cdbam = CDBAM_FromRoot) = 0;

    /** locks the main database for exclusive access: use if a group of transactions should be atomic */
    virtual bool Lock() = 0;

    /** unlocks the main database: rememver unlocking a locked database after use */
    virtual void UnLock() = 0;

    /** finds the overall path leading to the current node */
    virtual bool SubTreeName(Streamable &name,const char *sep = ".") = 0;

    /** the name of the current node returned in @param name */
    virtual bool NodeName(BString &name) = 0;

    /** the type of the current node returned in @param name */
    virtual bool NodeType(BString &name) = 0;

    /** the function can be used to create a new subtree */
    virtual bool AddChildAndMove(const char *subTreeName,SortFilterFn *sorter=NULL) = 0;

    /** how many branches from this node. Negative number implies that the location is a leaf */
    virtual int  NumberOfChildren()=0;

    /** move to the specified location. The movement is relative to the current location
        UpNode is one level up
        RootNode is all the way up
    */
    virtual bool Move(const char *subTreeName) = 0;

    /** negative number move up. >=0 chooses the subtree, 0 is the first of the children */
    virtual bool MoveToChildren(int childNumber=0) = 0;

    /**  0 means remain where you are, > 0 brothers on the right < 0 on the left */
    virtual bool MoveToBrother(int steps = 1) = 0;

    /** -1 = Root, one level up for each positive */
    virtual bool MoveToFather(int steps = 1) = 0;

    /** Copy the pointed subtree in cdbv into the current subtree */
    virtual bool CopyFrom(CDBVirtual *cdbv) = 0;

    /** Copy the pointed subtree in cdbv into the current subtree */
    inline bool  CopyTo(CDBVirtual *cdbv){  if (cdbv ) return cdbv->CopyFrom(this); else return True;   }

    /** moves back to the root */
    inline  bool MoveToRoot(){ return MoveToFather(-1); }

    /** from node search on the right of the tree for the subtree identified by the string name.
        On success nodes points to the node containing the subtree or leaf
        Will not follow links.
        @param cdbam  = CDBAM_SkipCurrent search in the current subtree excluding current node
                      any other flag are NOT SUPPORTED */
    virtual bool FindSubTree(const char *configName,CDBAddressMode cdbam = CDBAM_None)=0;

    /** The total number of nodes.
        @param cdbam CDBAM_SubTreeOnly allows to measure the current subtree
                     CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int  Size(CDBAddressMode cdbam) = 0;

    /** in the left to right bottom to top order the absolute location of a node
        @param cdbam CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int  TreePosition(CDBAddressMode cdbam = CDBAM_LeafsOnly) = 0;

    /** move to a location within the whole (sub)tree.
        The nodes are numbered from left to right and from subnode to supernode
        @param cdbam if it contains CDBAM_LeafsOnly   it will not account the group nodes
                     if it contains CDBAM_SubTreeOnly addresses only the subtree
                     if it contains CDBAM_Relative    it is a step from current location and is assumed global
        If the node does not exist returns False and remains in the start position   */
    virtual bool TreeMove(int index,CDBAddressMode cdbam = CDBAM_LeafsOnly) = 0;

    /** if cdbaim is CDBAIM_Strict, it expects all the indexes to be found
        form 0 to n-1 and the same number in all subtrees.
        @param caseSensitive is the configName case sensitive
    */
    virtual bool GetArrayDims(int *size,int &maxDim,const char *configName,CDBArrayIndexingMode cdbaim = CDBAIM_Flexible, bool caseSensitive = True) = 0;

    /** if size == NULL it treats the input as a monodimensional array of size nDim
        if nDim = 0 this is a standard variable equivalent of a vector of size 1
        if size does not agree with the actual dimensions of the matrix the routine will fail   
        @param caseSensitive is the configName case sensitive
    */
    virtual bool ReadArray (void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName, bool caseSensitive = True) = 0;

    /** if size == NULL it treats the input as a monodimensional array of size nDim
        if nDim = 0 this is a standard variable equivalent of a vector of size 1 */
    virtual bool WriteArray(const void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName,SortFilterFn *sorter=NULL) = 0;

    /** remove an entry or subtree (position is relative!)
        to delete a link use the linkTo as the leaf name
        to delete a subtree simply specify the group node
    */
    virtual bool Delete(const char *configName) = 0;

    /** whether a certain entry exists */
    virtual bool Exists(const char *configName) = 0;

    /** linkFrom is the path where the link is created from */
    virtual bool Link(const char *linkFrom,const char *linkTo,SortFilterFn *sorter=NULL) = 0;

//##################################################################/
//##                                                              ##
//##            Complex load/save functions                       ##
//##                                                              ##
//##################################################################/

    /** Reads a database from a stream */
    virtual bool ReadFromStream(StreamInterface &stream,StreamInterface *err=NULL,SortFilterFn *sorter=NULL) = 0;

    /** enable reports of parser during ReadFromStream into err */
    virtual void EnableParserReports(bool flag) = 0;

    /** writes the database to stream without any ordering */
    virtual bool WriteToStream(StreamInterface &stream,StreamInterface *err=NULL,CDBWriteMode mode=CDBWM_Tree) = 0;

    /** load from environment or from any NULL terminated list of chars */
    virtual bool LoadFromEnvironment(char **env) = 0;

    /** from CDB to memory */
    virtual bool ReadStructure (const char *className,char *address,Streamable *err=NULL) = 0;

    /** either copies or references a memory structure,
        at address @param address and of type @param className
        CDB transforms the memory, MMCDB just references it  */
    virtual bool WriteStructure(const char *className,char *address,const char *variableName=NULL,Streamable *err=NULL) = 0;

#endif

};

#endif

