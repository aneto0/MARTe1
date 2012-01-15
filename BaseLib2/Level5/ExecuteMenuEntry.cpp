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

#include "ExecuteMenuEntry.h"

OBJECTLOADREGISTER(ExecuteMenuEntry,"$Id: ExecuteMenuEntry.cpp,v 1.1 2008/05/13 16:05:38 fpiccolo Exp $")


bool ExecuteMenuAction(StreamInterface &in,StreamInterface &out,void *userData){
    if(userData == NULL) return False;
    ExecuteMenuEntry *eme = (ExecuteMenuEntry *)userData;
    if(eme->action == NULL) return False;

    return eme->action(in,out,eme);
}
