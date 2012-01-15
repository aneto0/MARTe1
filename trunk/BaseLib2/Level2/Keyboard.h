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
 * manages the direct access to keyboard
 */
#if !defined(KEYBOARD_H)
#define KEYBOARD_H

#include "System.h"

class Keyboard{

public:
#if !(defined(_VXWORKS)||defined(_RTAI)|| defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    /** read a char without echoing */
    static  char GetChar(){
        return _getch();
    }
#else
    static char GetChar(){
        return getchar();
    }

#endif
};

#endif

