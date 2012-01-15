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

#include "HttpUtilities.h"

static const char *Http2Convert=" #%<>";

bool HttpEncode(Streamable &converted,const char *original){

    if (original == NULL) return False;
    while (*original != 0){
        char c = *original++;
        if (strchr(Http2Convert,c)){
            converted.Printf("%%%2x",c);
        } else {
            if (!converted.PutC(c)) return False;
        }
    }
    return True;
}

int HexDecode(char c){
    if ((c>='0') && (c<='9')) return c-'0';
    if ((c>='a') && (c<='f')) return c-'a'+ 10;
    if ((c>='A') && (c<='F')) return c-'A'+ 10;
    return -1;
}

bool HttpDecode(Streamable &destination, Streamable &source){
    char c;
    while (source.GetC(c)){
        if (c == '%'){
            char buffer[2];
            uint32 size = 2;
            source.Read(buffer,size);
            c = HexDecode(buffer[0])*16 + HexDecode(buffer[1]);
        }
        if (!destination.PutC(c)) return False;
    }
    return True;

}


