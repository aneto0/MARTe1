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

#include "Base64Codec.h"


const char *Base64Alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static class Base64Codec{
public:

    //
    char  decodingTable[256];
    //
    Base64Codec(){
        int i;
        for (i = 0; i<256;i++) decodingTable[i] = 0xFF;
        for (i = 0; i<64;i++)  decodingTable[Base64Alphabet[i]] = i;
        decodingTable['='] = 0xFE;
    }
}b64;


bool B64Encode(FString &input, FString &output){
    int32 size = input.Size();
    const unsigned char *in = (const unsigned char *)input.Buffer();
    output = "";
    uint32 word = 0;
    uint32 code ;
    while(size>0){
        word = in[0];
        code = word >> 2;
        word &=0x3;
        output += Base64Alphabet[code];
        if (size>1) {
            word <<=8;
            word |= in[1];
            code = word >> 4;
            word &=0xF;
            output += Base64Alphabet[code];
            if (size>2) {
                word <<=8;
                word |= in[2];
                code = word;
                word &=0x3F;
                code >>=6;
                output += Base64Alphabet[code];
                output += Base64Alphabet[word];
            } else { // 4 bits available
                word <<=2;
                output += Base64Alphabet[word];
                output += "=";
            }
        } else { // 2 bits available
            word <<=4;
            output += Base64Alphabet[word];
            output += "==";
        }
        size -=3;
        in+=3;
    }

    return True;
}

bool B64Decode(FString &input, FString &output){
    uint32 size = input.Size();
    const char *in = input.Buffer();
    output = "";
    uint32 word = 0;
    uint32 code ;
    char key;
    char c;
    while(size>=4){
        c = in[0];
        if (c == '=') return True;
        key = b64.decodingTable[c];
        if (key == (char)0xFF) return False;
        word = key;

        c = in[1];
        if (c == '=') {
            word <<=2;
            output += word;
            return True;
        }
        key = b64.decodingTable[c];
        if (key == (char)0xFF) return False;
        word <<=6;
        word |= key;
        code = word >> 4;
        word &= 0xF;
        output += code;

        c = in[2];
        if (c == '=') {
            word <<=4;
            output += word;
            return True;
        }
        key = b64.decodingTable[c];
        if (key == (char)0xFF) return False;
        word <<=6;
        word |= key;
        code = word >> 2;
        word &= 0x3;
        output += code;

        c = in[3];
        if (c == '=') {
            return True;
        }
        key = b64.decodingTable[c];
        if (key == (char)0xFF) return False;
        word <<=6;
        word |= key;
        output += word;

        size -=4;
        in+=4;
    }
    return True;
}

