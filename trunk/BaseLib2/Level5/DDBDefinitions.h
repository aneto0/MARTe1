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
 * Definition of types, enums, macros and the like for the DDB class and related
 * objects.
 */
#if !defined (DDBDEFINITIONS_H)
#define DDBDEFINITIONS_H

#include "Object.h"
#include "StreamInterface.h"
#include "ConfigurationDataBase.h"

class DDBInterfaceAccessMode;

extern "C" {

    /**  */
    bool DDBIAMObjectSaveSetup(const DDBInterfaceAccessMode& ddbiam, ConfigurationDataBase &info, StreamInterface* s);
}


/** The DDB access mode.
    The structure actually contains 2 conceptually different variables. The first is
    a coding for the type of ddbInterface used by its owner (and it really is an access
    mode to the DDB), the second represent the state of the DDB while adding interfaces
    and is used to detect errors.
    Users can access the DDB using 5 different interfaces or modalities:
    -# As a placeholder.
    -# As a reader.
    -# As a writer.
    -# As a patcher.
    -# As an exclusive writer.

    These access modes are encoded by a number of bits in a DDBAccessMode variable as shown in the following table.

    <TABLE>
        <TR>
            <TD>Access Type</TD><TD>R</TD><TD>W</TD><TD>P</TD><TD>E</TD>
        </TR>
        <TR>
            <TD>Placeholder</TD><TD>0</TD><TD>0</TD><TD>0</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Reader</TD><TD>1</TD><TD>0</TD><TD>0</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Writer</TD><TD>0</TD><TD>1</TD><TD>0</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Patcher</TD><TD>0</TD><TD>0</TD><TD>1</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Exclusive Writer</TD><TD>0</TD><TD>1</TD><TD>0</TD><TD>1</TD>
        </TR>
    </TABLE>

    Different users however (and also a single user), can access the same signal using
    different interfaces provided that they are compatible. Below it is shown the table
    of the compatible interfaces.

    <TABLE>
        <TR>
            <TD>Access Type</TD><TD>R</TD><TD>W</TD><TD>P</TD><TD>E</TD>
        </TR>
        <TR>
            <TD>Reader and Writer</TD><TD>1</TD><TD>1</TD><TD>0</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Reader and Exclusive Writer</TD><TD>1</TD><TD>1</TD><TD>0</TD><TD>1</TD>
        </TR>
        <TR>
            <TD>Reader and Patcher</TD><TD>1</TD><TD>0</TD><TD>1</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Writer and Patcher</TD><TD>0</TD><TD>1</TD><TD>1</TD><TD>0</TD>
        </TR>
        <TR>
            <TD>Reader, Writer and Patcher</TD><TD>1</TD><TD>1</TD><TD>1</TD><TD>0</TD>
        </TR>
    </TABLE>

    All of the other bits combinations are not valid. In particular it is worth noticing that
    if E is set also W is set.

    In a DDBItem object, where a list of users of the same signal is managed, the following access
    rules apply:
    -# Either there must be a user having the Writer access to the signal or all the users being Placeholders.
    -# No more than one user can have the Writer or Exclusive Writer access to that signal.
    -# If the writer has has the Exclusive Writer access the other users can't get the Patcher access.
 */
class DDBInterfaceAccessMode{

    friend bool DDBIAMObjectSaveSetup(const DDBInterfaceAccessMode& ddbiam, ConfigurationDataBase &info, StreamInterface* s);

    /** The variable coding the access mode. */
    int mode;

public:

    /** Standard constructor initialises to DDB_PlaceholderMode. */
    DDBInterfaceAccessMode() : mode(0) {}

    /** Constructor from integer. Unfortunately is not possible to
        avoid usign it to make the constants. */
    DDBInterfaceAccessMode(const int intMode) :
        mode(intMode) {}

    /** Copy constructor. */
    DDBInterfaceAccessMode(const DDBInterfaceAccessMode& reference) :
        mode(reference.mode) {}

    /** Assignement operator. */
    DDBInterfaceAccessMode& operator=(const DDBInterfaceAccessMode& right){
        mode=right.mode;
        return *this;
    }

    /** Bitwise-or and assignement operator. */
    DDBInterfaceAccessMode& operator|=(const DDBInterfaceAccessMode& right){
        mode|=right.mode;
        return *this;
    }

    /** Bitwise-and and assignement operator. */
    DDBInterfaceAccessMode& operator&=(const DDBInterfaceAccessMode& right){
        mode&=right.mode;
        return *this;
    }

    /** Bitwise-not operator. */
    DDBInterfaceAccessMode operator~() const{
        DDBInterfaceAccessMode result(~mode);
        return result;
    }

    /** Is equal to operator. */
    bool operator==(const DDBInterfaceAccessMode& reference) const{
        return mode==reference.mode;
    }

    /** Check *this with a mask. */
    bool CheckMask(const DDBInterfaceAccessMode& mask) const{
        return ((mode&mask.mode)==mask.mode);
    }

    /** Print method. */
    void Print(StreamInterface& s) const{
        ConfigurationDataBase cdb;
        ObjectSaveSetup(cdb,NULL);
        cdb->WriteToStream(s);
    }

    /** Save Parameters to CDB*/
    bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err)const{
            return DDBIAMObjectSaveSetup(*this, info,err);
    }

};

/** Bitwise-or operator. */
inline DDBInterfaceAccessMode operator|(const DDBInterfaceAccessMode left,
                                        const DDBInterfaceAccessMode right){
    DDBInterfaceAccessMode result(left);
    return result|=right;
}

/** Bitwise-and operator. */
inline DDBInterfaceAccessMode operator&(const DDBInterfaceAccessMode left,
                                        const DDBInterfaceAccessMode right){
    DDBInterfaceAccessMode result(left);
    return result&=right;
}

//Should this stuff be under extern C?

/** The value of the Placeholder. */
const DDBInterfaceAccessMode DDB_PlaceholderMode(0x00);

/** The value of the ReadMode. */
const DDBInterfaceAccessMode DDB_ReadMode(0x01);

/** The value of the WriteMode. */
const DDBInterfaceAccessMode DDB_WriteMode(0x02);

/** The value of the PatchMode. */
const DDBInterfaceAccessMode DDB_PatchMode(0x04);

/** The value of the WriteExclusive. */
const DDBInterfaceAccessMode DDB_ExclusiveWriteMode(0x0a);

/** The mask for the value of the bit WriteExclusive. */
const DDBInterfaceAccessMode DDB_ExclusiveWriteBit(0x08);

/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/


/** A new implementation of the DDBAccessMode. */
class DDBItemStatus{

    /** The variable coding the status of the DDBItem. */
    int status;

public:

    /** Standard constructor initialises to OK. */
    DDBItemStatus() : status(0) {}

    /** Constructor from integer. Unfortunately is not possible to
        avoid usign it to make the constants. */
    DDBItemStatus(const int intStatus) :
        status(intStatus) {}

    /** Copy constructor. */
    DDBItemStatus(const DDBItemStatus& reference) :
        status(reference.status) {}

    /** Assignement operator. */
    DDBItemStatus& operator=(const DDBItemStatus right){
        status=right.status;
        return *this;
    }

    /** Bitwise-or and assignement operator. */
    DDBItemStatus& operator|=(const DDBItemStatus right){
        status|=right.status;
        return *this;
    }

    /** Bitwise-and and assignement operator. */
    DDBItemStatus& operator&=(const DDBItemStatus right){
        status&=right.status;
        return *this;
    }

    /** Bitwise-not operator. */
    DDBItemStatus operator~() const{
        DDBItemStatus result(~status);
        return result;
    }

    /** Is equal to operator. */
    bool operator==(const DDBItemStatus& reference) const{
        return status==reference.status;
    }

    /** Check *this with a mask. */
    bool CheckMask(const DDBItemStatus& mask) const{
        return ((status&mask.status)==mask.status);
    }

    /** Print method. */
    void Print(StreamInterface& s) const;

};

/** Bitwise-or operator. */
inline DDBItemStatus operator|(const DDBItemStatus& left,
                               const DDBItemStatus& right){
    DDBItemStatus result(left);
    return result|=right;
}

/** Bitwise-and operator. */
inline DDBItemStatus operator&(const DDBItemStatus& left,
                               const DDBItemStatus& right){
    DDBItemStatus result(left);
    return result&=right;
}

/** The DDBItem status is OK. */
const DDBItemStatus DDB_NoError(0x00);

/** More than one user set as Writer on the signal. */
const DDBItemStatus DDB_WriteConflictError(0x01);

/** One or more user set as Patcher of a signal which is exclusively written by another user. */
const DDBItemStatus DDB_WriteExclusiveViolation(0x02);

/** The signal is read by a user but it is not written by any user. */
const DDBItemStatus DDB_MissingSourceError(0x04);

/** It has not been possible to arrange the items in the DDB as requested by the
    consecutive bit. */
const DDBItemStatus DDB_ConsecutivityViolation(0x08);

/** Set when a reader of a signal try to access it beyond the limit specified
    by the writer. */
const DDBItemStatus DDB_SizeMismatch(0x10);

/** Set when a signal is usized. */
const DDBItemStatus DDB_UndefinedSize(0x20);

/** Set when a mismatch between type is found. */
const DDBItemStatus DDB_TypeMismatch(0x40);

/** Set when a mismatch between type is found. */
const DDBItemStatus DDB_UndefinedType(0x80);

/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/

/** The type describing the way data are stored in the DDB. */
class DDBStoringProperties{
private:
    /** The value coding the property mask. */
    int32 properties;

public:

    /** Constructor. Standard constructor initialises to OK. */
    DDBStoringProperties() : properties(0) {}

    /** Constructor from integer. Unfortunately is not possible to
        avoid usign it to make the constants. */
    DDBStoringProperties(const int intProperties) :
        properties(intProperties) {}

    /** Copy constructor. */
    DDBStoringProperties(const DDBStoringProperties& reference) :
        properties(reference.properties) {}

    /** Assignement operator. */
    DDBStoringProperties& operator=(const DDBStoringProperties right){
        properties=right.properties;
        return *this;
    }

    /** Bitwise-or and assignement operator. */
    DDBStoringProperties& operator|=(const DDBStoringProperties right){
        properties|=right.properties;
        return *this;
    }

    /** Bitwise-and and assignement operator. */
    DDBStoringProperties& operator&=(const DDBStoringProperties right){
        properties&=right.properties;
        return *this;
    }

    /** Bitwise-not operator. */
    DDBStoringProperties operator~() const{
        DDBStoringProperties result(properties);
        return result;
    }

    /** Is equal to operator. */
    bool operator==(const DDBStoringProperties& reference) const{
        return properties==reference.properties;
    }

    /** Check *this with a mask. */
    bool CheckMask(const DDBStoringProperties& mask) const{
        return ((properties&mask.properties)==mask.properties);
    }

    /** Print method. */
    void Print(StreamInterface& s) const;

};

DDBStoringProperties operator|(const DDBStoringProperties& left,
			       const DDBStoringProperties& right);

DDBStoringProperties operator&(const DDBStoringProperties& left,
			       const DDBStoringProperties& right);

/** The signal structure is inserted without using fully qualified names. */
const DDBStoringProperties DDB_Default(0x00);

/** The signal structure is inserted without using fully qualified names. */
const DDBStoringProperties DDB_FlatNamed(0x01);

/** The DDB tries to allocate the signal just next to the previous. */
const DDBStoringProperties DDB_Consecutive(0x02);

/** The dimensions of signal are not fully specified. */
const DDBStoringProperties DDB_Unsized(0x04);

#endif
