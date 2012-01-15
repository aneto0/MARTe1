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

#include "CFGUploader.h"
#include "MessageEnvelope.h"
#include "Message.h"

OBJECTLOADREGISTER(CFGUploader,"$Id: CFGUploader.cpp,v 1.7 2011/11/14 15:15:25 aneto Exp $")

bool CFGUploader::PrintHTTPForm(HtmlStream &hmStream){
    hmStream.SSPrintf(HtmlTagStreamMode, "form enctype=\"multipart/form-data\" method=\"post\"");
    hmStream.SSPrintf(HtmlTagStreamMode, "p");
    hmStream.SSPrintf(HtmlTagStreamMode, "input type=\"checkbox\" name=\"displayCDB\"");
    hmStream.Printf("Display CDB");
    hmStream.SSPrintf(HtmlTagStreamMode, "/p");
    hmStream.SSPrintf(HtmlTagStreamMode, "p");
    hmStream.SSPrintf(HtmlTagStreamMode, "input type=\"checkbox\" name=\"waitReply\"");
    hmStream.Printf("Wait reply");
    hmStream.SSPrintf(HtmlTagStreamMode, "/p");
    hmStream.SSPrintf(HtmlTagStreamMode, "p");
    hmStream.SSPrintf(HtmlTagStreamMode, "p");
    hmStream.SSPrintf(HtmlTagStreamMode, "input type=\"checkbox\" name=\"rebuildAll\"");
    hmStream.Printf("Rebuild All");
    hmStream.SSPrintf(HtmlTagStreamMode, "/p");
    hmStream.SSPrintf(HtmlTagStreamMode, "p");
    hmStream.Printf("Configuration file location");
    hmStream.SSPrintf(HtmlTagStreamMode, "br");
    hmStream.SSPrintf(HtmlTagStreamMode, "input type=\"file\" name=\"cfgFile\" size=\"40\"");
    hmStream.SSPrintf(HtmlTagStreamMode, "/p");
    hmStream.SSPrintf(HtmlTagStreamMode, "/div");
    hmStream.SSPrintf(HtmlTagStreamMode, "input type=\"submit\" value=\"Send\"");
    hmStream.SSPrintf(HtmlTagStreamMode, "/form");
    return True;
}

bool CFGUploader::ProcessHttpMessage(HttpStream &hStream){    
    HtmlStream hmStream(hStream);
    hmStream.SSPrintf(HtmlTagStreamMode, "html");
    hmStream.SSPrintf(HtmlTagStreamMode, "head");
    hmStream.SSPrintf(HtmlTagStreamMode, "title");
    hmStream.Printf("MARTe Configuration File Uploader");
    hmStream.SSPrintf(HtmlTagStreamMode, "/title");
    hmStream.SSPrintf(HtmlTagStreamMode, "/head");
    hmStream.SSPrintf(HtmlTagStreamMode, "body");
    hmStream.SSPrintf(HtmlTagStreamMode, "h1");
    hmStream.Printf("MARTe Configuration File Uploader");
    hmStream.SSPrintf(HtmlTagStreamMode, "/h1");

    FString cfgFile;
    cfgFile.SetSize(0);
    if (hStream.Switch("InputCommands.cfgFile")){
        hStream.Seek(0);
        hStream.GetToken(cfgFile, "");
        hStream.Switch((uint32)0);
    }

    FString displayCDB;
    displayCDB.SetSize(0);
    if (hStream.Switch("InputCommands.displayCDB")){
        hStream.Seek(0);
        hStream.GetToken(displayCDB, "");
        hStream.Switch((uint32)0);
    }

    FString waitReply;
    waitReply.SetSize(0);
    if (hStream.Switch("InputCommands.waitReply")){
        hStream.Seek(0);
        hStream.GetToken(waitReply, "");
        hStream.Switch((uint32)0);
    }

    FString rebuildAll;
    rebuildAll.SetSize(0);
    if (hStream.Switch("InputCommands.rebuildAll")){
        hStream.Seek(0);
        hStream.GetToken(rebuildAll, "");
        hStream.Switch((uint32)0);
    }    

    if(cfgFile.Size() > 0){        
        //Try to upload the file to MARTe
        GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
        GCRTemplate<Message>         message(GCFT_Create);                
        message->Init(0, "ChangeConfigFile");
        
	/*GCNamedObject level1Sender;
        level1Sender.SetObjectName("LEVEL1");
        envelope->SetSender(level1Sender);*/

        //Insert the cdb
        ConfigurationDataBase level1CDB;
        cfgFile.Seek(0);
        level1CDB->ReadFromStream(cfgFile);
        if(rebuildAll.Size() > 0){
            FString value = "True";
            FString key   = "RebuildAll";
            level1CDB->MoveToRoot();
            CDBExtended cdbe(level1CDB);
            cdbe.WriteFString(value, key.Buffer());
        }        
        if(displayCDB.Size() > 0){
            hmStream.SSPrintf(HtmlTagStreamMode, "h2");
            hmStream.Printf("Going to upload the following CDB");
            hmStream.SSPrintf(HtmlTagStreamMode, "/h2");            
            level1CDB->WriteToStream(hmStream);
        }

        message->Insert(level1CDB);
        //Send the message
        if(waitReply.Size() > 0){
            envelope->PrepareMessageEnvelope(message, marteLocation.Buffer(), MDRF_ManualReply, this);
        }
        else{
            envelope->PrepareMessageEnvelope(message, marteLocation.Buffer());
        }

        GCRTemplate<MessageEnvelope>   reply;
        //prepare the reply
        if(waitReply.Size() > 0){
            SendMessageAndWait(envelope, reply, uploadTimeoutMSec);
            if(!reply.IsValid()){
                AssertErrorCondition(Warning, "Reply from %s isn't valid!", marteLocation.Buffer());
                hmStream.SSPrintf(HtmlTagStreamMode, "h1");
                hmStream.Printf("Reply from %s isn't valid!\n", marteLocation.Buffer());
                hmStream.SSPrintf(HtmlTagStreamMode, "/h1");
            }
            else{
                GCRTemplate<Message> replyMessage = reply->GetMessage();
                if(!replyMessage.IsValid()){
                    AssertErrorCondition(Warning, "The reply message from %s is not valid", marteLocation.Buffer());
                }
                else{
                    hmStream.SSPrintf(HtmlTagStreamMode, "h1");
                    hmStream.Printf("%s replied: %s", marteLocation.Buffer(), replyMessage->Content());
                    hmStream.SSPrintf(HtmlTagStreamMode, "/h1");
                }
            }
        }
        else{
            SendMessage(envelope);
            hmStream.SSPrintf(HtmlTagStreamMode, "h2");
            hmStream.Printf("File sent!");
            hmStream.SSPrintf(HtmlTagStreamMode, "/h2");
        }
        hStream.Printf("<a href=\".\">BACK</a><br>");
    }
    else{
        PrintHTTPForm(hmStream);
    }
    hmStream.SSPrintf(HtmlTagStreamMode, "/body");
    hmStream.SSPrintf(HtmlTagStreamMode, "/html");


    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.WriteReplyHeader(True);
    return True;
}
