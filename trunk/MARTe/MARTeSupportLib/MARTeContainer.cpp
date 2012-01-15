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

#include "MARTeContainer.h"
#include "CDBExtended.h"
#include "HtmlStream.h"
#include "GlobalObjectDataBase.h"
#include "MessageDispatcher.h"
#include "StartStopMessageHandlerInterface.h"
#include "RealTimeThread.h"
#include "CDBBrowserMenu.h"
#include "File.h"
#include "Directory.h"
#include "DirectoryMenuBrowser.h"
#include "GenericAcqModule.h"


bool MARTeCObjectLoadSetup(MARTeContainer &mc,ConfigurationDataBase &info,StreamInterface *err){
    mc.CleanUp();
    mc.needStartStopMessage.CleanUp();
    mc.realTimeThreads.CleanUp();

    CDBExtended cdbx(info);
    if(!mc.GCReferenceContainer::ObjectLoadSetup(cdbx,err)){
        mc.CleanUp();
        mc.AssertErrorCondition(InitialisationError,"ObjectLoadSetup: GCReferenceContainer::ObjectLoadSetup failed for object %s",mc.Name());
        return False;
    }
    if(!mc.HttpInterface::ObjectLoadSetup(cdbx,err)){
        mc.CleanUp();
        mc.AssertErrorCondition(InitialisationError,"ObjectLoadSetup: HttpInterface::ObjectLoadSetup failed for object ",mc.Name());
        return False;
    }    
    if(!cdbx.ReadFString(mc.stateMachineName,"StateMachineName")){
        mc.CleanUp();
        mc.AssertErrorCondition(InitialisationError,"ObjectLoadSetup: StateMachineName has not been specified",mc.Name());
        return False;
    }    
    if(!cdbx.ReadFString(mc.level1Name,"Level1Name")){
        mc.CleanUp();
        mc.AssertErrorCondition(InitialisationError,"ObjectLoadSetup: Level1Name has not been specified",mc.Name());
        return False;
    }    
    float temp;
    cdbx.ReadFloat(temp,"CommunicationSecTimeOut",10.0);
    mc.commTimeout.SetTimeOutSec(temp);    
    for(int n = 0; n < mc.Size(); n++){
        GCReference obj = mc.Find(n);
        if(obj.IsValid()){
            GCRTemplate<StartStopMessageHandlerInterface> messageHandler = obj;
            if(messageHandler.IsValid()) mc.needStartStopMessage.Insert(messageHandler);


            GCRTemplate<RealTimeThread> rtThread = obj;
            if(rtThread.IsValid())       mc.realTimeThreads.Insert(rtThread);
        }
    }

    if (mc.needStartStopMessage.Size() == 0){
        mc.AssertErrorCondition(Information,"MARTeContainer::ObjectLoadSetup: No StartStopMessageHandler has been found");
    }

    if (mc.realTimeThreads.Size()      == 0){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: No RealTimeThread has been found");
        return False;
    }

    GCReference messages = mc.Find("Messages");
    if(!messages.IsValid()){
        mc.AssertErrorCondition(Warning,"MARTeContainer::ObjectLoadSetup: No Messages section was specified");
    }
    GCReference messageFatalError = mc.Find("FatalErrorMessage", GCFT_Recurse);
    if(!messageFatalError.IsValid()){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: Failed finding FatalErrorMessage");
        return False;
    }

    mc.sendFatalErrorMessage = messageFatalError;
    if(!mc.sendFatalErrorMessage.IsValid()){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: FatalErrorMessage is not a Valid MessageDeliveryRequest");
        return False;
    }

    GCReference messageConfigError = mc.Find("ConfigLoadErrorMessage", GCFT_Recurse);
    if(!messageConfigError.IsValid()){
        mc.AssertErrorCondition(Warning, "MARTeContainer::ObjectLoadSetup: Failed finding ConfigLoadErrorMessage. Using FatalErrorMessage.");
        messageConfigError = messageFatalError;
    }

    mc.sendConfigLoadErrorMessage = messageConfigError;
    if(!mc.sendConfigLoadErrorMessage.IsValid()){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: ConfigLoadErrorMessage is not a Valid MessageDeliveryRequest");
        return False;
    }

    GCReference messageConfigOK = mc.Find("ConfigLoadOKMessage", GCFT_Recurse);
    if(!messageConfigOK.IsValid()){
        mc.AssertErrorCondition(Warning, "MARTeContainer::ObjectLoadSetup: Failed finding ConfigLoadOKMessage.");
    }
    else{
        mc.sendConfigLoadOKMessage = messageConfigOK;
        if(!mc.sendConfigLoadOKMessage.IsValid()){
            mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: ConfigLoadOKMessage is not a Valid MessageDeliveryRequest");
            return False;
        }
    }

    GCReference pool = mc.Find("DriverPool");
    if(!pool.IsValid()){
        mc.AssertErrorCondition(Warning,"MARTeContainer::ObjectLoadSetup: No driver pool was specified");
    }

    FString menuContainerName;
    if(!cdbx.ReadFString(menuContainerName,"MenuContainerName")){
        mc.AssertErrorCondition(InitialisationError,"ObjectLoadSetup: MenuContainerName has not been specified, Assuming MARTeMenu",mc.Name());
        menuContainerName = "MARTeMenu";
    }

    /* Create the Menu Interface to add to MARTeMenu*/
    GCReference menu = mc.Find(menuContainerName.Buffer());
    if(!menu.IsValid()){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: Failed finding MenuContainer %s. No Menu Interface is created.", menuContainerName.Buffer());
        // The user is allowed to not specify a menu
        return True;
    }

    GCRTemplate <MenuContainer>   marteTempMenu(menu);
    if(!marteTempMenu.IsValid()){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: Object %s is not a MenuContainer class. No Menu Interface is created.", menuContainerName.Buffer());
        return False;
    }

    mc.marteCommonMenu = marteTempMenu;

    if(!mc.CreateMenuInterfaces()){
        mc.AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: Failed creating menu interfaces." );
        return False;
    }

    mc.lastReceivedCDB = cdbx;
    return True;
}

bool MARTeContainer::CreateMenuInterfaces(){



    if(!marteCommonMenu.IsValid()){
        AssertErrorCondition(FatalError,"CreateMenuInterfaces: %s: marteCommonMenu is not valid",Name());
        return False;
    }

    // Clean the container, and insert the marteStaticMenu 
    marteCommonMenu->CleanUp();
    marteCommonMenu->Insert(marteStaticMenu);

    /* RealTimeThreads Menus */
    for (int i = 0; i < Size(); i++){
        GCReference obj = Find(i);
        if (obj.IsValid()){
            GCRTemplate <RealTimeThread> thread(obj);
            if (thread.IsValid()){
                GCRTemplate <GCNamedObject>   namedThread(thread);
                GCRTemplate <MenuContainer>   threadMenu(GCFT_Create);
                if (!threadMenu.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuContainer class for RealTimeThread ", Name());
                    return False;
                }

                FString menuName;
                menuName.Printf("%sMenu",namedThread->Name());
                threadMenu->SetObjectName(menuName.Buffer());
                threadMenu->SetTitle("RealTimeThread Menu");

                /* External Time Triggering Service */
                GCRTemplate <MenuEntry>       trigger(GCFT_Create);
                if(!trigger.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::ObjectLoadSetup: %s: Failed creating MenuEntry class for ExternalTimeTriggeringService for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                trigger->SetObjectName("TriggeringServiceMenu");
                trigger->SetTitle("Triggering Service Menu");
                trigger->SetUp(TriggerMenu, NULL, NULL, (thread.operator->()));
                threadMenu->Insert(trigger);

                /* DDB Menu Container */
                GCRTemplate <MenuContainer>       ddb(GCFT_Create);
                if(!ddb.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuContainer class for DDB for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                ddb->SetObjectName("DDBMenu");
                ddb->SetTitle("DDBMenu");

                GCRTemplate<DDB> ddbReference;
                for(int nOfObjects = 0; nOfObjects < thread->Size(); nOfObjects++){
                    GCReference gc = thread->Find(nOfObjects);
                    GCRTemplate<DDB> ddbGCR(gc);
                    if(ddbGCR.IsValid()){
                        ddbReference = ddbGCR;
                    }
                }

                if(!ddbReference.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: RealTimeThread %s does not have a valid DDB", Name(), namedThread->Name());
                    return False;
                }

                GCRTemplate <MenuEntry>           ddbBrowse(GCFT_Create);
                if(!ddbBrowse.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuEntry class for Browsing DDB for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                ddbBrowse->SetUp(DDBBrowseMenu, NULL, NULL, (thread.operator->()));
                ddbBrowse->SetObjectName("BrowseDDBMenu");
                ddbBrowse->SetTitle("Browse DDB Menu");
                ddb->Insert(ddbBrowse);

                GCRTemplate <MenuEntry>           ddbDump(GCFT_Create);
                if(!ddbDump.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuEntry class for Dumping DDB for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                ddbDump->SetTitle("DumpDDBMenu");
                ddbDump->SetTitle("Dumping DDB Menu");
                ddbDump->SetUp(DDBDumpMenu, NULL, NULL, (thread.operator->()));
                ddb->Insert(ddbDump);

                threadMenu->Insert(ddb);

                /* GAMs Menu Container */
                GCRTemplate <MenuContainer>       gams(GCFT_Create);
                if(!gams.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuContainer class for GAMs for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                for(int nOfGAMs = 0; nOfGAMs < thread->Size(); nOfGAMs++){
                    GCReference gc = thread->Find(nOfGAMs);
                    GCRTemplate<GAM> gamReference(gc);
                    if(gamReference.IsValid()){
                        gams->Insert(gamReference->Menu());
                    }
                }

                gams->SetObjectName("GAMsMenu");
                gams->SetTitle("GAMs Menu");
                threadMenu->Insert(gams);

                GCRTemplate <MenuContainer>       onlineGams(GCFT_Create);
                if(!onlineGams.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuContainer class for Online GAMs for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                onlineGams->SetObjectName("OnlineGAMsMenu");
                onlineGams->SetTitle("Online GAMs Menu");
                threadMenu->Insert(onlineGams);

                GCRTemplate <MenuContainer>       offlineGams(GCFT_Create);
                if(!offlineGams.IsValid()){
                    AssertErrorCondition(InitialisationError,"MARTeContainer::CreateMenuInterfaces: %s: Failed creating MenuContainer class for Offline GAMs for RealTimeThread %s", Name(), namedThread->Name());
                    return False;
                }

                offlineGams->SetObjectName("OfflineGAMsMenu");
                offlineGams->SetTitle("Offline GAMs Menu");
                threadMenu->Insert(offlineGams);

                marteCommonMenu->Insert(threadMenu);
            }
        }
    }

    return True;
}

bool MARTeCObjectSaveSetup(MARTeContainer &mc,ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdbx(info);
    bool ret   = mc.GCReferenceContainer::ObjectSaveSetup(cdbx,err);
    ret = ret && mc.HttpInterface::ObjectSaveSetup(cdbx,err);
    return ret;
}

bool MARTeCProcessMessage(MARTeContainer &mc,GCRTemplate<MessageEnvelope> envelope){

    if(!envelope.IsValid()){
        mc.AssertErrorCondition(FatalError,"ProcessMessage: %s: Received invalid envelope",mc.Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (message.IsValid()){
        FString sender  = envelope->Sender();
        // Skip the address in the name
        if(sender[0] == '('){
            const char* name = sender.Buffer();
            while ( (*name != ')') && (*name != 0)) name++;
            if (*name == 0)                   return False;
            sender = (name + 1);
        }

        FString content = message->Content();
        mc.AssertErrorCondition(Information,"ProcessMessage: %s: Received Message %s from %s",mc.Name(), content.Buffer(), sender.Buffer());
        ////////////////////
        // Level1 Message //
        ////////////////////
        if ((strstr(sender.Buffer(), mc.level1Name.Buffer()) != NULL) || content == "ChangeConfigFile"){

            bool error = False;
            GCReference cdb = message->Find(0);
            if (!cdb.IsValid()){
                mc.AssertErrorCondition(FatalError,"ProcessMessage: %s: Received empty level1 Message",mc.Name());
                GMDSendMessageDeliveryRequest(mc.sendConfigLoadErrorMessage, TTInfiniteWait ,False);
                error = True;
            }                

            ConfigurationDataBase    level1Message(cdb);
            if (!level1Message.IsValid()){
                mc.AssertErrorCondition(FatalError,"ProcessMessage: %s: Received invalid level1 Message",mc.Name());
                GMDSendMessageDeliveryRequest(mc.sendConfigLoadErrorMessage, TTInfiniteWait ,False);
                error = True;
            }

            if (!mc.ProcessLevel1Message(level1Message)){
                GMDSendMessageDeliveryRequest(mc.sendConfigLoadErrorMessage, TTInfiniteWait ,False);
                error = True;
            }

            if (envelope->ReplyExpected()){
                GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
                GCRTemplate<Message> gcrtm(GCFT_Create);
                if(error){
                    gcrtm->Init(2000,"ERROR");
                }
                else{
                    gcrtm->Init(0   ,"OK");
                }
                gcrtme->PrepareReply(envelope,gcrtm);
                MessageHandler::SendMessage(gcrtme);
            }

            if(!error){
                if(mc.sendConfigLoadOKMessage.IsValid()){
                    GMDSendMessageDeliveryRequest(mc.sendConfigLoadOKMessage, TTInfiniteWait ,False);
                }
            }

            return True;

        ///////////////////////////
        // State Machine Message //
        ///////////////////////////
        }
        else if((strstr(sender.Buffer(), mc.stateMachineName.Buffer())) != NULL){
            if(!mc.ForwardMessageRequest(envelope)){
                mc.AssertErrorCondition(FatalError,"ProcessMessage: %s: Failed Forwarding Message %s",mc.Name(), content.Buffer());
                GMDSendMessageDeliveryRequest(mc.sendFatalErrorMessage, TTInfiniteWait ,False);
                return True;
            }
        }
        //Requesting the last configuration
        else if(content == "RetrieveLastConfiguration"){
            if(envelope->ReplyExpected()){
                GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
                GCRTemplate<Message>         gcrtm(GCFT_Create);
                gcrtm->Init(0, "OK");
                gcrtm->Insert(mc.lastReceivedCDB);
                gcrtme->PrepareReply(envelope, gcrtm);
                MessageHandler::SendMessage(gcrtme);
                return True;
            }
            return False;
        }
        else {
            //////////////////////////////////////////////
            // Check if Message is sent by local object //
            //////////////////////////////////////////////

            GCReference thread = mc.Find(sender.Buffer());
            // Received message from local thread
            if(thread.IsValid()){
                if(content == "ERROR"){
                    // Inform the other threads of the error state 
                    for(int i = 0; i < mc.realTimeThreads.Size(); i++){
                        GCRTemplate<RealTimeThread> rt = mc.Find(i);
                        if(rt.IsValid()){
                            if(sender == rt->Name()){
                                // Do not send the message to the sender as well
                            }else{

                                GCRTemplate<Message> gcrtm(GCFT_Create);
                                if(!gcrtm.IsValid()){
                                    mc.AssertErrorCondition(FatalError, "MARTeContainer::ProcessMessage: %s: Failed creating response message for Level1", mc.Name());
                                    return False;
                                }

                                GCRTemplate<MessageEnvelope> mec(GCFT_Create);
                                if (!mec.IsValid()){
                                    mc.AssertErrorCondition(FatalError,"MARTeContainer::ProcessMessage: %s: Failed creating copy for Level1", mc.Name());
                                    return False;
                                }

                                gcrtm->Init(0, "ERROR");

                                mec->PrepareMessageEnvelope(gcrtm,rt->Name());
                                MessageHandler::SendMessage(mec);
                            }
                        }
                    }
                    GMDSendMessageDeliveryRequest(mc.sendFatalErrorMessage, TTInfiniteWait ,False);
                }
            }else{

                if(content == "START"){
                    if(!mc.StartAllActivities()){
                        mc.AssertErrorCondition(FatalError,"ProcessMessage: %s: Failed Starting Activities",mc.Name());
                        GMDSendMessageDeliveryRequest(mc.sendFatalErrorMessage, TTInfiniteWait ,False);
                        return True;
                    }
                    return True;
                }

                if (content == "STOP"){
                    if(!mc.StopAllActivities()){
                        mc.AssertErrorCondition(FatalError,"ProcessMessage: %s: Failed Stopping Activities",mc.Name());
                        GMDSendMessageDeliveryRequest(mc.sendFatalErrorMessage, TTInfiniteWait ,False);
                        return True;
                    }
                    return True;
                }
            }
        }
    } else {
        mc.AssertErrorCondition(CommunicationError,"ProcessMessage: %s: Empty Message Envelope from %s", mc.Name(), envelope->Sender());
        return False;
    }

    if (envelope->ManualReplyExpected()){
        GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
        GCRTemplate<Message> gcrtm(GCFT_Create);

        gcrtme->PrepareReply(envelope,gcrtm);
        MessageHandler::SendMessage(gcrtme);
    }
    return True;
}

bool MARTeCProcessHttpMessage(MARTeContainer &mc,HttpStream &hStream){

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf("<html><head><title>%s</title></head><body>\n",mc.Name());
    hStream.Printf("<table\n>");

    for(int i = 0; i < mc.Size(); i++){
        GCRTemplate<HttpInterface> rt = mc.Find(i);
        if(rt.IsValid()){
            GCRTemplate<GCNamedObject> no(rt);
            if(no.IsValid())hStream.Printf("<tr><td><A HREF=\"%s\">%s</td></tr>\n",no->Name(),no->Name());
        }
    }

    hStream.Printf("</table\n>");
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);    
    return True;
}

bool MARTeContainer::ForwardMessageRequest(GCRTemplate<MessageEnvelope> gcrtme){
    if(!gcrtme.IsValid()){
        AssertErrorCondition(CommunicationError,"MARTeContainer::ForwardMessageRequest: Received invalid Message Envelope to be forwarded");
        return False;
    }
    GCRTemplate<Message> gcrtm = gcrtme->GetMessage();
    if(!gcrtm.IsValid()){
        AssertErrorCondition(CommunicationError,"MARTeContainer::ForwardMessageRequest: Received invalid Message to be forwarded");
        return False;
    }

    GCRTemplate<MessageEnvelope> newEnv(GCFT_Create);
    for(int i = 0; i < needStartStopMessage.Size(); i++){
        GCReference obj           = needStartStopMessage.Find(i);
        GCRTemplate<GCNamedObject> namedObject(obj);
        FString destinationName;
        GetUniqueName(destinationName);
        destinationName += ".";
        destinationName += namedObject->Name();

        if(namedObject.IsValid()){
            if(!newEnv->PrepareMessageEnvelope(gcrtm, destinationName.Buffer(), MDRF_ManualReply,this)){
                AssertErrorCondition(CommunicationError,"MARTeContainer::ForwardMessageRequest: Failed Preparing Message %s for %s", gcrtm->Content(), destinationName.Buffer());
                return False;
            }
            GCRTemplate<MessageEnvelope>   reply;
            SendMessageAndWait(newEnv,reply,commTimeout);
            if(!reply.IsValid()){
                AssertErrorCondition(CommunicationError,"MARTeContainer::ForwardMessageRequest: Failed Sending %s Message for %s",  gcrtm->Content(), destinationName.Buffer());
                return False;
            }
            GCRTemplate<Message> replyMessage = reply->GetMessage();
            if(!replyMessage.IsValid()){
                AssertErrorCondition(InitialisationError,"MARTeContainer::ForwardMessageRequest: %s: Object %s returned an invalid Message",Name(), destinationName.Buffer());
                return False;
            }
            FString replyContent = replyMessage->Content();
            if(replyContent == "ERROR"){
                AssertErrorCondition(InitialisationError,"MARTeContainer::ForwardMessageRequest: %s: Object %s returned ERROR Message",Name(), destinationName.Buffer());
                return False;
            }
        }else{
            AssertErrorCondition(CommunicationError,"MARTeContainer::ForwardMessageRequest: Object %d is not of GCNamedObject Type",i);
            return False;
        }
    }
    return True;
}

bool MARTeContainer::StartAllActivities(){
    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(1,"START");
    for(int i = 0; i < needStartStopMessage.Size(); i++){
        GCReference obj           = needStartStopMessage.Find(i);
        GCRTemplate<GCNamedObject> namedObject(obj);
        FString destinationName;
        GetUniqueName(destinationName);
        destinationName += ".";
        destinationName += namedObject->Name();
        if(namedObject.IsValid()){
            if(!gcrtme->PrepareMessageEnvelope(gcrtm, destinationName.Buffer(), MDRF_ManualReply, this)){
                AssertErrorCondition(CommunicationError,"MARTeContainer::StartAllActivities: Failed Preparing Start Message for %s", destinationName.Buffer());
                return False;
            }

            GCRTemplate<MessageEnvelope>   reply;
            SendMessageAndWait(gcrtme,reply,commTimeout);
            if(!reply.IsValid()){
                AssertErrorCondition(CommunicationError,"MARTeContainer::StartAllActivities: Failed Sending Start Message for %s", destinationName.Buffer());
                return False;
            }

            GCRTemplate<Message> replyMessage = reply->GetMessage();
            if(!replyMessage.IsValid()){
                AssertErrorCondition(InitialisationError,"MARTeContainer::StartAllActivities: %s: Object %s returned an invalid Message",Name(), destinationName.Buffer());
                return False;
            }
            FString replyContent = replyMessage->Content();
            if(replyContent == "ERROR"){
                AssertErrorCondition(InitialisationError,"MARTeContainer::StartAllActivities: %s: Object %s returned ERROR Message",Name(), destinationName.Buffer());
                return False;
            }
        } else {
            AssertErrorCondition(CommunicationError,"MARTeContainer::StartAllActivities: Object %d is not of GCNamedObject Type",i);
            return False;
        }
    }
    return True;
}

bool MARTeContainer::StopAllActivities(){
    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(1,"STOP");

    for(int i = 0; i < needStartStopMessage.Size(); i++){
        GCReference obj           = needStartStopMessage.Find(i);
        GCRTemplate<GCNamedObject> namedObject(obj);
        FString destinationName;
        GetUniqueName(destinationName);
        destinationName += ".";
        destinationName += namedObject->Name();
        if(namedObject.IsValid()){
            if(!gcrtme->PrepareMessageEnvelope(gcrtm, destinationName.Buffer(), MDRF_ManualReply, this)){
                AssertErrorCondition(CommunicationError,"MARTeContainer::StopAllActivities: Failed Preparing Stop Message for %s", destinationName.Buffer());
                return False;
            }

            GCRTemplate<MessageEnvelope>   reply;
            SendMessageAndWait(gcrtme,reply,commTimeout);
            if(!reply.IsValid()){
                AssertErrorCondition(CommunicationError,"MARTeContainer::StopAllActivities: Failed Sending Stop Message for %s", destinationName.Buffer());
                return False;
            }
            GCRTemplate<Message> replyMessage = reply->GetMessage();
            if(!replyMessage.IsValid()){
                AssertErrorCondition(InitialisationError,"MARTeContainer::StopAllActivities: %s: Object %s returned an invalid Message",Name(), destinationName.Buffer());
                return False;
            }
            FString replyContent = replyMessage->Content();
            if(replyContent == "ERROR"){
                AssertErrorCondition(InitialisationError,"MARTeContainer::StopAllActivities: %s: Object %s returned ERROR Message",Name(), destinationName.Buffer());
                return False;
            }

        }else{
            AssertErrorCondition(CommunicationError,"MARTeContainer::StopAllActivities: Object %d is not of GCNamedObject Type",i);
            return False;
        }
    }

    return True;
}

bool BrowseLevel1Message(StreamInterface &in,StreamInterface &out,void *userData){
    MARTeContainer *marte = (MARTeContainer *)userData;
    if(marte == NULL) return False;

    CDBBrowserMenu level1Browse;
    ConfigurationDataBase         cdb = marte->lastReceivedCDB;
    level1Browse.LinkTo("Level1 Message", cdb);
    return level1Browse.TextMenu(in,out);
}

bool DumpLevel1Message(StreamInterface &in,StreamInterface &out,void *userData){
    MARTeContainer *marte = (MARTeContainer *)userData;
    if(marte == NULL) return False;

    if(marte->lastReceivedCDB.IsValid()){
        FString fileName =  marte->Name();
        fileName += "Level1.cfg";
        File level1Dump;
        if(!level1Dump.OpenNew(fileName.Buffer())){
            out.Printf("DumpLevel1Message: Failed Opening File %s\n", fileName.Buffer());
            return False;
        }
        ConfigurationDataBase x(marte->lastReceivedCDB);
        x->WriteToStream(level1Dump,&out);
        level1Dump.Close();
        return True;
    }
    return True;
}

bool LoadLevel1Message(StreamInterface &in,StreamInterface &out,void *userData){
    MARTeContainer *marte = (MARTeContainer *)userData;
    if(marte == NULL) return False;

    FString fileName;
    in.GetToken(fileName,"\n\t");
    if(fileName.Size() == 0) return False;

    File level1Message;
    if(!level1Message.OpenRead(fileName.Buffer())){
        out.Printf("LoadLevel1Message: Cannot Open file %s \n", fileName.Buffer());
        return False;
    }

    ConfigurationDataBase level1;
    if(!level1->ReadFromStream(level1Message, &out)){
        out.Printf("LoadLevel1Message: ReadFromStream failed for file %s \n", fileName.Buffer());
        return False;
    }

    if(!marte->ProcessLevel1Message(level1)){
        out.Printf("LoadLevel1Message: ProcessLevel1Message failed for file %s \n", fileName.Buffer());
        return False;
    }

    return True;
}

bool MARTeContainer::ProcessLevel1Message(ConfigurationDataBase &level1){

    // Save a Copy
    lastReceivedCDB = level1;
    CDBExtended cdb(lastReceivedCDB);

    FString rebuildAll;
    if(!cdb.ReadFString(rebuildAll,"RebuildAll","0")) {
        cdb.ReadFString(rebuildAll,"ReloadAll","0");
    }
    if((rebuildAll == "1") || (rebuildAll == "True") || (rebuildAll == "TRUE")){
        if(!cdb->Move("+MARTe")){
            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: +MARTe node not found",Name());
            return False;
        }
        /* Stop All activities */        
        if(!StopAllActivities()){
            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: StopAllActivities Failed ",Name());
            return False;
        }

        /* Reconstruct All Objects */
        if(!ObjectLoadSetup(cdb,NULL)){
            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: ObjectLoadSetup Failed ",Name());
            return False;
        }
        /* Start All Activities */
        if(!StartAllActivities()){
            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: StartAllActivities Failed ",Name());
            return False;
        }

        cdb->MoveToRoot();
    }else{
        if(cdb->Exists("+MARTe")){
            cdb->Move("+MARTe");
        }

        int nOfChildren = cdb->NumberOfChildren();
        for(int i = 0; i < nOfChildren; i++){
            cdb->MoveToChildren(i);

            FString nodeName;
            cdb->NodeName(nodeName);

            FString classType;
            if(cdb.ReadFString(classType,"Class")){
                if(classType == "RealTimeThread"){

                    if(nodeName[0] == '+'){
                        nodeName = (nodeName.Buffer() + 1);
                    }

                    // Check that the Object is in the Container;
                    {
                        GCReference obj           = Find(nodeName.Buffer());
                        if(!obj.IsValid()){
                            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: Failed Finding %s Object in the list",Name(), nodeName.Buffer());
                            return False;
                        }
                    }

                    FString absNodeName = "MARTe."; 
                    absNodeName += nodeName;

                    // Send the message to the Object
                    {
                        GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
                        GCRTemplate<Message> gcrtm(GCFT_Create);
                        gcrtm->Init(0,"LEVEL1");
                        gcrtm->Insert(cdb);

                        if(!gcrtme->PrepareMessageEnvelope(gcrtm,absNodeName.Buffer(),MDRF_ManualReply,this)){
                            AssertErrorCondition(CommunicationError,"MARTeContainer::ForwardMessageRequest: Failed Preparing Start Message for %s", absNodeName.Buffer());
                            return False;
                        }

                        GCRTemplate<MessageEnvelope>   reply;
                        SendMessageAndWait(gcrtme,reply,TimeoutType(120000));
                        if(!reply.IsValid()){
                            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: Failed Sending Message to %s",Name(), absNodeName.Buffer());
                            return False;
                        }

                        GCRTemplate<Message> replyMessage = reply->GetMessage();
                        if(!replyMessage.IsValid()){
                            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: Object %s returned an invalid Message",Name(), absNodeName.Buffer());
                            return False;
                        }

                        FString replyContent = replyMessage->Content();
                        if(replyContent == "ERROR"){
                            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: Object %s returned ERROR Message",Name(), absNodeName.Buffer());
                            return False;
                        }
                    }
                }
            }
            cdb->MoveToFather();
        }

        if(!CreateMenuInterfaces()){
            AssertErrorCondition(InitialisationError,"MARTeContainer::ProcessLevel1Message: %s: Failed creating menu interfaces.", Name());
            return False;
        }

    }

    return True;
}

MARTeContainer::MARTeContainer(){
    commTimeout.SetTimeOutSec(2.0);

    // Create the Static Menu
    GCRTemplate <MenuContainer>  localMenu(GCFT_Create);
    marteStaticMenu                         = localMenu;
    if(!marteStaticMenu.IsValid()){
        AssertErrorCondition(FatalError,"MARTeContainer::MARTeContainer: marteStaticMenu is not valid");
        return;
    }

    /***************/
    /* Level1 Menu */
    /***************/
    GCRTemplate <MenuEntry>           dumpLevel1(GCFT_Create);
    if(!dumpLevel1.IsValid()){
        AssertErrorCondition(InitialisationError,"MARTeContainer::MARTeContainer: Failed creating MenuEntry class for Dumping Level1");
        return;
    }

    dumpLevel1->SetObjectName("DumpLevel1");
    dumpLevel1->SetTitle("Dump Level1 Message");
    dumpLevel1->SetUp(DumpLevel1Message,NULL,NULL, this);

    GCRTemplate <MenuEntry>           browseLevel1(GCFT_Create);
    if(!browseLevel1.IsValid()){
        AssertErrorCondition(InitialisationError,"MARTeContainer::MARTeContainer: Failed creating MenuEntry class for Browsing Level1");
        return;
    }

    browseLevel1->SetObjectName("BrowseLevel1");
    browseLevel1->SetTitle("Browse Level1 Message");
    browseLevel1->SetUp(BrowseLevel1Message,NULL,NULL, this);

#if 0
    GCRTemplate<DirectoryMenuBrowser> fileList(GCFT_Create);
    if(!fileList.IsValid()){
        AssertErrorCondition(InitialisationError,"MARTeContainer::MARTeContainer: Failed creating MenuEntry class for Level1 Load");
        return;
    }
    fileList->SetObjectName("Level1Load");
    fileList->SetTitle("Load Level1 Message");
    fileList->SetUp(LoadLevel1Message, this);
#endif
    GCRTemplate <MenuContainer>   level1Menu(GCFT_Create);
    if(!level1Menu.IsValid()){
        AssertErrorCondition(InitialisationError,"MARTeContainer::MARTeContainer: Failed creating MenuContainer class for Level1");
        return;
    }

    level1Menu->SetObjectName("Level1Menu");
    level1Menu->SetTitle("Level1");
    level1Menu->Insert(dumpLevel1);
    level1Menu->Insert(browseLevel1);
    //level1Menu->Insert(fileList);

    marteStaticMenu->SetObjectName("MARTeStaticMenu");
    marteStaticMenu->SetTitle("MARTe Static Menu");
    marteStaticMenu->Insert(level1Menu);
}

OBJECTLOADREGISTER(MARTeContainer,"$Id: MARTeContainer.cpp,v 1.38 2011/10/21 17:03:38 aneto Exp $")
