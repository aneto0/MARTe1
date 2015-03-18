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
typedef struct {

//*** MAYBE REPLACE with finite set of options ( *' ' *0  *' '0x *' ', ....)
    /// character used to pad the representation to its full length
    char                     paddingCharacter:8; 
    /// maximum size of representation  max 255
    unsigned int             length:8;

    /// minimum number of meaningful digits (unless overridden by length)  max 255
    unsigned int             precision:8;

    /// true means use the full length and pad with the padding character 
    bool                     align:1;

    /// true means to left align instead of right align;
    bool                     leftAlign:1; 

    /// in case of a float, this field is used to determine how to print it
    Notation::Float          floatNotation:2;

    /// used for ints, floats, pointers, char * etc...
    Notation::Binary         binaryNotation:2;

    /** in case of binaryNotation not 0 prepend the appropriate
        sequence of chars to indicate hex/binary or octal */
    bool                     binaryNotationFull;

	/** takes a printf like string already pointing at the character after % (see below format)
	    and parses it recovering all the useful information, discarding all redundant ones,
		and fills up the fields in this structure.
	    At the end the pointer string is moved to the next character after the parsed block
		
		The overall printf-like format supported is the following:
		%[flags][width][.precision][length]type
		Note that the full printf would be this:
		%[parameter][flags][width][.precision][length]type
		which is not supported
			
		[flags]:
		-  	Left-align the output of this place-holder (the default is to right-align the output).
		+   Prepends a plus for positive signed-numeric types. positive = '+', negative = '-'. (the default doesn't prepend anything in front of positive numbers).
		' ' Prepends a space for positive signed-numeric types. positive = ' ', negative = '-'. This flag is ignored if the '+' flag exists. (the default doesn't prepend anything in front of positive numbers).
        0  	Prepends zeros for numbers when the width option is specified. (the default prepends spaces). Example: printf("%2d", 3) produces " 3", while printf("%02d", 3) produces in "03".
        #   Alternate form. For 'g' and 'G', trailing zeros are not removed. For 'f', 'F', 'e', 'E', 'g', 'G', the output always contains a decimal point. For 'o', 'x', 'X', or '0', '0x', '0X', respectively, is prepended to non-zero numbers.
		
		[width][.precision]  two numbers 
		Width specifies the MAXIMUM or EXACT number of characters to output, depending on the padding [flags] being set on or not 
		NOTE that in a normal printf this would be the MINIMUM or EXACT number of characters... 
        
		Precision usually specifies a maximum limit on the output, depending on the particular formatting type. 
		For floating point numeric types, it specifies the number of digits to the right of the decimal point that the output should be rounded. 
		For the string type, it limits the number of characters that should be output, after which the string is truncated.
		
		[length]type
		This is read and parsed only to set special display formats like %x %b etc... as the actual data type comes with the data
	*/
	bool InitialiseFromString(const char *&string){
	    return false;
	}
} FormatDescriptor;

#endif

