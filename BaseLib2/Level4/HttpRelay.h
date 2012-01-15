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

/** 
 * @file
 * Relay support 
 */
#if !defined (HTTP_RELAY_H)
#define HTTP_RELAY_H

#include "HttpClient.h"
#include "HttpInterface.h"
#include "URL.h"
#include "Threads.h"
#include "Sleep.h"
#include "CDBExtended.h"
#include "MutexSem.h"

void __thread_decl RelayThread(void *data);

OBJECT_DLL(HttpRelay)
class HttpRelay : public GCNamedObject, public HttpInterface {
OBJECT_DLL_STUFF(HttpRelay)

private:

    /** Flag to control connection configuration status */
    bool                    connectionConfigured;
    /** Thread id */
    TID                     tid;

public:
    /// These are public because they need
    /// to be used in the RelayThread

    /** Refresh period in miliseconds */
    int32                   updateContentPeriodMsec;
    /** Timeout for the Get operation */
    TimeoutType             msecTimeout;
    /** The url to be relayed */
    URL                     url;
    /** Flag to control thread status */
    bool                    running;
    /** String containing the html */
    FString                 html;
    /** The http client object */
    HttpClient              httpClient;
    /** The refresh period of the relayed web page */
    int32                   relayedPageRefreshPeriodSec;
    /** Critical section mutex sem */
    MutexSem                mux;

public:

    HttpRelay() {
        /** Default refresh period is one second */
        updateContentPeriodMsec     = 1000;
        /** Object not yet configured */
        connectionConfigured        = False;
        /** Thread not yet running */
        running                     = False;
        /** Default is no refreshing (zero)*/
        relayedPageRefreshPeriodSec = 0;
        /** Default msec timeout is 10 times the refresh period */
        msecTimeout.SetTimeOutSec(10.0*updateContentPeriodMsec/1000.0);
        /** Create the semaphore */
        mux.Create();
    };

    ~HttpRelay() {
        /** If the thread is running, stop it */
        if(running) {
            Stop();
        }
        /** Destroy semaphore */
        mux.Close();
    };

    /** Start relaying */
    bool Start();

    /** Terminate relaying */
    bool Stop();

    /** Configure object and Start relaying */
    bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** Set the url to be relayed */
    bool SetUrl(FString urlFullAddress);

    /** Process Http request */
    bool ProcessHttpMessage(HttpStream &hStream);
};
#endif

