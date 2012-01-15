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

#include "HttpBasicService.h"
#include "InternetAddress.h"
#include "BString.h"
#include "CDBExtended.h"
#include "HttpStream.h"
#include "HttpRealm.h"
#include "HttpInterface.h"
#include "HttpClient.h"
#include "Processes.h"


class HSSearchFilter: public SearchFilterT<GCReference>{
    /** the path name to be found */
    FString                     name;

    /** pointer within name */
    uint32                      nameIndex;

    /** the last realm found during the search the process */
    GCRTemplate<HttpRealm>      realm;

public:

    HSSearchFilter(const char *name){
        this->name = name;
        nameIndex = 0;
    }

    virtual ~HSSearchFilter(){};


    uint32       NameIndex(){
        return nameIndex;
    }

    virtual bool Test(GCReference data){

        GCRTemplate<GCNamedObject> gcno;
        gcno = data;

        if (!gcno.IsValid()) return False;

        int len = strlen(gcno->Name());
        if (strncmp(gcno->Name(),name.Buffer()+ nameIndex,len)==0){
            char terminator = name.Buffer()[nameIndex + len];
            // partial match
            if (terminator == '.') {
                nameIndex += (len+1);
                return True;
            }

            // full match
            if (terminator == 0) {
                nameIndex += len;
                return True;
            }
        }
        return False;
    }

    virtual SFTestType Test2 (GCReference data,SFTestType mode){
        if (!data.IsValid()) return SFTTWrongPath;

        if (mode == SFTTBack){

            GCRTemplate<HttpInterface> gch;
            gch = data;
            if (Test(gch)) return SFTTFound;

            // move nameIndex back
            if (nameIndex > 0){
                nameIndex--;
                if (nameIndex > 0){
                    nameIndex--;
                    while ((nameIndex > 0) && (name[nameIndex-1] != '.')){
                        nameIndex--;
                    }
                }
            }

            // not found, just back-tracking
            return SFTTNull;
        }

        if (!Test(data)) return SFTTWrongPath;

        // whether we have completed the search
        bool fullMatch = (nameIndex == name.Size());

        // if has HttpInterface, grab the associated realm
        // so that it will be inherited by the subfolders
        GCRTemplate<HttpInterface> gch;
        gch = data;
        if (gch.IsValid()) {
            if ((gch->Realm()).IsValid()){
                realm = gch->Realm();
            }
            if (fullMatch){
                return SFTTFound;
            }

            // a simple partial match
            GCRTemplate<GCReferenceContainer> gcrc;
            gcrc = data;
            if (!gcrc.IsValid()) return SFTTFound;

        }

        // search the subnodes before coming back here
        return SFTTNotFound;

    };

    GCRTemplate<HttpRealm> GetRealm(){
        return realm;
    }

};

MCCExitType HSUserServiceThreadEx(HttpBasicService &hs,MultiClientClassConnectionInfo *info){
    TCPSocket  *clientSocket = ((HttpBasicServiceConnectionInfo *)info)->socket;
    if (clientSocket == NULL) return MCCTerminate;

    clientSocket->SetBlocking(True);
    HttpStream hstream(clientSocket);

    while(1){
        SocketSelect sel;
        sel.AddWaitOnReadReady(clientSocket);//TODO ADD EXCEPT
        bool ret = sel.WaitRead(1000);
        if (ret == False) {
            return MCCContinue;
        }

        if (!hstream.ReadHeader()){
            hs.AssertErrorCondition(CommunicationError,"HSUserServiceThread:Error while reading http header\n");
            clientSocket->Close();
            break;
        }

        if (hs.verboseLevel >=10){
            hs.AssertErrorCondition(Information,"Processing request [%s]",hstream.path.Buffer());
        }

        GCRTemplate<HttpInterface>hi;
        GCRTemplate<HttpRealm> realm;

        if (hstream.path.Size()>0){
            // search for destination
            HSSearchFilter searchFilter(hstream.path.Buffer());
            hi = hs.webRoot->Find(&searchFilter,GCFT_Recurse);
            realm = searchFilter.GetRealm();

            // save remainder of address
            hstream.unMatchedUrl = hstream.url.Buffer()+searchFilter.NameIndex();
            if (hstream.unMatchedUrl.Buffer()[hstream.unMatchedUrl.Size()-1] == '/') {
                hstream.unMatchedUrl.SetSize(hstream.unMatchedUrl.Size()-1);
            }
        }
        if (!hi.IsValid()){
            hi = hs.webRoot;
            if (hi.IsValid()) {
                if ((hi->Realm()).IsValid()){
                    realm = hi->Realm();
                }
            }
        }

        bool pagePrepared = False;
        if (hi.IsValid()){
            // check security
//            GCRTemplate<HttpRealm> realm = searchFilter.GetRealm();
            if (realm.IsValid()){
                if (!hstream.SecurityCheck(realm,clientSocket->Source().HostNumber())){
                    hstream.Printf(
                             "<HTML><HEAD>\n"
                             "<TITLE>401 Authorization Required</TITLE>\n"
                             "</HEAD><BODY>\n"
                             "<H1>Authorization Required</H1>\n"
                             "This server could not verify that you\n"
                             "are authorized to access the document you\n"
                             "requested.  Either you supplied the wrong\n"
                             "credentials (e.g., bad password), or your\n"
                             "browser doesn't understand how to supply\n"
                             "the credentials required.<P>\n"
                             "</BODY></HTML>\n"
                    );
                    hstream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
                    FString realmMsg;
                    realm->GetAuthenticationRequest(realmMsg);
                    hstream.SSPrintf("OutputHttpOtions.WWW-Authenticate",realmMsg.Buffer());

                    // force reissuing of a new thread
                    hstream.keepAlive = False;
                    if (!hstream.WriteReplyHeader(True,401)){
                        hs.AssertErrorCondition(CommunicationError,"HSUserServiceThread:Error while writing page back\n");
                        clientSocket->Close();
                        break;
                    }
                    pagePrepared = True;
                }
            }

            if (!pagePrepared){
                pagePrepared = hi->ProcessHttpMessage(hstream);
            }
        }

        if (!pagePrepared){
            hstream.Printf("<HTML>Page Not Found!</HTML>");
            hstream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
            if (!hstream.keepAlive){
                hstream.SSPrintf("OutputHttpOtions.Connection","Close");
            }
            if (!hstream.WriteReplyHeader(True,200)){
                hs.AssertErrorCondition(CommunicationError,"HSUserServiceThread:Error while writing page back\n");
                clientSocket->Close();
                break;
            }
        }

        if (!hstream.keepAlive) break;
    }

    clientSocket->Close();

    return MCCTerminate;
}

bool HSUserListenerInitialize(HttpBasicService &hs){

    if (hs.verboseLevel >=2){
        hs.AssertErrorCondition(Information,"Server starting");
    }
    if (hs.server.Open() == False) return False;
    hs.server.SetBlocking(False);

    /* try use the HttpServiceRelay to obtain a port */
    HttpClient hc;
    URL url;
    FString s;

#if defined (_VXWORKS) || defined (_RTAI)
    // thread based systems   // still does not make much sense since thread is recycled...
    s.Printf("%s/?HttpServiceName=%s&TaskId=%i",hs.httpRelayURL.Buffer(),Threads::GetName(),Threads::ThreadId());
#else
    // process based systems
    s.Printf("%s/?HttpServiceName=%s&TaskId=%i",hs.httpRelayURL.Buffer(),Processes::ProcessName(),Processes::GetThisPid());
#endif
//    url.Init(httpRelayServer.Buffer(),s.Buffer(),8080);
//    hs.httpRelayURL.Seek(0);
//    url.Load(hs.httpRelayURL);
    s.Seek(0);
    url.Load(s);
    s.SetSize(0);

    if (!hc.Get(url,s,10000)) {
        hs.AssertErrorCondition(Information,"Cannot connect to HttpServiceRelay, using configured port %i!",hs.port);
    } else {
        s.Seek(0);
        int oldPort = hs.port;
        hs.port = atoi(s.Buffer());
        hs.AssertErrorCondition(Information,"port changed from %i to %i as instructed by HttpServiceRelay ",oldPort,hs.port );
    }

    return hs.server.Listen(hs.port,255);
}

void HSUserListenerTerminate(HttpBasicService &hs){
    hs.server.Close();
    if (hs.verboseLevel >=2){
        hs.AssertErrorCondition(Information,"Server stopping");
    }
}

bool HSUserListenerThread(HttpBasicService &hs,MultiClientClassConnectionInfo *&info){
    HttpBasicServiceConnectionInfo *sinfo = new HttpBasicServiceConnectionInfo;

    sinfo->socket = hs.server.WaitConnection(1000);
    if (sinfo->socket == NULL) {
        delete sinfo;
        sinfo = NULL;
        return False;
    }
    if (hs.verboseLevel >=5){
        BString hostname;
#if defined(_VX5500)
        hs.AssertErrorCondition(Information,"HSUserListenerThread:accepted connection from a host");
#else
        hs.AssertErrorCondition(Information,"HSUserListenerThread:accepted connection from %s",sinfo->socket->Source().HostName(hostname));
#endif
    }

    info = sinfo;
    return True;
};

bool HSObjectLoadSetup(
            HttpBasicService &          hs,
            ConfigurationDataBase &     info,
            StreamInterface *           err){

    bool ret = hs.GCNamedObject::ObjectLoadSetup(info, err);
    CDBExtended &cdbx = (CDBExtended &)info;

    int32 port;
    if (!cdbx.ReadInt32(port,"Port",80)){
        hs.AssertErrorCondition(Information,"ObjectLoadSetup:using default port 80");
    }
    hs.port = port;

    cdbx.ReadFString(hs.httpRelayURL,"HttpRelayURL",hs.httpRelayURL.Buffer());
    hs.AssertErrorCondition(Information,"ObjectLoadSetup:using default HttpRelayURL %s",hs.httpRelayURL.Buffer());

    if (!cdbx.ReadInt32(hs.verboseLevel,"VerboseLevel",0)){
        hs.AssertErrorCondition(Information,"ObjectLoadSetup:using default verboseLevel 0");
    }

    FString root;
    if (!cdbx.ReadFString(root,"Root","")){
        hs.AssertErrorCondition(Information,"ObjectLoadSetup:mapping web to root of GlobalObjectDataBase");
    } else {
        hs.webRoot = GODBFindByName(root.Buffer());
        if (!hs.webRoot.IsValid()){
            hs.AssertErrorCondition(ParametersError,"ObjectLoadSetup:cannot find root directory %s",root.Buffer());
            return False;
        }
    }

    if (!hs.webRoot.IsValid()){
        hs.webRoot = GetGlobalObjectDataBase();
    }

    return ret;
}

OBJECTREGISTER(HttpBasicService,"$Id: HttpBasicService.cpp,v 1.12 2011/11/25 11:41:13 aneto Exp $")


