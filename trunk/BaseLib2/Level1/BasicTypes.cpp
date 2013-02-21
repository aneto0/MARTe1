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

#include "BasicTypes.h"
#include "ErrorManagement.h"
#include "StreamInterface.h"

inline void NumericShift (int64 &N,int shift){
    if (shift >= 64){
        N = 0;
    } else
    if (shift <= -64){
        if (N < 0) N = ~0;
        else N = 0;
    } else
    if (shift < 0) {
        N >>= (-shift);
    } else {
        N <<= shift;
    }
}

inline void NumericShift (uint64 &N,int shift){
    if (shift >= 64){
        N = 0;
    } else
    if (shift <= -64){
        N = 0;
    } else
    if (shift < 0) {
        N >>= (-shift);
    } else {
        N <<= shift;
    }
}

inline void NumericShift (int32 &N,int shift){
    if (shift >= 32){
        N = 0;
    } else
    if (shift <= -32){
        if (N < 0) N = ~0;
        else N = 0;
    } else
    if (shift < 0) {
        N >>= (-shift);
    } else {
        N <<= shift;
    }

}

inline void NumericShift (uint32 &N,int shift){
    if (shift >= 32){
        N = 0;
    } else
    if (shift <= -32){
        N = 0;
    } else
    if (shift < 0) {
        N >>= (-shift);
    } else {
        N <<= shift;
    }

}


/** Returns the string format */
const char *BTConvertToString(const BasicTypeDescriptor &bt,BString &string){
    string = "";
    if (bt.type == BTDTInteger){
        if ((bt.flags == BTDSTNone)) {
            char buffer[32];
            sprintf(buffer,"int%i",bt.size);
            string = buffer;
        } else
        if ((bt.flags == BTDSTUnsigned)) {
            char buffer[32];
            sprintf(buffer,"uint%i",bt.size);
            string = buffer;
        }
    } else
    if (bt.type == BTDTFloat){
        if (bt.size == 32) {
            string = "float";
        } else
        if (bt.size == 64) {
            char buffer[32];
            sprintf(buffer,"uint%i",bt.size);
            string = "double";
        }
    }

    if (string.Size() == 0){
        char buffer[32];
        int *p = (int *)&bt;
        sprintf(buffer,"BT%08x",*p);
        string = buffer;
    }
    return string.Buffer();
}

/** Copies from a string format */
bool BTConvertFromString(BasicTypeDescriptor &bt,const char *name){
    if (name == NULL) return False;
    if (strncmp(name,"BT",2)==0){
        name +=2;
        int *p = (int *)&bt;
        sscanf(name,"%08x",p);
        return True;
    } else
    if (strncmp(name,"int",3)==0){
        name +=3;
        bt.type = BTDTInteger;
        bt.flags = BTDSTNone;
        int32 bitSize = sizeof(int) * BYTE_BIT_SIZE;
        if(strlen(name) != 0){
            sscanf(name,"%i", &bitSize);
        }
        bt.size = bitSize;
    } else
    if (strncmp(name,"uint",4)==0){
        name +=4;
        bt.type = BTDTInteger;
        bt.flags = BTDSTUnsigned;
        int32 bitSize = sizeof(unsigned int) * BYTE_BIT_SIZE;
        if(strlen(name) != 0){
            sscanf(name,"%i", &bitSize);
        }
        bt.size = bitSize;
    } else
    if (strcmp(name,"unsigned int")==0){
        bt.size  = sizeof(unsigned int) * BYTE_BIT_SIZE;
        bt.type  = BTDTInteger;
        bt.flags = BTDSTUnsigned;
    } else
    if (strcmp(name,"float")==0){
        bt.size = sizeof(float) * BYTE_BIT_SIZE;
        bt.type = BTDTFloat;
        bt.flags = BTDSTNone;
    } else
    if (strcmp(name,"double")==0){
        bt.size = sizeof(double) * BYTE_BIT_SIZE;
        bt.type = BTDTFloat;
        bt.flags = BTDSTNone;
    } else {
        return False;
    }
    return True;
}

bool IntToInt(
        int64 *&            destination,
        int &               destinationBitShift,
        int                 destinationBitSize,
        bool                destinationIsSigned,
        int64 *&            source,
        int &               sourceBitShift,
        int                 sourceBitSize,
        bool                sourceIsSigned
                ){

    uint64 sourceData;

    int bitsAvail = 64 - sourceBitShift;
    if (sourceBitSize < bitsAvail){
        sourceBitShift += sourceBitSize;
        sourceData = source[0];
//        sourceData <<= (64 - sourceBitShift);
        NumericShift(sourceData,(64 - sourceBitShift));
    } else {
        sourceData = source[0];
        sourceBitShift = sourceBitSize-bitsAvail;
//        sourceData >>= sourceBitShift;
        NumericShift(sourceData,-sourceBitShift);
        source++;
        uint64 sourceData2;
        sourceData2 = source[0];
//        sourceData2 <<= (64 - sourceBitShift);
        NumericShift(sourceData2,(64 - sourceBitShift));
        sourceData |= sourceData2;
    }

    int64 &sourceDataSigned = (int64 &)sourceData;
    uint64 finalData;
    int64 &finalDataSigned = (int64 &)finalData;

    // now normalize to the left
    if (sourceIsSigned){
        bool negative = (sourceDataSigned < 0);
//        sourceDataSigned >>= (64-sourceBitSize);
        NumericShift(sourceDataSigned,sourceBitSize-64);

        if (destinationIsSigned){
            int delta = sourceBitSize - destinationBitSize;
            if (delta > 0){
                uint64 mask = ~0;
                //setting mask to check overflow
//                mask <<= (destinationBitSize-1);
                NumericShift(mask,(destinationBitSize-1));
                if (negative){
                    mask &= ~sourceData;
                } else {
                    mask &= sourceData;
                }
                if (mask != 0){    //overflow
                    finalData = 1;
//                    finalData = finalData << (destinationBitSize-1);
                    NumericShift(finalData,(destinationBitSize-1));
                    if (!negative){
                        finalData = ~finalData;
                    }
                } else {
                    finalDataSigned  = sourceDataSigned;
                }
            } else {  //delta<0
                finalDataSigned  = sourceDataSigned;
            }
        } else { // destination unsigned
            if (negative){
                finalData = 0;
            } else {
                int delta = sourceBitSize - destinationBitSize - 1;
                if (delta > 0){
                    uint64 mask = ~0;
//                    mask <<= destinationBitSize;
                    NumericShift(mask,destinationBitSize);
                    mask &= sourceData;
                    if (mask != 0){
                        finalData = ~0;
                    } else {
                        finalData  = sourceDataSigned;
                    }
                } else {
                    finalData    = sourceDataSigned;
                }
            }
        }
    } else {  //source unsigned
//        sourceData       >>= (64-sourceBitSize);
        NumericShift(sourceData,sourceBitSize-64);
        if (destinationIsSigned){
            int delta = sourceBitSize - destinationBitSize + 1;
            if (delta > 0){
                uint64 mask = ~0;
//                mask <<= (destinationBitSize - 1);
                NumericShift(mask,destinationBitSize-1);
                mask &= sourceData;
                if (mask != 0){
                    finalData = 1;
//                    finalData = finalData << (destinationBitSize-1);
                    NumericShift(finalData,destinationBitSize-1);
                    finalData = ~finalData;
                } else {
                    finalDataSigned  = sourceData;
                }
            } else {
                finalDataSigned    = sourceData;
            }
        } else {  // both unsigned
            int delta = sourceBitSize - destinationBitSize;
            if (delta > 0){
                uint64 mask = ~0;
//                mask <<= (destinationBitSize);
                NumericShift(mask,destinationBitSize);
                mask &= sourceData;
                if (mask != 0){
                    finalData = ~0;
                } else {
                    finalData  = sourceData;
                }
            } else {
                finalData      = sourceData;
            }
        }
    }
    //setting mask to make finalData on "destinationBitSize" bit
    uint64 mask = ~0;
//    mask <<= destinationBitSize;
    NumericShift(mask,destinationBitSize);
    mask = ~mask;
    finalData &= mask;

    //put in line the final data obtained at each cycle
    uint64 outputData0;
    uint64 outputData1;

    outputData0 = destination[0];
    uint64 mask2 = mask;
//    mask2 <<= destinationBitShift;
    NumericShift(mask2,destinationBitShift);
    outputData0 &= ~mask2;
    uint64 finalDataT = finalData;
//    finalDataT <<= destinationBitShift;
    NumericShift(finalDataT,destinationBitShift);
    outputData0 |= finalDataT;
    destination[0] = outputData0;

    bitsAvail = 64 - destinationBitShift;
    if (destinationBitSize > bitsAvail){
        outputData1 = destination[1];
        uint64 mask2 = mask;
//        mask2 >>= (64 - destinationBitShift);
        NumericShift(mask2,destinationBitShift - 64);
        outputData1 &= ~mask2;
        uint64 finalDataT = finalData;
//        finalDataT >>= (64 - destinationBitShift);
        NumericShift(finalDataT,destinationBitShift - 64);
        outputData1 |= finalDataT;
        destination[1] = outputData1;

    }
    destinationBitShift += destinationBitSize;

    if (destinationBitShift >= 64){
        destination++;
        destinationBitShift -= 64;
    }

    return True;
}


bool IntToInt64(
        int64 *&            destination,
        int64 *&            source,
        int &               sourceBitShift,
        int                 sourceBitSize,
        bool                sourceIsSigned
                ){

    uint64 sourceData;

    int bitsAvail = 64-sourceBitShift;
    if (sourceBitSize < bitsAvail){
        sourceBitShift += sourceBitSize;
        sourceData = source[0];
//        sourceData <<= (64 - sourceBitShift);
        NumericShift(sourceData,64 - sourceBitShift);
    } else {
        sourceData = source[0];
        sourceBitShift = sourceBitSize-bitsAvail;
//        sourceData >>= sourceBitShift;
        NumericShift(sourceData,-sourceBitShift);
        source++;
        uint64 sourceData2;
        sourceData2 = source[0];
//        sourceData2 <<= (64 - sourceBitShift);
        NumericShift(sourceData2,64 - sourceBitShift);
        sourceData |= sourceData2;
    }

    int64 &sourceDataSigned = (int64 &)sourceData;
    uint64 finalData;
    int64 &finalDataSigned = (int64 &)finalData;

    // now normalize to the left
    if (sourceIsSigned){
//        sourceDataSigned >>= (64-sourceBitSize);
        NumericShift(sourceDataSigned,sourceBitSize - 64);
        finalDataSigned  = sourceDataSigned;
    } else {
//        sourceData       >>= (64-sourceBitSize);
        NumericShift(sourceData,sourceBitSize - 64);
        int delta = sourceBitSize - 63;
        if (delta > 0){
            uint64 mask = ~0;
            mask <<= 63;
            mask &= sourceData;
            if (mask != 0){
                finalData = 1 ;
                finalData <<= 63;
                finalData = ~finalData;
            } else {
                finalDataSigned  = sourceData;
            }
        } else {
            finalDataSigned    = sourceData;
        }
    }

    destination[0] = finalData;
    destination++;

    return True;
}

bool IntToInt32(
        int32 *&            destination,
        int32 *&            source,
        int &               sourceBitShift,
        int                 sourceBitSize,
        bool                sourceIsSigned
                ){

    uint32 sourceData;

    int bitsAvail = 32-sourceBitShift;
    if (sourceBitSize < bitsAvail){
        sourceBitShift += sourceBitSize;
        sourceData = source[0];
//        sourceData <<= (32 - sourceBitShift);
        NumericShift(sourceData,32 - sourceBitShift);
    } else {
        sourceData = source[0];
        sourceBitShift = sourceBitSize-bitsAvail;
//        sourceData >>= sourceBitShift;
        NumericShift(sourceData,-sourceBitShift);
        source++;
        uint32 sourceData2;
        sourceData2 = source[0];
//        sourceData2 <<= (32 - sourceBitShift);
        NumericShift(sourceData2,32 - sourceBitShift);
        sourceData |= sourceData2;
    }

    int32 &sourceDataSigned = (int32 &)sourceData;
    uint32 finalData;
    int32 &finalDataSigned = (int32 &)finalData;

    // now normalize to the left
    if (sourceIsSigned){
//        sourceDataSigned >>= (32-sourceBitSize);
        NumericShift(sourceDataSigned,sourceBitSize - 32);
        finalDataSigned  = sourceDataSigned;
    } else {
//        sourceData       >>= (32-sourceBitSize);
        NumericShift(sourceData,sourceBitSize - 32);
        int delta = sourceBitSize - 31;
        if (delta > 0){
            uint32 mask = ~0;
            mask <<= 31;
            mask &= sourceData;
            if (mask != 0){
                finalData = 1 << 31;
                finalData = ~finalData;
            } else {
                finalDataSigned  = sourceData;
            }
        } else {
            finalDataSigned    = sourceData;
        }
    }

    destination[0] = finalData;
    destination++;

    return True;
}

bool Int64ToString(BString &bs,int64 n,BTDFormat format){
/*uint32 desiredSize,char desiredPadding,char mode*/
    uint32 desiredSize = format.intInfo.minNumberOfFigures;
    char desiredPadding = 0;
    if (format.intInfo.padding == 1) desiredPadding = ' ';
    if (format.intInfo.padding == 2) desiredPadding = '0';

    bs = "";

    char buffer[32];
    uint32 ix = sizeof(buffer)-1;
    buffer[ix] = 0;

    bool maxNeg = ((-n == n) && (n != 0));

    if (format.intInfo.mode == 0/*(mode == 'i') || (mode == 'd')*/){
        bool sign = (n < 0);
            while ((n != 0) && (ix > 0)){
                ix--;
                #ifndef _RTAI
                    int nr = n % 10;
                #else
                    int64 temp = n;
                    int nr = do_div(temp, 10);
                #endif
                buffer[ix] = '0' + abs(nr);
                #ifndef _RTAI
                    n = n / 10;
                #else
                    do_div(n, 10);
                #endif
            }
//        }
        if (ix == (sizeof(buffer)-1)){
            ix--;
            buffer[ix] = '0';
        }

        uint32 ccount=sizeof(buffer) - 1 - ix;

        if ((desiredPadding == '0') && sign) bs += '-';
        if (desiredPadding != 0){
            while (desiredSize > ccount) {
                bs += desiredPadding;
                desiredSize--;
            }
        } else {
            while (desiredSize > ccount) {
                bs += ' ';
                desiredSize--;
            }
        }
        if ((desiredPadding != '0') && sign) bs += '-';

        int i;
        for (i=0;i<ccount;i++){
            bs += buffer[ix+i];
        }
//        CWrite(cs,buffer+ix,ccount);
    } else
    if (format.intInfo.mode == 1/*(mode == 'x') || (mode == 'X') || (mode == 'p')*/){
        while ((n != 0) && (ix > (sizeof(buffer) - 17))){
            ix--;
            int nr = n & 0xF;
            if (nr >= 10){
                buffer[ix] = 'A' + abs(nr) - 10;
            } else {
                buffer[ix] = '0' + abs(nr);
            }
            n = n >> 4;
        }
        // a 0 at least!
        if (ix == (sizeof(buffer)-1)){
            ix--;
            buffer[ix] = '0';
        }
        buffer[--ix] = 'x';
        buffer[--ix] = '0';

        uint32 ccount = sizeof(buffer) - 1 - ix ;
        if (desiredPadding == 0) desiredPadding = ' ';
        while (desiredSize > ccount) {
            bs += desiredPadding;
            desiredSize--;
        }

        int i;
        for (i=0;i<ccount;i++){
            bs += buffer[ix+i];
        }

    } else
    if (format.intInfo.mode == 2){
        while ((n != 0) && (ix > (sizeof(buffer) - 22))){
            int nr = n & 0x7;
            ix--;
            buffer[ix] = '0' + abs(nr);
            n = n >> 3;
        }
        if (ix == (sizeof(buffer)-1)){
            ix--;
            buffer[ix] = '0';
        }
        buffer[--ix] = '0';

        uint32 ccount=sizeof(buffer) - 1 - ix;
        if (desiredPadding == 0) desiredPadding = ' ';
        while (desiredSize > ccount) {
            bs += desiredPadding;
            desiredSize--;
        }

        int i;
        for (i=0;i<ccount;i++){
            bs += buffer[ix+i];
        }
    }
    return True;
}


int64 StringToInt64(const char *&string, int maxNumberOfBits, bool isSigned){
    // skip characters
    while ((*string != 0) && ((*string == ' ') || (*string == '\t'))) string++;


    int64 n = 0;
    char c;
    bool sign    = False;
    bool octal   = False;
    bool hex     = False;
    bool decimal = True;

    // accounts for sign and detects octals and hex
    if (*string == '-') {
        sign = True;
        string++;
    }
    if ((*string == '0') && (!sign)){
        octal = True;
        decimal = False;
        string++;
    }
    if (((*string == 'X')||(*string == 'x')) && octal) {
        octal = False;
        hex   = True;
        string++;
    }

    // calculates invalid bits mask
    if (maxNumberOfBits > 64) maxNumberOfBits = 64;
    if (isSigned && !sign && decimal) maxNumberOfBits --;
    uint64 mask = 0;
    mask = ~mask;
//    mask = mask << maxNumberOfBits;
    NumericShift(mask,maxNumberOfBits);

    uint64 unn = 0;
    int64   nn = 0;
    uint64 un  = 0;
    if (hex){
        while ((c = *string) != 0){
            unn = un << 4;

            if ((c >= '0') && ( c <= '9')){
                unn = unn + c - '0';
            } else
            if ((c >= 'A') && ( c <= 'F')){
                unn = unn + c - 'A' + 10;
            } else
            if ((c >= 'a') && ( c <= 'f')){
                unn = unn + c - 'a' + 10;
            } else break;

            // check out of range !
            if (unn < un) break;
            un = unn;

            string++;
        }
        n = (int64)un;
        nn = (int64)unn;
        sign = (n < 0);
    } else
    if (octal){
        while ((c = *string) != 0){
            unn = un << 3;

            if ((c >= '0') && ( c <= '7')){
                unn = unn + c - '0';
            } else break;

            // check out of range !
            if (unn < un) break;
            un = unn;

            string++;
        }
        n = (int64)un;
        nn = (int64)unn;
        sign = (n < 0);
    } else {
        while ((c = *string) != 0){
            nn = n * 10;

            if ((c >= '0') && ( c <= '9')){
                if (sign){
                    nn = nn - c + '0';
                } else {
                    nn = nn + c - '0';
                }
            } else break;

            // check out of range !
            if (sign){
                if (nn > n) break;
                if (~nn & mask) break;
            } else {
                if (nn < n) break;
                if (nn & mask) break;
            }
            n = nn;

            string++;
        }
    }
    if (sign){
        if ((~nn & mask)||(nn > n)) {
            n = 1;
//            n <<= maxNumberOfBits;
            NumericShift(n,maxNumberOfBits);
        }
    } else {
        if ((nn & mask)||(nn < n)) {
            n = 1;
            for (int i=0;i<(maxNumberOfBits-1);i++){
                n = (n << 1) | 1;
            }
        }
    }
    return (int64)n;
}


/* this version accounts for the 0 */
static void strncopy(char *destination,const char *source,int size){
    size--;
    char s;
    while (((s = *source++)!= 0) && (size > 0)) {
        *destination++ = s;
        size--;
    }
    *destination++ = 0;
}

bool Int64ToInt(
        int64 *&            destination,
        int &               destinationBitShift,
        int                 destinationBitSize,
        bool                destinationIsSigned,
        int64 *&            sourceDataSigned
                ){
    int bitsAvail = 64;
    int sourceBitSize = 64;  //*
    //uint64 &sourceDataSigned = (uint64 &)sourceDataSigned;
    uint64 sourceData = (uint64)sourceDataSigned[0];
    uint64 finalData;
    int64 &finalDataSigned = (int64 &)finalData;

    // now normalize to the left
    bool negative = (*sourceDataSigned < 0);

    if (destinationIsSigned){
        int delta = sourceBitSize - destinationBitSize;
        if (delta > 0){
            uint64 mask = ~0;
            //setting mask to check overflow
//            mask <<= (destinationBitSize-1);
            NumericShift(mask,destinationBitSize-1);
            if (negative){
                mask &= ~sourceData;
            } else {
                mask &= sourceData;
            }
            if (mask != 0){    //overflow
                finalData = 1;
//                finalData <<= (destinationBitSize-1);
                NumericShift(finalData,destinationBitSize-1);
                if (!negative){
                    finalData = ~finalData;
                }
            } else {
                finalDataSigned  = sourceData;
            }
        } else {  //delta<0
            finalDataSigned  = sourceData;
        }
    } else { // destination unsigned
        if (negative){
            finalData = 0;
        } else {
            int delta = sourceBitSize - destinationBitSize - 1;
            if (delta > 0){
                uint64 mask = ~0;
//                mask <<= destinationBitSize;
                NumericShift(mask,destinationBitSize);
                mask &= sourceData;
                if (mask != 0){
                    finalData = ~0;
                } else {
                    finalData  = sourceData;
                }
            } else {
                finalData    = sourceData;
            }
        }
    }

    //setting mask to make finalData on "destinationBitSize" bit
    uint64 mask = ~0;
//    mask <<= destinationBitSize;
    NumericShift(mask,destinationBitSize);
    mask = ~mask;
    finalData &= mask;

    //put in line the final data obtained at each cycle
    uint64 outputData0;
    uint64 outputData1;

    outputData0 = destination[0];
    uint64 mask2 = mask;
//    mask2 <<= destinationBitShift;
    NumericShift(mask2,destinationBitShift);
    outputData0 &= ~mask2;
    uint64 finalDataT = finalData;
//    finalDataT <<= destinationBitShift;
    NumericShift(finalDataT,destinationBitShift);
    outputData0 |= finalDataT;
    destination[0] = outputData0;

    bitsAvail = 64 - destinationBitShift;
    if (destinationBitSize > bitsAvail){
        outputData1 = destination[1];
        uint64 mask2 = mask;
//        mask2 >>= (64 - destinationBitShift);
        NumericShift(mask2,destinationBitShift - 64);
        outputData1 &= ~mask2;
        uint64 finalDataT = finalData;
//        finalDataT >>= (64 - destinationBitShift);
        NumericShift(finalDataT,destinationBitShift - 64);
        outputData1 |= finalDataT;
        destination[1] = outputData1;

    }
    destinationBitShift += destinationBitSize;

    if (destinationBitShift >= 64){
        destination++;
        destinationBitShift -= 64;
    }

    return True;
}

bool Int32ToInt(
        int64 *&            destination,
        int &               destinationBitShift,
        int                 destinationBitSize,
        bool                destinationIsSigned,
        int32 *&               sourceDataSigned
                 ){
    uint32 sourceData       = (uint32)sourceDataSigned[0];
    uint64 finalData;
    int64 &finalDataSigned = (int64 &)finalData;
    int sourceBitSize = 32;
    bool negative = (*sourceDataSigned < 0);
    if (destinationIsSigned){
        int delta = sourceBitSize - destinationBitSize;
        if (delta > 0){
            uint64 mask = ~0;
            mask <<= (destinationBitSize-1);
            NumericShift(mask,destinationBitSize-1);
            if (negative){
                mask &= ~sourceData;
            } else {
                mask &= sourceData;
            }
            if (mask != 0){
                finalData = 1;
//                finalData <<= (destinationBitSize-1);
                NumericShift(finalData,destinationBitSize-1);
                if (!negative)
                    finalData = ~finalData;
            } else {
                finalDataSigned  = sourceData;
            }
        } else {
            finalDataSigned  = sourceData;
        }
    } else {
        if (negative){
            finalData = 0;
        } else {
            int delta = sourceBitSize - destinationBitSize - 1;
            if (delta > 0){
                uint64 mask = ~0;
//                mask <<= destinationBitSize;
                NumericShift(mask,destinationBitSize);
                mask &= sourceData;
                if (mask != 0){
                    finalData = ~0;
                } else {
                    finalData  = sourceData;
                }
            } else {
                finalData    = sourceData;
            }
        }
    }
    uint64 mask = ~0;
//    mask <<= destinationBitSize;
    NumericShift(mask,destinationBitSize);
    mask = ~mask;
    finalData &= mask;

    uint64 outputData0;
    uint64 outputData1;
    outputData0 = destination[0];

    uint64 mask2 = mask;
//    mask2 <<= destinationBitShift;
    NumericShift(mask2,destinationBitShift);
    outputData0 &= ~mask2;
    uint64 finalDataT = finalData;
//    finalDataT <<= destinationBitShift;
    NumericShift(finalDataT,destinationBitShift);
    outputData0 |= finalDataT;
    destination[0] = outputData0;
    int bitsAvail;
    bitsAvail = 64 - destinationBitShift;
    if (destinationBitSize > bitsAvail){
        outputData1 = destination[1];
        uint64 mask2 = mask;
//        mask2 >>= (64 - destinationBitShift);
        NumericShift(mask2,destinationBitShift - 64);
        outputData1 &= ~mask2;
        uint64 finalDataT = finalData;
//        finalDataT >>= (64 - destinationBitShift);
        NumericShift(finalDataT,destinationBitShift - 64);
        outputData1 |= finalDataT;
         destination[1] = outputData1;
    }
    destinationBitShift += destinationBitSize;
    if (destinationBitShift >= 64){
        destination++;
        destinationBitShift -= 64;
    }

    return True;

}

/***************************************************************/
/*  STRING READ ACCESS FUNCTIONS                               */
/***************************************************************/

static const char *mSeparators = " \n\t\r";
static const char *sSeparators = ",;:";
static const char *separators = " \n\t\r,;:";

static char * BTSGDF_GetToken(char *&string){

    // skip useless characters
    while ((strchr(mSeparators,*string)!=NULL) && (*string != 0)){
        string++;
    }

    // find end of token
    char *end = string;

    // find next useless characters
    while ((strchr(separators,*end)==NULL) && (*end != 0)){
        end++;
    }

    int size = end-string;
    char *result = (char *)malloc(size+1);
    strncpy(result,string,size);
    result[size] = 0;

    // skip blank separators
    while ((strchr(mSeparators,*end)!=NULL) && (*end != 0)){
        end++;
    }

    // skip next real separator
    if (strchr(sSeparators,*end)!=NULL) end++;

    string = end;
    return result;

}

static  char *  BTSGDF_CArray(const void *&p){
    char *string = (char *)p;

    char *result =  BTSGDF_GetToken(string);
    p = string;

    return result;
}

static  char *  BTSGDF_CString(const void *&p){
    char **strings = (char **)p;
    char *string = strings[0];
    char *result =  strdup(string);
    // copy back
    strings++;
    p = strings;

    return result;
}

static  char *  BTSGDF_BStringArray(const void *&p){
    BString *strings = (BString *)p;
    const char *string = strings->Buffer();
    char *result =  strdup(string);
    // copy back
    strings++;
    p = strings;

    return result;
}

static  char *  BTSGDF_Stream(const void *&p){
    StreamInterface *stream = (StreamInterface *)p;
    int maxSize = 4096;
    char *result = (char *)malloc(maxSize);

    stream->GetToken(result,sSeparators,maxSize,NULL,mSeparators);
    int size = strlen(result);
    if (size < (maxSize-1)){
        result = (char *)realloc((void *&)result,size+1);
    }

    return result;
}

/***************************************************************/
/*  STRING WRITE ACCESS FUNCTIONS                              */
/***************************************************************/

static void BTSPDF_CArray(const void *&p,const char *s,int maxSize,int pos,char sep){
    if (s == NULL) return;
    char *string = (char *)p;
    if (string == NULL) return;

    if (pos == 0) string[0] = 0;

    int len = strlen(string);

    if ((len < (maxSize-1)) && (pos != 0)){
        string[len] = sep;
        len++;
    }

    int len2 = strlen(s);
    int left = maxSize - 1 - len;
    if (len2 > left) len2= left;
    memcpy(string+len,s,len2);
    len = len+len2;


    string[len] = 0;
}

static void BTSPDF_CString(const void *&p,const char *s,int maxSize,int pos,char sep){
    char **strings = (char **)p;

    if (*strings != NULL) free ((void *&)(*strings));

    if (s != NULL) *strings = strdup(s);

    strings++;
    p = strings;
}

static void BTSPDF_BString(const void *&p,const char *s,int maxSize,int pos,char sep){
    BString *strings = (BString *)p;
    if (pos == 0) *strings = "";

    if (s != NULL) {
        if (pos != 0) *strings += sep;
        *strings += s;
    }

    p = strings;
}

static void BTSPDF_BStringArray(const void *&p,const char *s,int maxSize,int pos, char sep){
    BString *strings = (BString *)p;

    if (s != NULL) strings[0] = s;

    strings++;
    p = strings;
}


static void BTSPDF_Stream(const void *&p,const char *s,int maxSize,int pos,char sep){
    StreamInterface *stream = (StreamInterface *)p;

    if (stream == NULL) return;

    if (s != NULL) {
        if (pos != 0) stream->PutC(sep);
        uint32 len = strlen(s);
        stream->Write(s,len);
    }

}


BTStringPutDataFunction BTSPDFArray[16] = {
    NULL,
//static const BTDSubTypes    BTDSTCString        = 1;
    BTSPDF_CString,
//static const BTDSubTypes    BTDSTCArray         = 2;
    BTSPDF_CArray,
//static const BTDSubTypes    BTDSTBString        = 3;
    BTSPDF_BString,
//static const BTDSubTypes    BTDSTFString        = 4;
    BTSPDF_Stream,
//static const BTDSubTypes    BTDSTBStringArray   = 5;
    BTSPDF_BStringArray,
//static const BTDSubTypes    BTDSTFStringArray   = 6;
    NULL,
//static const BTDSubTypes    BTDSTStream         = 7;
    BTSPDF_Stream,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};

BTStringGetDataFunction BTSGDFArray[16] = {
    NULL,
//static const BTDSubTypes    BTDSTCString        = 1;
    BTSGDF_CString,
//static const BTDSubTypes    BTDSTCArray         = 2;
    BTSGDF_CArray,
//static const BTDSubTypes    BTDSTBString        = 3;
    NULL,
//static const BTDSubTypes    BTDSTFString        = 4;
    BTSGDF_Stream,
//static const BTDSubTypes    BTDSTBStringArray   = 5;
    BTSGDF_BStringArray,
//static const BTDSubTypes    BTDSTFStringArray   = 6;
    NULL,
//static const BTDSubTypes    BTDSTStream         = 7;
    BTSGDF_Stream,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};

/** convert from one basic type to another */
bool BTConvert( int                 numberOfElements,
                BasicTypeDescriptor destinationBTD,
                void *              destination,
                BasicTypeDescriptor sourceBTD,
                const void *        source
                ){

  //printf("@BTConvert!\n"); // DEBUG
  //printf("numberOfElements = %d\n", numberOfElements);
    switch (sourceBTD.type){
/***************************************************************/
/***************************************************************/
/*  FROM INTEGER                                               */
/***************************************************************/
/***************************************************************/
        case BTDTInteger:{
            bool sourceIsSigned;
            switch (sourceBTD.flags){
                case BTDSTUnsigned:{
                    sourceIsSigned = False;
                }break;
                case BTDSTNone:{
                    sourceIsSigned = True;
                } break;
                default:{
                    CStaticAssertErrorCondition(FatalError,"BTConvert: sourceBTD.flags = %i is inappropriate for integer",sourceBTD.flags);
                    return False;
                }
            }

	    int32 *sourcePtr32 = NULL;
	    int64 *sourcePtr   = NULL;
	    if((destinationBTD.size == 32) && (sourceBTD.size == 32) && (destinationBTD.type != BTDTFloat)) {
	      sourcePtr32 = (int32 *)source;
	    } else {
	      // we copy integers 64 bits at a time
	      sourcePtr   = (int64 *)source;
	    }

            // if we copy compacted then this the number of bits to start copy the next
            int sourceShift = 0;

            switch (destinationBTD.type){
    /***************************************************************/
    /*  FROM INTEGER TO INTEGER                                    */
    /***************************************************************/
                case BTDTInteger:{
// 		    printf("integer2integer\n"); // DEBUG
                    bool destinationIsSigned;
                    switch (destinationBTD.flags){
                        case BTDSTUnsigned:{
                            destinationIsSigned = False;
                        }break;
                        case BTDSTNone:{
                            destinationIsSigned = True;
                        } break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for integer",destinationBTD.flags);
                            return False;
                        }
                    }
		    
		    int32 *destinationPtr32 = NULL;
		    int64 *destinationPtr   = NULL;
		    if((destinationBTD.size == 32) && (sourceBTD.size == 32)) {
		      destinationPtr32 = (int32 *)destination;
		    } else {
		      destinationPtr   = (int64 *)destination;
		    }

                    // if we copy compacted then this the number of bits to start copy the next
                    int destinationShift = 0;
                    for (int i = 0;i < numberOfElements;i++){
		      if((destinationBTD.size == 32) && (sourceBTD.size == 32)) {
			IntToInt32(destinationPtr32, sourcePtr32, sourceShift, sourceBTD.size, sourceIsSigned);
		      } else {
			IntToInt(destinationPtr,destinationShift,destinationBTD.size,destinationIsSigned, sourcePtr,sourceShift,sourceBTD.size,sourceIsSigned);
		      }
                    }
                } break;
    /***************************************************************/
    /*  FROM INTEGER TO FLOAT                                      */
    /***************************************************************/
                case BTDTFloat:{
// 		    printf("integer2float\n"); // DEBUG
                    bool destinationIsDouble = False;
                    switch (destinationBTD.size){
                        case 64:{
                            destinationIsDouble = True;
                        }break;
                        case 32:{

                        }break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.size = %i is inappropriate for float",destinationBTD.size);
                            return False;
                        }
                    };
                    switch (destinationBTD.flags){
                        case BTDSTNone:{
                        } break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for float",destinationBTD.flags);
                            return False;
                        }
                    };
                    if (destinationIsDouble){
                        double *dp = (double *)destination;
                        int64 buffer = 0;
                        for (int i = 0;i < numberOfElements;i++){
                            int64 *bp = &buffer;
                            IntToInt64(bp,sourcePtr,sourceShift,sourceBTD.size,sourceIsSigned);
                            *dp++ = buffer;
                        }
                    } else {
                        float *fp = (float *)destination;
                        int64 buffer = 0;
                        for (int i = 0;i < numberOfElements;i++){
                            int64 *bp = &buffer;
                            IntToInt64(bp,sourcePtr,sourceShift,sourceBTD.size,sourceIsSigned);
                            *fp++ = buffer;
                        }
                    }

                }break;
    /***************************************************************/
    /*  FROM INTEGER TO STRING                                     */
    /***************************************************************/
                case BTDTString:{
// 		    printf("integer2string\n"); // DEBUG
                    BTStringPutDataFunction writeFunction = BTSPDFArray[destinationBTD.flags];
                    const void *writeData = destination;

                    if (writeFunction == NULL){
                        CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for string",sourceBTD.flags);
                        return False;
                    }

                    char sep = ' ';
                    if (destinationBTD.Format().stringInfo.separator == 1) sep = ',';
                    if (destinationBTD.Format().stringInfo.separator == 2) sep = ';';
                    if (destinationBTD.Format().stringInfo.separator == 3) sep = '\n';

                    // only for CARRAY
                    int size = destinationBTD.CArraySize();

                    int64 *s = (int64*)source;
                    for (int i = 0;i < numberOfElements;i++){
#if defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

		        int64 destinationOdd;
#endif
                        int64 destination;
                        int64 *d = &destination;
                        int destinationBitShift = 0;
                        IntToInt(d, destinationBitShift, 64, True,
				 s, sourceShift,sourceBTD.size, sourceIsSigned);
#if defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

			if((i % 2) == 0) {
			    destinationOdd  = destination;
			} else {
                            BString converted;
			    Int64ToString(converted,destination,sourceBTD.Format());
			    writeFunction(writeData,converted.Buffer(),size,i,sep);
                            BString convertedOdd;
			    Int64ToString(convertedOdd,destinationOdd,sourceBTD.Format());
			    writeFunction(writeData,convertedOdd.Buffer(),size,i,sep);
			}
#else
			
// 			if(i == 0) {
// 			  printf("BTConvert() -> numberOfElements = %d \t (sourceBTD == BTDInt32) = %d\n", numberOfElements, (sourceBTD == BTDInt32));
// 			}
                        BString converted;
                        Int64ToString(converted,destination,sourceBTD.Format());
                        writeFunction(writeData,converted.Buffer(),size,i,sep);
// 			if(i < 10) {
// 			    printf("BTConvert() -> i=%d, source=%d, source=%d, IntToInt=%d, Int64ToString=%s, writeData=%s\n", i, (int32)(*((int32 *)s)), (int32)(*((int32 *)((int32 *)(s)+1))),  (int32)destination, converted.Buffer(), converted.Buffer(), (char *)writeData);
//                         }
#endif
                    }
                }break;
    /***************************************************************/
    /*  FROM INTEGER TO ??????                                     */
    /***************************************************************/
                default:{
// 		    printf("integer2????\n"); // DEBUG
                    CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.type = %i is unknown type",sourceBTD.type);
                    return False;
                }break;
            }
        }break;


/***************************************************************/
/***************************************************************/
/*  FROM FLOAT                                                 */
/***************************************************************/
/***************************************************************/
        case BTDTFloat:{
            bool sourceIsDouble = False;
            float *sourcePtrf = (float *)source;
            double *sourcePtrd = (double *)source;
            // if we copy compacted then this the number of bits to start copy the next
            switch (sourceBTD.size){
                case 64:{
                    sourceIsDouble = True;
                }break;
                case 32:{
                }break;
                default:{
                    CStaticAssertErrorCondition(FatalError,"BTConvert: sourceBTD.size = %i is inappropriate for float",sourceBTD.size);
                        return False;
                }
            };
            switch (sourceBTD.flags){
                case BTDSTNone:{

                } break;
                default:{
                    CStaticAssertErrorCondition(FatalError,"BTConvert: sourceBTD.flags = %i is inappropriate for float",sourceBTD.flags);
                        return False;
                }
            };

            switch (destinationBTD.type){
    /***************************************************************/
    /*  FROM FLOAT TO INTEGER                                      */
    /***************************************************************/
                case BTDTInteger:{
// 		    printf("float2integer\n"); // DEBUG
                    bool destinationIsSigned;
                    switch (destinationBTD.flags){
                        case BTDSTUnsigned:{
                            destinationIsSigned = False;
                        }break;
                        case BTDSTNone:{
                            destinationIsSigned = True;
                        } break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for integer",destinationBTD.flags);
                            return False;
                        }
                    }
                    //if (destinationIsSigned){ Works for unsigned too!!!
                        if (destinationBTD.size == 64){
                            // we copy integers 64 bits at a time
                            int64 *destinationPtr = (int64 *)destination;
                            if (sourceIsDouble){
                                  for (int i = 0;i < numberOfElements;i++){
                                      destinationPtr[0] = (int64)*sourcePtrd;
                                      sourcePtrd++;
                                      destinationPtr++;
                                  }
                            } else {
                                  for (int i = 0;i < numberOfElements;i++){
                                      destinationPtr[0] = (int64)*sourcePtrf;
                                      sourcePtrf++;
                                      destinationPtr++;
                                  }
                            }
                        }  //  !destinationIs64
                        else if (destinationBTD.size == 32){
                            int32 *destinationPtr = (int32 *)destination;
                            if (sourceIsDouble){
                                  for (int i = 0;i < numberOfElements;i++){
                                      destinationPtr[0] = (int32)*sourcePtrd;
                                      sourcePtrd++;
                                      destinationPtr++;
                                  }
                            } else {
                                  for (int i = 0;i < numberOfElements;i++){
                                      destinationPtr[0] = (int32)*sourcePtrf;
                                      sourcePtrf++;
                                      destinationPtr++;
                                  }
                            }
                        } else { //  !destinationIs32 and !destinationIs64
                            // if we copy compacted then this the number of bits to start copy the next
                            int destinationShift = 0;
                            int64 *destinationPtr = (int64 *)destination;
                            if (sourceIsDouble){
                                for (int i = 0;i < numberOfElements;i++){
                                    int64 sourceData = (int64)sourcePtrd[0];  //compiler trunc
                                    int64 *sourcePtr = &sourceData;
                                    Int64ToInt(destinationPtr,destinationShift,destinationBTD.size,destinationIsSigned,
                                             sourcePtr);
                                    sourcePtrd++;
                                }
                            } else {
                                  for (int i = 0;i < numberOfElements;i++){
                                      int32 sourceData = (int32)sourcePtrf[0];  //compiler trunc
                                      int32 *sourcePtr = &sourceData;
                                      Int32ToInt(destinationPtr,destinationShift,destinationBTD.size,destinationIsSigned,
                                              sourcePtr);
                                      sourcePtrf++;
                                }
                            }

                        }

			//}  //  !destinationIsSigned
                } break;
    /***************************************************************/
    /*  FROM FLOAT TO FLOAT                                        */
    /***************************************************************/
                case BTDTFloat:{
// 		    printf("float2float\n"); // DEBUG
                    bool destinationIsDouble = False;
                    switch (destinationBTD.size){
                        case 64:{
                            destinationIsDouble = True;
                        }break;
                        case 32:{

                        }break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.size = %i is inappropriate for float",destinationBTD.size);
                            return False;
                        }
                    };
                    switch (destinationBTD.flags){
                        case BTDSTNone:{
                        } break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for float",destinationBTD.flags);
                            return False;
                        }
                    };
                    //from float to double
                    if (destinationIsDouble){
                        if (!sourceIsDouble){
                            double *dp = (double *)destination;
                            for (int i = 0;i < numberOfElements;i++){
                                double sourceData = (double)sourcePtrf[0];   //compiler conversion
                                dp[0] = sourceData;
                                dp++;
                                sourcePtrf++;;
                            }
                        } else {
                            double *dp = (double *)destination;
                            for (int i = 0;i < numberOfElements;i++){
                                *dp++ = *sourcePtrd++;
                            }
                        }
                    //from double to float:
                    } else {  //destination float
                        if (sourceIsDouble){
                            float *fp = (float *)destination;
                            for (int i = 0;i < numberOfElements;i++){
                                double sourceData = (double)sourcePtrd[0];  //compiler conversion
                                fp[0] = sourceData;
                                fp++;
                                sourcePtrd++;
                            }
                        } else {
                            float *fp = (float *)destination;
                            for (int i = 0;i < numberOfElements;i++){
                                *fp++ = *sourcePtrf++;
                            }
                        }
                    }

                }break;
    /***************************************************************/
    /*  FROM FLOAT TO STRING                                       */
    /***************************************************************/
                case BTDTString:{
// 		  printf("float2string\n"); // DEBUG
                    char format[10];
                    {
                        const char *desiredPadding = "";
                        if (sourceBTD.Format().floatInfo.padding == 1) desiredPadding = " ";
                        if (sourceBTD.Format().floatInfo.padding == 2) desiredPadding = "0";

                        if (sourceBTD.Format().floatInfo.fixedFormat){
                            sprintf(format,"%%%s%if",desiredPadding,sourceBTD.Format().floatInfo.minNumberOfFigures);
                        } else {
                            sprintf(format,"%%%s%ie",desiredPadding,sourceBTD.Format().floatInfo.minNumberOfFigures);
                        }
                    }

                    BTStringPutDataFunction writeFunction = BTSPDFArray[destinationBTD.flags];;
                    const void *writeData = destination;

                    if (writeFunction == NULL){
                        CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for string",sourceBTD.flags);
                        return False;
                    }

                    // only for CARRAY
                    int size = destinationBTD.CArraySize();

                    char sep = ' ';
                    if (destinationBTD.Format().stringInfo.separator == 1) sep = ',';
                    if (destinationBTD.Format().stringInfo.separator == 2) sep = ';';
                    if (destinationBTD.Format().stringInfo.separator == 3) sep = '\n';

                    char buffer[128];
                    for (int i = 0;i < numberOfElements;i++){
                        double x;
                        if (sourceIsDouble){
                            x = sourcePtrd[i];
                        } else {
                            x = sourcePtrf[i];
                        }
                        snprintf(buffer,sizeof(buffer)-1,format,x);
                        writeFunction(writeData,buffer,size,i,sep);
                    }
                }break;
    /***************************************************************/
    /*  FROM FLOAT TO ??????                                       */
    /***************************************************************/
                default:{
// 		    printf("float2???\n"); // DEBUG
                    CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.type = %i is unknown type",destinationBTD.type);
                    return False;
                }break;
            }

        }break;

/***************************************************************/
/***************************************************************/
/*  FROM STRING                                                */
/***************************************************************/
/***************************************************************/
        case BTDTString:{
            BTStringGetDataFunction accessFunction = BTSGDFArray[sourceBTD.flags];
            const void *accessData = source;
            int sourceSize = 0;

            if (sourceBTD.flags == BTDSTBString){
                accessFunction = BTSGDF_CArray;
                BString * bs = (BString * )source;
                accessData = bs->Buffer();
            }

            if (accessFunction == NULL){
                CStaticAssertErrorCondition(FatalError,"BTConvert: sourceBTD.flags = %i is inappropriate for string",sourceBTD.flags);
                return False;
            }

            switch (destinationBTD.type){
            /*-------------------------------------------------------------*/
            /*  FROM STRING: TO INTEGER                                    */
            /*-------------------------------------------------------------*/
                case BTDTInteger:{
// 		    printf("string2integer\n"); // DEBUG
                    bool isSigned = True;
                    switch (destinationBTD.flags){
                        case BTDSTUnsigned:{
                            isSigned = False;
                        }break;
                        case BTDSTNone:{
                            isSigned = True;
                        } break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for integer",destinationBTD.flags);
                            return False;
                        }
                    }

                    // if we copy compacted then this the number of bits to start copy the next
                    int destinationShift = 0;
                    int64 *destinationPtr = (int64 *)destination;
                    for (int i = 0;i < numberOfElements;i++){
                        char *token = accessFunction(accessData);
                        const char *tokenCopy = token;
                        int64 converted = StringToInt64(tokenCopy,destinationBTD.size,isSigned);
                        int sourceShift = 0;
                        int64 *sourcePtr = &converted;
                        IntToInt(destinationPtr,destinationShift,destinationBTD.size,isSigned,
                                     sourcePtr,sourceShift,64,True);
                        free((void *&)token);
                    }
                } break;
    /***************************************************************/
    /*  FROM STRING TO FLOAT                                       */
    /***************************************************************/
                case BTDTFloat:{
// 		    printf("string2float\n"); // DEBUG
                      bool destinationIsDouble = False;
                    switch (destinationBTD.size){
                        case 64:{
                            destinationIsDouble = True;
                        }break;
                        case 32:{

                        }break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.size = %i is inappropriate for float",destinationBTD.size);
                            return False;
                        }
                    };
                    switch (destinationBTD.flags){
                        case BTDSTNone:{
                        } break;
                        default:{
                            CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for float",destinationBTD.flags);
                            return False;
                        }
                    };
                    //from float to double
                    if (destinationIsDouble){
                        double *dp = (double *)destination;
                        for (int i = 0;i < numberOfElements;i++){
                            char *token = accessFunction(accessData);
                            double converted = atof(token);
                            free((void *&)token);
                            *dp++ = converted;
                        }
                    //from double to float:
                    } else {  //destination float
                        float *fp = (float *)destination;
                        for (int i = 0;i < numberOfElements;i++){
                            char *token = accessFunction(accessData);
                            double converted = atof(token);
                            free((void *&)token);
                            *fp++ = converted;                
                        }
                    }

//                    return False;
                } break;
    /***************************************************************/
    /*  FROM STRING TO STRING                                      */
    /***************************************************************/
                case BTDTString:{
// 		    printf("string2string\n"); // DEBUG
                    BTStringPutDataFunction writeFunction = BTSPDFArray[destinationBTD.flags];
                    const void *writeData = destination;

                    if (writeFunction == NULL){
                        CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.flags = %i is inappropriate for string",sourceBTD.flags);
                        return False;
                    }

                    // only for CARRAY
                    int size = destinationBTD.CArraySize();

                    char sep = ' ';
                    if (destinationBTD.Format().stringInfo.separator == 1) sep = ',';
                    if (destinationBTD.Format().stringInfo.separator == 2) sep = ';';
                    if (destinationBTD.Format().stringInfo.separator == 3) sep = '\n';

                    for (int i = 0;i < numberOfElements;i++){
                        char *token = accessFunction(accessData);
                        writeFunction(writeData,token,size,i,sep);
                        free((void *&)token);
                    }
                } break;
    /***************************************************************/
    /*  FROM STRING TO ??????                                      */
    /***************************************************************/
                default:{
// 		    printf("string2???\n"); // DEBUG
                    CStaticAssertErrorCondition(FatalError,"BTConvert: destinationBTD.type = %i is unknown type",sourceBTD.type);
                    return False;
                }break;
            }
        }break;
/***************************************************************/
/***************************************************************/
/*  FROM ??????                                                */
/***************************************************************/
/***************************************************************/
        default:{
// 	    printf("????\n"); // DEBUG
            CStaticAssertErrorCondition(FatalError,"BTConvert: sourceBTD.type = %i is unknown type",sourceBTD.type);
            return False;
        }break;


    }

    return True;
}


