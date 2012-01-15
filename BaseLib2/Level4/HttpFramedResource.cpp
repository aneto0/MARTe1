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

#include "HttpFramedResource.h"
#include "FString.h"

OBJECT_DLL(HttpFrameLink)

class HttpFrameLink: public GCNamedObject{
OBJECT_DLL_STUFF(HttpFrameLink)

public:
    FString link;

    virtual     bool        ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        GCNamedObject::ObjectLoadSetup(info,err);
        CDBExtended cdbx(info);

        if (!cdbx.ReadFString(link,"Link","")){
            AssertErrorCondition(ParametersError,"ObjectLoadSetup: Link undefined");
            return False;
        }
        return True;
    }

};

OBJECTLOADREGISTER(HttpFrameLink,"$Id$")


/** Simply list the content of a GCReferenceContainer*/
class HFRPageBuilder:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

    /** the page base address */
    const char *        baseUrl;

public:
    /** @param s = NULL the code will use printf to write to the console */
    HFRPageBuilder(StreamInterface *s,const char *baseUrl){
        stream = s;
        this->baseUrl = baseUrl;
        if (baseUrl != NULL) {
            // if the page url contains a / then we can use relative addresses
            if (baseUrl[strlen(baseUrl)-1] == '/') this->baseUrl = NULL;
        }
    }

    /** actual function */
    virtual void Do(GCReference data){
        if (stream == NULL) return;

        GCRTemplate<HttpFrameLink> hfl = data;
        if (hfl.IsValid()){
            stream->Printf("<FRAME SRC=\"%s/\">\n",hfl->link.Buffer());
            return;
        }
        GCRTemplate<GCNamedObject> gcno;
        const char * name;
        name      = "??";
        gcno = data;
        if (gcno.IsValid()){
            name = gcno->Name();
            GCRTemplate<HttpInterface> hi;
            hi = data;
            if (hi.IsValid()){
                if ((baseUrl == NULL) || (baseUrl[0] == 0)){
                    stream->Printf("<FRAME SRC=\"%s/\">\n",name);
                } else {
                    stream->Printf("<FRAME SRC=\"/%s/%s/\">\n",baseUrl,name);
                }
            }
        }
    }
};


bool HttpFramedResource::ProcessHttpMessage(HttpStream &hStream){

    // the sub-components are addressed by the server directly
    const char *address = hStream.unMatchedUrl.Buffer();
    if ((address != NULL) && (address[0] !=0) ) return False;

    HFRPageBuilder hfrpb(&hStream,hStream.url.Buffer());

    hStream.Printf("<html>\n<head><TITLE>%s</TITLE>\n",Comment());
    if (flags && HFRVertical)
        hStream.Printf("<FRAMESET ROWS=\"");
    else
        hStream.Printf("<FRAMESET COLS=\"");


    int i;
    for (i = 0;i < sizeOfWeights;i++){
        hStream.Printf("%f",weights[i]);
        if (i != (sizeOfWeights-1)){
            hStream.Printf(", ");
        }
    }
    hStream.Printf("\">\n");

    Iterate(&hfrpb);
    hStream.Printf("</HTML>\n");

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;
}





OBJECTLOADREGISTER(HttpFramedResource,"$Id$")






