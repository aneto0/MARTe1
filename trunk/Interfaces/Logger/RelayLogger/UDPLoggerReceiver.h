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
#if !defined (UDP_LOGGER_RECEIVER)
#define UDP_LOGGER_RECEIVER

#include "ObjectMacros.h"
#include "GCReferenceContainer.h"
#include "CDBExtended.h"
#include "UDPSocket.h"
#include "UDPMessageIterator.h"
#include "MessageQueue.h"
#include "Threads.h"
#include "LoggerMessageQueue.h"
#include "Sleep.h"
#include "InternetAddress.h"
#include "MenuContainer.h"
#include "UDPLoggerPlugin.h"

OBJECT_DLL(UDPLoggerReceiver)

/** The thread which handles the UDP connections*/
void UDPMessageReceiver(void *args);

class UDPLoggerReceiver: public MenuContainer{

private:
OBJECT_DLL_STUFF(UDPLoggerReceiver)
    
    /** The server socket */
    UDPSocket                           udpServer;

    /** The port this server is listening to */
    int32                               port;
    
    /** This flag will go to false when the UDP Logger is to be stopped*/
    bool                                alive;
    
    
public:
    UDPLoggerReceiver():port(37767), alive(True){
    }
    
    virtual ~UDPLoggerReceiver(){
        CleanUp();
        Shutdown();
    }
    
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){
        
        bool ret = MenuContainer::ObjectLoadSetup(info, err);
        CDBExtended &cdbx = (CDBExtended &)info;
        
        if (!cdbx.ReadInt32(port, "Port", 32767)){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default port 32767");
        }
        
        if(NumberOfReferences() == 0){
            AssertErrorCondition(Warning, "No listeners were set!");
        }
                
        Threads::BeginThread(UDPMessageReceiver, this);
        return True;
    }
   
    bool IsAlive(){
        return alive;
    }
    
    void Shutdown(){        
        alive = False;
        uint32 size = 0;
        UDPSocket sock;        
        sock.Open();
        sock.Connect((char *)sock.Source().LocalAddress(), port);
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

#endif
