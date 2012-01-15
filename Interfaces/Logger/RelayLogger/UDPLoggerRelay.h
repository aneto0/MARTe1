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
#if !defined(UDP_LOGGER_RELAY)
#define UDP_LOGGER_RELAY

#include "LoggerMessageQueue.h"
#include "LoggerMessage.h"
#include "GCReferenceContainer.h"
#include "Threads.h"
#include "UDPLoggerPlugin.h"
#include "UDPSocket.h"
#include "CDBExtended.h"

OBJECT_DLL(UDPLoggerRelay)

/**
 * Describes a remote relay connection
 */
class UDPRelayConnection : public GCNamedObject {
private:
    UDPSocket socket;

    uint32  lastAccessTime;

public:

    UDPRelayConnection(){
    }

    void Init(const char* remoteAddress, uint32 port){
        socket.Open();
        socket.Connect((char *)remoteAddress, port);
        FString name;
        name.Printf("%s:%i", remoteAddress, port);
        SetObjectName(name.Buffer());
        Ping();
    }

    virtual ~UDPRelayConnection(){
        AssertErrorCondition(Information, "Closing Socket: %s\n", Name());
        socket.Close();
    }

    /**
     * Updates the last access time
     */
    void Ping(){
        lastAccessTime = time(NULL);
    }

    /**
     * @return The last time it was used
     */
    uint32  GetLastAccessTime(){
        return lastAccessTime;
    }

    UDPSocket& GetSocket(){
        return socket;
    }
};

/** Thread which is responsible for managing the remote connections*/
void UDPLoggerRelayManager(void *args);

/** Thread responsible for managing the remote sockets*/
void UDPLoggerRelayServer(void *args);

/** Thread responsible for sending the history to a specific destination socket
 * Each history request will create a new thread.
 */
void UDPLoggerRelayHistoryBroadcast(void *args);

// to allow deleting of dead clients
class UDPLogRelayFilter: public SearchFilterT<GCReference>{
    FString name;

public:
    UDPLogRelayFilter(const char* remoteAddress, uint32 port){
        name.Printf("%s:%i", remoteAddress, port);
    }

    virtual ~UDPLogRelayFilter(){};

    bool Test(GCReference data){
        GCRTemplate<GCNamedObject> gcno;
        gcno = data;

        if (!gcno.IsValid())
            return False;

        return name == gcno->Name();
    }
};

class UDPLoggerRelay : public UDPLoggerPlugin{
private:
OBJECT_DLL_STUFF(UDPLoggerRelay)

    /**The maximum time for which a socket is allowed not to send a ping*/
    int32 maxNotPingTimeSecs;

    /**The server port*/
    int32 relayServerPort;

    /**The history message queue*/
    LoggerMessageQueue historyMessageQ;

    /**The maximum number of messages which can be saved on the queue*/
    int32 maxHistoryQueueMessages;

public:

    UDPLoggerRelay(){
    }

    virtual ~UDPLoggerRelay(){
        //Clean up will remove all entries which will call the correct
        //deconstructors and close all the sockets
        CleanUp();
        Shutdown();
    }

    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = UDPLoggerPlugin::ObjectLoadSetup(info, err);
        CDBExtended &cdbx = (CDBExtended &)info;

        if (!cdbx.ReadInt32(maxNotPingTimeSecs, "MaxNotPingTimeSecs", 30)){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default MaxNotPingTimeSecs: %d", maxNotPingTimeSecs);
        }

        if (!cdbx.ReadInt32(relayServerPort, "RelayServerPort", 44444)){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default RelayServerPort: %d", relayServerPort);
        }

        if (!cdbx.ReadInt32(maxHistoryQueueMessages, "MaxHistoryQueueMessages", 10000)){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default MaxHistoryQueueMessages: %d", maxHistoryQueueMessages);
        }

        Threads::BeginThread(UDPLoggerRelayManager, this);
        Threads::BeginThread(UDPLoggerRelayServer, this);
        return True;
    }

    virtual bool TextMenu(
                    StreamInterface &               in,
                    StreamInterface &               out)
    {
        MEMORYDisplayAllocationStatistics(&out);
        out.Printf("\n\n\n\nThe size of the internal logger queue is: %d\n", loggerMessageQ.Size());
        out.Printf("\n\n\n\nNumber of open relay sockets is: %d\n", Size());
        int currentTime = 0;
        out.Printf("N  %-55s %s\n", "Address", "Number of seconds since last ping");
        for(int i=0; i<Size(); i++){
            GCRTemplate<UDPRelayConnection> logRelay = Find(i);
            currentTime = time(NULL) - logRelay->GetLastAccessTime();
            out.Printf("%d  %-55s %d\n", (i + 1), logRelay->Name(), currentTime);
        }

        BasicConsole* console = dynamic_cast<BasicConsole*>(&in);

        if(console != NULL){
            WaitRead(in, out);
        }

        return True;
    }

    /** */
    virtual void ProcessMessage(GCRTemplate<LoggerMessage> msg);

    virtual void AddMessageToHistoryQueue(GCRTemplate<LoggerMessage> loggerMsg);

    void Shutdown(){
        alive = False;
        uint32 size = 0;
        UDPSocket sock;
        sock.Open();
        sock.Connect((char *)sock.Source().LocalAddress(), relayServerPort);
        sock.Write("", size);
        sock.Close();

        int counter = 0;
        while(alive == False && counter < 10){
            counter++;
            SleepMsec(100);
        }

        if(alive == False){
            AssertErrorCondition(Warning, "Closing on timeout");
        }
        else{
            AssertErrorCondition(Information, "Successfully closed");
        }
    }
};

/**
 * Contains information about an history request
 */
class HistoryRequest{
public:
    LoggerMessageQueue *historyMessageQ;
    char* remoteAddress;
    uint32 port;
    uint32 numberOfMessages;

    HistoryRequest( LoggerMessageQueue *historyMessageQ,
                    const char* remoteAddress,
                    uint32 port,
                    uint32 numberOfMessages)
    {
        this->historyMessageQ = historyMessageQ;
        this->remoteAddress = strdup(remoteAddress);
        this->port = port;
        this->numberOfMessages = numberOfMessages;
    }

    ~HistoryRequest(){
        if(remoteAddress != NULL){
            free((void *&)remoteAddress);
        }
    }
};


#endif
