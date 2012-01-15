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
#if !defined(UDP_LOGGER_PLUGIN)
#define UDP_LOGGER_PLUGIN

#include "LoggerMessageQueue.h"
#include "LoggerMessage.h"
#include "GCReferenceContainer.h"
#include "Threads.h"
#include "BString.h"
#include "CDBExtended.h"
#include "MenuInterface.h"
#include "HttpInterface.h"
#include "MessageHandler.h"

OBJECT_DLL(UDPLoggerPlugin)
/** @file
    Every interface which wishes to receive udp message must implement this interface*/

/** Thread responsible for watching the queue and broadcasting events*/
void UDPLoggerPluginReceiver(void *args);

class UDPLoggerPlugin : 
        public GCReferenceContainer, 
        public MenuInterface{
        //public HttpInterface{
private:
OBJECT_DLL_STUFF(UDPLoggerPlugin)
    /** The message queue*/
    LoggerMessageQueue  loggerMessageQ;
    
    /** Is the plugin still alive*/
    bool                alive;
    
    /** a specific title, distinct from the object name */
    BString             title;
    
    /** This function will be called whenever there is a new message to process available*/
    virtual void ProcessMessage(GCRTemplate<LoggerMessage> msg){
        FString str;
        msg->FormatMessage(str);
        CStaticAssertErrorCondition(Information, str.Buffer());
    }
    
protected:
    /** Starts the thread*/
    void Start(){
        alive = True;
        Threads::BeginThread(UDPLoggerPluginReceiver, this);
    }
    
    /** Safely shutdown the plugin*/
    void Stop(){
        alive = False;        
        loggerMessageQ.Close();
        Flush();
    }
    
    /** Flushes all messages from the queue*/
    void Flush(){        
        while(alive == False){
            GCRTemplate<LoggerMessage> gcrlm = loggerMessageQ.Get();        
            if(gcrlm.IsValid()){
                ProcessMessage(gcrlm);
            }
            else{
                alive = True;
            }
        }
    }
    
public:
    
    UDPLoggerPlugin(){       
        Start();
    }
    
    virtual ~UDPLoggerPlugin(){
        Stop();
    }
    
    void NewMessageAvailable(GCRTemplate<LoggerMessage> msg){    
        loggerMessageQ.Add(msg);
    }
    
    /** Checks if this plugin is still alive*/
    bool IsAlive(){
        return alive;
    }
    
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){
        
        bool ret = GCReferenceContainer::ObjectLoadSetup(info, err);
        CDBExtended &cdbx = (CDBExtended &)info;
        
        BString title;
        if (cdbx.ReadBString(title, "Title")){
            SetTitle(title.Buffer());
        }
        
        return ret;
    }
    
    /** to be able to choose the labelling policy */
    virtual const char *Title(){
        if (title.Size() == 0)
            return GCReferenceContainer::Name();
        else
            return title.Buffer();
    }

    virtual void SetTitle(const char *title){
        this->title = title;
    }
        
    void WaitRead(StreamInterface &in, StreamInterface &out){
        out.Printf("\nHit any key\n");
        char buffer[1];
        uint32 size=1;
        in.Read(buffer, size);
    }
};

#endif
