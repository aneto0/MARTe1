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
#if !defined(UDP_LOGGER_PROXY)
#define UDP_LOGGER_PROXY

#include "LoggerMessageQueue.h"
#include "LoggerMessage.h"
#include "GCReferenceContainer.h"
#include "Threads.h"
#include "UDPLoggerPlugin.h"

OBJECT_DLL(UDPLoggerProxy)
/** @file
    Broadcast the received messages to all its listeners*/
        
class UDPLoggerProxy : public UDPLoggerPlugin{
private:
OBJECT_DLL_STUFF(UDPLoggerProxy)
    
public:
    
    UDPLoggerProxy() : UDPLoggerPlugin(){
    }
        
    /** This function will be called whenever there is a new message to process available*/
    virtual void ProcessMessage(GCRTemplate<LoggerMessage> msg){        
        for(int i=0; i<Size(); i++){
            GCRTemplate<UDPLoggerPlugin> plugin = Find(i);
            if(plugin.IsValid()){
                plugin->NewMessageAvailable(msg);
            }
        }
    }
};

#endif

