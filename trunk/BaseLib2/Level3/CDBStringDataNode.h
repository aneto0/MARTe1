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

#if !defined(CONFIGURATION_DATABASE_STRING_NODE)
#define CONFIGURATION_DATABASE_STRING_NODE

/** 
 * @file
 * Abstraction of all nodes containing data
 */
#include "CDBNode.h"
#include "CDBTypes.h"
#include "System.h"

class CDBDataNode;
OBJECT_DLL(CDBDataNode)

class CDBDataNode: public CDBNode {
OBJECT_DLL_STUFF(CDBDataNode)

protected:
    /** 1 => elements points to a string directly
        >1 it is a pointer to an array of pointers
        */
            uint32      numberOfElements;

    /**
        vector of pointers or direct pointer to data if numberOfElements == 1
        the number of allocated elements depends on the value of numberOfElements
        0 elements should be NULL
        1 elements points to the only element
        > 1 it is a vector of pointers     */
            char **     elements;

protected:

    /** constructor */
                        CDBDataNode()                   {
                                                            elements            = NULL;
                                                            numberOfElements    = 0;
                                                        }

    /** destructor */
                        ~CDBDataNode()                  {
                                                            CleanUp();
                                                        }

    /** resize data. removes redundant elements*/
            void        SetSize(int size);

    /** resize to 0*/
            void        CleanUp();


    /** False if the element was already set */
            bool        WriteElement(   uint32          elementNumber,
                                        const char *    value,
                                        int32           maxSize=-1);

    /** if elementNumber is outside range returns False    */
            const char *ReadElement(    uint32 elementNumber) const;

    /** How much data elelments contained by this node. -1 means it is not a node that has data */
    virtual int32       NumberOfElements()      const   {
                                                            if (elements == NULL)      return 0;
                                                            return numberOfElements;
                                                        }

    /** whether it is to be considered a leaf node */
    virtual bool        HasData()               const   { return True; }


};

class CDBStringDataNode;
OBJECT_DLL(CDBStringDataNode)

/** a config entry consisting simply on a string */
class CDBStringDataNode: public CDBDataNode {
OBJECT_DLL_STUFF(CDBStringDataNode)

protected:

    /** */
            bool        WriteDouble(    const double *  values,
                                        uint32          size);

    /** */
            bool        WriteFloat(     const float *   values,
                                        uint32          size);

    /** */
            bool        WriteInteger(   const int32 *   values,
                                        uint32          size);

    /** */
            bool        WriteUnsignedInteger(
                                        const uint32 *  values,
                                        uint32          size);

    /** */
            bool        WritePointer(
                                        const void **   values,
                                        uint32          size);

    /** */
            bool        WriteChar(      const char *    values,
                                        uint32          size);

    /** */
            bool        WriteString(    const char **   values,
                                        uint32          size);

    /** */
            bool        WriteString(    const FString * values,
                                        uint32          size);

    /** */
            bool        WriteString(    const BString * values,
                                        uint32          size);

    /** */
            bool        ReadDouble(     double *        values,
                                        uint32          size);

    /** */
            bool        ReadFloat(      float *         values,
                                        uint32          size);

    /** */
            bool        ReadInteger(    int32 *         values,
                                        uint32          size);
    /**
     * Read int 64 arrays
     * @param values the array with the parsed values
     * @param size the size of the array
     * @return True if the array is sucessfully read
     */
            bool        ReadInteger64(    int64 *         values,
                                          uint32          size);

    /** */
            bool        ReadPointer(    intptr *        values,
                                        uint32          size);

    /** */
            bool        ReadUnsignedInteger(
                                        uint32 *        values,
                                        uint32          size);

    /** */
            bool        ReadChar(       char *          values,
                                        uint32          size);

    /** */
            bool        ReadString(     char **         values,
                                        uint32          size);

    /** */
            bool        ReadString(     FString *       values,
                                        uint32          size);

    /** */
            bool        ReadString(     BString *       values,
                                        uint32          size);

    /** will insert the {} around the formatted */
            bool        ReadFormatted(  Streamable *    formatted,
                                        uint32          indentChars,
                                        uint32          maxElements);

    /** the string must exclude the { } delimiters but include "" for each multiple char item */
            bool        WriteFormatted( const char *    formatted,
                                        const char *    seps    =   " {}\n\t\r");


public:

    /** constructor */
                        CDBStringDataNode(
                                        const char *    name = "" )
                                                        { Init(name); }

    /** destructor */
                        ~CDBStringDataNode()            {}

    /** writes data on a node.
        @param valueType specifies data type
        @param size specifies how many elements */
    virtual bool        WriteContent(   const void *    value,
                                        const CDBTYPE & valueType,
                                        int             size);

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
