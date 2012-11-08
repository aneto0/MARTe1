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
 * $Id: MessageTriggeringTimeService.h 3 2012-01-15 16:26:07Z aneto $
 *
**/
/**
 * @file 
 * Forwards configuration messages received using a TCP interface to MARTe
 */
#ifndef TCP_CONFIGURATION_HANDLER_H 
#define TCP_CONFIGURATION_HANDLER_H

#include "GCReferenceContainer.h"
#include "CDBExtended.h"
#include "TCPSocket.h"
#include "MessageHandler.h"

OBJECT_DLL(TCPConfigurationHandler)
class TCPConfigurationHandler : public GCNamedObject, public MessageHandler{
OBJECT_DLL_STUFF(TCPConfigurationHandler)

private:
    /**
     * Handle connection requests
     */
    friend void ConnectionHandlerFn(TCPConfigurationHandler &tcpmh);
    bool ConnectionHandler();

    /**
     * Parse the client request and send the message
     */
    bool HandleRequest(FString &msg);

    /**
     * Handle the client requests and forward the configuration requests
     **/
    void ClientHandler(TCPSocket *client);

    /**
     * This flag is true while the TCP server is supposed to be running
     */
    bool keepAlive;

    /**
     * TCP server port
     */
    int32 serverPort;

    /**
     * The server socket
     */
    TCPSocket server;
    
    /**
     * The TCP server thread identifier
     */
    int32 serverTID;
    
    /**
     * The TCP server cpu mask
     */
    int32 cpuMask;

    /**
     * Timeout to send the messages
     */
    TimeoutType msgTimeout;

    /**
     * The location of MARTe
     */
    FString MARTeLocation;

public:
    TCPConfigurationHandler();
    
    virtual ~TCPConfigurationHandler();
    
    /**
     * @sa Object::ObjectLoadSetup
     */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err);
};

#endif

