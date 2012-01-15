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
#include "UDPLoggerRelay.h"
#include "Sleep.h"
#include "SXMemory.h"
#include "FString.h"

OBJECTLOADREGISTER(UDPLoggerRelay, "$Id: UDPLoggerRelay.cpp,v 1.5 2008/04/15 17:30:29 aneto Exp $")

void UDPLoggerRelay::AddMessageToHistoryQueue(GCRTemplate<LoggerMessage> loggerMsg){ 
    while(historyMessageQ.Size() > maxHistoryQueueMessages){        
        historyMessageQ.Remove(0);
    }
    
    historyMessageQ.Add(loggerMsg);
}

/** This function will be called whenever there is a new message to process available*/
void UDPLoggerRelay::ProcessMessage(GCRTemplate<LoggerMessage> loggerMsg){       
    FString temp;
    uint32  size;
    loggerMsg->EncodeToText(temp);
    size = temp.Size();
    for(int i=0; i<Size(); i++){
        GCRTemplate<UDPRelayConnection> logRelay = Find(i);
        if(logRelay.IsValid()){
            logRelay->GetSocket().Write(temp.Buffer(), size);
        }
    }
    AddMessageToHistoryQueue(loggerMsg);    
}

/** Thread responsible for checking the remote sockets*/
void UDPLoggerRelayManager(void *args){
    UDPLoggerRelay *udpLoggerRelay = (UDPLoggerRelay *)args;
    while(udpLoggerRelay->IsAlive()){
        SleepSec((float)udpLoggerRelay->maxNotPingTimeSecs);    
        uint32 cTime = time(NULL);
        if(!udpLoggerRelay->Lock()){
            udpLoggerRelay->AssertErrorCondition(Warning, "UDPLoggerRelayManager Lock failed!");
        }
        for(int i=0; i<udpLoggerRelay->Size(); i++){
            GCRTemplate<UDPRelayConnection> logRelay = udpLoggerRelay->Find(i);
            if((cTime - logRelay->GetLastAccessTime()) > udpLoggerRelay->maxNotPingTimeSecs){                
                udpLoggerRelay->AssertErrorCondition(Warning, "Going to close socket: %s because no ping requests were received on the last %d seconds", logRelay->Name(), udpLoggerRelay->maxNotPingTimeSecs);
                //Deconstructor should close socket
                udpLoggerRelay->Remove(i);                
            }
        }
        if(!udpLoggerRelay->UnLock()){
            udpLoggerRelay->AssertErrorCondition(Warning, "UDPLoggerRelayManager Lock failed!");
        }
    }
}

/** Thread responsible for managing the remote sockets*/
void UDPLoggerRelayServer(void *args){
    UDPLoggerRelay *udpLoggerRelay = (UDPLoggerRelay *)args;
    UDPSocket serverSocket;    
    if(!serverSocket.Open()){
        udpLoggerRelay->AssertErrorCondition(FatalError, "Cannot Open UDP server\n");
        return;
    }

    if(!serverSocket.Listen(udpLoggerRelay->relayServerPort, 255)){
        udpLoggerRelay->AssertErrorCondition(FatalError, "UDP server Cannot Listen in port %d\n", udpLoggerRelay->relayServerPort);
        return;
    }

    if(!serverSocket.SetBlocking(True)){
        udpLoggerRelay->AssertErrorCondition(FatalError, "UDP server Cannot Set blocking mode\n");
        return;
    }
    
    char    buffer[1024];
    uint32  read = 1024;
    int     port = 0;    
    int     historyN = 0;
    int     stayConnected = 0;
    FString token;
    BString hostName;
    
    while(udpLoggerRelay->IsAlive()){
        read = 1024;
        memset(buffer, 0, read);
        if(serverSocket.Read(buffer, read)){
            if(buffer == ""){
                continue;
            }
            port = -1;
            historyN = -1;
            token.SetSize(0);
            
            FString bufferStr;
            SXMemory sxm(buffer, read);
            while(sxm.GetToken(token,"|", NULL, NULL)){
                if(port == -1){
                    port = atoi(token.Buffer());
                }
                else if(port != -1 && historyN == -1){
                    historyN = atoi(token.Buffer());
                }
                else if(port != -1 && historyN != -1){
                    stayConnected = atoi(token.Buffer());
                }
                token.SetSize(0);
            }            
            if(port == -1){
                continue;
            }
            if(historyN < 0){
                stayConnected = 1;
            }
            
            serverSocket.Source().HostName(hostName);
            if(stayConnected == 1){                
                UDPLogRelayFilter filter(hostName.Buffer(), port);
                GCRTemplate<UDPRelayConnection> relayConn = udpLoggerRelay->Find(&filter);
                if(relayConn.IsValid()){
                    relayConn->Ping();
                }
                else{
                    GCRTemplate<UDPRelayConnection> relayConn(GCFT_Create);
                    relayConn->Init(hostName.Buffer(), port);
                    udpLoggerRelay->Lock();
                    udpLoggerRelay->Insert(relayConn);
                    udpLoggerRelay->UnLock();                
                }
            }
            
            if(historyN > 0){                                
                HistoryRequest *historyRequest = new HistoryRequest(&udpLoggerRelay->historyMessageQ, hostName.Buffer(), port, historyN);
                Threads::BeginThread(UDPLoggerRelayHistoryBroadcast, historyRequest);
            }
        }
    }
    
    serverSocket.Close();
    udpLoggerRelay->alive = True;
}

/** Thread responsible for sending the history to a specific destination socket*/
void UDPLoggerRelayHistoryBroadcast(void *args){
    HistoryRequest *historyRequest = (HistoryRequest *)args;
    uint32 historySize = historyRequest->historyMessageQ->Size();
    FString temp;
    uint32  size;
    UDPSocket socket;
    socket.Open();
    socket.Connect(historyRequest->remoteAddress, historyRequest->port);
    for(int i=0; i<historyRequest->numberOfMessages && i < historySize; i++){        
        GCRTemplate<LoggerMessage> loggerMsg = historyRequest->historyMessageQ->Peek(i);
        if(loggerMsg.IsValid()){
            temp.SetSize(0);
            loggerMsg->EncodeToText(temp);
            size = temp.Size();
            socket.Write(temp.Buffer(), size);
        }
        
	SleepMsec(5);
        //Should I sleep in between... may be the creation of the packets
        //is slow enough...
    }
    
    socket.Close();
        
    delete historyRequest;
}
