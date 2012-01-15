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

#include "MessageServer.h"
#include "FastPollingMutexSem.h"
#include "GlobalObjectDataBase.h"
#include "BasicUDPSocket.h"
#include "SocketTimer.h"
#include "CDBExtended.h"
#include "Console.h"
#include "TCPSocket.h"


OBJECTLOADREGISTER(MessageServer,"$Id$")

int32 MSServerPort = 0;

int32 MSGetServerPort(){
    return MSServerPort;
}

void MSProcessIncomingMessages(MessageServer &ms){

   //BasicUDPSocket serverSocket;
    TCPSocket serverSocket;
    TCPSocket connectedSocket;

    serverSocket.Open();
    serverSocket.Listen(ms.serverPort);

    FString messageBuffer;
    messageBuffer.SetSize(10000);

    SocketTimer timer;
    timer.SetMsecTimeout(1000);
    timer.AddWaitOnReadReady(&serverSocket);

    while(ms.threadToContinue){

        if (timer.WaitRead()){            
            if(serverSocket.WaitConnection(TTInfiniteWait, &connectedSocket)){
/*                uint32 size = messageBuffer.Size();
                const char *buffer = messageBuffer.BufferReference();
                if (connectedSocket.BasicRead((void *)buffer, size)){
                    if (size >= messageBuffer.Size()){
                        ms.AssertErrorCondition(CommunicationError,"message is too large > %i bytes ",messageBuffer.Size());
                    } else {*/
    /*if (0){
        Console con;
        con.SSPrintf(ColourStreamMode,"%i %i",Black,Red);
        con.Printf("%s",buffer);
        con.SSPrintf(ColourStreamMode,"%i %i",White,Black);
    }*/
                ConfigurationDataBase cdb;
//                messageBuffer.Seek(0);
//                cdb->ReadFromStream(messageBuffer);
                if(cdb->ReadFromStream(connectedSocket)){                                                    
                    GCReference gc;
                    if (!gc.ObjectLoadSetup(cdb,NULL)){
                        ms.AssertErrorCondition(FatalError,"message cannot be converted to object");
                    } else {
                        GCRTemplate<MessageEnvelope> envelope;
                        envelope = gc;

                        if (envelope.IsValid()){
                            MessageHandler::SendMessage(envelope);

                        } else {
                            ms.AssertErrorCondition(FatalError,"message cannot be converted to an MessageEnvelope");
                        }
                    }
                }
                else{
                    ms.AssertErrorCondition(FatalError,"Error creating CDB from stream");
                }
            }
            connectedSocket.Close();
        }
    }
    serverSocket.Close();
}


void MessageServerThreadFN(void *arg){

    Threads::SetPriorityLevel(0);
    MessageServer *ms = (MessageServer *)arg;

    // calls the method
    if (ms != NULL){

        // set to True to allow the server to continue
        ms->threadToContinue = True;

        // mark thread start!
        ms->threadStartEvent.Post();

        // do the task
        MSProcessIncomingMessages(*ms);

        // mark thread end!
        ms->msThreadID = (TID)0;

        // wake other task
        ms->threadStartEvent.Post();

    }
}




bool
MSObjectLoadSetup(MessageServer &ms,ConfigurationDataBase &info,StreamInterface *err){

    if (MSServerPort != 0) {
        ms.AssertErrorCondition(ParametersError,"ObjectLoadSetup: Only one ServerPort can be specified");
        return False;
    }

    bool ret = ms.GCNamedObject::ObjectLoadSetup(info,err);

    CDBExtended cdbx(info);

    bool ret1 = cdbx.ReadInt32 (ms.serverPort,"ServerPort",0);
    if (!ret1){
        ms.AssertErrorCondition(ParametersError,"ObjectLoadSetup: ServerPort must be specified");
        ret = False;
    }

    MSServerPort = ms.serverPort;
    return ret;

}

bool
MSObjectSaveSetup(MessageServer &ms,ConfigurationDataBase &info,StreamInterface *err){

    bool ret = ms.GCNamedObject::ObjectSaveSetup(info,err);

    CDBExtended cdbx(info);

    cdbx.WriteInt32 (ms.serverPort,"ServerPort");

    return ret;

}





