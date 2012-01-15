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

/** 
 * @file 
 * Stream attributes and definitions
 */
#if !defined (STREAM_ATTRIBUTES)
#define STREAM_ATTRIBUTES

#include "GenDefs.h"

/** The  colours codes*/
typedef enum Colours SAColours;

/** codes to choose the attribute stream to read/write to
    some classes will implement only some of these
*/
enum SAStreamModes{
    /** write to the console */
    NormalStreamMode    = 0x0000,

    /** change colour , equivalent to  "colour"
        R/W fg and bg colours,  ascii, two integers,  first is the fg second the bg  */
    ColourStreamMode    = 0x0001,

    /** change cursor position , equivalent to "cursor"
        R/W the cursor position, ascii, two integers X,Y */
    CursorStreamMode    = 0x0002,

    /** change the buffer size , equivalent to "size"
        R/W the buffer size,  ascii, two integers DX,DY */
    SizeStreamMode      = 0x0003,

    /** change window location , equivalent to "window"
        R/W the window size, ascii, two integers DX,DY */
    WindowStreamMode    = 0x0004,

    /** change the type of font. (fontType)
        possible values are monospaced sans-serif ...  */
    FontTypeStreamMode  = 0x0005,

    /** (fontStyle) maps into CSS 'font-style' italic bold ...*/
    FontStyleStreamMode = 0x0006,

    /** Chooses what screen to show
        screen number is added to. Up to 4096 screens*/
    MultiScreenSelector = 0x1000,

    /** writes an html/xml tag: simply wraps the transaction with a <> bracket set */
    HtmlTagStreamMode   = 0x2000,

    /** */
    NullStreamMode    = 0xFFFFFFFF
};

extern "C"{

    /** convert names to SAStreamModes
        the current list of valid names is
        "normal"
        "colour"
        "cursor"
        "size"
        "window"
    */
    SAStreamModes SANameToStreamModes(const char *name);

    /** converts a color to RGB*/
    int SAColorsToRGB(SAColours color);

}


#endif
