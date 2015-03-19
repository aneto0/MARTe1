/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
 permissions and limitations under the Licence. 
 *
 * $Id: $
 *
 **/
/**
 * @file
 */
#ifndef FORMAT_DESCRIPTOR_H
#define FORMAT_DESCRIPTOR_H

#include "GeneralDefinitions.h"
/** 
    Used to choose the float and binary representation modes
*/
struct Notation{
    enum Float{
        /** 
          0.99  3.5 35000.2 etc...
        */
        FixedPointNotation  =   0,

        /** 
          9.9E-1 3.5E0 3.5E4 etc....
        */
        ExponentNotation    =   1,

        /** 
          990E-3 3.5E0 35E3 etc....
        */
        EngineeringNotation =   2,

        /** 
          Most meaningful notation fitting within constraints
          Choice among FixedPoint, Exponent Notation and Engineering notation with size symbols
          3000 -> 3K  3.6E12  3.6T
        */
        SmartNotation       =   3
    };

    enum Binary {

        /** 
          print in the native format (integer base 10, float as ieee or whatever ....)
        */
        NormalNotation      =   0,

        /** 
          print in hexadecimal -> the data pointed to by the pointer is treated as a char *
          each character is printed as two hexes (0-F)
        */
        HexNotation         =   1,

        /** 
          print in binary -> the data pointed to by the pointer is treated as a char *
          each character is printed as 8 bits (0-1)
        */
        BitNotation         =   2,

        /** 
          print in octal -> the data pointed to by the pointer is treated as a uint24 *
          each character is printed as three octals (0-7)
        */
        OctalNotation       =   3
    };
};


/** Describes how a basic type shall be printed (transformed into a string) */
class FormatDescriptor{
public:
//*** MAYBE REPLACE with finite set of options ( *' ' *0  *' '0x *' ', ....)
    /// maximum size of representation  max 255
    uint32                   width:8;

    /**
        minimum (whenever applicable) number of meaningful digits (unless overridden by width)  max 64 
		differently from printf this includes characters before comma
		excludes characters used for the exponents or for the sign and the .
		0.34 has precision 2        -> (precision =8)  0.3400000 
		234 has precision 3         -> (precision =8)  234.00000 
		2345678000 has precision 10 -> (precision =8) unchanged still precision 10
		2.345678E9 has precision 7  -> (precision =8) 2.3456780E9 
		234 (int) has precision 3   -> (precision =8) unchanged  still precision 3  
		0x4ABCD has precision 5     -> (precision =8) unchanged  still precision 5  
		1-64 
	*/	 
    uint32                   precision:8;

    /**
		True means produce a number of characters equal to width 
		fill up using spaces 
	*/
    bool                     pad:1;

    /** 
		True means to produce pad spaces after printing the object representation
    */
    bool                     leftAlign:1; 

    
    /// in case of a float, this field is used to determine how to print it
    Notation::Float          floatNotation:2;

    /// used for ints, floats, pointers, char * etc...
    Notation::Binary         binaryNotation:2;

	/** 
		only meaningful for numbers in Hex/octal/binary
		Fills the number on the left with 0s up to the full representation 
		Number of 0s depends on the size of the number (hex 64 bit ==> numbers+trailing zero = 16)
	*/
    bool                     binaryPadded:1;

    /** 
		only meaningful for numbers
		Add the missing + or 0x 0B or 0o
	*/
    bool                     fullNotation:1;

    //       
    int32                    spareBits:8;
    
	/** takes a printf like string already pointing at the character after % (see below format)
	    and parses it recovering all the useful information, discarding all redundant ones,
		and fills up the fields in this structure.
	    At the end the pointer string is moved to the next character after the parsed block
		
		The overall printf-like format supported is the following:
		%[flags][width][.precision]type
		Note that the full printf would be this:
		%[parameter][flags][width][.precision][length]type
		!which is not supported!
			
		[flags]: // slightly different from standard printf notation
		' ' Activates padding:
			fills up to width using spaces
		-  	Left-align : put padding spaces after printing the object
		#   Activate fullNotation:
			+ in front of integers
			0x/0b/0o in front of Hex/octal/binary
        0  	Prepends zeros for Hex Octal and Binary notations (binaryPadded activated)
			Number of zeros depends on number precision and chosen notation (64 bit int and binary notation = up to 64 zeros)
		
		[width][.precision]  two numbers 
		[width] specifies the MAXIMUM or EXACT number of characters to output, depending on the padding [flags] being set on or not 
		NOTE that in a normal printf this would be the MINIMUM or EXACT number of characters... 
        [Precision] 
		This is the minimum number of meaningful digits used to represent a number 
		Differently from printf this includes numbers before .
		if the exact representation of the number uses less digits [precision] is not considered
		if [width] is such that a numeric representation with the given precision cannot be fully represented than the number is replaced with a ?
		type
		This is one character among the following
		
		d,i,u,s,c --> no effect (format depends on actual data type not the letter here!)
        f --> fixed point numeric format selected
        e --> exponential format
		g --> smart format (more powerful than printf)
		a,x,p --> activate exadecimal display
        o --> activate octal display
        b --> activate binary display
	*/
    bool InitialiseFromString(const char *&string);

    /**
     * Format descriptor is not initialised
     */
	FormatDescriptor(){
    }

	/** 
		constructor from unsigned integer
		Just copy bit by bit
	*/
	FormatDescriptor(uint32 x){
		uint32 *p = (uint32 * )this;
		*p = x;
	}
    
	/**
        Copy operator 
    */
    void operator=(const FormatDescriptor &src){
        uint32 *d = (uint32 *)this;
        const uint32 *s = (const uint32 *)&src;
        *d = *s;
    }

	/**
        or bits operator
    */
    void operator |=(const FormatDescriptor &src){
        uint32 *d = (uint32 *)this;
        const uint32 *s = (const uint32 *)&src;
        *d |= *s;
    }
    
    /**
        Two FormatDescriptors are equal if every field is equal
    */
    bool operator ==(const FormatDescriptor &src){
        uint32 *p = (uint32 * )this;
        uint32 *s = (uint32 * )&src;
		return *p == *s;
    }

    /**
        Two FormatDescriptors are different if one of the fields is different 
    */
    bool operator !=(const FormatDescriptor &src){
        uint32 *p = (uint32 * )this;
        uint32 *s = (uint32 * )&src;
		return *p != *s;
    }

	/** 
		constructor from unsigned integer
		Just copy bit by bit
	*/
    FormatDescriptor(uint8 width, uint8 precision, bool pad, bool leftAlign, 
                        Notation::Float floatNotation,Notation::Binary binaryNotation, 
                        bool binaryPadded, bool fullNotation ){

		this->width = width;
		this->precision = precision;
		this->pad = pad;
		this->leftAlign = leftAlign;
		this->floatNotation = floatNotation;
		this->binaryNotation = binaryNotation;
		this->binaryPadded = binaryPadded;
		this->fullNotation = fullNotation;
        this->spareBits = 0;
	}
};



/**

START .CPP FILE

DATA TABLES FIRST

*/

/** to implement a look-up table between characters in the 
    printf format encoding and the set of bits to be set in 
    the FormatDescriptor 
*/
struct FDLookup{
    // the character in the printf format
    char character;
    // the set of flags
    FormatDescriptor format;
};

/// 0 terminated vector of FDLookups 
static const FDLookup flagsLookup[] = {
    { ' ', 	FormatDescriptor(0,0,True ,False,Notation::FixedPointNotation, Notation::NormalNotation,False,False) },  // ' '	
	{ '-', 	FormatDescriptor(0,0,True ,True ,Notation::FixedPointNotation, Notation::NormalNotation,False,False) },  // '-'
    { '0', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::NormalNotation,True ,False) },  // '0'
    { '#', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::NormalNotation,False,True)  },  // '#'
    { 0  ,  FormatDescriptor(0)}
};

/// 0 terminated vector of FDLookups 
static const FDLookup typesLookup[] = {
    { 'd', 	FormatDescriptor(0) },   //d	
    { 'i', 	FormatDescriptor(0) },  //i
    { 'u', 	FormatDescriptor(0) },  //u
    { 's', 	FormatDescriptor(0) },  //s
    { 'c', 	FormatDescriptor(0) },  //c
    { 'f', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::NormalNotation,False,False) },  //f
    { 'e', 	FormatDescriptor(0,0,False,False,Notation::ExponentNotation  , Notation::NormalNotation,False,False) },  //e
    { 'g', 	FormatDescriptor(0,0,False,False,Notation::SmartNotation     , Notation::NormalNotation,False,False) },  //g
    { 'a', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::HexNotation   ,False,False) },  //a
    { 'x', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::HexNotation   ,False,False) },  //x
    { 'p', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::HexNotation   ,False,False) },  //p
    { 'o', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::OctalNotation ,False,False) },  //o
    { 'b', 	FormatDescriptor(0,0,False,False,Notation::FixedPointNotation, Notation::BitNotation   ,False,False) },  //b
    { 0  ,  FormatDescriptor(0)}
};

/**

FUNCTIONS BELOW

*/


/// strchr equivalent
static inline const FDLookup *LookupCharacter(char c, const FDLookup *lookupTable){
    if (lookupTable == NULL) {
        return NULL;
    }
    while(lookupTable->character != 0){
        if (lookupTable->character == c){
            return lookupTable;
        }
        lookupTable++;
    }
    return NULL;
}


/// add to format the identified flag
static bool ParseCharacter(char c, FormatDescriptor &format,const FDLookup *lookupTable){
    
    // find the position of c in flagsLookup
     const FDLookup *found = LookupCharacter(c, lookupTable);
    
    // not found!
    if (found == NULL ) {
        return False;
    }
    
    // add the missing bits
    format |= found->format;   
    
    return True;
}

/**
    0-9 if it is a digit 
    otherwise negative
*/
static inline int32 GetDigit(char c){
    int32 digit = c - '0';
    if (digit > 9) {
        return -1;
    }
    return digit;
}

/** 
    parses a number out of string 
    returns 0 if no number is encountered
*/
static inline uint32 GetIntegerNumber(const char *&string){
    if (string == NULL) return False;
    
    uint32 number = 0;
    int32 digit = 0;
    // 
    while ((digit = GetDigit(string[0]))>0){
        number *= 10;
        number += digit;
        string++;
    }
    
    return number;
}

bool FormatDescriptor::InitialiseFromString(const char *&string){
    
    // prepare clean FormatDescriptor
    // copy to this only if parsing successful
    FormatDescriptor temporaryFormat(0);
    
    /// check pointer
    if (string == NULL) {
        return False;
    }
    
    // expect at least a character
    if (string[0] == 0) {
        return False;
    }
    
    //parse options
    while (ParseCharacter(string[0],temporaryFormat,&flagsLookup[0])) {
        string++;
    }
    
    // get any integer number from string if any
    temporaryFormat.width = GetIntegerNumber(string);
    
    // after a dot look for the precision field
    if (string[0] == '.') {        
        string++;
        // get any integer number from string if any
        temporaryFormat.precision = GetIntegerNumber(string);        
    }
    
    // the next must be the code!
    if (ParseCharacter(string[0],temporaryFormat,&typesLookup[0])){
        *this = temporaryFormat;
        return True;
    }
           
	return False;
}



#endif

