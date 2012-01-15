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

#include "HttpClassListResource.h"
#include "ObjectRegistryDataBase.h"
#include "FString.h"
#include "HtmlStream.h"

OBJECTLOADREGISTER(HttpClassListResource,"$Id: HttpClassListResource.cpp,v 1.4 2008/02/04 14:11:57 fisa Exp $")


bool HttpClassListResource::ProcessHttpMessage(HttpStream &hStream){


    hStream.Printf(
        "<HTML>\n"
        "<HEAD><TITLE>%s</TITLE></HEAD>\n"
        "<STYLE type=\"text/css\">\n"
        "  BODY { background: black; color: green}\n"
        "  A:link { color: red }\n"
        "  A:visited { color: maroon }\n"
        "  A:active { color: fuchsia }\n"
        "</STYLE>\n"
        "<H1>%s</H1><UL>\n"
        ,
        Comment(),
        Comment()
        );

    FString className;
    if (hStream.Switch("InputCommands.Class")){
        hStream.Seek(0);
        uint32 size = hStream.Size();
        className.SetSize(size);
        hStream.Read(className.BufferReference(),size);
        hStream.Switch(NormalStreamMode);
    }
    hStream.Switch(NormalStreamMode);

    HtmlStream hmStream(hStream);
    {

        hmStream.Printf("B = Object can be built by name; A = Allocated ojects; D = code from DLL ;S = structure public\n");
        hmStream.SSPrintf(HtmlTagStreamMode,"TABLE border=2");
        hmStream.SSPrintf(HtmlTagStreamMode,"TR");
        hmStream.SSPrintf(HtmlTagStreamMode,"TD");
        hmStream.Printf("Flags");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
        hmStream.SSPrintf(HtmlTagStreamMode,"TD");
        hmStream.Printf("Name");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
        hmStream.SSPrintf(HtmlTagStreamMode,"TD");
        hmStream.Printf("Version");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
        hmStream.SSPrintf(HtmlTagStreamMode,"TD");
        hmStream.Printf("Size");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
        hmStream.SSPrintf(HtmlTagStreamMode,"TD");
        hmStream.Printf("Allocated");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
        hmStream.SSPrintf(HtmlTagStreamMode,"TD");
        hmStream.Printf("");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
        hmStream.SSPrintf(HtmlTagStreamMode,"/TR");

        ObjectRegistryItem *p = ObjectRegistryDataBaseList();

        while (p != NULL){
            hmStream.SSPrintf(HtmlTagStreamMode,"TR");

            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.SSPrintf(ColourStreamMode,"%i %i",Green,Black);
            if (p->Tools() != NULL){
                if (p->Tools()->buildFn != NULL){
                    hmStream.Printf("B");
                }
            }
            if (p->Library() != NULL){
                hmStream.Printf("L");
            }
            if (p->nOfAllocatedObjects > 0){
                hmStream.Printf("A");
            }
            if (p->structure > 0){
                hmStream.Printf("S");
            }

            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD align=left");

            hmStream.SSPrintf(ColourStreamMode,"%i %i",Red,Black);
            hmStream.Printf("%s",p->ClassName());

            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");

            hmStream.SSPrintf(ColourStreamMode,"%i %i",DarkRed,Black);
            hmStream.Printf("%s",p->Version());

            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");

            hmStream.SSPrintf(ColourStreamMode,"%i %i",Red,Black);
            hmStream.Printf("%i",p->Size());

            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            if (p->nOfAllocatedObjects > 0){

                hmStream.SSPrintf(ColourStreamMode,"%i %i",DarkRed,Black);

                hmStream.Printf("%i",p->nOfAllocatedObjects);
            }
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            if (p->structure > 0){
                if (strcmp(className.Buffer(),p->ClassName())==0){
                    hmStream.SSPrintf(HtmlTagStreamMode,"TABLE");

                    ClassStructureEntry **ce = p->structure->Members();
                    if (ce){
                        while(*ce != NULL){
                            hmStream.SSPrintf(HtmlTagStreamMode,"TR");

                            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
                            hmStream.SSPrintf(ColourStreamMode,"%i %i",Green,Black);
                            hmStream.Printf("%s %s",(*ce)->type,(*ce)->modif);
                            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");

                            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
                            hmStream.SSPrintf(ColourStreamMode,"%i %i",Green,Black);
                            hmStream.Printf("%s",(*ce)->name);
                            for (int i = 0;i <CSE_MAXSIZE; i++){
                                if ((*ce)->sizes[i] > 1){
                                    hmStream.Printf("[%i]",(*ce)->sizes[i]);
                                }
                            }
                            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");

                            hmStream.SSPrintf(HtmlTagStreamMode,"/TR");
                            ce++;
                        }
                    }
                    hmStream.SSPrintf(HtmlTagStreamMode,"/TABLE");
                } else {
                    hmStream.SSPrintf(ColourStreamMode,"%i %i",Red,Black);

                    hmStream.SSPrintf(HtmlTagStreamMode,"A HREF=/%s?Class=%s NAME=+",hStream.url.Buffer(),p->ClassName());
                    hmStream.Printf("+");
                    hmStream.SSPrintf(HtmlTagStreamMode,"/A");
                }
            }

            //hmStream.Printf("%30s %20s %x\n",p->ClassName(),p->Version(),p->Size());
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"/TR");

            p = (ObjectRegistryItem *)p->Next();
        }

        hmStream.SSPrintf(HtmlTagStreamMode,"/TABLE");
    }


    hStream.Printf("</BODY></HTML>\n");

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);


    return True;
}







