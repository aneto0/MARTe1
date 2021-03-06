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
 * $Id: RelayLogger.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
#include <stdio.h>
#include "System.h"
#include "Sleep.h"
#include "GAMAdapter.h"
#include "Adapter.h"

const char *webConfig = ""
    "+WEB = {"
    "    Class = HttpGroupResource"
    "    +BROWSE = {"
    "        Title = \"Http Object Browser\""
    "        Class = HttpGCRCBrowser"
    "        AddReference = {OBJBROWSE THRBROWSE WebStatistic}"
    "    }"
    "}"
    "+HTTPSERVER = {"
    "    Class = HttpService"
    "    Port = 8084"
    "    HttpRelayURL = \"ignore.me:1234\""
    "    VerboseLevel = 10"
    "    Root = WEB"
    "}"
    "+OBJBROWSE = {"
    "    Class = HttpClassListResource"
    "}"
    "+THRBROWSE = {"
    "    Class = HttpThreadListResource"
    "}"
    "+Adapter = {"
    "    Class = BaseLib2Adapter::AdapterMessageHandler"
    "}"
    "+StateMachine = {"
    "    Class = StateMachine"
    "    +INITIAL = {"
    "        Class = StateMachineState"
    "        StateCode = 0x0"
    "        +START = {"
    "            Class = StateMachineEvent"
    "            NextState = IDLE"
    "            Value = START"
    "            +STARTALL = {"
    "                Class = MessageDeliveryRequest"
    "                Sender = StateMachine"
    "                Destinations = \"HTTPSERVER Adapter\""
    "                MsecTimeOut = 1000"
    "                Flags = NoReply"
    "                Message = {"
    "                    Class = Message"
    "                    Content = START"
    "                    +RemoteStateMachine = {"
    "                        Class = Message"
    "                        Content = Test"
    "                    }"
    "                }"
    "            }"
    "        }"
    "    }"
    "    +IDLE = {"
    "         Class = StateMachineState"
    "         StateCode = 0x500"
    "    }"
    "}";

const char *config = ""
    "+WebStatistic = {"
    "    Class = WebStatisticGAM"
    "    Signals = {"
    "        SignalU = {"
    "            SignalName = usecTime"
    "            SignalType = uint32"
    "        }"
    "    }"
    "}";


class AdapterMessageListenerTest : public BaseLib2::AdapterMessageListener {
public:
    bool HandleBaseLib2Message(const char *destination, const char *content, unsigned int code) {
        printf("Received message to %s with content %s and code %d\n", destination, content, code);
    }
};


int main(int argc, char **argv) {
    AdapterMessageListenerTest listener;
    BaseLib2::Adapter *adapter = BaseLib2::Adapter::Instance();
    adapter->SetAdapterMessageListener(&listener);
    bool ok = adapter->LoadObjects(webConfig);
    printf("ok = %d\n", ok);
    adapter->SendMessageToBaseLib2("StateMachine", "START", 0); 

    BaseLib2::uint32 idx;
    BaseLib2::GAMAdapter *gamAdapter = BaseLib2::GAMAdapter::Instance();
    ok = gamAdapter->AddGAM("WebStatistic", config, idx);
    printf("ok = %d idx = %d\n", ok, idx);
    ok = gamAdapter->AddGAMInputSignal(idx, "usecTime", "uint32");
    printf("ok = %d idx = %d\n", ok, idx);

    void *inputToGAM;
    void *outputFromGAM;
    ok = gamAdapter->FinaliseGAM(idx, inputToGAM, outputFromGAM);
    printf("ok = %d idx = %d %p %p\n", ok, idx, inputToGAM, outputFromGAM);
    BaseLib2::uint32 *test = (BaseLib2::uint32 *)inputToGAM;
    unsigned int i = 0;
    while(1) {
        *test = i++;
        gamAdapter->ExecuteGAM(idx, 0x00010000);
        SleepSec(0.1);
    } 
    return 0;
}


