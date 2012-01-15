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
 * A container of CDBNodes and of an Object
 */
#if !defined(CONFIGURATION_DATABASE_OBJECTNODE)
#define CONFIGURATION_DATABASE_OBJECTNODE

#include "CDBGroupNode.h"
#include "ObjectRegistryDataBase.h"

class CDBObjectNode;
OBJECT_DLL(CDBObjectNode)

#define CDBObjectNodeVersion "$Id: CDBObjectNode.h,v 1.2 2006/08/21 17:19:33 fisa Exp $"

class CDBObjectNode: public CDBGroupNode{

OBJECT_DLL_STUFF(CDBObjectNode)

public:
    /** the class of the object */
    const char *        classType;

    /** the address of the object */
    Object *            objectAddress;

public:
    /** constructor */
                        CDBObjectNode(  const char *    name,
                                        Object *        object,
                                        const char *    type)
                        {
                            CDBGroupNode::Init(name);
                            classType       = strdup(type);
                            objectAddress   = object;
                        }

    /** destructor */
    virtual             ~CDBObjectNode()
                        {
                            subTree.Reset();
                            if (classType) free((void *&)classType);
                            if (objectAddress) delete objectAddress;
                        }

    /** reads content from a data node.
        @param configName is the address of the parameter relative to the current node
        @param array is the data in wahtever form specified by valueType
        @param valueType specifies tha data type in array
        @param size is a vector of matrix dimensions if size == NULL it treats the input as a monodimensional array of size nDim
        @param nDim how many dimensions the array possesses or the vector size if size = NULL
        Address and ClassType are always valid configNames. The first is integer the second string
    */
    virtual bool        ReadArray(      const char *    configName,
                                        void *          array,
                                        const CDBTYPE & valueType,
                                        const int *     size,
                                        int             nDim);

    /** reads data from a node.
        @param valueType specifies data type
        @param size specifies how many elements
        @param value contains a pointer to memory
        where to write the data */
    virtual bool        ReadContent(    void *          value,
                                        const CDBTYPE & valueType,
                                        int             size,
                                        va_list         argList);
};

#endif
