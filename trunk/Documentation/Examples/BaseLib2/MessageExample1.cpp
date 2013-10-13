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
 * $Id: GODBExample1.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 * @file Exchange of messages between objects
 */
#include "GCNamedObject.h"
#include "ConfigurationDataBase.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"
#include "Sleep.h"

static EventSem exampleDone;

//An object that knows how to receive and send messages
OBJECT_DLL(Echo)
class Echo : public GCNamedObject, public MessageHandler{
OBJECT_DLL_STUFF(Echo)
private:
    FString echoDestination;
public:
    float   aFloatValue;

    Echo(){
        echoDestination.SetSize(0);
        aFloatValue = 0;
    }

    virtual ~Echo(){
    }

    bool ObjectLoadSetup(ConfigurationDataBase &cdb,StreamInterface *err){
        //Automatically read the object name
        GCNamedObject::ObjectLoadSetup(cdb, err);
        CDBExtended cdbe(cdb);
        if(!cdbe.ReadFString(echoDestination, "Destination", "")){
            AssertErrorCondition(Warning, "%s::No destination was set", Name());
        }
        return True;
    }

    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
        //Always check if the envelope is valid
        if(!envelope.IsValid()){
            AssertErrorCondition(FatalError, "%s::ProcessMessage: envelope is not valid!!", Name());
            return False;
        }
        //Get the message
        GCRTemplate<Message> message = envelope->GetMessage();
        if (!envelope.IsValid()) {
            AssertErrorCondition(FatalError, "%s::ProcessMessage: message is not valid!!", Name());
            return False;
        }

        int32 code = message->GetMessageCode().Code();
        const char *header = message->Content();
        AssertErrorCondition(Information, "%s::ProcessMessage: processing message %s (code: %d) from %s]", Name(), header, code, envelope->Sender());

        FString ECHOSTR  = "ECHO";
        FString STARTSTR = "START";
        if(STARTSTR == header){
            if(echoDestination.Size() > 0){
                //Send a message to our destination and wait for a reply
                //This message will contain a ConfigurationDatabase (it can contain any number of objects of any type, it is a GCReferenceContainer)
                CDBExtended cdbToSend;
                cdbToSend.WriteFloat(5.0, "aFloatValue");
                //Create the message and the envelope
                GCRTemplate<Message> messageToSend(GCFT_Create);
                messageToSend->Init(0, "ECHO");
                messageToSend->Insert(cdbToSend);
                GCRTemplate<MessageEnvelope> envelopeToSend(GCFT_Create);
                envelopeToSend->PrepareMessageEnvelope(messageToSend, echoDestination.Buffer(), MDRF_ManualReply, this);

                AssertErrorCondition(Information, "%s sent message to %s", Name(), echoDestination.Buffer());
                //Prepare the reply envelope where we will get the anwser from
                GCRTemplate<MessageEnvelope> replyEnvelope;
                SendMessageAndWait(envelopeToSend, replyEnvelope);
                if(!replyEnvelope.IsValid()) {
                    AssertErrorCondition(FatalError, "%s::failed SendMessageAndWait to %s", Name(), echoDestination.Buffer());
                }
                GCRTemplate<Message> replyMessage = replyEnvelope->GetMessage();
                replyEnvelope->CleanUp();

                if (!replyMessage.IsValid()) {
                    AssertErrorCondition(FatalError, "%s::no message in the reply from %s", Name(), echoDestination.Buffer());
                    return False;
                }

                //Read what was replied 
                CDBExtended cdbReceived = replyMessage->Find(0);
                if(cdbReceived.IsValid()){
                    if(!cdbReceived.ReadFloat(aFloatValue, "aFloatValue")){
                        AssertErrorCondition(FatalError, "%s::no aFloatValue in the reply from %s", Name(), echoDestination.Buffer());
                        return False;
                    }
                }
                exampleDone.Post();
            }
        }
        else if(ECHOSTR == header){
            //Check the content of what was sent
            CDBExtended cdbReceived = message->Find(0);
            if(!cdbReceived.ReadFloat(aFloatValue, "aFloatValue")){
                AssertErrorCondition(FatalError, "%s::no aFloatValue from %s", Name(), envelope->Sender());
                return False;
            }
            //Reply back the requested value multiplied by -1
            if(envelope->ManualReplyExpected()){
                GCRTemplate<Message> replyMessage(GCFT_Create);
                if(!replyMessage.IsValid()){
                    AssertErrorCondition(FatalError, "%s: Failed creating response message for ECHO", Name());
                    return False;
                }
                GCRTemplate<MessageEnvelope> replyEnvelope(GCFT_Create);
                if(!replyEnvelope.IsValid()){
                    AssertErrorCondition(FatalError, "%s: Failed creating response message envelope for ECHO", Name());
                    return False;
                }
                
                cdbReceived.WriteFloat(-aFloatValue, "aFloatValue");
                replyMessage->Init(0, "OK");
                replyMessage->Insert(cdbReceived);
                replyEnvelope->PrepareReply(envelope, replyMessage);
                AssertErrorCondition(Information, "%s sent message to %s", Name(), envelope->Sender());
                MessageHandler::SendMessage(replyEnvelope);
            }
        }

        return True;
    }
};
OBJECTLOADREGISTER(Echo, "$Id: MessageExample1.cpp 3 2012-01-15 16:26:07Z aneto $")

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Configuration stored in an FString (usually this is a file or a tcp stream)
    //Notice the + and the Class = 
    FString cdbTxt = 
        "+Listener1 = {\n"
        "    Class         = Echo\n"
        "    Destination   = Listener2\n"
        "}\n"
        "+Listener2 = {\n"
        "    Class         = Echo\n"
        "}\n";
    //Create the configuration database and load from a string
    ConfigurationDataBase cdb;
    if(!cdb->ReadFromStream(cdbTxt)){
        CStaticAssertErrorCondition(FatalError, "Failed reading from stream!");
        return -1;
    }

    //Let the GlobalObjectDataBase automatically create the objects
    if(!GetGlobalObjectDataBase()->ObjectLoadSetup(cdb, NULL)){
        CStaticAssertErrorCondition(FatalError, "Failed to load cdb");
        return -1;
    }
    
    exampleDone.Create();
    exampleDone.Reset();

    int32 i=0;
    for(i=0; i<GetGlobalObjectDataBase()->Size(); i++){
        GCRTemplate<Echo> echoObj = GetGlobalObjectDataBase()->Find(i);
        if(echoObj.IsValid()){
            CStaticAssertErrorCondition(FatalError, "aFloatValue of %s is %f\n", echoObj->Name(), echoObj->aFloatValue);
            GCRTemplate<Message> message(GCFT_Create);
            message->Init(0, "START");
            GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
            
            bool ret = True;
            ret &= envelope->PrepareMessageEnvelope(message, echoObj->Name());
            ret &= MessageHandler::SendMessage(envelope);
            if(!ret){
                CStaticAssertErrorCondition(FatalError, "Failed to send message to %s\n", echoObj->Name());
            }
        }
    }

    exampleDone.Wait();
    
    for(i=0; i<GetGlobalObjectDataBase()->Size(); i++){
        GCRTemplate<Echo> echoObj = GetGlobalObjectDataBase()->Find(i);
        if(echoObj.IsValid()){
            CStaticAssertErrorCondition(FatalError, "aFloatValue of %s is %f\n", echoObj->Name(), echoObj->aFloatValue);
        }
    }

    return 0;
}

