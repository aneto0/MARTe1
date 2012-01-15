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
 * @file A very simple TCP server
 */
#include "ErrorManagement.h"
#include "FString.h"
#include "TCPSocket.h"

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Create a server running in port 12468
    int32     port = 12468;
    TCPSocket server;
    //Open the socket
    if(!server.Open()){
        CStaticAssertErrorCondition(FatalError, "Failed to open server socket");
        return -1;
    }
    //Set in server mode
    if(!server.Listen(port)){
        CStaticAssertErrorCondition(FatalError, "Failed to create server running in port %d", port);
        server.Close();
        return -1;
    }
    //Wait for a connection
    TCPSocket *client = server.WaitConnection();
    if(client == NULL){
        CStaticAssertErrorCondition(FatalError, "Failed waiting for a connection in port %d", port);
        server.Close();
        return -1;
    }
    //Set the client in blocking mode for the read
    client->SetBlocking(True);
    //Print information from the client
    FString hostname;
    client->Source().HostName(hostname);
    CStaticAssertErrorCondition(Information, "Accepted a connection from %s", hostname.Buffer());
    //Read a line from the client socket
    FString line;
    client->GetLine(line);
    CStaticAssertErrorCondition(Information, "Read line from socket: %s", line.Buffer());
    //Housekeeping
    server.Close();
    return 0;
}

