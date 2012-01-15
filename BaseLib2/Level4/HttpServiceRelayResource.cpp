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

#include "HttpServiceRelayResource.h"
#include "HttpUrl.h"
#include "Processes.h"
#include "BasicTCPSocket.h"

class HSRRItem: public HttpUrl{
private:
    int32   taskId;
    int32   port;
    BString server;

public:
    HSRRItem()
    {
        taskId = 0;
        port = 0;
    }

    void Setup(int32 taskId,const char *serviceName,const char *server,int32 port)
    {
        SetSize(0);
        Printf("http://%s:%i/",server,port);
        FString name;
        name.Printf("%s(0x%08x@%s)",serviceName,taskId,server);
        SetObjectName(name.Buffer());
        this->taskId = taskId;
        this->port = port;
        this->server = server;
    }

    bool PortIsFree()
    {
        BasicTCPSocket test;
        test.Open();
//        test.SetBlocking(True);
        AssertErrorCondition(Information,"PortIsFree: Checking port %i at %s\n",Port() ,Server());

        bool ret = test.Connect(Server(),Port());
        test.Close();
        return !ret;
//        return !Processes::IsAlive(taskId);
    }

    int32 TaskId()
    {
        return taskId;
    }

    int32 Port()
    {
        return port;
    }

    const char *Server()
    {
        return server.Buffer();
    }
};

class HSRRSearchFilter: public SearchFilterT<GCReference>{

    int32 port;

    FString server;

public:

    HSRRSearchFilter(const char *server,int32 port)
    {
        this->port = port;
        this->server = server;
    }

    virtual ~HSRRSearchFilter()
    {
    }

    virtual bool Test(GCReference data)
    {

        GCRTemplate<HSRRItem> gchsrri;
        gchsrri = data;

        if (!gchsrri.IsValid()) return False;

        return (server == gchsrri->Server()) && (gchsrri->Port() == port);
    }

};

void HttpServiceRelayResource::CleanUnusedPorts(){

    Lock();
    for (int i=Size()-1; i >= 0;i--){
        GCRTemplate<HSRRItem> gchsrri = Find(i);
        if (gchsrri.IsValid()){
            if (gchsrri->PortIsFree()){
                AssertErrorCondition(Information,"CleanUnusedPorts(): %s:%i is free\n",gchsrri->Server(),gchsrri->Port());
//printf("%s:%i is free\n",gchsrri->Server(),gchsrri->Port());
                Remove(i);
            } else {
//                    printf("%i is alive\n",gchsrri->TaskId());
            }
        }
    }
    UnLock();

}


bool HttpServiceRelayResource::ProcessHttpMessage(HttpStream &hStream){

    FString HttpServiceName;

    if (!hStream.Switch("InputCommands.HttpServiceName")){
        CleanUnusedPorts();
        return HttpGroupResource::ProcessHttpMessage(hStream);
    }

    hStream.Seek(0);
    hStream.GetToken(HttpServiceName,"");
    hStream.Switch((uint32)0);

    if (!hStream.Switch("InputCommands.TaskId")){
        CleanUnusedPorts();
        return HttpGroupResource::ProcessHttpMessage(hStream);
    }

    int32 taskId;

    FString taskIdString;
    hStream.Seek(0);
    hStream.GetToken(taskIdString,"");
    hStream.Switch((uint32)0);

    taskId = atoi(taskIdString.Buffer());

    if (!hStream.Switch("Peer")){
        CleanUnusedPorts();
        return HttpGroupResource::ProcessHttpMessage(hStream);
    }

    FString peer;
    hStream.Seek(0);
    hStream.GetToken(peer,"");
    hStream.Switch((uint32)0);


    // try allocate a port among the unused ones
    Lock();
    bool found = False;
    int32 port = nextHttpPort;
    while(!found){
        HSRRSearchFilter hssrsf(peer.Buffer(),port);
        GCRTemplate<HSRRItem> gchsrri = Find(&hssrsf);
        if (gchsrri.IsValid()){
            if (gchsrri->PortIsFree()){
                found = True;
                Remove(&hssrsf);
//printf("replacing obsolete\n");
                AssertErrorCondition(Information,"ProcessHttpMessage(): replacing obsolete %s:%i \n",gchsrri->Server(),gchsrri->Port());
                GCRTemplate<HSRRItem> gchsrri(GCFT_Create);
                gchsrri->Setup(taskId,HttpServiceName.Buffer(),peer.Buffer(),port);
                Insert(gchsrri);
            } else {
                port++;
            }
        } else {
//            found = True;
//printf("not in list \n");
            GCRTemplate<HSRRItem> gchsrri(GCFT_Create);
            gchsrri->Setup(taskId,HttpServiceName.Buffer(),peer.Buffer(),port);
            if (gchsrri->PortIsFree()){
                Insert(gchsrri);
                found = True;
                AssertErrorCondition(Information,"ProcessHttpMessage(): adding %s:%i \n",gchsrri->Server(),gchsrri->Port());
//printf("selected port %i for %s \n",port,peer.Buffer());
            }
        }
    }

//    GCRTemplate<HSRRItem> gchsrri(GCFT_Create);
//    gchsrri->Setup(taskId,HttpServiceName.Buffer(),port);
//    Insert(gchsrri);
    UnLock();

    hStream.Printf("%i", port);
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/plain");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;

}

OBJECTLOADREGISTER(HttpServiceRelayResource,"$Id: HttpServiceRelayResource.cpp,v 1.4 2008/05/08 13:24:53 fpiccolo Exp $")



