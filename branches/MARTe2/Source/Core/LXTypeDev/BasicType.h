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
#ifndef BASIC_TYPE_H
#define BASIC_TYPE_H


/** the types of data in the field type of BasicTypeDescriptor*/
struct BasicType{
    enum Identifier{
        /** An integer   pointer = intxx * */
        SignedInteger         = 0,

        /** An integer   pointer = uintxx * */
        UnsignedInteger       = 1,

        /** standard float   pointer = float32 or float64 * */
        Float                 = 2,

        /** pointer to a zero terminated string   pointer = const char * */
        CCString              = 3,

        /** pointer to a pointer to a zero terminated string that has been allocated using malloc 
        pointer = char ** 
        BTConvert will free and re allocate memory 
    */
        PCString              = 4,

    /** pointer to an array of characters of size specified in size field 
        pointer = char[] 
        string will be 0 terminated and (size-1) terminated
        string is not raed only - can be written to up to size-1
    */
        CArray                = 5,

    /** BString class, 
        size field is meaningless
         pointer = BString *
        it is a pointer to a single BString
    */
        BString               = 6,

    /** FString class, size field is meaningless
        void * is FString *
        it is a pointer to a single FString
        the parts will be separated using the
        character specified in the format field  */

    //MAYBE OBSOLETE 
        FString               = 7,

    /** BString class, size field is meaningless
        void * is BString *
        it is an array of BStrings matching the
        input size  */

    //MAYBE OBSOLETE 
        STBStringArray        = 8,

    /** FString class, size field is meaningless
        void * is FString *
        it is an array of FStrings matching the
        input size  */

    //MAYBE OBSOLETE 
        FStringArray          = 9,

    /** StreamInterface class, size field is meaningless */
        Stream                = 10,

        /** Pointers */
        Pointer               = 11,

        /** A signed integer of size in bits   pointer = intxx * */
        SignedBitSet          = 12,

        /** An unsigned integer of size  pointer = uintxx * */
        UnSignedBitSet        = 13,

        /** Not a Basic Type */
        None                  = 15
    };
};
#endif

