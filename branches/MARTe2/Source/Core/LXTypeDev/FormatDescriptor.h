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
    /// maximum size of representation  max 255
    unsigned int             length:8;

    /**
        minimum (whenever applicable) number of meaningful digits (unless overridden by length)  max 64 
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
    unsigned int             precision:6;

    /**
		true means produce a number of characters equal to length  
		fill up using spaces 
	*/
    bool                     pad:1;

    /** 
		true means to produce pad spaces after printing the object representation
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
		[Width] specifies the MAXIMUM or EXACT number of characters to output, depending on the padding [flags] being set on or not 
		NOTE that in a normal printf this would be the MINIMUM or EXACT number of characters... 
        [Precision] 
		This is the minimum number of meanigful digits used to represent a number 
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
	bool InitialiseFromString(const char *&string){
	    return false;
	}
	/** 
		constructor from unsigned integer
		Just copy bit by bit
	*/
	FormatDescriptor(uint32 x){
		uint32 * p = (uint32 * )this;
		*p = x;
	}
	
	/** 
		constructor from unsigned integer
		Just copy bit by bit
	*/
	FormatDescriptor(uint8 length, uint8 precision, bool pad, bool leftAlign, 
                        Notation::Float floatNotation,Notation::Binary binaryNotation, 
                        bool binaryPadded, bool fullNotation ){}
		uint32 * p = (uint32 * )this;
		*p = x;
	}
} FormatDescriptor;

// lists all flags 
static const char *flagsLookup = " -#0";

// lists all the types
static const char *typesLookup = "diuscfegaxpob";

// what flags at what type
static const FormatDescriptor typesFlags[] = {
	FormatDescriptor(0),  //d	
	FormatDescriptor(0),  //i
	FormatDescriptor(0),  //u
	FormatDescriptor(0),  //s
	FormatDescriptor(0),  //c
	FormatDescriptor(0,0,false,false,Notation::FixedPointNotation, Notation::NormalNotation,false,false),  //f
	FormatDescriptor(0,0,false,false,Notation::ExponentNotation  , Notation::NormalNotation,false,false),  //e
	FormatDescriptor(0,0,false,false,Notation::SmartPointNotation, Notation::NormalNotation,false,false),  //g
	FormatDescriptor(0,0,false,false,Notation::FixedPointNotation, Notation::HexNotation   ,false,false),  //a
	FormatDescriptor(0,0,false,false,Notation::FixedPointNotation, Notation::HexNotation   ,false,false),  //x
	FormatDescriptor(0,0,false,false,Notation::FixedPointNotation, Notation::HexNotation   ,false,false),  //p
	FormatDescriptor(0,0,false,false,Notation::FixedPointNotation, Notation::OctalNotation ,false,false),  //o
	FormatDescriptor(0,0,false,false,Notation::FixedPointNotation, Notation::BitNotation   ,false,false)   //b
}    


bool InitialiseFromString(const char *&string){
	return false;
}



#endif

