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
 * CDB C Interface that can be used by C applications to 
 * read and write configuration files
 */

#if !defined(CONFIGURATION_DATABASE_C_INTERFACE)
#define CONFIGURATION_DATABASE_C_INTERFACE

/** valid types for CDB C interface */
typedef enum cdbCDataType {

/** a single precision floating point */
    CDBC_float     = 1,

/** a double precision floating point */
    CDBC_double    = 2,

/** a 32 bit integer */
    CDBC_int32     = 3,

/** an unsigned 32 bit integer */
    CDBC_uint32    = 4,

/** a character */
    CDBC_char      = 5,

/** a string content delivered as char ** => char *p="bbb"  use (&p). On output char *p = NULL, p will be allocated by function */
    CDBC_String    = 6

} CDBCDataType;

#ifdef __cplusplus
    extern "C" {
#endif

/** if local = 0 then only the messages generated locally are managed
    if local = 1 then all the library (BaseLib2) messages are managed */
typedef void (*ErrorReportingFunction)(int local,const char *format,...);

/** sets the error reporting  function
    intercepts also the BaseLib error stream if interceptAllErrors is not 0
    if errorFunction == NULL the default behaviour is restored */
void CDBCISetErrorFunction(
                    ErrorReportingFunction  errorFunction,
                    int                     interceptAllErrors);

/** */
typedef void * CDBReference;

/** Create an empty reference
    if cdbr == NULL just creates, otherwise it copies */
CDBReference    CDBCICreate(
                    CDBReference    cdbr
                );

/** Destroy a CDB reference */
void            CDBCIDestroy(
                    CDBReference   *cdbr
                );

/** Clean the current subtree
    return 1 on succes 0 on failure */
int            CDBCIClean(
                    CDBReference    cdbr
                );

/** Loads from a memory buffer into the current subtree
    return 1 on success 0 on failure. */
int             CDBCILoad(
                    CDBReference    cdbr,
                    const char *    buffer
                );

/** Save from the current subtree into a memory buffer
    return 1 on success 0 on failure
    buffer is a zero terminated string. It must be freed using CDBCIFree */
int             CDBCISave(
                    CDBReference    cdbr,
                    char **         buffer
                );

/** Move to a specific node
    Return 1 on success 0 on failure
    Command is a . separated list of commands
    each command can be
    1) a node name to move to
    2) root -> move to root
    3) father -> move to father
    4) brothernn -> nn > 0 move to the brothers on the right < 0 on the left
    5) childnn -> nn >= 0 move to the nnth child
    if location is not NULL then the current location is returned.
    location is a zero terminated string. It must be freed using CDBCIFree
    */
int             CDBCIMove(
                    CDBReference    cdbr,
                    const char *    command,
                    char **         location
                );

/** read a value in the specified format
    the data (float,double,int) is in the C standard order
    the data (char *) is stored as a single array separated by 0s
    arraySizes is a 0 terminated array of ints.
    It lists the array dimensions.
    *arrayValue,arraySizes are allocated by the library and
    MUST be deallocated with CDBCIFree */
int             CDBCIReadArray(
                    CDBReference    cdbr,
                    CDBCDataType    type,
                    void **         arrayValues,
                    int  **         arraySizes
                );

/** write a value in the raw format
    the data (float,double,int) is in the C standard order
    the data (char *) is stored as a single array separated by 0s
    arraySizes is a 0 terminated array of ints.  */
int             CDBCIWriteArray(
                    CDBReference    cdbr,
                    const char *    entryName,
                    CDBCDataType    type,
                    void *          arrayValues,
                    int  *          arraySizes
                );

/** checks the existance of a node, given the node name
    Returns 1 if the node exists, 0 otherwise.
*/
int             CDBCIExist(
                    CDBReference    cdbr,
                    const char *    entryName
                );

/** returns the number of children of the present node.  
    A negative number is returned if the cdbr is invalid or
    the present node is a leaf.
*/
int             CDBCINumberOfChildren(
                    CDBReference    cdbr
                );

/** returns the name of the node, partial or full depending
    on the value of the command parameter. If the full parameter
    is 1 the whole path will be returned, otherwise the node name
    will be returned.
    The pointer to the string will be allocated by the function 
    and must be freed by the user.

    If the cdbr reference is invalid a NULL pointer will be returned.
*/
char            *CDBCINodeName(
                    CDBReference    cdbr,
		    int             full
                );



/**  to deallocate any of the allocated data */
void            CDBCIFree(
                    void *          data
                );

/** Switch number parsing in the Lexical Analyser on/off
 * on=1 number parsing is active
 * on=0 number partial is not active
 */
void            CDBCISwitchNumberParsing(int on);

/** Function pointers */
typedef CDBReference(*cdbCICreate)(CDBReference);
typedef void(*cdbCIDestroy)(CDBReference   *);
typedef int(*cdbCIClean)(CDBReference);
typedef int(*cdbCILoad)(CDBReference,const char *);
typedef int(*cdbCISave)(CDBReference,char **);
typedef int(*cdbCIMove)(CDBReference,const char *,char **);
typedef int(*cdbCIReadArray)(CDBReference,CDBCDataType,void **,int  **);
typedef int(*cdbCIWriteArray)(CDBReference,const char *,CDBCDataType,void *,int  *);
typedef int(*cdbCIExist)(CDBReference,const char *);
typedef int(*cdbCINumberOfChildren)(CDBReference);
typedef char*(*cdbCINodeName)(CDBReference,int);
typedef void(*cdbCIFree)(void *);
typedef void(*cdbCISetErrorFunction)(ErrorReportingFunction,int);
typedef void(*cdbCISwitchNumberParsing)(int);

/** Function table struct */
typedef struct bl2cdbCI{
    cdbCISetErrorFunction    CDBCISetErrorFunction;
    cdbCICreate              CDBCICreate;
    cdbCIDestroy             CDBCIDestroy;
    cdbCIClean               CDBCIClean;
    cdbCILoad                CDBCILoad;
    cdbCISave                CDBCISave;
    cdbCIMove                CDBCIMove;
    cdbCIReadArray           CDBCIReadArray;
    cdbCIWriteArray          CDBCIWriteArray;
    cdbCIExist               CDBCIExist;
    cdbCINumberOfChildren    CDBCINumberOfChildren;
    cdbCINodeName            CDBCINodeName;
    cdbCIFree                CDBCIFree;
    cdbCISwitchNumberParsing CDBCISwitchNumberParsing;
}BL2cdbCI;

/** Returns the API struct */
BL2cdbCI *GetCDBCI();

#ifdef __cplusplus
    }
#endif

#endif

