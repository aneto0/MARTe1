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
 * @file A very simple TCP client
 */
#include "ErrorManagement.h"
#include "FString.h"
#include "TCPSocket.h"

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Connect to a server running in localhost and in port 12468
    int32     port = 12468;
    FString   host = "localhost";
    TCPSocket client;
    //Open the socket
    if(!client.Open()){
        CStaticAssertErrorCondition(FatalError, "Failed to open client socket");
        return -1;
    }
    //Connect to the server
    if(!client.Connect(host.Buffer(), port)){
        CStaticAssertErrorCondition(FatalError, "Failed to connect to %s:%d", host.Buffer(), port);
        client.Close();
        return -1;
    }
    //Write a line
    FString line = "Hello!";
    uint32  size = line.Size();
    if(!client.Write(line.Buffer(), size)){
        CStaticAssertErrorCondition(FatalError, "Failed to write to socket");
    }
    //Notice that the actual number of bytes wrote is updated on the size variable
    CStaticAssertErrorCondition(Information, "Successfully wrote %d out of %lld bytes to %s:%d", size, line.Size(), host.Buffer(), port);
    //Housekeeping
    client.Close();
    return 0;
}

