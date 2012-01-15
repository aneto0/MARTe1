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
 * A container of CDBNodes (branches of the tree)
 */
#if !defined(CONFIGURATION_DATABASE_GROUPNODE)
#define CONFIGURATION_DATABASE_GROUPNODE

#include "FString.h"
#include "CDBNode.h"
#include "CDBNodeRef.h"
#include "CDBLinkNode.h"
#include "LinkedListHolder.h"


/** A filter to cleanup a tree from empty branches */
class CDBTreePurger: public SearchFilter {
public:
    /** */
    CDBTreePurger(){
    }
    /** */
    bool Test(LinkedListable *data){
        CDBNode *cdbn = (CDBNode *)data;

        cdbn->DeleteSubTree();
        return ((cdbn->NumberOfReferences() <= 0) &&
                (cdbn->NumberOfChildren()   <= 0));
    }
};

class CDBGroupNode;
OBJECT_DLL(CDBGroupNode)
#define CDBGroupNodeVersion "$Id: CDBGroupNode.h,v 1.6 2011/10/20 18:25:23 aneto Exp $"

class CDBGroupNode: public CDBNode{

OBJECT_DLL_STUFF(CDBGroupNode)

public:
    /** the nodes underneath */
    LinkedListHolder subTree;

public:
    /** constructor */
                        CDBGroupNode(   const char *    name="" )
                        {
                            CDBNode::Init(name);
                        }

    /** destructor */
    virtual             ~CDBGroupNode()
                        {
                            subTree.Reset();
                        }

    /** -1 means it is not a ConfigDataNode (intermediate node) */
    virtual int32       NumberOfChildren() const
                        {
                            return subTree.ListSize();
                        }

    /** allows accessing the subtrees and uptrees (-1 -2...).
        the links are not expanded by this function */
    virtual CDBNode *   Children(       int             childNumber) ;

    /** allows accessing the subtrees by name,
        @param childName is the name of the node. It cannot be a full subtree
        @param followLink if true Links are followed
        @param functionMode see CDBNMode for explanation
        @param sortFn used only if adding a node
        @param caseSensitive the childName is case sensitive*/
    virtual CDBNode *   Children(       const char *    childName,
                                        CDBNMode        functionMode       = CDBN_SearchOnly,
                                        SortFilterFn *  sortFn             = NULL,
                                        const char *    containerClassName = NULL,
                                        bool            caseSensitive      = True);

    /** remove a child by name */
    virtual bool        RemoveChild(    CDBNode *       child)
                        {
                            return subTree.ListExtract(child) ;
                        }

    /** removes all unreferenced subtrees */
    virtual void        DeleteSubTree()
                        {
                            CDBTreePurger cdbtp;
                            subTree.ListSafeDelete(&cdbtp);
                        }

    /** apply sorting */
    virtual bool        SortChildren(   SortFilterFn *  sortFn)
                        {
                            subTree.ListBSort(sortFn);
                            return True;
                        }

    /** how many elements in the subtree */
    virtual int         SubTreeSize(    bool            leafOnly);

    /** reads data from a node.
        @param valueType specifies data type
        @param size specifies how many elements
        @param value contains a pointer to memory
        where to write the data */
    virtual bool        ReadContent(    void *          value,
                                        const CDBTYPE & valueType,
                                        int             size,
                                        va_list         argList);

    /** Writes content into a data node. Creates a data node or modifies an existing
        @param configName is the address of the parameter relative to the current node
        @param array is the data in wahtever form specified by valueType
        @param valueType specifies tha data type in array
        @param size is a vector of matrix dimensions if size == NULL it treats the input as a monodimensional array of size nDim
        @param nDim how many dimensions the array possesses or the vector size if size = NULL
        @param functionMode allows specifying the creation of a node different frpm the CDBStringDataNode
        @param sortFn allows inserting newly created nodes in a specific order */
    virtual bool        WriteArray(     const char *    configName,
                                        const void *    array,
                                        const CDBTYPE & valueType,
                                        const int *     size,
                                        int             nDim,
                                        CDBNMode        functionMode    = CDBN_CreateStringNode,
                                        SortFilterFn *  sortFn          = NULL);

    /** reads content from a data node.
        @param configName is the address of the parameter relative to the current node
        @param array is the data in wahtever form specified by valueType
        @param valueType specifies tha data type in array
        @param size is a vector of matrix dimensions if size == NULL it treats the input as a monodimensional array of size nDim
        @param nDim how many dimensions the array possesses or the vector size if size = NULL 
        @param caseSensitive the configName is case sensitive
    */
    virtual bool        ReadArray(      const char *    configName,
                                        void *          array,
                                        const CDBTYPE & valueType,
                                        const int *     size,
                                        int             nDim,
                                        bool            caseSensitive = True);
};




#endif
