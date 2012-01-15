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

#include "HttpMessageSendResource.h"
#include "MessageEnvelope.h"
#include "MessageDeliveryRequest.h"
#include "MessageDispatcher.h"
#include "FString.h"

/** Simply list the content of a GCReferenceContainer*/
class HMSPageBuilder:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

public:
    /** @param s = NULL the code will use printf to write to the console */
    HMSPageBuilder(StreamInterface *s = NULL){
        stream = s;
    }

    /** actual function */
    virtual void Do(GCReference data){
        GCRTemplate<GCNamedObject> gcno;
        const char * name;
        gcno = data;
        if (gcno.IsValid()){
            name = gcno->Name();
        } else return;
        GCRTemplate<MessageEnvelope> me;
        me = data;
        if (me.IsValid()){
            GCRTemplate<Message> message = me->GetMessage();
            if (stream){
                stream->Printf(
                    "<FORM><TR>\n"
                    "<TD><BUTTON name=\"ACTION\" value=\"%s\" type=\"submit\">%s</BUTTON></TD>\n"
                    "<TD>%s</TD>\n",
                    name,name,me->Destination());
                if (message.IsValid()){
                    int32 code = message->GetMessageCode().Code();
                    if (code > UserMessageCode.Code()){
                        stream->Printf(
                            "<TD>U+%i</TD>\n",code-UserMessageCode.Code()
                        );
                    } else {
                        stream->Printf(
                            "<TD>%i</TD>\n",code
                        );
                    }
                    stream->Printf(
                        "<TD>%s</TD>\n",
                        message->Content()
                    );
                } else {
                    stream->Printf(
                        "<TD>NO CONTENT</TD>\n"
                        "<TD>NO CONTENT</TD>\n"
                    );
                }
                stream->Printf(
                    "</TR></FORM>\n"
                );
            }
            return;
        }
        GCRTemplate<MessageDeliveryRequest> mdr;
        mdr = data;
        if (mdr.IsValid()){
            GCRTemplate<Message> message = mdr->GetMessage();
            if (stream){
                stream->Printf(
                    "<FORM><TR>\n"
                    "<TD><BUTTON name=\"ACTION\" value=\"%s\" type=\"submit\">%s</BUTTON></TD>\n"
                    "<TD>%s</TD>\n",
                    name,name,mdr->Destinations());
                if (message.IsValid()){
                    int32 code = message->GetMessageCode().Code();
                    if (code > UserMessageCode.Code()){
                        stream->Printf(
                            "<TD>U+%i</TD>\n",code-UserMessageCode.Code()
                        );
                    } else {
                        stream->Printf(
                            "<TD>%i</TD>\n",code
                        );
                    }
                    stream->Printf(
                        "<TD>%s</TD>\n",
                        message->Content()
                    );
                } else {
                    stream->Printf(
                        "<TD>NO CONTENT</TD>\n"
                        "<TD>NO CONTENT</TD>\n"
                    );
                }
                stream->Printf(
                    "</TR></FORM>\n"
                );
            }
            return;
        }

    }
};


bool HttpMessageSendResource::ProcessHttpMessage(HttpStream &hStream){

    // the sub-components are addressed by the server directly
    const char *address = hStream.unMatchedUrl.Buffer();
    if ((address != NULL) && (address[0] !=0)) return False;

    FString command;
    if (hStream.Switch("InputCommands.ACTION")){
        hStream.Seek(0);
        hStream.GetToken(command,"");
        hStream.Switch((uint32)0);

//        printf("%s\n",command.Buffer());


        GCRTemplate<MessageEnvelope> envelope = Find(command.Buffer());
        if (envelope.IsValid()){
            GMDSendMessageEnvelope(envelope);
        } else {
            GCRTemplate<MessageDeliveryRequest> mdr = Find(command.Buffer());
            GMDSendMessageDeliveryRequest(mdr);
        }
        //copy to the client
        hStream.WriteReplyHeader(True,204);

        return True;
    }


    HMSPageBuilder hmspb(&hStream);

    hStream.Printf("<html><head><TITLE>%s</TITLE>"
                  "</head><BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>",Comment(),Comment());
    hStream.Printf("<TABLE border=2>\n");
    hStream.Printf("<TR><TD>ACTION</TD><TD>TARGET</TD><TD>CODE</TD><TD>MESSAGE</TD></TR>\n");
    Iterate(&hmspb);
    hStream.Printf("</TABLE>\n");
    hStream.Printf("</UL></BODY>\n");

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;
}





OBJECTLOADREGISTER(HttpMessageSendResource,"$Id: HttpMessageSendResource.cpp,v 1.3 2007/11/27 16:09:03 fisa Exp $")






