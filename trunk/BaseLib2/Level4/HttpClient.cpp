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

#include "HttpClient.h"
#include "SXNull.h"
#include "MD5.h"
#include "Base64Codec.h"
#include "FastPollingMutexSem.h"


OBJECTREGISTER(HttpClient,"$Id: HttpClient.cpp,v 1.7 2008/12/09 11:25:12 aneto Exp $")

bool HttpClient::Connect(TimeoutType mSecTimeout){
    socket.Close();
    socket.Open();
    socket.SetBlocking(True);
    return socket.Connect(host.Buffer(),port,mSecTimeout);
}

static bool SearchKey(const char *key,const char *name,FString &value){
    value.SetSize(0);
    if (key == NULL) return False;
    if (name == NULL) return False;

    const char *p = strstr(key,name);
    if (p == NULL) return False;
    p+= strlen(name);
    if (p[0] != '=') return False;
    if (p[1] != '"') return False;
    p+=2;
    while(*p != '"') value += *p++;
    return True;
}

static bool SearchKeyNoQuote(const char *key,const char *name,FString &value){
    value.SetSize(0);
    if (key == NULL) return False;
    if (name == NULL) return False;

    FString namep;
    namep.Printf("%s=",name);
    const char *p = strstr(key,namep.Buffer());
    if (p == NULL) return False;
    p+= namep.Size();
    char buffer[64];
    buffer[0] = 0;
    if (!Streamable::GetCStringToken(p,buffer,",",64)) return False;
    value = buffer;
    return True;
}


void HttpClient::CalculateNonce(FString &nonce){
    nonce.SetSize(0);
    FString tid;
    tid.Printf("%08x%08x",Threads::ThreadId,this);
    unsigned char buffer[16];
    md5( (unsigned char *)tid.BufferReference(), tid.Size(),buffer);
    for (int  i=0;i<16;i++){
        nonce.Printf("%02x",buffer[i]);
    }
}


bool HttpClient::GenerateDigestKey( FString &key,
                                    const char *data,
                                    const char *uri,
                                    const char *command,
                                    int         ncNumber){
    key.SetSize(0);

    FString qop;
    qop.Printf("auth");
//    if (!SearchKeyNoQuote(data,"qop",qop)) return False;

    FString userPasswd;
    B64Decode(authorisation,userPasswd);

    FString user;
    FString passwd;

    userPasswd.Seek(0);
    userPasswd.GetToken(user,":");
    userPasswd.GetToken(passwd,"\n\t ");

    FString realm;
    if (!SearchKey(data,"realm",realm)) return False;

    FString HA1;
    {
        HA1.SetSize(0);
        unsigned char buffer[16];
        FString toEncode;
        toEncode.Printf("%s:%s:%s",user.Buffer(),realm.Buffer(),passwd.Buffer());
        md5( (unsigned char *)toEncode.BufferReference(), toEncode.Size(),buffer);
        for (int i=0;i<16;i++){
            HA1.Printf("%02x",buffer[i]);
        }
    }

    FString HA2;
    {
        unsigned char buffer[16];
        HA2.SetSize(0);
        FString toEncode;
        toEncode.Printf("%s:%s",command,uri);
        md5( (unsigned char *)toEncode.BufferReference(), toEncode.Size(),buffer);
        for (int i=0;i<16;i++){
            HA2.Printf("%02x",buffer[i]);
        }
    }

    FString response;
    FString nc;
    nc.Printf("%08x",ncNumber);
    FString cnonce;
    CalculateNonce(cnonce);
    FString nonce;
    if (!SearchKey(data,"nonce",nonce)) return False;
    {
        unsigned char buffer[16];

        FString toEncode;
        toEncode.SetSize(0);
        toEncode.Printf("%s:%s:%s:%s:%s:%s",
            HA1.Buffer(),nonce.Buffer(),nc.Buffer(),
            cnonce.Buffer(),qop.Buffer(),HA2.Buffer()
            );
        md5( (unsigned char *)toEncode.BufferReference(), toEncode.Size(),buffer);
        for (int i=0;i<16;i++){
            response.Printf("%02x",buffer[i]);
        }
    }

    FString opaque;
    if (!SearchKey(data,"opaque",opaque)) return False;

    key.Printf("Digest "
               "username=\"%s\","
               "realm=\"%s\","
               "nonce=\"%s\","
               "uri=\"%s\","
               "qop=%s,"
               "nc=%s,"
               "cnonce=\"%s\","
               "response=\"%s\","
               "opaque=\"%s\"",
                user.Buffer(),
                realm.Buffer(),
                nonce.Buffer(),
                uri,
                qop.Buffer(),
                nc.Buffer(),
                cnonce.Buffer(),
                response.Buffer(),
                opaque.Buffer());


    return True;


}

static FastPollingMutexSem fpms;

bool HttpClientGet(HttpClient &client, URL url, Streamable &s, TimeoutType msecTimeout, int operationId){
    // absolute time for timeout !
    int64 startCounter = HRT::HRTCounter();
    int64 maxTicks     = startCounter + msecTimeout.HRTTicks();

    // a counter of the transactions
    fpms.FastLock();
    if (operationId == -1){
        operationId = ++client.lastOperationId;
    }
    fpms.FastUnLock();

    if (url.Protocol() != URLP_HTTP){
        client.AssertErrorCondition(ParametersError,"Get:Only HTTP is supported\n");
        return False;
    }

    const char *uri = url.URI();
    const char *host = url.Server();
    uint32 port = url.Port();

    bool reConnect = !client.socket.IsConnected();

    if (host != NULL){
        if (!(client.host == host)){
            client.host = host;
            reConnect = True;
        }
    }
    if (port != 0){
        if (client.port != port){
            client.port = port;
            reConnect = True;
        }
    }

    /* connect to remote host */
    if (reConnect){
        if (!client.Connect(msecTimeout)){
            return False;
        }
    }

    /* create and send request */
    HttpStream hs(&client.socket);
    hs.keepAlive = True;
    if (client.authorisationKey.Size() > 0){
        hs.SSPrintf("OutputHttpOtions.Authorization", client.authorisationKey.Buffer());
    }
    hs.WriteHeader(True, HSHCGet, uri);

    /* read reply */
    if (!hs.ReadHeader()){
        return False;
    }

    if (msecTimeout.IsFinite()){
        int64 lastCounter = HRT::HRTCounter();
        int64 ticksLeft = maxTicks - lastCounter;
        if (ticksLeft < 0) {
            client.AssertErrorCondition(Timeout, "Get: Timeout on completion");
            return False;
        }
        msecTimeout.SetTimeOutHRTTicks(ticksLeft);
    }


    // check reply for 200 (authorisation request)
    if (hs.httpCommand == HSHCReplyOK){
        // read body
        return hs.CompleteReadOperation(&s,msecTimeout);
    }

    // a dump of useless data
    SXNull nullStream;
    if(&s == NULL){
        s = nullStream;
    }

    // check reply for 401 (authorisation request)
    if (hs.httpCommand == HSHCReplyAUTH) {
        // discard body
        if (!hs.CompleteReadOperation(&nullStream,msecTimeout)) return False;
        if (client.authorisation.Size() < 3){
            client.AssertErrorCondition(FatalError,"Get: authorisation information needed to access %s",uri);
            return False;
        }

        if (!hs.Switch("InputHttpOtions.WWW-Authenticate")) return False;
        FString authRequest;
        hs.Seek(0);
        hs.GetToken(authRequest,"\n\r");
        hs.Switch((uint32)0);

        FString newAuthorisationKey;
        // identify basic or digest and call appropriate function
        // to generate newAuthorisationKey
        if (strncasecmp("Basic ",authRequest.Buffer(),6)==0){
            newAuthorisationKey = client.authorisation;
        }  else
        if (strncasecmp("Digest ",authRequest.Buffer(),7)==0){
            client.GenerateDigestKey(newAuthorisationKey,authRequest.Buffer()+7,uri,"GET",operationId);
        }  else {
            client.AssertErrorCondition(FatalError,"Get: authorisation request of unknown type: %s",authRequest.Buffer());
            return False;
        }
        // avoid trying again with the same key
        if (newAuthorisationKey == client.authorisationKey){
            client.AssertErrorCondition(FatalError,"Get: authorisation provided is not good enough for %s",uri);
            return False;
        }
        client.authorisationKey = newAuthorisationKey;

        // close if the server says so...
        if (!hs.keepAlive) client.socket.Close();

        // try again
        return client.Get(url,s,msecTimeout,operationId);

    }

    // discard body
    bool ret = hs.CompleteReadOperation(&s,msecTimeout);

    // close if the server says so...
    if (!hs.keepAlive) client.socket.Close();
    return ret;
}



