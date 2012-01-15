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


#include "CDBNodeRef.h"
#include "CDBGroupNode.h"

/** a dummy reference to avoid ever returning NULL */
CDBNode nullCDBNode;

/** Only destroy if it is an empty GroupNode
    on success moves the node to the father    */
inline bool EraseIfEmptyGroupNode(CDBNode *&cdbn){
    if (cdbn == &nullCDBNode) return False;
    if ((cdbn->NumberOfReferences() <=0) &&
        (cdbn->NumberOfChildren()   <=0) &&
        (!cdbn->IsDataNode()           ) &&
        (!cdbn->IsLinkNode()           ) &&
        (!cdbn->IsRootNode()           )) {

        // note that the reference from the father
        // node is aumatically deleted by the distructor
        CDBNode *toDelete = cdbn;
        cdbn = cdbn->Father();
        delete toDelete;
        return True;
    }
    return False;

}

/** Only destroy if it is an unreferenced leaf/node/link
    on success moves the node to the father    */
inline bool EraseIfUnreferenced(CDBNode *&cdbn){
    if (cdbn == &nullCDBNode) return False;
    if ((cdbn->NumberOfReferences() <=0) &&
        (cdbn->NumberOfChildren()   <=0) &&
        (!cdbn->IsRootNode()           )) {

        // note that the reference from the father
        // node is aumatically deleted by the distructor
        CDBNode *toDelete = cdbn;
        cdbn = cdbn->Father();
        delete toDelete;
        return True;
    }
    return False;
}


/** starting from cdbn move up removing empty group nodes
    on success moves the node to the father    */
inline bool PruneEmptyBranch(CDBNode *&cdbn) {
    if (cdbn == &nullCDBNode) return False;
    while (EraseIfEmptyGroupNode(cdbn));
    return True;
}

bool CDBNodeRef::MoveReference(CDBNode *cdbn,CDBNR_MoveRefMode mode){
    bool eraseNode = (mode == CDBNR_Delete);

    bool ret = True;
    if (cdbn == NULL) return False;
    if (&(GetReference()) == cdbn) return True;

    CDBNode *oldReference = &GetReference();

    reference = cdbn;
    reference->IncreaseReferenceCount();

    if (oldReference != NULLCDBNODE){
        oldReference->DecreaseReferenceCount();
        CDBNode *p = oldReference;
        if (eraseNode) ret = EraseIfUnreferenced(p);
        while(EraseIfEmptyGroupNode(p));
    }
    return ret;
}

bool CDBNodeRef::ReadContent(void *value,const CDBTYPE &valueType,int size,...){
    va_list argList;
    va_start(argList,size);
    bool ret = GetReference().ReadContent(value,valueType,size,argList);
    va_end(argList);
    return ret;

}


