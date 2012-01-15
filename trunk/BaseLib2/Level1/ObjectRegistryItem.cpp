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

#include "System.h"
#include "ObjectRegistryItem.h"
#include "ObjectRegistryDataBase.h"

uint32 NameToClassId(const char *name){
    if (name == NULL) return 0;
    unsigned char *c = (unsigned char *)name;
    uint32 s1 = 0;
    uint32 s2 = 0;
    uint32 s3 = 0;
    uint32 s4 = 0;
    while (*c != 0){
        uint32 s = c[0];
        uint32 s0 = s;
        s1 += s0;
        s0 *= s;
        s2 += s0;
        s0 *= s;
        s3 += s0;
        s0 *= s;
        s4 += s0;
        c++;
    }
    s2 <<= 8;
    s3 <<= 16;
    s4 <<= 24;
    uint32 s=  ((s1 & 0xFF) | (s2 & 0xFF00) | (s3 & 0xFF0000) | (s4 & 0xFF000000));
    s &= 0x7FFFFFFF;
    return s;
}


void ORISetup(ObjectRegistryItem *  ori,
                const char *        className,
                const char *        version,
                const char *        defaultExtension,
                uint32              defaultFlags,
                ObjectTools *       tools){

    ori->nOfAllocatedObjects = 0;

    if (className == NULL) return;

    ori->className = className;
    ori->classId = NameToClassId(className);

    if (defaultExtension == NULL)
        ori->defaultExtension = className;
    else
        ori->defaultExtension = defaultExtension;

    ori->defaultFlags = defaultFlags;

    const char *v = strstr(version,",v ");
    if (v != NULL){
        v += 3;
        const char *w = v;
        while((*w != ' ') && (*w != 0)) w++;
        if (*w == ' ') w--;
//        while((*w == ' ') && (*w != 0)) w++;
//        while((*w != ' ') && (*w != 0)) w++;
        uint32 len = w-v+1;
        if (len > (sizeof(ori->version)-1)) len = sizeof(ori->version)-1;
        strncpy(ori->version,v,len);
    } else strncpy(ori->version,version,sizeof(ori->version)-1);
    ObjectRegistryDataBaseAdd(ori);

    ori->tools = NULL;
    if (tools != NULL) ori->tools = tools;
};

void ORISetupStructure(
                ObjectRegistryItem *    ori,
                const char *            className,
                ClassStructure *        structure){

    ObjectRegistryItem *ordbi = ObjectRegistryDataBaseFind((char *)className);
    if (ordbi != NULL) ordbi->structure = structure;
    else {
        ORISetup(ori,className,"","",0,NULL);
        ori->structure = structure;
    }

}

void ORIInit(ObjectRegistryItem *ori){
    ori->structure          = NULL;
    ori->className          = "";
    ori->defaultExtension   = "";
    ori->tools              = NULL;
    ori->ll                 = NULL;
    strcpy(ori->version,"raw");
}

void ORIFinish(ObjectRegistryItem *ori){
    LoadableLibrary *ll = ori->ll;
    ObjectRegistryDataBaseDelete(ori);
#if defined (LOGGER_FLUSH)
    LSProcessPendingErrors();
#endif
    // the code will finally disappear

// If you destroy the code the object cannot finish
// complete the destruction of the object members
    if (ll != NULL) delete ll;

}


