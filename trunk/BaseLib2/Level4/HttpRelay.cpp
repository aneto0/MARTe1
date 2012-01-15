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

#include "HttpRelay.h"

void __thread_decl RelayThread(void *data) {
    
    HttpRelay *httpRelayPtr = (HttpRelay *)data;
    
    while(httpRelayPtr->running) {
        httpRelayPtr->html.SetSize(0);
        httpRelayPtr->mux.Lock();
        if(!httpRelayPtr->httpClient.Get(httpRelayPtr->url, httpRelayPtr->html, httpRelayPtr->msecTimeout)) {
            CStaticAssertErrorCondition(FatalError, "HttpRelay: %s: Get() failed", httpRelayPtr->Name());
        }
        httpRelayPtr->mux.UnLock();
        SleepMsec(httpRelayPtr->updateContentPeriodMsec);
    }
};

bool HttpRelay::Start() {
    if(!connectionConfigured) {
        AssertErrorCondition(FatalError, "HttpRelay: %s: Start(): Connection not yet configured", Name());
        return False;
    }

    if(running) {
        AssertErrorCondition(FatalError, "HttpRelay: %s: Start(): Thread should already be running", Name());
        return False;
    } else {
        if((tid = Threads::BeginThread(RelayThread, this, THREADS_DEFAULT_STACKSIZE, "RelayThread")) == (TID)(-1)) {
            AssertErrorCondition(FatalError, "HttpRelay: %s: Start(): Unable to start RelayThread", Name());
            running = False;
        } else {
            running = True;
        }
    }

    return running;
};

bool HttpRelay::Stop() {
    running = False;
    SleepMsec(updateContentPeriodMsec);
    if(Threads::IsAlive(tid)) {
        if(Threads::Kill(tid)) {
            tid = (TID)(-1);
            return True;
        } else {
            AssertErrorCondition(FatalError, "HttpRelay: %s: Stop(): Failed to kill thread", Name());
            return False;
        }
    } else {
        tid = (TID)(-1);
        return True;
    }
};

bool HttpRelay::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err) {
    CDBExtended cdb(cdbData);

    if(!GCNamedObject::ObjectLoadSetup(cdb, NULL)) {
        AssertErrorCondition(InitialisationError, "HttpRelay: %s: ObjectLoadSetup(): Failed on parent's ObjectLoadSetup", Name());
        return False;
    }

    FString urlFullAddress;
    if(!cdb.ReadFString(urlFullAddress, "URL")) {
        AssertErrorCondition(InitialisationError, "HttpRelay: %s: ObjectLoadSetup(): URL not specified", Name());
        return False;
    }

    if(!SetUrl(urlFullAddress)) {
        AssertErrorCondition(InitialisationError, "HttpRelay: %s: ObjectLoadSetup(): SetUrl() failed", Name());
        return False;
    }

    /** Default is to update the relayed content every second */
    cdb.ReadInt32(updateContentPeriodMsec, "UpdateContentPeriodMsec", 1000);
    /** Default is to have no refreshing in the relayed web page */
    cdb.ReadInt32(relayedPageRefreshPeriodSec, "RelayedPageRefreshPeriodSec", 0);

    if(Start()) {
        return True;
    } else {
        AssertErrorCondition(InitialisationError, "HttpRelay: %s: ObjectLoadSetup(): Unable to Start()", Name());
        return False;
    }
};

bool HttpRelay::SetUrl(FString urlFullAddress) {
    if(!url.Load(urlFullAddress)) {
        AssertErrorCondition(FatalError, "HttpRelay: %s: SetUrl(): unable to load url", Name());
        connectionConfigured = False;
    } else {
        connectionConfigured = True;
    }
    return connectionConfigured;
};

bool HttpRelay::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);
    if(relayedPageRefreshPeriodSec != 0) {
        hStream.Printf("<META HTTP-EQUIV=\"REFRESH\" CONTENT=\"%d\">\n", relayedPageRefreshPeriodSec);
    }
    
    mux.Lock();
    hStream.Printf("%s", html.Buffer());
    mux.UnLock();

    return True;
};

OBJECTLOADREGISTER(HttpRelay, "$Id$")
