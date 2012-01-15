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

#include "MessageHandler.h"
#include "MessageEnvelope.h"
#include "GlobalObjectDataBase.h"
#include "MenuContainer.h"
#include "FString.h"
#include "InternetAddress.h"
#include "TCPSocket.h"
#include "MessageServer.h"

bool
MHConstructor(MessageHandler &mh){
    // by default create a thread
    mh.immediate   = False;

    mh.threadID    = (TID)0;
    mh.threadPriorityLevel = 0;
    mh.threadPriorityClass = 1;

    /** default of 10000ms */
    mh.globalTimeout            = 1000;
    mh.globalDestructionTimeout = 10000;

    mh.messageQueues.SetTimeout(mh.globalTimeout);
    GCRTemplate<MessageQueue> myMessageQueue(GCFT_Create);
    myMessageQueue->SetTimeout(mh.globalTimeout);
    mh.messageQueue = myMessageQueue;
    mh.messageQueues.Insert(myMessageQueue);

    mh.event.Create();
    mh.event.Reset();

    mh.threadIdMutex.Create();

    return True;
}

void  MessageHandlerThreadFN(void *arg){
    // create reference so nobody can delete the object
    MessageHandler *mh = (MessageHandler *)arg;

    // calls the method
    if (mh != NULL){

        Threads::SetPriorityLevel(mh->threadPriorityLevel);
        switch (mh->threadPriorityClass){
            case 0:{
                Threads::SetIdleClass();
            } break;
            case 2:{
                Threads::SetHighClass();
            } break;
            case 3:{
                Threads::SetRealTimeClass();
            } break;
            case 1:
            default:
            {
                Threads::SetNormalClass();
            } break;
        }

        mh->threadID = Threads::ThreadId();
        mh->event.Post();
        mh->keepAlive = True;
        while (mh->keepAlive){
            // do the task
            mh->ProcessMessageQueue();
        }

        mh->threadID = (TID) 0;
        mh->event.Post();
    }
}

void
MessageHandler::ProcessMessageQueue(){

    // take a new envelope
    GCRTemplate<MessageEnvelope> envelope = messageQueue->GetMessage(globalTimeout);

    // check validity and loop
    while (envelope.IsValid()){

        // automatic reply soon after reading the Q
        if (envelope->EarlyAutomaticReplyExpected()){
            GCRTemplate<MessageEnvelope> reply(GCFT_Create);
            reply->PrepareAutomaticReply(envelope);
            SendMessage(reply);
//            envelope->flags = envelope->flags & MDRF_ReplyNMask;
        }

        GCRTemplate< GCNOExtender<BString> > subAddressRef = envelope->Remove("SubAddress___!!!");
        const char *subAddress = NULL;
        if (subAddressRef.IsValid()){
            subAddress = subAddressRef->Buffer();
        }

        bool done = False;
        // no subAddres support for MenuMessages
        if (!subAddress){
            GCRTemplate<Message> message = envelope->GetMessage();
            if (message.IsValid()){
                if (message->GetMessageCode() == MenuMessage){
                    MenuInterface *mi = dynamic_cast<MenuInterface *>(this);
                    if (mi != NULL) {
                        done = mi->ProcessMenuMessage(envelope);
                    }
                }
            }
        }

        if (!done) {
            done = ProcessMessage2(envelope,subAddress);
        }

        // automatic reply after processing MSG
        if (envelope->LateAutomaticReplyExpected()){
            GCRTemplate<MessageEnvelope> reply(GCFT_Create);
            reply->PrepareAutomaticReply(envelope);
            SendMessage(reply);
        }

        // take a new one
        envelope = messageQueue->GetMessage(globalTimeout);
    }
}


bool
MHHandleMessage(MessageHandler &mh,GCRTemplate<MessageEnvelope> envelope,const char *subAddress)
{
    if (mh.immediate){
        return mh.ProcessMessage2(envelope,subAddress);
    }

    // if subAddress specified, store the information in the envelope
    if (subAddress != NULL){
        GCRTemplate< GCNOExtender<BString> > subAddressRef(GCFT_Create);
        subAddressRef->SetObjectName("SubAddress___!!!");
        BString *bs = subAddressRef.operator->();
        (*bs) = subAddress;
        envelope->Insert(subAddressRef);
    }

    mh.threadIdMutex.Lock(mh.globalTimeout);
    if (mh.threadID == 0){

        // label this thread
        FString threadName;
        GCNamedObject *gcno = dynamic_cast<GCNamedObject *>(&mh);
        if (gcno){
            threadName.Printf("MessageHandlerFor%s",gcno->Name());
        } else {
            threadName.Printf("MessageHandlerFor[0x%x]",&mh);
        }

        mh.keepAlive = True;
        // create a new one
        mh.threadID = Threads::BeginThread(MessageHandlerThreadFN,&mh,THREADS_DEFAULT_STACKSIZE,threadName.Buffer());

        // wait for thread start notification
        if (!mh.event.Wait(mh.globalTimeout)){
            CStaticAssertErrorCondition(Timeout,"MHHandleMessage: Message Handling Thread did not start within %i msecs",mh.globalTimeout.msecTimeout);

            return False;
        }
    }
    mh.threadIdMutex.UnLock();

    // lock everything!
    if (!mh.messageQueues.Lock2(mh.globalTimeout)){
        CStaticAssertErrorCondition(Timeout,"MHHandleMessage messageQueues.Lock() failed");
        return False;
    }

    // find top Q
    GCRTemplate<MessageQueue> topQueue = mh.messageQueues.Find(0);

    if (topQueue.IsValid()){
    // add new envelope
        topQueue->SendMessage(envelope,mh.globalTimeout);
    } else {
        CStaticAssertErrorCondition(Timeout,"MHHandleMessage cannot get     present top Q (timeout)");
        mh.messageQueues.UnLock();
        return False;
    }

    mh.messageQueues.UnLock();

    return True;
}

bool
MHSendMessageRemotely(
                        GCRTemplate<MessageEnvelope>    envelope,
                        const char *                    serverAddress,
                        int                             serverPort)
{

    int32 replyPort = MSGetServerPort();
    if (replyPort == 0){
        CStaticAssertErrorCondition(FatalError, "MHSendMessageRemotely:no MessageServer installed");
        return False;
    }

    // convert objects to string
    ConfigurationDataBase cdb;
    FString err;

    if (!envelope.ObjectSaveSetup(cdb,&err)){
        CStaticAssertErrorCondition(FatalError, "MHSendMessageRemotely:failed converting envelope to cdb: %s",err.Buffer());
        return False;
    }

    // amend source in the CDB (cannot modify private data in Envelope)
    const char *sender = envelope->Sender();
    FString fullSenderAddress;
    fullSenderAddress.Printf("(@%s:%i).%s",InternetAddress::LocalAddress(),replyPort  ,sender);
    cdb->WriteArray(&fullSenderAddress,CDBTYPE_FString,NULL,1,"Sender");

    TCPSocket socket;
    if(!socket.Open()){
        //
        return False;
    }
    if(!socket.Connect(serverAddress,serverPort)){
        //
        return False;
    }
    if(!cdb->WriteToStream(socket)){
        //
        return False;
    }
    if(!socket.Close()){
        //
        return False;
    }

    /*if(!socket.CompleteWrite(message.Buffer(),size)){
    }*/

/*    FString message;
    cdb->WriteToStream(message);
//printf(" sending \n%s\n to %s:%i\n",message.Buffer(),serverAddress,serverPort);
    message +='\n';
    uint32 size = message.Size()+1;

    UDPSocket socket;
    socket.Open();
    socket.Connect(serverAddress,serverPort);
    socket.BasicWrite(message.Buffer(),size);
    socket.Close();*/
    return True;
}

/** */
bool
MHSendMessage(
                        GCRTemplate<MessageEnvelope>    gcrtme)
{
    if (!gcrtme.IsValid()){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessage: reference to envelope is not valid");
        return False;
    }

    const char *unMatched = NULL;

    const char *destination = gcrtme->Destination();

    // detect remote targets
    if (strncmp("(@",destination,2)==0){

        // skip (@
        FString destination = (gcrtme->Destination()+2);
        FString serverAddress;
        char term = 0;
        destination.Seek(0);
        // search for : or ) do not expect . or 0
        destination.GetToken(serverAddress,":)",&term);

        if   ((term ==  0  ) ||
              (term ==  '.')) {
            CStaticAssertErrorCondition(FatalError,"MHSendMessageRemotely: syntax error in destination %s",gcrtme->Destination());
            return False;
        }

        // default
        int32 serverPort = 8888;
        if    (term =  ':') {
            FString serverPortString;
            // search for : or ) do not expect . or 0
            destination.GetToken(serverPortString,".)",&term);

            if   ((term ==  0  ) ||
                  (term ==  '.')) {
                CStaticAssertErrorCondition(FatalError,"MHSendMessageRemotely: syntax error in destination (part 2) %s",gcrtme->Destination());
                return False;
            }

            serverPort = atoi(serverPortString.Buffer());
        }

        // remove the specification of server and the '.'
        gcrtme->SetDestination(destination.Buffer()+destination.Position()+1);

        return MHSendMessageRemotely(gcrtme,serverAddress.Buffer(),serverPort);
    }

    // Find the object globally by name
    GCReference gc = GODBFindByName(gcrtme->Destination(),&unMatched);
    if (!gc.IsValid()){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessage: failed finding an object named %s",gcrtme->Destination());
        return False;
    }

    // check for a MessageHandler interface
    GCRTemplate<MessageHandler> gcrtmh = gc;
    if (!gcrtmh.IsValid()){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessage: object named %s has no MessageHandler capability",gcrtme->Destination());
        return False;
    }

    // report unmatched find
    if (unMatched){
        CStaticAssertErrorCondition(Information,"MessageHandler::SendMessage: object named %s partial matched (%s unmatched)",gcrtme->Destination(),unMatched);
    }

    return gcrtmh->HandleMessage(gcrtme,unMatched);

}

bool MHSendMessageAndWait(
                        MessageHandler &                mh,
                        GCRTemplate<MessageEnvelope>    envelope,
                        GCRTemplate<MessageEnvelope> &  reply,
                        TimeoutType                     timeout
                    )
{

    if (!envelope.IsValid()){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait no message envelope !");
        return False;
    }

    GCRTemplate<Message> originalMessage;
    originalMessage = envelope->GetMessage();

    if (!originalMessage.IsValid()){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait no message in envelope");
        return False;
    }

    /* create own message Q */
    GCRTemplate<MessageQueue> myQueue(GCFT_Create);
    myQueue->SetTimeout(timeout);

    /* present top Q */
    GCRTemplate<MessageQueue> topQueue;

    FString name;
    name.Printf("THREAD%s_MSG_QUEUE%08x",Threads::GetName,myQueue.operator->());
    myQueue->SetObjectName(name.Buffer());

    if (!mh.messageQueues.Lock2(timeout)){
        CStaticAssertErrorCondition(Timeout,"MessageHandler::SendMessageAndWait waitQueues.Lock() failed");
        return False;
    }

    if (mh.messageQueues.Size()<=0){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait no top Q present!!!");
        mh.messageQueues.UnLock();
        return False;
    }

    topQueue = mh.messageQueues.Find(0);
    if (!topQueue.IsValid()){
        CStaticAssertErrorCondition(Timeout,"MessageHandler::SendMessageAndWait cannot get present top Q (timeout)");
        mh.messageQueues.UnLock();
        return False;
    }

    if (!mh.messageQueues.Insert(myQueue,0)){
        CStaticAssertErrorCondition(Timeout,"MessageHandler::SendMessageAndWait waitQueues.Insert(Queue) failed");
        mh.messageQueues.UnLock();
        return False;
    }

    mh.messageQueues.UnLock();

    if (!envelope->ReplyExpected()){
        CStaticAssertErrorCondition(Warning,"MessageHandler::SendMessageAndWait no message reply policy -> MDRF_EarlyAutomaticReply");
        mh.EnableReplyOnEnvelope(envelope);
    }

    bool ok = True;
    if (!mh.SendMessage(envelope)){
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait SendMessage() failed");
        ok = False;
    } else {

        // take a new envelope
        reply = myQueue->GetMessage(timeout);

        // check validity and loop
        while (reply.IsValid()){

            GCRTemplate<Message> message;
            if (reply->AutomaticReply()) {
                message = reply->GetMessage();
            } else {
                message = reply->GetOriginal();
            }
            if (message.IsValid()){
                if (message == originalMessage){
                    break;
                }
            }

            // pass the message forward
            topQueue->SendMessage(message,timeout);

            // take a new one
            reply = myQueue->GetMessage(timeout);
        }
    }

    if (!mh.messageQueues.Lock2(timeout)){
        CStaticAssertErrorCondition(Timeout,"MessageHandler::SendMessageAndWait: messageQueues.Lock() failed");
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait: handler dead: bad Queue on top!!!");
        return False;
    }

    //flush Queue to next
    while (myQueue->Size() > 0){
        // take a new envelope
        GCRTemplate<MessageEnvelope> me = myQueue->GetMessage(timeout);

        // check validity and loop
        if (me.IsValid()){

            // pass the message forward
            if (!topQueue->SendMessage(me,timeout)){
                CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait failed passing message on to top Q!!");
            }

        } else {
            CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait flushing Queue found bad object!!");
        }
    }


    GCReference removed = mh.messageQueues.Remove(name.Buffer());
    if (!removed.IsValid()){
        CStaticAssertErrorCondition(Timeout,"MessageHandler::SendMessageAndWait waitQueues.Remove(Queue) failed");
        CStaticAssertErrorCondition(FatalError,"MessageHandler::SendMessageAndWait: handler dead: bad Queue on top!!!");
        ok =  False;
    }

    mh.messageQueues.UnLock();

    return ok;

}





