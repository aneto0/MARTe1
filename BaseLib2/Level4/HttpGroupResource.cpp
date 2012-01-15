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

#include "HttpGroupResource.h"
#include "FString.h"
#include "HttpUrl.h"

/** Simply list the content of a GCReferenceContainer*/
class HGRPageBuilder:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

    /** the page base address */
    const char *        baseUrl;

public:
    /** @param s = NULL the code will use printf to write to the console */
    HGRPageBuilder(StreamInterface *s,const char *baseUrl){
        stream = s;
        this->baseUrl = baseUrl;

        if (baseUrl != NULL)  {
            // if the page url contains a / then we can use relative addresses
            if (baseUrl[strlen(baseUrl)-1] == '/') this->baseUrl = NULL;
        }
    }

    /** actual function */
    virtual void Do(GCReference data){
        if (stream == NULL) return;

        GCRTemplate<GCNamedObject> gcno;
        const char * name;
        const char * className;
        const char * comment;
        className = "unknown class";
        name      = "??";
        comment   = "??";
        gcno = data;
        if (gcno.IsValid()){
            name = gcno->Name();
            className = gcno->ClassName();
        }

        GCRTemplate<HttpUrl> hu;
        hu = data;
        if (hu.IsValid()){
            stream->Printf("<TR>\n");
            stream->Printf(
            "<TD>%s</TD><TD><A HREF=\"%s/\">%s</A> </TD>\n",
            "LINK",hu->Buffer(),name);
            stream->Printf("</TR>\n");
            return;
        }


        GCRTemplate<HttpInterface> hi;
        hi = data;
        if (hi.IsValid()){
            comment = hi->Comment();
        }
        stream->Printf("<TR>\n");
        if ((baseUrl == NULL) || (baseUrl[0] == 0)){
            stream->Printf(
            "<TD>%s</TD><TD><A HREF=\"%s/\">%s</A> </TD>\n",
            className,name,comment);
        } else {
            stream->Printf(
            "<TD>%s</TD><TD><A HREF=\"/%s/%s/\">%s</A> </TD>\n",
            className,baseUrl,name,comment);
        }
        stream->Printf("</TR>\n");
    }
};


bool HttpGroupResource::ProcessHttpMessage(HttpStream &hStream){

    // the sub-components are addressed by the server directly
    const char *address = hStream.unMatchedUrl.Buffer();
    if ((address != NULL) && (address[0] !=0)) return False;

    HGRPageBuilder hgrpb(&hStream,hStream.url.Buffer());

    hStream.Printf("<html><head><TITLE>%s</TITLE>"
                  "</head><BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>",Comment(),Comment());
    hStream.Printf("<TABLE>\n");
    Iterate(&hgrpb);
    hStream.Printf("</TABLE>\n");
    hStream.Printf("</UL></BODY>\n");

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;
}





OBJECTLOADREGISTER(HttpGroupResource,"$Id$")






