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
 * An unimplemented CDB just to create valid references 
 */

#if !defined(NULL_CONFIGURATION_DATABASE)
#define NULL_CONFIGURATION_DATABASE

#include "CDBVirtual.h"

class CDBNull: public CDBVirtual{
protected:

#if !defined(_CINT)


public:

    /** creates a new reference to a database, or if that is not possible it creates a copy
        @param cdbcm = CDBCM_CopyAddress ensures that the new object points at the same location  */
    virtual CDBVirtual *Clone(CDBCreationMode cdbcm){ return NULL;}

    /** remove all the content from the database
        unless some other database instance exists and points to a subtree
        one can reques to delete only the current subtree.    */
    virtual void CleanUp(CDBAddressMode cdbam = CDBAM_FromRoot){ }

    /** locks the main database for exclusive access: use if a group of transactions should be atomic */
    virtual bool Lock(){ return False;}

    /** unlocks the main database: rememver unlocking a locked database after use */
    virtual void UnLock(){ }

    /** finds the overall path leading to the current node */
    virtual bool SubTreeName(Streamable &name,const char *sep = "."){ return False;}

    /** the name of the current node */
    virtual bool NodeName(BString &name){ return False;}

    /** the type of the current node */
    virtual bool NodeType(BString &name){ return False;}

    /** the function can be used to create a new subtree */
    virtual bool AddChildAndMove(const char *subTreeName,SortFilterFn *sorter=NULL){ return False;}

    /** how many branches from this node. Negative number implies that the location is a leaf */
    virtual int  NumberOfChildren(){ return 0;}

    /** move to the specified location. The movement is relative to the current location
        UpNode is one level up
        RootNode is all the way up
    */
    virtual bool Move(const char *subTreeName){ return False;}

    /** negative number move up. >=0 chooses the subtree, 0 is the first of the children */
    virtual bool MoveToChildren(int childNumber=0){ return False;}

    /**  0 means remain where you are, > 0 brothers on the right < 0 on the left */
    virtual bool MoveToBrother(int steps = 1){ return False;}

    /** -1 = Root, one level up for each positive */
    virtual bool MoveToFather(int steps = 1){ return False;}

    /** from node search on the right of the tree for the subtree identified by the string name.
        On success nodes points to the node containing the subtree or leaf
        Will not follow links.
        @param cdbam  = CDBAM_SkipCurrent search in the current subtree excluding current node
                      any other flag are NOT SUPPORTED */
    virtual bool FindSubTree(const char *configName,CDBAddressMode cdbam = CDBAM_None){ return False;}

    /** The total number of nodes.
        @param cdbam CDBAM_SubTreeOnly allows to measure the current subtree
                     CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int  Size(CDBAddressMode cdbam){ return 0;}

    /** in the left to right bottom to top order the absolute location of a node
        @param cdbam CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int  TreePosition(CDBAddressMode cdbam = CDBAM_LeafsOnly){ return 0;}

    /** move to a location within the whole (sub)tree.
        The nodes are numbered from left to right and from subnode to supernode
        @param cdbam if it contains CDBAM_LeafsOnly   it will not account the group nodes
                     if it contains CDBAM_SubTreeOnly addresses only the subtree
                     if it contains CDBAM_Relative    it is a step from current location and is assumed global
        If the node does not exist returns False and remains in the start position   */
    virtual bool TreeMove(int index,CDBAddressMode cdbam = CDBAM_LeafsOnly){ return False;}

    /** if cdbaim is CDBAIM_Strict, it expects all the indexes to be found form 0 to n-1 and the same number in all subtrees. */
    virtual bool GetArrayDims(int *size,int &maxDim,const char *configName,CDBArrayIndexingMode cdbaim = CDBAIM_Flexible, bool caseSensitive = True){ return False;}

    /** if size == NULL it treats the input as a monodimensional array of size nDim
        if size does not agree with the actual dimensions of the matrix the routine will fail   */
    virtual bool ReadArray (void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName, bool caseSensitive = True){ return False;}

    /** if size == NULL it treats the input as a monodimensional array of size nDim */
    virtual bool WriteArray(const void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName,SortFilterFn *sorter=NULL){ return False;}

    /** remove an entry or subtree (position is relative!)
        to delete a link use the linkTo as the leaf name
        to delete a subtree simply specify the group node
    */
    virtual bool Delete(const char *configName){ return False;}

    /** whether a certain entry exists */
    virtual bool Exists(const char *configName){ return False;}

    /** linkFrom is the path where the link is created from */
    virtual bool Link(const char *linkFrom,const char *linkTo,SortFilterFn *sorter=NULL){ return False;}

    /** Copy the pointed subtree in cdbv into the current subtree */
    virtual bool CopyFrom(CDBVirtual *cdbv){ return False; }

//##################################################################/
//##                                                              ##
//##            Complex load/save functions                       ##
//##                                                              ##
//##################################################################/

    /** Reads a database from a stream */
    virtual bool ReadFromStream(StreamInterface &stream,StreamInterface *err=NULL,SortFilterFn *sorter=NULL){ return False;}

    /** enable reports of parser during ReadFromStream into err */
    virtual void EnableParserReports(bool flag){ }

    /** writes the database to stream without any ordering */
    virtual bool WriteToStream(StreamInterface &stream,StreamInterface *err=NULL,CDBWriteMode mode=CDBWM_Tree){ return False;}

    /** load from environment or from any NULL terminated list of chars */
    virtual bool LoadFromEnvironment(char **env){ return False;}

    /** from CDB to memory */
    virtual bool ReadStructure (const char *className,char *address,Streamable *err=NULL){ return False;}

    /** either copies or references a memory structure,
        at address @param address and of type @param className
        CDB transforms the memory, MMCDB just references it  */
    virtual bool WriteStructure(const char *className,char *address,const char *variableName=NULL,Streamable *err=NULL){ return False;}

#endif //cint
};


#endif



