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

#include "TCPMessageHandler.h"
#include "MessageDispatcher.h"

void ConnectionHandlerFn(TCPMessageHandler &tcpmh){
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

TCPMessageHandler::TCPMessageHandler(){
    serverPort = -1;
    serverTID  = 0;
    cpuMask    = 0;
    keepAlive  = False;
    msgTimeout = TTInfiniteWait;
}

TCPMessageHandler::~TCPMessageHandler(){
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

bool TCPMessageHandler::ConnectionHandler(){
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

void TCPMessageHandler::ClientHandler(TCPSocket *client){
    //Read a line from the client socket
    FString line;
    line.SetSize(0);
    while(client->GetLine(line)){
        HandleRequest(line);
        line.SetSize(0);
    }
}

bool TCPMessageHandler::HandleRequest(FString &req){
    FString destination;
    FString code;
    FString content;
    req.Seek(0);
    printf("%s\n", req.Buffer());
    if(!req.GetToken(destination, "|")){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Could not read the message destination", Name());
        return False;
    }
    if(!req.GetToken(code, "|")){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Could not read the message code", Name());
        return False;
    }
    if(!req.GetToken(content, "|")){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Could not read the message content", Name());
        return False;
    }

    AssertErrorCondition(Information, "%s: HandleRequest : D=%s,C=%s,CT=%s", Name(), destination.Buffer(), code.Buffer(), content.Buffer());
    GCRTemplate<Message> msg(GCFT_Create);
    if(!msg.IsValid()){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Failed to create message", Name());
        return False;
    }
    GCRTemplate<MessageEnvelope> env(GCFT_Create);
    if (!env.IsValid()){
        AssertErrorCondition(FatalError, "%s: HandleRequest: Failed to creating envelope", Name());
        return False;
    }
    msg->Init(atoi(code.Buffer()), content.Buffer());
    env->PrepareMessageEnvelope(msg, destination.Buffer(), MDRF_ManualReply, this);

    GCRTemplate<MessageEnvelope>   reply;
    MessageHandler::SendMessageAndWait(env, reply, msgTimeout);
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

bool TCPMessageHandler::ObjectLoadSetup(ConfigurationDataBase &cdb, StreamInterface *err){
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
    keepAlive = True;
    serverTID = Threads::BeginThread((void (__thread_decl *)(void *))&ConnectionHandlerFn,this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
    return True;
}

OBJECTLOADREGISTER(TCPMessageHandler, "$Id: TCPMessageHandler.cpp,v 1.2 2011/12/07 13:55:43 aneto Exp $")

