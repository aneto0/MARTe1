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

#include "Console.h"
#include "FString.h"

OBJECTREGISTER(Console,"$Id$")

bool ConsoleSSWrite(Console &con,const void* buffer, uint32 &size,TimeoutType msecTimeout){
    switch (con.selectedStream){
        case ColourStreamMode:
        {
            FString s;
            s.Write(buffer,size);
            s.Seek(0);
            FString attrib1;
            s.GetToken(attrib1," ,;.\n\t");
            int fg = atoi(attrib1.Buffer());
            FString attrib2;
            s.GetToken(attrib2," ,;.\n\t");
            int bg = atoi(attrib2.Buffer());
            return ConsoleSetColour(con,(SAColours) fg,(SAColours) bg);
        } break;

        case CursorStreamMode:
        {
            FString s;
            s.Write(buffer,size);
            s.Seek(0);
            FString attrib1;
            s.GetToken(attrib1," ,;.\n\t");
            int X = atoi(attrib1.Buffer());
            FString attrib2;
            s.GetToken(attrib2," ,;.\n\t");
            int Y = atoi(attrib2.Buffer());
            return ConsoleSetCursorPosition(con,X,Y);
        } break;


        case SizeStreamMode  :
        {
            FString s;
            s.Write(buffer,size);
            s.Seek(0);
            FString attrib1;
            s.GetToken(attrib1," ,;.\n\t");
            int X = atoi(attrib1.Buffer());
            FString attrib2;
            s.GetToken(attrib2," ,;.\n\t");
            int Y = atoi(attrib2.Buffer());
            return ConsoleSetSize(con,X,Y);
        } break;

        case WindowStreamMode:
        {
            FString s;
            s.Write(buffer,size);
            s.Seek(0);
            FString attrib1;
            s.GetToken(attrib1," ,;.\n\t");
            int X = atoi(attrib1.Buffer());
            FString attrib2;
            s.GetToken(attrib2," ,;.\n\t");
            int Y = atoi(attrib2.Buffer());
            return ConsoleSetWindowSize(con,X,Y);
        } break;

        case NormalStreamMode:{

        } break;

        default:{
            return False;
        }
    }

    return con.BasicConsole::Write(buffer,size,msecTimeout);
}

bool ConsoleSSRead(Console &con,void* buffer, uint32 &size,TimeoutType msecTimeout){
    switch (con.selectedStream){
        case ColourStreamMode:
        {
            int X=0;
            int Y=0;
            FString s;
            s.Printf("%i %i",X,Y);
            s.Seek(0);
            uint32 requiredSize = s.Size()+1;
            if (size > requiredSize) size = requiredSize;
            char *p = (char *)buffer;
            memcpy(p,s.Buffer(),size-1);
            p[size-1]=0;
            return True;
        } break;

        case CursorStreamMode:
        {
            int X=0;
            int Y=0;
            if (!ConsoleGetCursorPosition(con,X,Y)) return False;
            FString s;
            s.Printf("%i %i",X,Y);
            s.Seek(0);
            uint32 requiredSize = s.Size()+1;
            if (size > requiredSize) size = requiredSize;
            char *p = (char *)buffer;
            memcpy(p,s.Buffer(),size-1);
            p[size-1]=0;
            return True;
        } break;

        case SizeStreamMode  :
        {
            int X=0;
            int Y=0;
            if (!ConsoleGetSize(con,X,Y)) return False;
            FString s;
            s.Printf("%i %i",X,Y);
            s.Seek(0);
            uint32 requiredSize = s.Size()+1;
            if (size > requiredSize) size = requiredSize;
            char *p = (char *)buffer;
            memcpy(p,s.Buffer(),size-1);
            p[size-1]=0;
            return True;
        } break;

        case WindowStreamMode:
        {
            int X=0;
            int Y=0;
            if (!ConsoleGetWindowSize(con,X,Y)) return False;
            FString s;
            s.Printf("%i %i",X,Y);
            s.Seek(0);
            uint32 requiredSize = s.Size()+1;
            if (size > requiredSize) size = requiredSize;
            char *p = (char *)buffer;
            memcpy(p,s.Buffer(),size-1);
            p[size-1]=0;
            return True;
        } break;

        case NormalStreamMode:{

        } break;

        default:{
            return False;
        }
    }

    return con.BasicConsole::Read(buffer,size,msecTimeout);
}

