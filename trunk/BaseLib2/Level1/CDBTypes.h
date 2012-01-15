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
 * CDB definitions
 */
#if !defined(CONFIGURATION_DATABASE_TYPES)
#define CONFIGURATION_DATABASE_TYPES

#include "System.h"
#include "BasicTypes.h"
#include "BString.h"


#if !defined(_CINT)

/** A set of values defining different ways of parsing the file. */
enum CDBWriteMode{
/** @name Set of mutually exclusive values
    @{
*/
    /** mask to identify the bits containing the mutually exclusive value */
    CDBWM_Modes     = 0x000000FF,

    /** write in tree mode with indents and \n */
    CDBWM_Tree      = 0x00000001,

    /** use commas as separators. disabled by CDBWM_Tree */
    CDBWM_Comma     = 0x00000002,

/** @}
    @name Set of flags to be ored.     Create the mode by oring them and then cast the result back to CDBWriteMode
    @{
*/
    /** do not indent. option for CDBWM_Tree */
    CDBWM_NoIndent  = 0x00000100,

    /** remove the no indent. remove an option for CDBWM_Tree */
    CDBWM_Indent    = ~CDBWM_NoIndent,

    /** do not put any space before next name = sentence  because of name joining */
    CDBWM_NameJoin  = 0x00000200,

    /** */
    CDBWM_NameNoJoin= ~CDBWM_NameJoin,

    /** */
    CDBWM_AllmanStyle = 0x00000400,

    /** */
    CDBWM_NoAllmanStyle = ~CDBWM_AllmanStyle

/** @}
*/
};

/** allows combining CDBWriteMode elements using | */
static inline CDBWriteMode operator|(CDBWriteMode a,CDBWriteMode b){ return (CDBWriteMode) ((int)a|(int)b); }

/** allows combining CDBWriteMode elements using & */
static inline CDBWriteMode operator&(CDBWriteMode a,CDBWriteMode b){ return (CDBWriteMode) ((int)a&(int)b); }

/** allows combining CDBWriteMode elements using ~ */
static inline CDBWriteMode operator~(CDBWriteMode a){ return (CDBWriteMode) (~(int)a); }

/** specify a set of flags to determine the addressing of the data.
    for instance it specifies whether an operation is relative to the
    local context or to the whole database. These flags can be ored together */
enum CDBAddressMode {

    /** no flags. None of the options below is active */
    CDBAM_None          = 0x00000000,

    /** operation is relative to the whole database. in the case of CleanUp this means erasing all the nodes */
    CDBAM_FromRoot      = 0x00000001,

    /** operation is relative to the current pointed node but excluding it */
    CDBAM_SkipCurrent   = 0x00000002,

    /** operation involves only the subtree */
    CDBAM_SubTreeOnly   = 0x00000004,

    /** do not count the nodes */
    CDBAM_LeafsOnly     = 0x00000008,

    /** relative to current position */
    CDBAM_Relative      = 0x00000010

};

/** allows combining CDBAddressMode elements using | */
static inline CDBAddressMode operator|(CDBAddressMode a,CDBAddressMode b){ return (CDBAddressMode) ((int)a|(int)b); }

/** allows combining CDBAddressMode elements using & */
static inline CDBAddressMode operator&(CDBAddressMode a,CDBAddressMode b){ return (CDBAddressMode) ((int)a&(int)b); }

/** allows combining CDBAddressMode elements using ~ */
static inline CDBAddressMode Not(CDBAddressMode a){ return (CDBAddressMode) ~(int)a; }


/** specify a set of flags to determine the how the size of ana array is computed. */
enum CDBArrayIndexingMode {

    /** calculates the envelop that contains a sparse array */
    CDBAIM_Flexible     = 0x00000000,

    /** operation expects a full array with no missing rows or columns */
    CDBAIM_Strict       = 0x00000001,

    /** thsi is not the first call in a recursion. only added internally */
    CDBAIM_Recursion    = 0x00000002

};

/** allows combining CDBAddressMode elements using | */
static inline CDBArrayIndexingMode operator|(CDBArrayIndexingMode a,CDBArrayIndexingMode b){ return (CDBArrayIndexingMode) ((int)a|(int)b); }

/** some flags for controlling methods of CDBNode */
enum CDBNMode{
    /* the mode of operation mask  */
    CDBN_ModeMask           =   0x000000FF,

    /* mode of operation: none */
    CDBN_None               =   0x00000000,

    /* mode of operation: just search */
    CDBN_SearchOnly         =   0x00000001,

    /* mode of operation: search first return found on create if missing */
    CDBN_SearchAndCreate    =   0x00000002,

    /* mode of operation: search first then fail if found or create if missing */
    CDBN_CreateOnly         =   0x00000003,

    /* if set then link following is enabled  */
    CDBN_FollowLink         =   0x00000100,

    /* if set then allow matching a subname  */
    CDBN_PartialMatch       =   0x00000200,

    /* match all the name but the last segment  */
    CDBN_MatchAllButLast    =   0x00000400,

    /* creation mask  */
    CDBN_CreateMask         =   0xFF000000,

    /* creation mask: create a group node  */
    CDBN_CreateGroupNode    =   0x00000000,

    /* creation mask: create a data node  */
    CDBN_CreateStringNode   =   0x01000000,

    /* creation mask: create a link node  */
    CDBN_CreateLinkNode     =   0x02000000
};

/** allow using | operator */
static inline CDBNMode operator|(CDBNMode a,CDBNMode b){
    return (CDBNMode) ((int)a | (int)b);
}


/** specifies some details of how a CDB is created. These values can be ored. */
enum CDBCreationMode{
    /** No flags */
    CDBCM_None        = 0,
    /** Copy the address when creating a reference */
    CDBCM_CopyAddress = 1
};


#define CDBDataType BasicTypeDescriptor

///** enums for different types of data storable and retruevable from CDB */
//enum CDBDataType {

/** a single precision floating point */
static const CDBDataType CDB_float     = BTDFloat;

/** a double precision floating point */
static const CDBDataType CDB_double    = BTDDouble;

/** a 32 bit integer */
static const CDBDataType CDB_int32     = BTDInt32;

/** an unsigned 32 bit integer */
static const CDBDataType CDB_uint32    = BTDUint32;

/** a 64 bit integer */
static const CDBDataType CDB_int64     = BTDInt64;

/** a pointer*/
static const CDBDataType CDB_Pointer   = BTDPointer;

/** a character */
static const CDBDataType CDB_char      = BTDInt8;

/** a string content delivered as FString */
static const CDBDataType CDB_FString   = BasicTypeDescriptor (0,BTDTString,BTDSTFStringArray);

/** a string content delivered as char ** => char *p="bbb"  use (&p). On output char *p = NULL, p will be allocated by function */
static const CDBDataType CDB_String    = BTDCString;

/**  */
static const CDBDataType CDB_None      = BasicTypeDescriptor (1,BTDTNone,BTDSTNone);

/** format the data in the CDB text file format: name = value */
static const CDBDataType CDB_CDBStyle  = BasicTypeDescriptor (2,BTDTNone,BTDSTNone);

/** format the data in the CDB text file format: name = value but evaluate variable fields */
static const CDBDataType CDB_CDBEval   = BasicTypeDescriptor (3,BTDTNone,BTDSTNone);

/** data in object native format delivered using char *. Used only for writing */
static const CDBDataType CDB_Interpret = BasicTypeDescriptor (4,BTDTNone,BTDSTNone);

/** data in object native format delivered using FString. Used only for reading */
static const CDBDataType CDB_Content   = BasicTypeDescriptor (5,BTDTNone,BTDSTNone);

/** a string content delivered as BString */
static const CDBDataType CDB_BString   = BTDBStringArray;
//};


/** the description of a datatype to be stored in CDB */
struct CDBTYPE{

    /** describes the data type read/written to the CDB*/
    CDBDataType     dataType;

    /** the size of the element */
    int             size;

    /** NULL means use standard container: CDBStringDataNode */
    const char *    containerClassName;

    CDBTYPE(    CDBDataType     dataType,
                int             size,
                const char *    containerClassName
    ){
        this->dataType              = dataType;
        this->size                  = size;
        this->containerClassName    = containerClassName;
    }
};

/** constant to request the transformation to and from a float */
static const CDBTYPE CDBTYPE_float       (CDB_float    ,sizeof(float)           ,NULL);
//static const CDBTYPE CDBTYPE_float     = {CDB_float    ,sizeof(float)           ,NULL};
/** constant to request the transformation to and from a double */
static const CDBTYPE CDBTYPE_double      (CDB_double   ,sizeof(double)          ,NULL);
/** constant to request the transformation to and from a int32 */
static const CDBTYPE CDBTYPE_int32       (CDB_int32    ,sizeof(int32)           ,NULL);
/** constant to request the transformation to and from a int32 */
static const CDBTYPE CDBTYPE_int64       (CDB_int64    ,sizeof(int64)           ,NULL);
/** constant to request the transformation to and from a uint32 */
static const CDBTYPE CDBTYPE_uint32      (CDB_uint32   ,sizeof(uint32)          ,NULL);
/** constant to request the transformation to and from a char */
static const CDBTYPE CDBTYPE_char        (CDB_char     ,sizeof(char)            ,NULL);
//** constant to request the transformation to and from a int as hex */
//static const CDBTYPE CDBTYPE_hex       = (CDB_hex      ,sizeof(uint32)          ,NULL);
//** constant to request the transformation to and from a int as octal */
//static const CDBTYPE CDBTYPE_octal     = (CDB_octal    ,sizeof(uint32)          ,NULL);
/** constant to request the transformation to and from a char * */
static const CDBTYPE CDBTYPE_String      (CDB_String   ,sizeof(char *)          ,NULL);
/** constant to request the transformation to and from a BString
    data is the address of a vector of BStrings */
static const CDBTYPE CDBTYPE_BString     (CDB_BString  ,sizeof(BString)         ,NULL);
/** constant to get the raw data stored in the node  */
static const CDBTYPE CDBTYPE_NULL        (CDB_None,0                            ,NULL);
/** constant to request the transformation to and from a uint32 */
static const CDBTYPE CDBTYPE_Pointer     (CDB_Pointer, sizeof(size_t)           ,NULL);

#endif 

#endif


