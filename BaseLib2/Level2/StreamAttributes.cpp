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

#include "StreamAttributes.h"
#include "Streamable.h"
#include "CStreamBuffering.h"

const char *saStreamTable[] = {
    "normal",
    "colour" ,
    "cursor" ,
    "size"   ,
    "window" ,
    "fontType",
    "fontStyle",
    NULL
};

SAStreamModes SANameToStreamModes(const char *name){
    if ((name != NULL) && (name[0] != 0)) {
        for(int i=0;saStreamTable[i] != NULL;i++){
            if (strcmp(saStreamTable[i],name)== 0) return (SAStreamModes)i;
        }
    }

    if (strcmp(name,"HtmlTag")) return HtmlTagStreamMode;

    return NullStreamMode;
}

/** converts a color to RGB*/
int SAColorsToRGB(SAColours color){

    switch (color){

        case Black:         return 0;
        /** */
        case DarkBlue:      return 0x000080;
        /** */
        case DarkGreen:     return 0x008000;
        /** */
        case DarkCyan:      return 0x008080;
        /** */
        case DarkRed:       return 0x800000;
        /** */
        case DarkPurple:    return 0x800080;
        /** */
        case DarkYellow:    return 0x808000;
        /** */
        case Grey:          return 0xC0C0C0;
        /** */
        case DarkGrey:      return 0x808080;
        /** */
        case Blue:          return 0x0000FF;
        /** */
        case Green:         return 0x00FF00;
        /** */
        case Cyan:          return 0x00FFFF;
        /** */
        case Red:           return 0xFF0000;
        /** */
        case Purple:        return 0xFF00FF;
        /** */
        case Yellow:        return 0xFFFF00;
        /** */
        case White:         return 0xFFFFFF;
    }
    return 0;
}

