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


#include "GCRCItem.h"
#include "GlobalObjectDataBase.h"
#include "ErrorManagement.h"

/** get next element
    skip no GCRCItem derivatives */
GCRCItem *GCRCItem::Next(){
    LinkedListable *ll = LinkedListable::Next();
    GCRCItem *grci = NULL;
    if (ll) grci = dynamic_cast<GCRCItem *> (ll);
    while ((grci == NULL) && (ll != NULL)){
        ll = ll->Next();
        if (ll) grci = dynamic_cast<GCRCItem *> (ll);
    }
    return grci;
}


/* access the object reference */
void GCRCILReference(GCRCILink &gcrcil, GCReference &gc){
    gc = GODBFindByName(gcrcil.objectPath.Buffer());
    if (!gc.IsValid()){
        CStaticAssertErrorCondition(ParametersError,"GCRCILink:Reference cannot find %s in the GlobalObjectDataBase",gcrcil.objectPath.Buffer());
    } 
}

