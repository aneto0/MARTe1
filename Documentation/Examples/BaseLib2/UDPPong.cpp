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
 * $Id: SimpleTCPServer.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 * @file A reply to the UDPPing program
 */
#include "ErrorManagement.h"
#include "FString.h"
#include "UDPSocket.h"

static bool run = true;
static uint32 pongPort = 0;
static uint32 pingPort = 0;
static char *pingAddress = NULL;
static uint32 payloadSize = 0;

void ServerThread(void *threadID){
    UDPSocket server;
    if(!server.Open()){
        CStaticAssertErrorCondition(FatalError, "Failed to open server");
        return;
    }
    if(!server.SetBlocking(True)){
        CStaticAssertErrorCondition(InitialisationError, "Failed to set UDP server as blocking");
        server.Close();
        return;
    }
    if(!server.Listen(pongPort)){
        CStaticAssertErrorCondition(InitialisationError, "Failed to set UDP server listening in port %d", pongPort);
        server.Close();
        return;
    }
    UDPSocket client;
    if(!client.Open()){
        CStaticAssertErrorCondition(FatalError, "Failed to open client");
        server.Close();
        return;
    }
    if(!client.SetBlocking(True)){
        CStaticAssertErrorCondition(InitialisationError, "Failed to set UDP client as blocking");
        client.Close();
        server.Close();
        return;
    }
    if(!client.Connect(pingAddress, pingPort)) {
        CStaticAssertErrorCondition(InitialisationError, "Failed to connect client to %s:%d", pingAddress, pingPort);
        client.Close();
        server.Close();
        return;
    }
    char *mem = new char[payloadSize];
    while (run) {
        uint32 readBytes = payloadSize;
        if(server.Read(mem, readBytes)) {
            client.Write(mem, readBytes);
        }
    }
    client.Close();
    server.Close();
    delete [] mem;
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 
   
    if (argc != 6) {
        printf("Arguments are: affinitMask pongPort pingAddress pingPort payloadSize\n");
        return -1;
    } 
    uint32 affinity = atoi(argv[1]);
    pongPort = atoi(argv[2]);
    pingAddress = argv[3];
    pingPort = atoi(argv[4]);
    payloadSize = atoi(argv[5]);
    Threads::BeginThread(&ServerThread, NULL, THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, affinity);
    while(run) {
        SleepSec(1.0);
    }    
    return 0;
}

