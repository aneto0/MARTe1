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
 * @brief Memory based (read/write) CDB
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
#include "MMCDBItem.h"


#if !defined (MEMORYMAP_CDB_H)
#define MEMORYMAP_CDB_H


class MMCDB;

OBJECT_DLL(MMCDB)

/** a tool to access a block of memory as if it was a CDB
    the block of memory is described by a class or structure name
    the name must be registered using sinfo */
class MMCDB: public CDBVirtual{
    OBJECT_DLL_STUFF(MMCDB)

private: // stuff specific to MMCDB
    /** the list of steps that brought to the current state */
    StaticStackHolder     path;

    /** get the last path position*/
    MMCDBItem *             CurrentPosition();

    /** get the root path position*/
    MMCDBItem *             StartPosition();

    /** remove the last path position
        deallocation is performed only if this is the last left */
    void                    RemovePathElement();

    /** remove all the path elements until the one specified
        maxStep <= -2 means all
        maxStep == -1 means leave father
        maxStep > 0 means remove so many elements     */
    void                    UnWind(
                                MMCDBItem *     keep=NULL,
                                int             maxStep = -2);

public: // constructors
    /** does not work as copy constructor !*/
                            MMCDB(MMCDB &mmcdb):
                                path(sizeof(MMCDBItem *)/sizeof(intptr))
    {
    }

    /** */
                            MMCDB():
                                path(sizeof(MMCDBItem *)/sizeof(intptr))
    {
    }

    /** either copies or references a memory structure,
        at address @param address and of type @param className
        CDB transforms the memory, MMCDB just references it  */
    virtual bool            WriteStructure(
                                const char *    className,
                                char *          address,
                                const char *    variableName,
                                Streamable *    err=NULL);

    /** creates a new reference to a database, or if that is not possible it creates a copy
        @param cdbcm = CDBCM_CopyAddress ensures that the new object points at the same location  */
    CDBVirtual *            Clone(
                                CDBCreationMode cdbcm);


    /** */
    virtual                 ~MMCDB()
    {
        UnWind();
    }

    /** Cannot remove anything!! */
    virtual void            CleanUp(
                                CDBAddressMode  cdbam = CDBAM_FromRoot)
    {
    }

    /** no need for locking! */
    virtual bool            Lock()
    {
        return True;
    }

    /** no need for locking */
    virtual void            UnLock()
    {
        return ;
    }

    /** finds the overall path leading to the current node */
    virtual bool            SubTreeName(
                                Streamable &    name,
                                const char *    sep = ".");

    /** the name of the current node */
    virtual bool            NodeName(
                                BString &       name);

    /** the type of the current node */
    virtual bool            NodeType(
                                BString &       name);

    /** the structure cannot be modified so it will work as a Move */
    virtual bool            AddChildAndMove(
                                const char *    subTreeName,
                                SortFilterFn *  sorter=NULL)
    {
        return Move(subTreeName);
    }

    /** how many branches from this node. Negative number implies that the location is a leaf */
    virtual int             NumberOfChildren();

    /** move to the specified location. The movement is relative to the current location */
    virtual bool            Move(
                                const char *    subTreeName);

    /** negative number move up. >=0 chooses the subtree, 0 is the first of the children */
    virtual bool            MoveToChildren(
                                int             childNumber=0);

    /**  0 means remain where you are, > 0 brothers on the right < 0 on the left */
    virtual bool            MoveToBrother(
                                int             steps = 1);

    /** -1 = Root, one level up for each positive */
    virtual bool            MoveToFather(
                                int             steps = 1);

    /** Copy the pointed subtree in cdbv into the current subtree */
    virtual bool            CopyFrom(
                                CDBVirtual *    cdbv)
    {
        return False;
    }

    /** Copy the pointed subtree in cdbv into the current subtree */
    inline bool             CopyTo(
                                CDBVirtual *    cdbv)
    {
        if (cdbv ) return cdbv->CopyFrom(this);
        else return True;
    }

    /** moves back to the root */
    inline  bool            MoveToRoot()
    {
        return MoveToFather(-1);
    }

    /** from node search on the right of the tree for the subtree identified by the string name.
        On success nodes points to the node containing the subtree or leaf
        Will not follow links.
        @param cdbam  = CDBAM_SkipCurrent search in the current subtree excluding current node
                      any other flag are NOT SUPPORTED */
    virtual bool            FindSubTree(
                                const char *    configName,
                                CDBAddressMode  cdbam = CDBAM_None)
    {
        return False;
    }

    /** The total number of nodes.
        @param cdbam CDBAM_SubTreeOnly allows to measure the current subtree
                     CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int             Size(
                                CDBAddressMode  cdbam)
    {
        return -1;
    }

    /** in the left to right bottom to top order the absolute location of a node
        @param cdbam CDBAM_LeafsOnly   allows counting only the data nodes */
    virtual int             TreePosition(
                                CDBAddressMode  cdbam = CDBAM_LeafsOnly)
    {
        return -1;
    }

    /** move to a location within the whole (sub)tree.
        The nodes are numbered from left to right and from subnode to supernode
        @param cdbam if it contains CDBAM_LeafsOnly   it will not account the group nodes
                     if it contains CDBAM_SubTreeOnly addresses only the subtree
                     if it contains CDBAM_Relative    it is a step from current location and is assumed global
        If the node does not exist returns False and remains in the start position   */
    virtual bool            TreeMove(
                                int             index,
                                CDBAddressMode  cdbam = CDBAM_LeafsOnly)
    {
        return False;
    }

    /** if cdbaim is CDBAIM_Strict, it expects all the indexes to be found form 0 to n-1 and the same number in all subtrees. */
    virtual bool            GetArrayDims(
                                int *                size,
                                int &                maxDim,
                                const char *         configName,
                                CDBArrayIndexingMode cdbaim = CDBAIM_Flexible,
                                bool                 caseSensitive = True);

    /** if size == NULL it treats the input as a monodimensional array of size nDim
        if size does not agree with the actual dimensions of the matrix the routine will fail   */
    virtual bool            ReadArray (
                                void *          array,
                                const CDBTYPE & valueType,
                                const int *     size,
                                int             nDim,
                                const char *    configName,
                                bool            caseSensitive = True);

    /** if size == NULL it treats the input as a monodimensional array of size nDim */
    virtual bool            WriteArray(
                                const void *    array,
                                const CDBTYPE & valueType,
                                const int *     size,
                                int             nDim,
                                const char *    configName,
                                SortFilterFn *  sorter=NULL);

    /** remove an entry or subtree (position is relative!)
        to delete a link use the linkTo as the leaf name
        to delete a subtree simply specify the group node     */
    virtual bool            Delete(
                                const char *    configName)
    {
        return False;
    }

    /** whether a certain entry exists */
    virtual bool            Exists(
                                const char *    configName);

    /** linkFrom is the path where the link is created from */
    virtual bool            Link(
                                const char *    linkFrom,
                                const char *    linkTo,
                                SortFilterFn *  sorter=NULL)
    {
        return False;
    }

//##################################################################/
//##                                                              ##
//##            Complex load/save functions                       ##
//##                                                              ##
//##################################################################/

    /** Reads a database from a stream */
    virtual bool            ReadFromStream(
                                StreamInterface &stream,
                                StreamInterface *err=NULL,
                                SortFilterFn *  sorter=NULL)
    {
        return False;
    }

    /** enable reports of parser during ReadFromStream into err */
    virtual void            EnableParserReports(
                                bool            flag)
    {
    }

    /** writes the database to stream without any ordering */
    virtual bool            WriteToStream(
                                StreamInterface &stream,
                                StreamInterface *err=NULL,
                                CDBWriteMode    mode=CDBWM_Tree)
    {
        return False;
    }

    /** load from environment or from any NULL terminated list of chars */
    virtual bool            LoadFromEnvironment(
                                char **         env)
    {
        return False;
    }

    /** from CDB to memory */
    virtual bool            ReadStructure (
                                const char *    className,
                                char *          address,
                                Streamable *    err=NULL)
    {
        return False;
    }

    /** from memory to CDb */
    virtual bool            WriteStructure(
                                const char *    className,
                                char *          address,
                                Streamable *    err=NULL)
    {
        return False;
    }


};

#endif


