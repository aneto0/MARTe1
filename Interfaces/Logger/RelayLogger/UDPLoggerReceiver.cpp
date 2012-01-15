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
#include "UDPLoggerReceiver.h"
#include "ErrorManagement.h"
#include "LoggerMessage.h"
#include "UDPLoggerPlugin.h"


OBJECTLOADREGISTER(UDPLoggerReceiver,"$Id$")

void UDPMessageReceiver(void *args){
    UDPLoggerReceiver *loggerRec = (UDPLoggerReceiver *)args;
    UDPSocket *serverSocket = &loggerRec->udpServer;
        
    if (serverSocket->Open() == False){
        CStaticAssertErrorCondition(FatalError, "Cannot Open UDP server\n");
        return;
    }

    if (serverSocket->Listen(loggerRec->port, 255) == False){
        CStaticAssertErrorCondition(FatalError, "UDP server Cannot Listen in port %d\n", loggerRec->port);
        return;
    }

    if (serverSocket->SetBlocking(True) == False){
        CStaticAssertErrorCondition(FatalError, "UDP server Cannot Set blocking mode\n");
        return;
    }
    
    char    buffer[1024];
    uint32  read = 1024;
    BString hostName;
        
    while(loggerRec->IsAlive()){
        read = 1024;        
        if(serverSocket->Read(buffer, read)){            
            GCRTemplate<LoggerMessage> gcrlm(GCFT_Create);
            serverSocket->Source().HostName(hostName);
            if (gcrlm->LoadFromMessage(hostName.Buffer(), buffer, read)){                
                for(int i=0; i<loggerRec->Size(); i++){
                    GCRTemplate<UDPLoggerPlugin> plugin = loggerRec->Find(i);
                    if(plugin.IsValid()){                        
                        plugin->NewMessageAvailable(gcrlm);
                    }                    
                }                
            }
        }
    }
    
    serverSocket->Close();
    loggerRec->alive = True;
}

