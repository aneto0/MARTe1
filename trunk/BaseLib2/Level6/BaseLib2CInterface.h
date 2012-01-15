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
 * @brief C interface for BaseLib2
 */
#if !defined (GLOBAL_OBJECT_DATABASE_C_INTERFACE_H)
#define GLOBAL_OBJECT_DATABASE_C_INTERFACE_H

#ifdef __cplusplus
    extern "C" {
#endif


#include "CDBCInterface.h"

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
                );

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
                );

/** configuration is a CDB reference pointing to the subtree containing
    the desired configuration data
    This call configures the object does not create a new object to be inserted
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_InitialiseGODBObject(
                    const char     *context,
                    CDBReference    configuration
                );

/** delete the current subtree of the GODB
    deleteThisNode = 1 remove the container from ddb
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_DeleteGODBSubTree(
                    const char     *context,
                    int             deleteThisNode
                );

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
                );


/** dumps the current subtree
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_ShowGODBSubTree(
                    const char     *context,
                    char           *output,
                    int             bufferSize
                );

/** sends a message to an object within GODB
    returns 0 on success
    return -1 if context is a wrong path
    */
int     B2C_SendMessageToGODBObject(
                    const char     *context,
                    const char     *messageText,
                    int             messageCode
                );


#ifdef __cplusplus
    }
#endif

#endif



