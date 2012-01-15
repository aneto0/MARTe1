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

#include "HtmlStream.h"
#include "FString.h"

/** buffer is read and copied into the selected stream. */
bool HSWWrite(HtmlStream &hsw, const void* buffer, uint32 &size){
    StreamInterface *wrappedStream = hsw.wrappedStream;

    const char *p = (char *)buffer;
    switch (hsw.selectedStream){
        case NormalStreamMode:{
            for (int i = 0;i < size;i++){
                const char *replacement = NULL;
                switch(p[i]){
                    case '&' : replacement = "&amp;";                            break;
                    case '"' : replacement = "&quot;";                           break;
                    case '>' : replacement = "&gt;";                             break;
                    case '<' : replacement = "&lt;";                             break;
                    case '\n': replacement = "<BR>\n";                           break;
                    case '\t': replacement = "&nbsp;&nbsp;&nbsp;&nbsp;";         break;
                    case ' ' : replacement = "&nbsp;";                           break;
                }
                if (replacement != NULL) {
                    uint32 size = strlen(replacement);
                    wrappedStream->Write(replacement,size);
                } else {
                    wrappedStream->PutC(p[i]);
                }
            }
            return True;
        } break;
    }
    return hsw.buffer.Write(buffer,size);
}

bool HSWFlush(HtmlStream &hsw){
    StreamInterface *wrappedStream = hsw.wrappedStream;
    hsw.buffer.Seek(0);

    switch (hsw.selectedStream){
        case NormalStreamMode:{
        } break;

        case ColourStreamMode:{
            FString attrib1;
            hsw.buffer.GetToken(attrib1," ,;.\n\t");
            int fg = atoi(attrib1.Buffer());
            FString attrib2;
            hsw.buffer.GetToken(attrib2," ,;.\n\t");
            int bg = atoi(attrib2.Buffer());
            wrappedStream->Printf(
                "<SPAN STYLE=\"color: #%06x;background-color: #%06x;\">",
                SAColorsToRGB((SAColours)fg),SAColorsToRGB((SAColours)bg));
        } break;
        case FontTypeStreamMode:{
            wrappedStream->Printf(
                "<SPAN STYLE=\"font-family: %s;\">",hsw.buffer.Buffer());
        } break;
        case FontStyleStreamMode:{
            wrappedStream->Printf(
                "<SPAN STYLE=\"font-style: %s;\">",hsw.buffer.Buffer());
        } break;
        case HtmlTagStreamMode:{
            wrappedStream->Printf(
                "<%s>\n",hsw.buffer.Buffer());
        } break;
        default:{
            hsw.buffer.SetSize(0);
            return False;
        }
    }
    hsw.buffer.SetSize(0);
    return True;
}


