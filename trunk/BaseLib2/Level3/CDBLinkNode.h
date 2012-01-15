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

#if !defined(CONFIGURATION_DATABASE_LINKNODE)
#define CONFIGURATION_DATABASE_LINKNODE

/** 
 * @file
 * Implements the hard link between the nodes
 */
#include "FString.h"
#include "CDBStringDataNode.h"
#include "CDBNodeRef.h"

class CDBLinkNode;
OBJECT_DLL(CDBLinkNode)
#define CDBLinkNodeVersion "$Id$"

class CDBLinkNode: public CDBStringDataNode{
OBJECT_DLL_STUFF(CDBLinkNode)

public:
    /** */
    CDBLinkNode(const char *name): CDBStringDataNode(name)  { }

    /** whether it is a link to a different subtree */
    virtual bool        IsLinkNode()                  const { return True; }

    /** whether it is a link to a different subtree */
    virtual bool        IsDataNode()                  const { return False; }

    /** allows accessing the subtrees by name,
        @param followLink if true Links are followed */
    virtual CDBNode *   Children(   const char *childName,
                                    bool followLink = False){ return NULL;  }

    /** reads data from a node.
        @param valueType specifies data type
        @param size specifies how many elements
        @param value contains a pointer to memory
        where to write the data */
    virtual bool        ReadContent(    void *value,
                                        const CDBTYPE &valueType,
                                        int size,
                                        va_list argList);

};




#endif
