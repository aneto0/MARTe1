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
 * Descriptor for all the basic types int long short float double ...
 */
#if !defined (BASIC_TYPES_H)
#define BASIC_TYPES_H

#include "System.h"
#include "GenDefs.h"
#include "BString.h"

#include "ErrorManagement.h"

#if defined (_CINT)

struct BasicTypeDescriptor {
    int32 typeDescription;
};
#else


/** the types of data in the field type of BasicTypeDescriptor*/
enum BTDTypes{

    /** An integer */
    BTDTInteger         = 0,

    /** standard float */
    BTDTFloat           = 1,

    /** any type of string*/
    BTDTString          = 2,

    /** Pointers */
    BTDTPointer         = 4,

    /** Not a BTDType */
    BTDTNone            = 15

};

/** modifier to the type */
typedef int BTDSubTypes ;

/** Define some more details */
union BTDFormat{

/** The format depends on the endianity so that to fit in
the 14 bit space in the BasicType */
#if defined INTEL_BYTE_ORDER

    /** used in single strings and streams */
    struct {
        /** up to 32 figures */
        unsigned int minNumberOfFigures:5;

        /** padding
            0 = none
            1 = space
            2 = 0
            ...
        */
        int padding:3;

        /** mode
            0 = normal
            1 = hex
            2 = octal
            ...
        */
        int mode:3;

        /** */
        int unused: 21;

    } intInfo;

    /** used in floats to determine how many meaningful figures are there */
    struct {

        /** up to 32 figures */
        unsigned int minNumberOfFigures:5;

        /** padding
            0 = none
            1 = space
            2 = 0
            ...
        */
        int padding:3;

        /** 1 means using %f */
        bool fixedFormat:1;

        /** */
        int unused: 23;

    }floatInfo;

    /** used in strins of CARRAY and BSTRING type */
    struct {

        /** separator
            0 = space
            1 = comma
            2 = semicolon
            3 = newline
            ...
        */
        int separator:3;


        /* totalSize = size * 10 ^ sizeExp
            will be set by the constructor of BasicTypes */
        unsigned int sizeExp:4;

        /** */
        int unused: 25;

    } stringInfo;

#else // endianity switch

    /** used in single strings and streams */
    struct {

        /** */
        int unused: 21;

        /** up to 32 figures */
        unsigned int minNumberOfFigures:5;

        /** padding
            0 = none
            1 = space
            2 = 0
            ...
        */
        int padding:3;

        /** mode
            0 = normal
            1 = hex
            2 = octal
            ...
        */
        int mode:3;

    } intInfo;

    /** used in floats to determine how many meaningful figures are there */
    struct {

        /** */
        int unused: 23;

        /** up to 32 figures */
        unsigned int minNumberOfFigures:5;

        /** padding
            0 = none
            1 = space
            2 = 0
            ...
        */
        int padding:3;

        /** 1 means using %f */
        bool fixedFormat:1;

    } floatInfo;

    /** used in strins of CARRAY and BSTRING type */
    struct {

        /** */
        int unused: 25;

        /** separator
            0 = space
            1 = comma
            2 = semicolon
            3 = newline
            ...
        */
        int separator:3;


        /* totalSize = size * 10 ^ sizeExp
            will be set by the constructor of BasicTypes */
        unsigned int sizeExp:4;

    } stringInfo;

#endif // endianity switch

    /** constructor */
    BTDFormat(int x=0){
        int *p = (int *)this;
        *p = x;
    };

    /** its integer equivalent */
    int Value(){
        int *p = (int *)this;
        return *p;
    }
};

/** empty BTDFormat */
static const BTDFormat      BTDFNone;

/** no flag */
static const BTDSubTypes    BTDSTNone       = 0;

/** FLAGS relative to BTDTInteger */

/** modifier for the integer */
static const BTDSubTypes    BTDSTUnsigned   = 1;

/** FLAGS relative to BTDTString */

/** the void * is cast to char **
    it is assumed to be a vector of pointers of size matching
    that of the source,
    when used as destination the existing pointer is
    first freed and then replaced with a new malloced one*/
static const BTDSubTypes    BTDSTCString        = 1;

/** char[], size field is the array size,
    the string is always 0 terminated */
static const BTDSubTypes    BTDSTCArray         = 2;

/** BString class, size field is meaningless
    void * is BString *
    it is a pointer to a single BString
    the parts will be separarted using the
    character specified in the format field  */
static const BTDSubTypes    BTDSTBString        = 3;

/** FString class, size field is meaningless
    void * is FString *
    it is a pointer to a single FString
    the parts will be separated using the
    character specified in the format field  */
static const BTDSubTypes    BTDSTFString        = 4;

/** BString class, size field is meaningless
    void * is BString *
    it is an array of BStrings matching the
    input size  */
static const BTDSubTypes    BTDSTBStringArray   = 5;

/** FString class, size field is meaningless
    void * is FString *
    it is an array of FStrings matching the
    input size  */
static const BTDSubTypes    BTDSTFStringArray   = 6;

/** StreamInterface class, size field is meaningless */
static const BTDSubTypes    BTDSTStream         = 7;


/** Word Bit Size */
static const int32          BYTE_BIT_SIZE       = 8;

/** Word Bit Size */
static const int32          WORD32_BIT_SIZE     = BYTE_BIT_SIZE * 4;

struct BasicTypeDescriptor;


extern "C" {

    /** Returns the string format */
    const char *BTConvertToString(const BasicTypeDescriptor &bt,BString &string);

    /** Copies from a string format */
    bool BTConvertFromString(BasicTypeDescriptor &bt,const char *name);

    /** convert from one basic type to another
        @param numberOfElements is the number of elements to convert
        @param source and destination point respectively to the source
        data and the destination data
        @param sourceBTD and destinationBTD describe the two data areas */
    bool BTConvert( int                 numberOfElements,
                    BasicTypeDescriptor destinationBTD,
                    void *              destination,
                    BasicTypeDescriptor sourceBTD,
                    const void *        source
                    );

}


/** descriptor for all the basic types int long short float double ... */
struct BasicTypeDescriptor {

    friend const char *BTConvertToString(
                            const  BasicTypeDescriptor &bt,
                            BString &           string);
    friend bool BTConvertFromString(
                            BasicTypeDescriptor &bt,
                            const char *        name);
    friend bool BTConvert(  int                 numberOfElements,
                            BasicTypeDescriptor destinationBTD,
                            void *              destination,
                            BasicTypeDescriptor sourceBTD,
                            const void *        source
                        );

private:
    /** the size of the object, up to 1024bit. For some types the numberOfBits is 0 always
        for char[] is t
        he number of characters */
    uint32      size:10;

    /** actually a BTDFormat */
    int         format:14;

    /** up to 16 main types*/
    BTDTypes    type:4;

    /** 16 different subtypes (information partially reduntant) */
    BTDSubTypes flags:4;

public:
    /** to help building it */
    BasicTypeDescriptor()
    {
        this->size          = 32;
        this->type          = BTDTInteger;
        this->flags         = BTDSTNone;
        this->format        = 0;
    }

    /** to help building it */
    BasicTypeDescriptor(int32 equivalent)
    {
        int32 *p = (int32 *)this;
        *p = equivalent;
    }

    /** to help building it */
    BasicTypeDescriptor(uint32 size,BTDTypes type, BTDSubTypes flags,BTDFormat format = BTDFNone)
    {
        if (type == BTDTString){
            format.stringInfo.sizeExp = 0;
            while ((format.stringInfo.sizeExp < 15) && (size > 1024)){
                format.stringInfo.sizeExp ++;
                size = size / 10;
            }
        }
        if (size >= 1024) size = 1023;
        this->size          = size;
        this->type          = type;
        this->flags         = flags;
        this->format        = format.Value();
    }

    /** Copy Constructor */
    BasicTypeDescriptor(const BasicTypeDescriptor &desc)
    {
        this->size      = desc.size;
        this->type      = desc.type;
        this->flags     = desc.flags;
        this->format    = desc.format;
    }

    /** Operator ==
        omit format */
    BasicTypeDescriptor operator=(const BasicTypeDescriptor &desc)
    {
	this->size = desc.size;
        this->type = desc.type;
	this->flags = desc.flags;
	this->format = desc.format;
	return *this;
    }




    /** Operator ==
        omit format */
    bool operator==(const BasicTypeDescriptor &desc) const
    {
        bool ret = True;
        ret &= (this->size  == desc.size);
        ret &= (this->type  == desc.type);
        ret &= (this->flags == desc.flags);
        return ret;
    }

    bool operator!=(const BasicTypeDescriptor &desc) const
    {
        return !(this->operator==(desc));
    }

    /** How many bits */
    int32  BitSize() const
    {
        return size;
    }

    /** Size of the type in Bytes (minimum number of bytes to hold it ) */
    int32  ByteSize() const
    {
        return (size+BYTE_BIT_SIZE-1)/BYTE_BIT_SIZE;
    }

    /** Size of the Type in Words (minimum number of words to hold it ) */
    int32  Word32Size() const
    {
        return (size+WORD32_BIT_SIZE-1)/WORD32_BIT_SIZE;
    }

    /** The size of CArrays is extended beyond size using format.stringInfo.sizeExp*/
    uint32 CArraySize() const
    {
        uint32 size = this->size;
        int exp = Format().stringInfo.sizeExp;
        while (exp> 0){
            size = size * 10;
            exp--;
        }

        return size;
    }

    /** Returns the string format */
    const char *ConvertToString(BString &string) const
    {
	return BTConvertToString(*this,string);
    }

    /** Copies from a string format */
    bool ConvertFromString(const char *name)
    {
        return BTConvertFromString(*this,name);
    }

    /** Convert to int32 */
    int32 Value() const
    {
        int32 *p = (int32 *)this;
        return *p;
    }

    /** */
    BTDFormat Format() const
    {
        return BTDFormat(format);
    }

    /** */
    BTDTypes    Type() const
    {
        return type;
    }

    /**  */
    BTDSubTypes Flags() const
    {
        return flags;
    }

};


/** A 8 bit signed integer */
static const BasicTypeDescriptor BTDInt8  ( 8,BTDTInteger,BTDSTNone);

/** A 16 bit signed integer */
static const BasicTypeDescriptor BTDInt16 (16,BTDTInteger,BTDSTNone);

/** A 32 bit siganed integer */
static const BasicTypeDescriptor BTDInt32 (32,BTDTInteger,BTDSTNone);

/** A 64 bit signed integer */
static const BasicTypeDescriptor BTDInt64 (64,BTDTInteger,BTDSTNone);

/** A generic pointer */
static const BasicTypeDescriptor BTDPointer (8 * sizeof(size_t),BTDTPointer,BTDSTNone);

/** A 8 bit unsigned integer */
static const BasicTypeDescriptor BTDUint8 ( 8,BTDTInteger,BTDSTUnsigned);

/** A 16 bit unsigned integer */
static const BasicTypeDescriptor BTDUint16(16,BTDTInteger,BTDSTUnsigned);

/** A 32 bit unsigned integer */
static const BasicTypeDescriptor BTDUint32(32,BTDTInteger,BTDSTUnsigned);

/** A 64 bit unsigned integer */
static const BasicTypeDescriptor BTDUint64(64,BTDTInteger,BTDSTUnsigned);

/** A 32 bit float */
static const BasicTypeDescriptor BTDFloat (32,BTDTFloat,BTDSTNone);

/** A 64 bit float */
static const BasicTypeDescriptor BTDDouble(64,BTDTFloat,BTDSTNone);

/** A char * string */
static const BasicTypeDescriptor BTDCString(0,BTDTString,BTDSTCString);

/** A BString pointer */
static const BasicTypeDescriptor BTDBString(0,BTDTString,BTDSTBString);

/** A BString for each element */
static const BasicTypeDescriptor BTDBStringArray(0,BTDTString,BTDSTBStringArray);

/** A StreamInterface * */
static const BasicTypeDescriptor BTDStream (0,BTDTString,BTDSTStream);


extern "C" {


    /** skips trailing spaces and tabs
        recognises 0xNNNN as a hex number
        and 0NNN as a n octal number
        string at the end points at the terminator character
        maxNumberOfBits is how many bits to use including sign
        if the string saturates then the number return is the
        corresponing maxint
        if isSigned is False then the operation produces an unsigned int64 */
    int64 StringToInt64(const char *&string, int maxNumberOfBits=64,bool isSigned=True);

    /** writes on a BString */
    bool Int64ToString(BString &bs,int64 n,BTDFormat format);

    /** See StringToInt64 but with maxNumberOfBits set to 32 */
    inline int32 StringToInt32(const char *&string,bool isSigned=True){
        return StringToInt64(string,32,isSigned);
    }

    /** See StringToInt64 but with maxNumberOfBits set to 16 */
    inline int32 StringToInt16(const char *&string,bool isSigned=True){
        return StringToInt64(string,16,isSigned);
    }

    /** See StringToInt64 but with maxNumberOfBits set to 16 */
    inline int32 StringToInt8(const char *&string,bool isSigned=True){
        return StringToInt64(string,8,isSigned);
    }

    /** converts one int format into another. Operates with bit sizes up to 64 bits
        data is laoded and saved in bit compact form!
        destinationBitShift and sourceBitShift are the start position of the data,
        they are updated after each use
        destination and source are the addresses of output and input buffer,
        they also are updated */
    bool IntToInt(
            int64 *&            destination,
            int &               destinationBitShift,
            int                 destinationBitSize,
            bool                destinationIsSigned,
            int64 *&            source,
            int &               sourceBitShift,
            int                 sourceBitSize,
            bool                sourceIsSigned);

    /**  converts any int format into 64 bit signed. Operates with bit sizes up to 64 bits
        data is laoded and saved in bit compact form!*/
    bool IntToInt64(
        int64 *&            destination,
        int64 *&            source,
        int &               sourceBitShift,
        int                 sourceBitSize,
        bool                sourceIsSigned);

    /**  converts any int format into 64 bit signed. Operates with bit sizes up to 64 bits
        data is laoded and saved in bit compact form!*/
    bool IntToInt32(
        int32 *&            destination,
        int32 *&            source,
        int &               sourceBitShift,
        int                 sourceBitSize,
        bool                sourceIsSigned);        

    /**  converts 64 bit signed to any bit size */
    bool Int64ToInt(
            int64 *&            destination,
            int &               destinationBitShift,
            int                 destinationBitSize,
            bool                destinationIsSigned,
            int64 *&            sourceDataSigned);


    /**  converts 32 bit signed to any bit size */
    bool Int32ToInt(
            int64 *&            destination,
            int &               destinationBitShift,
            int                 destinationBitSize,
            bool                destinationIsSigned,
            int32 *&            sourceDataSigned);

};


/***************************************************************/
/*  ALLOWS EXPANSION TO STRING CONVERSION METHODS              */
/***************************************************************/

/** allows to read data from the input string   */
typedef char * (*BTStringGetDataFunction)(const void *&p);

/** allows to write data  maxSize  */
typedef void (*BTStringPutDataFunction)(const void *&p,const char *s,int maxSize,int pos,char sep);

extern BTStringPutDataFunction BTSPDFArray[16];

extern BTStringGetDataFunction BTSGDFArray[16];

#endif // CINT cannot see this

#endif

