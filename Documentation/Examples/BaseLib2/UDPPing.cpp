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
 * @file A UDPPing program. It expects an UDPPong to be replying.
 */
#include "ErrorManagement.h"
#include "FString.h"
#include "UDPSocket.h"

static bool run = true;
static bool serverReady = true;
static uint32 pongPort = 0;
static char *pongAddress = NULL;
static uint32 pingPort = 0;
static uint32 payloadSize = 0;
static uint32 microSecondPeriod = 0;
static uint32 numberOfCounts = 0;
static double meanDeltaT = 0;
static double minDeltaT = 100;
static double maxDeltaT = 0;

void ClientThread(void *ignore) {
    UDPSocket client;
    if(!client.Open()){
        CStaticAssertErrorCondition(FatalError, "Failed to open client");
        return;
    }
    if(!client.SetBlocking(True)){
        CStaticAssertErrorCondition(InitialisationError, "Failed to set UDP client as blocking");
        client.Close();
        return;
    }
    if(!client.Connect(pongAddress, pongPort)) {
        CStaticAssertErrorCondition(InitialisationError, "Failed to connect client to %s:%d", pongAddress, pongPort);
        client.Close();
        return;
    }
    char *mem = new char[payloadSize];
    uint64 *mem64 = (uint64 *)mem;
    uint64 periodCounts = microSecondPeriod * 1e-6 * HRT::HRTFrequency();
    uint64 now = HRT::HRTCounter();
    uint64 next = now + periodCounts;
    while (run) {
        uint32 readBytes = payloadSize;
        while(now < next) {
            now = HRT::HRTCounter();
        }
        next = now + periodCounts;
        mem64[0] = HRT::HRTCounter();
        client.Write(mem, readBytes);
    }
    client.Close();
    delete [] mem; 
}

void ServerThread(void *ignore){
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
    if(!server.Listen(pingPort)){
        CStaticAssertErrorCondition(InitialisationError, "Failed to set UDP server listening in port %d", pingPort);
        server.Close();
        return;
    }
    char *mem = new char[payloadSize];
    uint64 *mem64 = (uint64 *)mem;
    uint32 originalCounts = numberOfCounts;
    while (numberOfCounts > 0) {
        uint32 readBytes = payloadSize;
        serverReady = true;
        if(server.Read(mem, readBytes)) {
            uint64 deltaC = HRT::HRTCounter() - mem64[0];
            double deltaT = deltaC * HRT::HRTPeriod();
            meanDeltaT += deltaT;
            if (deltaT > maxDeltaT) {
                maxDeltaT = deltaT;
            }
            if (deltaT < minDeltaT) {
                minDeltaT = deltaT;
            }
        }
        numberOfCounts--;
    }
    server.Close();
    meanDeltaT *= 1e6;
    meanDeltaT /= originalCounts;
    minDeltaT *= 1e6;
    maxDeltaT *= 1e6;
    printf("mean = %e [us] max = %e [us] min = %e\n", meanDeltaT, maxDeltaT, minDeltaT);
    run = false;
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 
   
    if (argc != 9) {
        printf("Arguments are: affinitMaskClient affinitMaskServer pingPort pongAddress pongPort payloadSize numberOfCounts microSecondPeriod\n");
        return -1;
    } 
    uint32 affinityClient = atoi(argv[1]);
    uint32 affinityServer = atoi(argv[2]);
    pingPort = atoi(argv[3]);
    pongAddress = argv[4];
    pongPort = atoi(argv[5]);
    payloadSize = atoi(argv[6]);
    numberOfCounts = atoi(argv[7]);
    microSecondPeriod = atoi(argv[8]);
    Threads::BeginThread(&ServerThread, NULL, THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, affinityServer);
    while(!serverReady) {
        SleepSec(0.5);
    }
    Threads::BeginThread(&ClientThread, NULL, THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, affinityClient);
    while(run) {
        SleepSec(1.0);
    }    
    return 0;
}

