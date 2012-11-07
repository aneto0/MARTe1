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
 * $Id: MessageTriggeringTimeService.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

#include "TCPConfigurationHandler.h"
#include "MessageDispatcher.h"

void ConnectionHandlerFn(TCPConfigurationHandler &tcpmh){
    while(tcpmh.keepAlive){
        tcpmh.ConnectionHandler();
        CStaticAssertErrorCondition(FatalError, "Lost server connection");
        if(tcpmh.keepAlive){
            CStaticAssertErrorCondition(FatalError, "Retrying in 10 seconds");
            SleepSec(10.0);
        }
    }
    //Just to signal that we have shutdown
    tcpmh.keepAlive = True;
}

TCPConfigurationHandler::TCPConfigurationHandler(){
    serverPort = -1;
    serverTID  = 0;
    cpuMask    = 0;
    keepAlive  = False;
    msgTimeout = TTInfiniteWait;
}

TCPConfigurationHandler::~TCPConfigurationHandler(){
    keepAlive = False;
    //Open a connection to the server to force the shutdown
    FString   host = "localhost";
    TCPSocket client;
    //Open the socket
    if(!client.Open()){
        AssertErrorCondition(FatalError, "%s: failed to shutdown server. Waited for 1 second.", Name());
        return;
    }
    //Connect to the server
    if(!client.Connect(host.Buffer(), serverPort)){
        CStaticAssertErrorCondition(FatalError, "%s: Failed to connect to %s:%d", Name(), host.Buffer(), serverPort);
        client.Close();
    }
    //Write a line
    FString line = "";
    uint32  size = line.Size();
    if(!client.Write(line.Buffer(), size)){
        CStaticAssertErrorCondition(FatalError, "Failed to write to socket");
    }
    //Housekeeping
    client.Close();
    int32 exitCounter = 0;
    while(!keepAlive){
        exitCounter++;
        SleepMsec(10);
        if(exitCounter > 100){
            AssertErrorCondition(FatalError, "%s: failed to shutdown server. Waited for 1 second.", Name());
            break;
        }
    }
    if(exitCounter > 100){
        Threads::Kill(serverTID);
    }
    serverTID = 0;

}

bool TCPConfigurationHandler::ConnectionHandler(){
    //Open the server connection
    if(!server.Open()){
        AssertErrorCondition(FatalError, "%s: Failed to open the server socket", Name());
        return False;
    }
    //Set in server mode
    if(!server.Listen(serverPort)){
        CStaticAssertErrorCondition(FatalError, "Failed to create server running in port %d", serverPort);
        server.Close();
        return False;
    }
    //Wait for a connection
    TCPSocket *client = NULL;

    while(keepAlive){
        client = server.WaitConnection();
        if(client == NULL){
            AssertErrorCondition(FatalError, "%s: Failed waiting for a connection in port %d", serverPort, Name());
            server.Close();
            return False;
        }
        //Set the client in blocking mode for the read
        client->SetBlocking(True);
        //Print information from the client
        FString hostname;
        client->Source().HostName(hostname);
        AssertErrorCondition(Information, "%s: Accepted a connection from %s", Name(), hostname.Buffer());
        ClientHandler(client);
    }

    server.Close();
    return True;
}

void TCPConfigurationHandler::ClientHandler(TCPSocket *client){
    FString cfg;
    cfg.SetSize(0);
    //Read a line from the client socket
    FString line;
    line.SetSize(0);
    while(client->GetLine(line)){
        cfg += line;
        cfg += "\n";
        line.SetSize(0);
    }
    HandleRequest(cfg);
    client->Close();
}

bool TCPConfigurationHandler::HandleRequest(FString &req){
    GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
    GCRTemplate<Message>         message(GCFT_Create);                
    GCRTemplate<MessageEnvelope> reply;

    req.Seek(0);
    ConfigurationDataBase msgCDB;
    if(!msgCDB->ReadFromStream(req)){
        AssertErrorCondition(FatalError, "%s::HandleRequest: Failed to parse configuration request", Name());
        return False;
    }
    msgCDB->MoveToRoot();
    message->Init(0, "ChangeConfigFile");
    message->Insert(msgCDB);
    envelope->PrepareMessageEnvelope(message, MARTeLocation.Buffer(), MDRF_ManualReply, this);
    SendMessageAndWait(envelope, reply, msgTimeout);
    if(!reply.IsValid()){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Received an invalid reply", Name());
        return False;
    }
    GCRTemplate<Message> replyMessage = reply->GetMessage(); 
    if(!replyMessage.IsValid()){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Received an invalid reply message", Name());
        return False;
    }
    
    AssertErrorCondition(Information, "%s: HandleRequest: Received a reply with code=%d and content=%s", Name(), replyMessage->GetMessageCode(), replyMessage->Content());
    return True;
}

bool TCPConfigurationHandler::ObjectLoadSetup(ConfigurationDataBase &cdb, StreamInterface *err){
    if(!GCNamedObject::ObjectLoadSetup(cdb, err)){
        AssertErrorCondition(FatalError, "%s::ObjectLoadSetup: ObjectLoadSetup of GCNamedObject failed", Name());
        return False;
    }
    
    CDBExtended cdbe(cdb);
    if(!cdbe.ReadInt32(serverPort, "ServerPort")){
        AssertErrorCondition(FatalError, "%s::ObjectLoadSetup: PulseNumberMessageCode is compulsory when PulseNumberMessageDestinations are set", Name());
    }
    
    //CPU mask for the thread
    if(!cdbe.ReadInt32(cpuMask, "CPUMask", 0x1)){
        AssertErrorCondition(Warning, "%s::ObjectLoadSetup: CPUMask was not specified. Using default: %d", Name(), cpuMask);
    }

    //The msg send timeout, default is infinite
    int32 msgTimeoutMS = 0;
    if(cdbe.ReadInt32(msgTimeoutMS, "MSGTimeout")){
        msgTimeout = msgTimeoutMS;
    }

    //The location of the MARTe object
    if(!cdbe.ReadFString(MARTeLocation, "MARTeLocation")){
        AssertErrorCondition(InitialisationError, "%s::ObjectLoadSetup MARTeLocation was not specified", Name());
        return False;
    }

    keepAlive = True;
    serverTID = Threads::BeginThread((void (__thread_decl *)(void *))&ConnectionHandlerFn,this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
    return True;
}

OBJECTLOADREGISTER(TCPConfigurationHandler, "$Id: TCPConfigurationHandler.cpp,v 1.2 2011/12/07 13:55:43 aneto Exp $")

