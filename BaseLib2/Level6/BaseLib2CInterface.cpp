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
#define GCRLOADPOINTER

#include "BaseLib2CInterface.h"
#include "GlobalObjectDataBase.h"
#include "SXMemory.h"
#include "CDBVirtual.h"
#include "ConfigurationDataBase.h"


/** create an object of className and objectName
    it works if class exists and is of the right type (GCNamedObject derivative)
    the object is located in the path specified by context
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_CreateGODBObject(
                    const char     *context,
                    const char     *className,
                    const char     *objectName
                ){
    GCReference gcr = GODBFindByName(context);
    if (!gcr.IsValid()){
        return -1;
    }
    GCRTemplate<GCReferenceContainer> gcrc = gcr;
    if (!gcrc.IsValid()){
        return -2;
    }

    GCRTemplate<GCNamedObject> gcno(className);
    if (!gcno.IsValid()){
        return -3;
    }
    gcno->SetObjectName(objectName);

    if (gcrc->Insert(gcno)) return 0;

    return -99;
}

/** create an object to be inserted in the present GODB location (must be a container node)
    the object class, name and initialisation data is taken from CDB current subtree
    the object name is the current node name
    the object class is obtained from the ClassName leaf
    all the other leafs and sub-subtrees are for the object to use to initialise itself
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_CreateAndInitialiseGODBObject(
                    const char     *context,
                    CDBReference    configuration
                ){
    GCReference gcr = GODBFindByName(context);
    if (!gcr.IsValid()){
        return -1;
    }

    GCRTemplate<GCReferenceContainer> gcrc = gcr;
    if (!gcrc.IsValid()){
        return -2;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)configuration;
    if (cdbrv == NULL){
        return  -3;
    }

    // mark used by at least 2 so that when cdb gets out of scope the object is not destroyed
    while (cdbrv->NumberOfReferences() < 2) cdbrv->Increment();
    ConfigurationDataBase cdb(cdbrv);
    GCReference obj;
    if (!obj.ObjectLoadSetup(cdb,NULL)){

        return -4;
    }

    if (gcrc->Insert(obj)) return 0;

    return -99;

}

/** configuration is a CDB reference pointing to the subtree containing
    the desired configuration data
    This call configures the object does not create a new object to be inserted
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_InitialiseGODBObject(
                    const char     *context,
                    CDBReference    configuration
                ){
    GCReference gcr = GODBFindByName(context);
    if (!gcr.IsValid()){
        return -1;
    }

    GCRTemplate<GCReferenceContainer> gcrc = gcr;
    if (!gcrc.IsValid()){
        return -2;
    }

    CDBVirtual *cdbrv = (CDBVirtual *)configuration;
    if (cdbrv == NULL){
        return  -3;
    }

    // mark used by at least 2 so that when cdb gets out of scope the object is not destroyed
    while (cdbrv->NumberOfReferences() < 2) cdbrv->Increment();
    ConfigurationDataBase cdb(cdbrv);
    if (gcrc->ObjectLoadSetup(cdb,NULL)){
        return 0;
    }
    return -99;
}

/** delete the current subtree of the GODB
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_DeleteGODBSubTree(
                    const char     *context,
                    int             deleteThisNode
                ){


    GCReference gcr = GODBFindByName(context);
    if (!gcr.IsValid()){
        return -1;
    }

    GCRTemplate<GCReferenceContainer> gcrc = gcr;
    if (!gcrc.IsValid()){
        return -2;
    }
    gcrc->CleanUp();

    if (deleteThisNode == 1){
        GCRTemplate<GCReferenceContainer> godb = GetGlobalObjectDataBase();
        if (godb.IsValid()){
            godb->Remove(context);
        }
    }

    return 0;
}

/** describes the current subtree or its children
    order = -1 ==> current node
    order >= 0 ==> child i-th
    ObjectName ClassName must be pointers to buffers of size bufferSize
    containerClass is the pointer to an integer which
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_ListGODBNodes(
                    const char     *context,
                    int             order,
                    char           *objectName,
                    char           *className,
                    int             bufferSize,
                    int            *containerClass
                ){
    GCRTemplate<GCNamedObject> gcno = GODBFindByName(context);
    if (!gcno.IsValid()){
        return -1;
    }

    if (order >=0){
        GCRTemplate<GCReferenceContainer> gcrc = gcno;
        if (!gcrc.IsValid()){
            return -11;
        }
        gcno = gcrc->Find(order);
    }

    if (!gcno.IsValid()){
        return -2;
    }

    if (objectName) strncpy(objectName,gcno->Name(),bufferSize-1);
    if (className)  strncpy(className,gcno->ClassName(),bufferSize-1);
    if (containerClass){
        GCRTemplate<GCReferenceContainer> gcrc = gcno;
        if (gcrc.IsValid()){
            *containerClass = 1;
        } else {
            *containerClass = 0;
        }
    }
    return 0;
}


/** dumps the current subtree
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_ShowGODBSubTree(
                    const char     *context,
                          char     *output,
                    int             bufferSize
                ){
    GCRTemplate<GCReferenceContainer> gcrc = GODBFindByName(context);
    if (!gcrc.IsValid()){
        return -1;
    }

    SXMemory sxm(output,bufferSize);
    GCRCLister gcrcLister(&sxm);
    gcrc->Iterate(&gcrcLister);

    return 0;
}

/** sends a message to an object within GODB
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_SendMessageToGODBObject(
                    const char     *context,
                    const char     *messageText,
                    int             messageCode
                ){

    return -1;
}







