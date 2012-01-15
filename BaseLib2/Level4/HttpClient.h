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
 * Client class for the HTTP protocol
 */

#if !defined (HTTP_CLIENT_H)
#define HTTP_CLIENT_H

#include "System.h"
#include "HttpStream.h"
#include "GCNamedObject.h"
#include "TCPSocket.h"
#include "SocketTimer.h"
#include "URL.h"

class HttpClient;

/** The status a Http Client is*/
enum HCStatus{

    /** The initial status : not connected */
    HCSNotConnected = 0
};

extern "C" {
    bool HttpClientGet(HttpClient &client, URL url, Streamable &s, TimeoutType tt, int op);
}

OBJECT_DLL(HttpClient)
class HttpClient: public GCNamedObject{
    // stuff need for all the classes derived from ErrorSystem
    OBJECT_DLL_STUFF(HttpClient)

protected:
//    /** */
//    HCStatus        status;

    /** the socket being used in the connection */
    TCPSocket       socket;

    /** the host at which we are connected to */
    FString         host;

    /** the host port at which we are connected */
    int32           port;

    /** authorisation is <BASE64 of User:Password> */
    FString         authorisation;

    /** digest or basic cooked user:password*/
    FString         authorisationKey;

    /** a number counting the number of operations performed */
    int             lastOperationId;

private:
    /** connect to whatever specified server and port */
    bool                Connect(
                            TimeoutType     msecTimeout);

    /** */
    void                CalculateNonce(
                            FString &       nonce);

    /** */
    bool                GenerateDigestKey(
                            FString &       key,
                            const char *    data,
                            const char *    uri,
                            const char *    command,
                            int             nc);

public:
    /** initialise variables
        @param authorisation is <BASE64 of User:Password>    */
                        HttpClient(
                            const char *    authorisation   = NULL)
    {
        port                = 0;
        host                = "";
        this->authorisation = authorisation;
        lastOperationId     = 0;
    }

    /** implements a GET operation
        @param connectionId do not uses!! */
    bool                Get(
                            URL             url,
//                            const char *    uri,
                            Streamable &    s,
                            TimeoutType     msecTimeout,
//                            const char *    host            =   NULL,
//                            uint32          port            =   0,
                            int             operationId     =   -1){
        return HttpClientGet(*this, url, s, msecTimeout, operationId);
    }

private:
    friend bool HttpClientGet(HttpClient &client, URL url, Streamable &s, TimeoutType tt, int operationId);

};


#endif
