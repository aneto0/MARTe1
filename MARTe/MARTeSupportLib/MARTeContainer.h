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


#if !defined _MARTE_CONTAINER_
#define _MARTE_CONTAINER_

#include "System.h"
#include "GCRTemplate.h"
#include "ConfigurationDataBase.h"
#include "GCReferenceContainer.h"
#include "MessageHandler.h"
#include "HttpInterface.h"
#include "GCRTemplate.h"
#include "MessageDeliveryRequest.h"
#include "MenuContainer.h"
#include "MenuEntry.h"

class MARTeContainer;

extern "C" {
    bool MARTeCProcessMessage(MARTeContainer &sm,GCRTemplate<MessageEnvelope> envelope);
    bool MARTeCProcessHttpMessage(MARTeContainer &sm,HttpStream &hStream);
    bool MARTeCObjectLoadSetup(MARTeContainer &sm,ConfigurationDataBase &info,StreamInterface *err);
    bool MARTeCObjectSaveSetup(MARTeContainer &sm,ConfigurationDataBase &info,StreamInterface *err);
    bool BrowseLevel1Message(StreamInterface &in,StreamInterface &out,void *userData);
    bool DumpLevel1Message(StreamInterface &in,StreamInterface &out,void *userData);
    bool LoadLevel1Message(StreamInterface &in,StreamInterface &out,void *userData);
}

OBJECT_DLL(MARTeContainer)

class MARTeContainer: public GCReferenceContainer, public MessageHandler,public HttpInterface{

private:

    friend bool MARTeCObjectLoadSetup(MARTeContainer &sm,ConfigurationDataBase &info,StreamInterface *err);
    friend bool MARTeCObjectSaveSetup(MARTeContainer &sm,ConfigurationDataBase &info,StreamInterface *err);
    friend bool MARTeCProcessMessage(MARTeContainer &sm,GCRTemplate<MessageEnvelope> envelope);
    friend bool MARTeCProcessHttpMessage(MARTeContainer &sm,HttpStream &hStream);
    friend bool BrowseLevel1Message(StreamInterface &in,StreamInterface &out,void *userData);
    friend bool DumpLevel1Message(StreamInterface &in,StreamInterface &out,void *userData);
    friend bool LoadLevel1Message(StreamInterface &in,StreamInterface &out,void *userData);

private:

    /** Reference to the last received CDB */
    ConfigurationDataBase                  lastReceivedCDB;

    /** Send Fatal Error Message */
    GCRTemplate <MessageDeliveryRequest>   sendFatalErrorMessage;

    /** Send Config Load Error Message */
    GCRTemplate <MessageDeliveryRequest>   sendConfigLoadErrorMessage;

    /** Send Config Load OK Message */
    GCRTemplate <MessageDeliveryRequest>   sendConfigLoadOKMessage;

    /** Name of the StateMachine Object*/
    FString                                stateMachineName;

    /** Name of the CODAS Level1 Handling Object */
    FString                                level1Name;

    /** Communication timeout */
    TimeoutType                            commTimeout;

private:

    /** Static part of the Menu System*/
    GCRTemplate <MenuContainer>            marteStaticMenu;

    /** Reference to the general Menu System*/
    GCRTemplate <MenuContainer>            marteCommonMenu;

    /** Creates the system menus dynamically */
    bool                                   CreateMenuInterfaces();

private:

    /** Contains all object that need a start and stop message */
    GCReferenceContainer                   needStartStopMessage;

    /** Send Start Message to all object in needStartStopMessage*/
    bool  StartAllActivities();

    /** Send Stop Message to all object in needStartStopMessage*/
    bool StopAllActivities();

private:
    
    /** Contains all real time threads */
    GCReferenceContainer                   realTimeThreads;
    
    /** Forward a Message to all object in needStartStopMessage*/
    bool ForwardMessageRequest(GCRTemplate<MessageEnvelope> gcrtme);

private:

    bool ProcessLevel1Message(ConfigurationDataBase &level1);

public:

    /** Constructor */
    MARTeContainer();

    /** Destructor */
    virtual ~MARTeContainer()
    {
    }

    /** The message handling routine */
    virtual     bool                ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
        return MARTeCProcessMessage(*this,envelope);
    }

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream){
        return MARTeCProcessHttpMessage(*this,hStream);
    }

    /** */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MARTeCObjectLoadSetup(*this,info,err);
    }

    /** */
    virtual     bool                ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MARTeCObjectSaveSetup(*this,info,err);
    }


    OBJECT_DLL_STUFF(MARTeContainer)
};

#endif





