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

#if !defined(CONFIGURATION_DATABASE_NODE_REFERENCE)
#define CONFIGURATION_DATABASE_NODE_REFERENCE

/** 
 * @file
 * A reference to a CDBNode
 */

#include "CDBNode.h"
#include "CDBTypes.h"
#include "CDBVirtual.h"
#include "FString.h"
#include "Object.h"

/** flags to control the deleting of nodes when moving reference */
enum CDBNR_MoveRefMode {

    /** */
    CDBNR_None    = 0,

    /** */
    CDBNR_Delete  = 1

};

/** a dummy reference to avoid ever returning NULL */
extern CDBNode nullCDBNode;

#define NULLCDBNODE &nullCDBNode

class CDBNodeRef{

    /** the reference to the CDBNode */
    CDBNode                     *reference;

public:

    /** allows access to reference when followingh links */
    friend class CDBGroupNode;

    /** constructor*/
                    CDBNodeRef()    {
                                        reference = NULLCDBNODE;
                                        nullCDBNode.SetFather(&nullCDBNode);
                                    }

    /** destructor */
                    ~CDBNodeRef()   {
                                        MoveReference(NULLCDBNODE,CDBNR_None);
                                    }
public:

    /** use this function to access the reference. Returns reference  */
    inline          CDBNode &GetReference()
                                    {
                                        return *reference;
                                    }

    /** use this function to access the reference. Returns reference  */
    inline          CDBNode &operator()()
                                    {
                                        return *reference;
                                    }

    /** change the reference to the node
        if cdbn = NULL than it will refer to nullCDBNode */
                    bool MoveReference( CDBNode *cdbn,
                                        CDBNR_MoveRefMode mode);
public:


    /** copy a reference */
    inline          void operator= (CDBNodeRef &cdbr)
                                    {
                                        MoveReference(&cdbr.GetReference(),CDBNR_None);
                                    }

    /** compare two references */
    inline          bool operator== (CDBNodeRef &cdbr)
                                    {
                                        return &GetReference() == &cdbr.GetReference();
                                    }

    /** allows accessing the subtrees (>=0) and uptrees (-1 -2...) */
    inline          bool MoveToChildren(int childNumber=0)
                                    {
                                        return MoveReference(GetReference().Children(childNumber),CDBNR_None);
                                    }

    /** allows accessing the brothers  */
    inline          bool MoveToBrother(int steps = 1)
                                    {
                                        return MoveReference(GetReference().FindBrother(steps),CDBNR_None);
                                    }

    /** allows accessing the parents */
    inline          bool MoveToFather(int steps = 1)
                                    {
                                        return MoveToChildren(-steps);
                                    }

    /** allows accessing the subtrees and uptrees
        UpNode is one level up
        RootNode is all the way up
        Any movement onto unexisting subtree will fail
        Any movement out of unexisting subtree will erase temporary holders
        Movement is relative to current location
        @param functionMode. see Find of CDBNode
        can be used to create nodes of all types or to create an entire subtree
        */
    inline          bool Move(              const char *    childName,
                                            CDBNMode        functionMode    = CDBN_None,
                                            SortFilterFn *  sortFn          = NULL)
                                    {
                                        CDBNode *newNode = GetReference().Find(childName,functionMode,sortFn);
                                        if (newNode == NULL) return False;
                                        MoveReference(newNode,CDBNR_None);
                                        return True;
                                    }

    /** moves to a node pointing to the specified subtree */
    inline          bool FindSubTreeAndMove(const char *    subTreeName,
                                            CDBAddressMode  cdbam           = CDBAM_None)
                                    {
                                        CDBNode *newNode = GetReference().FindSubTree(subTreeName,cdbam);
                                        if (newNode == NULL) return False;
                                        MoveReference(newNode,CDBNR_None);
                                        return True;
                                    }

    /** Remove current node and move up
        the operation will complete only if
        the number of users is 0
        the up movement will happen anyway   */
    inline          bool DeleteAndMoveToFather()
                                    {
                                        if (GetReference().IsRootNode()) return True;
                                        return MoveReference(GetReference().Father(),CDBNR_Delete);
                                    };

    /** move to a location within the whole (sub)tree.
        The nodes are numbered from left to right
        and from subnode to supernode
        @param cdbam if it contains CDBAM_LeafsOnly   it will not account the group nodes
                     if it contains CDBAM_SubTreeOnly addresses only the subtree
                     if it contains CDBAM_Relative    it is a step from current location
        If the node does not exist returns False and remains in the start position   */
        inline      bool TreeMove(          int             index,
                                            CDBAddressMode  cdbam = CDBAM_LeafsOnly )
                                    {
                                        return MoveReference(GetReference().Find(index,cdbam),CDBNR_None);
                                    }

    /** writes data on a node.
        @param valueType specifies data type
        @param size specifies how many elements */
                    bool ReadContent(       void *          value,
                                            const CDBTYPE & valueType,
                                            int             size,
                                            ...);

    /** writes data on a node.
        @param valueType specifies data type
        @param size specifies how many elements */
        inline      bool WriteContent(      const void *    value,
                                            const CDBTYPE & valueType,
                                            int             size)
                                    {
                                        return GetReference().WriteContent(value,valueType,size);
                                    }
};



#endif
