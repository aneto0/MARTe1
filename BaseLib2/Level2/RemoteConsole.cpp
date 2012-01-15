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

#include "Streamable.h"
#include "RemoteConsole.h"
#include "TCPSocket.h"
#include "FString.h"


OBJECTREGISTER(RemoteConsole,"$Id: RemoteConsole.cpp,v 1.8 2008/11/11 14:04:43 fpiccolo Exp $")


void RemoteConsoleInit(RemoteConsole &con){
    con.connected               = False;
    con.consoleThread           = 0;
    con.remoteConnectionThread  = 0;
    con.newMessage.Create();
    con.bufferRequest.Create();
    con.newMessage.Reset();
    con.bufferRequest.Reset();
    con.resourceLock.Create();
    con.hasToContinue           = False;
    con.connectedSocket         = NULL;

    con.consoleThread           = Threads::BeginThread(RCConsoleThread,(void *)&con,THREADS_DEFAULT_STACKSIZE,"RCCONSOLE");
    con.remoteConnectionThread  = Threads::BeginThread(RCRemoteThread,(void *)&con,THREADS_DEFAULT_STACKSIZE,"RCREMOTE");
}

void RemoteConsoleFinish(RemoteConsole &con){
    Threads::Kill(con.consoleThread);
    if (con.hasToContinue == True){
        con.bufferRequest.Post();
        con.hasToContinue = False;
        while(con.hasToContinue == False) SleepMsec(10);
    }
}

bool RemoteConsoleWrite(RemoteConsole &con,const void* buffer, uint32 &size,TimeoutType msecTimeout){
    if (con.resourceLock.Lock()){
        bool ret= False;
        if (con.connected){
            ret = con.connectedSocket->Write(buffer,size,msecTimeout);
        } else {
            ret = ConsoleWrite(con,buffer,size,msecTimeout);
        }
        con.resourceLock.UnLock();
        return True;
    }
    return False;
}


bool RemoteConsoleRead(RemoteConsole &con,void* buffer, uint32 &size,TimeoutType msecTimeout){
    con.bufferReadSize = size;
    con.bufferRequest.Post();
    if (con.newMessage.Wait(msecTimeout)){
        con.newMessage.Reset();
        if (con.bufferReadSize <= 0 ){
            size = 0;
            return False;
        }

        memcpy(buffer,con.bufferPtr,con.bufferReadSize);
        size = con.bufferReadSize;
        return True;
    }
    return False;
}

void  __thread_decl RCConsoleThread(void *arg){

    RemoteConsole *con = (RemoteConsole *)arg;
    char localBuffer[4096];

    Console *console;
    console = con;

    while(True){
        bool readRequest = con->bufferRequest.Wait(1000);
        if (readRequest) {
            uint32 size=con->bufferReadSize;
            if (size>=sizeof(localBuffer)){
                size = sizeof(localBuffer)-1;
            }
            ConsoleRead(*console,localBuffer,size,TTInfiniteWait);

            if (!con->connected){
                if (con->resourceLock.Lock(0)){
                    con->bufferRequest.Reset();

                    con->bufferReadSize = size;
                    con->bufferPtr = localBuffer;
                    con->newMessage.Post();

                    con->resourceLock.UnLock();
                }
            } else {
                const char *msg = "\nSorry! Remotely Connected\n";
                uint32 size = strlen(msg)+1;
                ConsoleWrite(*console,msg,size,TTInfiniteWait);
            }
        }
    }
}


struct  RemoteConsoleClientThreadInfoStruct{
    Console     con;
    TCPSocket   socket;

};

void  __thread_decl RemoteConsoleClientThread(void *arg){
    RemoteConsoleClientThreadInfoStruct *info = (RemoteConsoleClientThreadInfoStruct *)arg;
#if 0
    char buffer[16384];

    uint32 size = sizeof(buffer)-1;
    while (info->socket.Read(buffer,size)){
        info->con.Write(buffer,size);
        size = sizeof(buffer)-1;
    }
#else
    int escape = 0;
    char c;
    while (info->socket.GetC(c)){
        switch (escape){
            case 0:{
                if (c == '\001') {
                    escape = 1;
                } else {
                    info->con.PutC(c);
                }
            } break;
            case 1:{
                if (c == '\002') {
                    FString line;
                    info->socket.GetLine(line);
                    info->con.Switch(atoi(line.Buffer()));
                    escape = 0;
                } else
                if (c == '\003') {
                    FString line;
                    info->socket.GetLine(line);
                    info->con.Switch(line.Buffer());
                    escape = 0;
                } else {
                    info->con.PutC('\001');
                    info->con.PutC(c);
                }
            } break;
        }


    }

#endif
}


void RemoteConsoleClient(const char *server,uint32 port){

    RemoteConsoleClientThreadInfoStruct info;
    char buffer[16384];

    TCPSocket socket;
    info.socket.Open();
    info.socket.SetBlocking(True);
    bool connected = info.socket.Connect(server,port,2000);

    if (connected){
        info.socket.Printf("\n");
        Threads::BeginThread(RemoteConsoleClientThread,(void *)&info,THREADS_DEFAULT_STACKSIZE,"RemoteConsoleClient");
        while(info.con.GetToken(buffer,"\n",sizeof(buffer)-2,NULL,NULL)){
            uint32 size = strlen(buffer);
            buffer[size] = '\n';
            buffer[size+1] = '0';
            size++;
            if (!info.socket.CompleteWrite(buffer,size)){
                info.con.Printf("Connection terminated by remote host\n");
                return ;
            }
            size = sizeof(buffer)-1;
        }
    }
    info.socket.Close();
}

void  __thread_decl RCRemoteThread(void *arg){
    RemoteConsole *con = (RemoteConsole *)arg;
    Console *console;
    console = con;

    char localBuffer[16384];
    con->hasToContinue = True;
    TCPSocket server;
    if (server.Open() == False) {
        CStaticAssertErrorCondition(FatalError,"RemoteConsole::RCRemoteThread:Cannot open socket\n");
        con->hasToContinue = False;
        return;
    }
    server.SetBlocking(False);
    if (server.Listen(con->listeningPort,255) == False){
        CStaticAssertErrorCondition(FatalError,"RemoteConsole::RCRemoteThread:Cannot bind socket\n");
        con->hasToContinue = False;
        return;
    }

    while(con->hasToContinue){
        con->connectedSocket = server.WaitConnection(1000/*,10*/);
        if (con->connectedSocket != NULL) {
            {
                BString hostname;
                con->resourceLock.Lock();
                con->connected = True;
                sprintf(localBuffer,"\nShell Disabled: Remotely Connected from %s\n",con->connectedSocket->Source().HostName(hostname));
                uint32 size = strlen(localBuffer)+1;
                ConsoleWrite(*console,localBuffer,size,TTInfiniteWait);
                con->resourceLock.UnLock();
            }
            bool ok = True;
            while(ok){
                bool readRequest = con->bufferRequest.Wait(20000);
                uint32 size=0;
                if (readRequest) {
                    con->connectedSocket->SetBlocking(True);
                    size=con->bufferReadSize;
                    if (size>=sizeof(localBuffer))
                        size = sizeof(localBuffer)-1;
                } else {
                    size = 0;
                    con->connectedSocket->SetBlocking(False);
                }
                ok = con->connectedSocket->Read(localBuffer,size);
                if (ok && (size>0)){
                    if (con->resourceLock.Lock(0)){

                        con->bufferReadSize = size;
                        con->bufferPtr = localBuffer;
                        con->newMessage.Post();

                        con->bufferRequest.Reset();
                        con->resourceLock.UnLock();
                    }
                }
            }
            {
                con->resourceLock.Lock();
                con->connected = False;
                sprintf(localBuffer,"\nShell Enabled\n");
                uint32 size = strlen(localBuffer)+1;
                ConsoleWrite(*console,localBuffer,size,TTInfiniteWait);
                con->resourceLock.UnLock();
            }
        }
    }
    con->hasToContinue = True;

}

