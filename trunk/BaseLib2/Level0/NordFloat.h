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
#ifndef _NORD_FLOAT
#define _NORD_FLOAT

/**
 * @file
 * Nord floating point implementation with network byte order format
 */

/** bias on SUN 32 bits floating */
#define SBIAS 127
/** bias on ND 48 bits floating*/
#define NBIAS 040000
/** SUN mantissa mask (bits 0 to 22) */
#define MANTMK 0x7FFFFF
/** SUN exponent rightmost bit*/
#define EXPSH 23
/** SUN mantissa to ND mantissa left shift*/
#define MANTSH 8

#include "System.h"

#pragma pack(1)

class NordFloat {
    /** */
    char data[6];

    /** From local endianity IEEE. */
    void ConvertFromIEEE(float x){
        register int temp = *(int *)&x;
        register int sign,mantissa,exponent;

        mantissa = temp & 0x007FFFFF;
        sign     = temp & 0x80000000;
        sign     >>= 16;
        exponent = temp & 0x7F800000;
        exponent >>= EXPSH;

        if (temp != 0) {
            if ((exponent == 0) && (mantissa != 0)) {
                exponent = exponent - SBIAS + NBIAS + 1 ;
                mantissa <<= ( MANTSH + 1);
                while ((mantissa & 0x80000000) == 0) {
                    mantissa <<= 1;
                    exponent--;
                }
            } else
            if (exponent == 255 && mantissa == 0) {
    /* signed infinity: translate by expo and fract all 1s */
               mantissa = 0xffffffff;
               exponent = 0x7fff;
            } else
            if (exponent == 255 && mantissa != 0) {
    /* NaN : translate by expo and fract all 0s */
               mantissa = 0;
               exponent = 0;
               sign = 1;
            } else {
    /* normal range: mantissa: shift left to bit 30 and put 1 in bit 31 */
               mantissa <<= MANTSH;
               mantissa = mantissa | 0x80000000;
    /*     exponent: remove SUN bias, add ND bias +1 */
               exponent = exponent - SBIAS + NBIAS + 1;
            }
        }

        exponent |= sign;

        data[0] = exponent >> 8;
        data[1] = exponent & 0xFF;
        data[2] = mantissa >> 24;
        data[3] = (mantissa >> 16 ) & 0xff;
        data[4] = (mantissa >> 8)  & 0xff;
        data[5] = mantissa & 0xff;
    }



    /** */
    float ConvertToIEEE(){
        unsigned int mantissa;
        mantissa  = (data[2] & 0xFF);
        mantissa <<= 8;
        mantissa |= (data[3] & 0xFF);
        mantissa <<= 8;
        mantissa |= (data[4] & 0xFF);
        mantissa <<= 8;
        mantissa |= (data[5] & 0xFF);
        unsigned int exponent;

        exponent &= 0x7FFF;
        exponent  = (data[0] & 0x7F);
        exponent <<= 8;
        exponent |= (data[1] & 0xFF);

        unsigned int sign = (data[0] & 0x80);

        if ((mantissa == 0) && (sign == 0)){
            exponent = 0;
            mantissa = 0;
            sign     = 0;
        } else
        if ((mantissa == 0xffffffff) && (exponent = 0x7fff)){
            exponent = 255;
            mantissa = 0;
        } else
        if ((mantissa == 0) && (sign == 1)){
            exponent = 0xFF;
            mantissa = 1;
        } else {
            exponent = exponent + SBIAS - NBIAS;
            mantissa >>= MANTSH;
            while ((mantissa & 0x00800000)==0){
                mantissa <<= 1;
                exponent --;
            }
            if (exponent != 1) {
                mantissa &= 0x007FFFFF;
                exponent --;
            }
        }

        float result;
        int &iresult = (int &)result;
        if (exponent > 255) {
            iresult = 0x7E000000;
        } else
        if (exponent < 0) {
            iresult = 0xFE000000;
        } else {
            iresult = mantissa & 0x007FFFFF;

            exponent <<= EXPSH;
            exponent &= 0x7F800000;
            iresult |= exponent;
            if (sign) iresult |= 0x80000000;
        }
        return result;
    }

public:

    /** Initializes to 0. */
    NordFloat(){
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        data[4] = 0;
        data[5] = 0;
    }

    /** From local endianity IEEE. */
    NordFloat(float x){ ConvertFromIEEE(x); }

    /** */
    void operator=(NordFloat x){ memcpy(this,&x,sizeof(NordFloat)); }

    /** From local endianity IEEE. */
    void operator=(float x){ ConvertFromIEEE(x); }

    /** */
    bool operator==(NordFloat x){ return memcmp(this,&x,sizeof(NordFloat)); }

    /** */
    bool operator==(float x){ return  (*this == NordFloat(x)); }

    /** */
    float operator()(){ return ConvertToIEEE(); }

};

#if(defined(__LP64__) || defined(__ILP64__) || defined(__LLP64__))
#pragma pack(8)
#else
#pragma pack(4)
#endif

#endif

