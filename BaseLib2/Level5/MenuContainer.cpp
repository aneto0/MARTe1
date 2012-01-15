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

#include "MenuContainer.h"
#include "MenuEntry.h"
#include "HtmlStream.h"
#include "SXNull.h"

OBJECTLOADREGISTER(MenuContainer,"$Id: MenuContainer.cpp,v 1.6 2008/09/22 17:17:34 fpiccolo Exp $")

bool MCProcessHttpMessage(
                        MenuContainer &mc,
                        HttpStream &hStream){

    FString command;
    if (hStream.Switch("InputCommands.ACTION")){
        hStream.Seek(0);
        hStream.GetToken(command,"");
    }

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf(
        "<html><head>"
        "<SPAN STYLE=\"color: #FFFFFF;background-color: #000000;\">"
        "<TITLE>%s</TITLE></head>"
        "<BODY BGCOLOR=\"#000000\" FGCOLOR=\"#FFFFFF\" LINK=\"#FF0000\" VLINK=\"#FF8080\" ALINK=\"#FF8080\" >"
        "<H1>%s</H1>",
        mc.Title(),mc.Title());

/** LIST ALL THE LINKS FIRST */

    hStream.Printf(
        "<TABLE BORDER=0 CELLSPACING=10>"
        "<TR><TD><H3>\n"
        "<SPAN STYLE=\"color: #FFFFFF;background-color: #000000;\">"
        " LINKS </H3></TD></TR>\n"
        "<TR><TD>\n"
        "<H3><A HREF=\"../\">BACK</A> \n"
        "</H3></TD>\n");

    int currentRow = 6;  
    int i;
    for (i=0;i<mc.Size();i++){
        GCRTemplate<MenuContainer>mct = mc.Find(i);
        GCRTemplate<HttpInterface>hi = mc.Find(i);
        if (mct.IsValid()){
            hStream.Printf(
                "<TD><H3><A HREF=\"%s/\">%s</A></H3></TD>\n",
                mct->Name(),mct->Title());

            currentRow += (2+strlen(mct->Title()));
            if (currentRow == 100){
                hStream.Printf("</TR>\n<TR>");
                currentRow = 0;
            }
        } else
        if (hi.IsValid()){
            GCRTemplate<GCNamedObject>gcno = hi;
            if (gcno.IsValid()){

                hStream.Printf(
                    "<TD><H3><A HREF=\"%s/\">%s</A></H3></TD>\n",
                    gcno->Name(),hi->Comment());

                currentRow += (2+strlen(hi->Comment()));
                if (currentRow == 100){
                    hStream.Printf("</TR>\n<TR>");
                    currentRow = 0;
                }
            }
        }
    }

    hStream.Printf("</TABLE>\n");

/** LIST ALL THE ACTIONS */


    hStream.Printf(
        "<TABLE BORDER=0 CELLSPACING=10>\n"
        "<TR><TD><H3>\n"
        "<FONT COLOR=#FFFFFF>\n"
        " ACTIONS </H3></TD></TR>\n"
        "<TR>\n"
        "<FORM>\n");

    for (i=0;i<mc.Size();i++){
        GCRTemplate<MenuInterface>mi    = mc.Find(i);
        GCRTemplate<MenuContainer>mct   = mi;
        if(!mct.IsValid()){
            GCRTemplate<GCNamedObject> gcno = mi;
            BString name = "Unnamed";
            if(gcno.IsValid()){
                name = gcno->Name();
            }
            if (mi.IsValid()){
                
                hStream.Printf(
                    "<TD><H3>\n"
                    "<BUTTON type=\"submit\" name=\"ACTION\" value=\"%s\">%s</BUTTON>\n"
                    "</H3></TD>\n",
                    name.Buffer(), mi->Title());
                
                currentRow += (2+strlen(mi->Title()));
                if (currentRow == 100){
                    hStream.Printf("</TR>\n<TR>");
                    currentRow = 0;
                }
            }
        }
    }

    hStream.Printf("</FORM></TABLE>\n");

    if (command.Size() > 0){

        GCRTemplate<MenuInterface> meCommand = mc.Find(command.Buffer());

        if (meCommand.IsValid()){
            HtmlStream hsw(hStream);
            hsw.Switch("fontType");
            hsw.Printf("monospace");
            hsw.Switch((uint32)0);
            SXNull nullStream;
            meCommand->TextMenu(nullStream,hsw);
        }

    }

    hStream +=
        "</BODY>\n";


//    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
//    hStream.WriteReplyHeader(True);

    return True;
}
