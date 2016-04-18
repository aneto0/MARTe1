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
 * $Id: MDSWriterDrv.cpp 3 2012-01-15 16:26:07Z aneto $
 *
 **/

#include "MDSWriterDrv.h"
#include "GlobalObjectDataBase.h"
#include "mdsobjects.h"
#include "stdio.h"

using namespace std;
/**
 * Receiver CallBack
 */
static void MDSThreadWriterCallback(void* userData){
    MDSWriterDrv *p = (MDSWriterDrv*)userData;   
    p->WriteDataToMDS();
}

void MDSWriterDrv::WriteDataToMDS(){
    //Counter for shared buffers
    int32 i = 0;
    //Counter for number of signals 
    int32 j = 0;
    bool ok = false;
    running = True;
    int64 lastWriteSize = 0;
    int64 lastTimeRefreshCount = HRT::HRTCounter();
    int64 refreshEveryCounts = refreshTime * HRT::HRTFrequency();
    uint32 offset = 0;
    while(running){
        //Wait for the next buffer to be free
        //sharedBufferMux.FastLock();
        while(!sharedBufferMux.FastTryLock());        
        sharedBufferSem.Reset();
        sharedBufferMux.FastUnLock();
        sharedBufferSem.Wait();
        while(sharedBuffer[i][0] == 1) {
            offset = 2;
            for(j = 0; j < numberOfOutputNodes; j++) {
                nodes[j]->Write((void *)(sharedBuffer[i] + offset));
                offset += nodes[j]->GetNumberOfWords();
            }
            //sharedBufferMux.FastLock();
            while(!sharedBufferMux.FastTryLock());
            sharedBuffer[i][0] = 0;
            numberOfFreeBuffers++;
            sharedBufferMux.FastUnLock();
            lastWriteSize = 0;
            i++;
            if(i == numberOfBuffers){
                i = 0;
            }
        }
        if((HRT::HRTCounter() - lastTimeRefreshCount) > refreshEveryCounts){
            lastTimeRefreshCount = HRT::HRTCounter();	
            if(!(eventName == "")){
                MDSplus::Event::setEvent((char *)eventName.Buffer());
            }
        }

    }

    running = True;
}


bool MDSWriterDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    AssertErrorCondition(Information, "MDSWriterDrv::ObjectLoadSetup: %s Loading signals", Name());

    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"MDSWriterDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return false;
    }

    if(numberOfInputChannels != 1) {
        AssertErrorCondition(InitialisationError,"MDSWriterDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed. Set NumberOfInputs=1 to account for the statistics channels",Name());
        return false;
    }
    if(!cdb.ReadFString(treeName, "TreeName")){
        AssertErrorCondition(InitialisationError, "MDSWriterDrv::ObjectLoadSetup: %s Failed reading TreeName", Name());
        return false;
    }
    if(!cdb.ReadInt32(pulseNumber, "PulseNumber")){
        AssertErrorCondition(InitialisationError, "MDSWriterDrv::ObjectLoadSetup: %s pulseNumber not specified", Name());
        return false;
    }
    cdb.ReadFString(eventName, "EventName", "updatejScope");
    if(!cdb.ReadInt32(refreshTime, "TimeRefresh", 5)){
        AssertErrorCondition(Warning, "MDSWriterDrv::ObjectLoadSetup: %s refreshTime not specified. Using default %d", Name(), refreshTime);
    }
    //Check for the latest pulse number
    if(pulseNumber == -1){
        try {
            tree = new MDSplus::Tree(treeName.Buffer(), -1);
            pulseNumber = tree->getCurrent(treeName.Buffer());
            pulseNumber++;
            tree->setCurrent(treeName.Buffer(), pulseNumber);
            tree->createPulse(pulseNumber);	
            delete tree; 
        }
        catch(MDSplus::MdsException &exc){
            pulseNumber = 1;
            delete tree;
            tree = NULL;
        }

    }
    //Create a pulse. It assumes that the tree template is already created!!
    try {
        tree = new MDSplus::Tree(treeName.Buffer(), pulseNumber);
    }
    catch(MDSplus::MdsException &exc){
        AssertErrorCondition(Warning, "MDSWriterDrv::ObjectLoadSetup: %s Failed opening tree %s with the pulseNUmber = %d. Trying to create pulse", Name(), treeName.Buffer(), pulseNumber);
        delete tree;
        tree = NULL;
    }
    if(tree == NULL){
        try{
            tree = new MDSplus::Tree(treeName.Buffer(), -1);
            tree->setCurrent(treeName.Buffer(), pulseNumber);
            tree->createPulse(pulseNumber);	
        }
        catch(MDSplus::MdsException &exc){
            AssertErrorCondition(FatalError, "MDSWriterDrv::ObjectLoadSetup: %s Failed opening tree %s with the pulseNUmber = %d", Name(), treeName.Buffer(), pulseNumber);
            delete tree;
            return False;
        }
    } 
    delete tree;

    //Open a pulse.
    try {
        tree = new MDSplus::Tree(treeName.Buffer(), pulseNumber);
    }
    catch(MDSplus::MdsException &exc){
        AssertErrorCondition(FatalError, "MDSWriterDrv::ObjectLoadSetup: %s Failed opening tree %s with the pulseNUmber = %d", Name(), treeName.Buffer(), pulseNumber);
        return false;
    } 
    numberOfOutputNodes = Size();
    if(numberOfOutputNodes == 0){
        AssertErrorCondition(Warning, "MDSWriterDrv::ObjectLoadSetup: %s no signals specified. Returning...", Name());
        return true;
    }
    nodes = new MDSWriterNode*[numberOfOutputNodes];
    uint32 i=0;
    bool ok = true;
    sharedBufferSizeWords = 2; //Synch + counter
    for(i=0; ok && (i<numberOfOutputNodes); i++){
        GCRTemplate<MDSWriterNode> node = Find(i);
        if(node.IsValid()){
            nodes[i] = node.operator->();
            ok = nodes[i]->AllocateTreeNode(tree);
            sharedBufferSizeWords += nodes[i]->GetNumberOfWords();
        }
        else{
            AssertErrorCondition(FatalError, "MDSWriterDrv::ObjectLoadSetup: %s found something that is not a MDSTreeNode. Returning...", Name());
            ok = false;
        }
    }
    if(!ok){
        return false;
    } 
    numberOfOutputChannels = sharedBufferSizeWords - 2;
    ////////////////////////////////
    //Initialization of the buffer//
    ////////////////////////////////
    if(!cdb.ReadInt32(numberOfBuffers, "NumberOfBuffers", -1)){
        AssertErrorCondition(InitialisationError,"MDSWriterDrv::ObjectLoadSetup: %s NumberOfBuffers was not specified",Name());
        return false;
    }
    if(numberOfBuffers < 1){
        AssertErrorCondition(InitialisationError,"MDSWriterDrv::ObjectLoadSetup: %s at least one buffer must be specified. NumberOfBuffers = %d",Name(), numberOfBuffers);
        return false;
    }
    numberOfFreeBuffers = numberOfBuffers;
    //Allocate the shared memory
    sharedBuffer = new int32*[numberOfBuffers];
    if(sharedBuffer == NULL){
        AssertErrorCondition(FatalError, "MDSWriterDrv::ObjectLoadSetup: %s Failed to allocated sharedBuffer for %d buffers", Name(), numberOfBuffers);
        return false;
    }
    for(i=0; i < numberOfBuffers; i++){
        sharedBuffer[i] = new int32[sharedBufferSizeWords];
        sharedBuffer[i][0] = 0;
    }

    if(!cdb.ReadInt32(cpuMask, "CpuMask", 0xFFFF)){
        AssertErrorCondition(Warning,"UDPDrv::ObjectLoadSetup: %s CpuMask was not specified. Using default: %d",Name(),cpuMask);
    }
    if(cdb.ReadInt32(threadPriority, "ThreadPriority", 31)) {
        if(threadPriority > 32 || threadPriority < 0) {
            AssertErrorCondition(InitialisationError, "UDPDrv::ObjectLoadSetup: %s ThreadPriority parameter must be <= 32 and >= 0", Name());
            return false;
        }
    }
    running = false;
    FString threadName = Name();
    threadName += "_WriteDataToMDS";

    threadID = Threads::BeginThread((ThreadFunctionType)MDSThreadWriterCallback, (void*)this, THREADS_DEFAULT_STACKSIZE * 100, threadName.Buffer(), XH_NotHandled, cpuMask);
    int counter = 0;
    while((!running) && (counter++ < 100)) {
        SleepMsec(1);
    }
    if(!running) {
        AssertErrorCondition(InitialisationError, "MDSWriterDrv::ObjectLoadSetup:%s WriteDataToMDS failed to start", Name());
        return false;
    }
    return True;
}

bool MDSWriterDrv::StopWriteDataToMDSThread(){
    running = false;
    uint32 counter = 0;
    while(!running){
        if(counter++ > 100){
            AssertErrorCondition(FatalError, "MDSWriterDrv::ObjectLoadSetup:%s WriteDataToDisk failed to stop after 10 s", Name());
            running = false;
            return false;
        }
        SleepMsec(100);
    } 
    running = false;
    return True;
}

bool MDSWriterDrv::WriteData(uint32 usecTime, const int32 *ibuffer){
    if(!running){
        AssertErrorCondition(FatalError,"MDSWriterDrv::WriteData: %s consumer thread is not running. For each run you are forced to upload a new configuration...",Name());
        return false;
    }
    //Buffer overrun! Fully cycle of writing in the shared buffer performed and the consumer thread
    //has still not consumed the latest buffer...
    while(!sharedBufferMux.FastTryLock());
    if(sharedBuffer[lastWriteIdx][0] != 0){
        AssertErrorCondition(FatalError,"MDSWriterDrv::WriteData: %s sharedbuffer overwritten for index: %d. Data lost. Try increasing number of buffers.",Name(),lastWriteIdx);
        sharedBufferMux.FastUnLock();
        return false;
    }	
    sharedBufferMux.FastUnLock();
    //add counter counter to the sharedBuffer
    sharedBuffer[lastWriteIdx][1] = usecTime;
    memcpy(&sharedBuffer[lastWriteIdx][2], ibuffer, (sharedBufferSizeWords - 2) * sizeof(int32));

    //sharedBufferMux.FastLock();
    while(!sharedBufferMux.FastTryLock());    
    sharedBuffer[lastWriteIdx][0] = 1;
    numberOfFreeBuffers--;
    sharedBufferSem.Post();
    sharedBufferMux.FastUnLock();
    lastWriteIdx++;
    if(lastWriteIdx == numberOfBuffers){
        lastWriteIdx = 0;
    }
    return True;
}

bool MDSWriterDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = false;
    //copy to the client
    hStream.WriteReplyHeader(false);

    hStream.Printf("<html><head><title>%s</title>\n", Name());
    hStream.Printf("<style>\n");
    hStream.Printf("table, td, tr {\n");
    hStream.Printf("    border: 1px solid black;\n");
    hStream.Printf("    table-layout: fixed;\n");
    hStream.Printf("    width: 20px;\n");
    hStream.Printf("    height: 20px\n");
    hStream.Printf("}\n");
    hStream.Printf("</style>\n");
    hStream.Printf("</head>\n");
    hStream.Printf("<body>\n");

    hStream.Printf("<h1>Shared buffer status</h1>\n");

    hStream.Printf("Number of free buffers = %d\n<br>\n", numberOfFreeBuffers);
    /* Data table */
    hStream.Printf("<table border=\"1\" align=\"left\">\n");
    int32 i=0;
    for(i=0; i<numberOfBuffers; i++){
        if(i % 40 == 0){
            if(i != 0){
                hStream.Printf("</tr>\n");
            }
            hStream.Printf("<tr>\n");
        }
        if(sharedBuffer[i][0] == 0){
            hStream.Printf("<td bgcolor=\"#00FF00\"></td>\n");
        }
        else{
            hStream.Printf("<td bgcolor=\"#FF0000\"></td>\n");
        }
    }
    hStream.Printf("</tr>\n");
    hStream.Printf("</table>\n");
    hStream.Printf("</body></html>\n");

}

int32 MDSWriterDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {
    memcpy(buffer, &numberOfFreeBuffers, sizeof(int32));
    return 0;
}

bool MDSWriterDrv::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
    if(!envelope.IsValid()){
        AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Received invalid envelope", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    FString sender               = envelope->Sender();
    if (message.IsValid()){
        int32   code    = message->GetMessageCode().Code();
        FString content = message->Content();
        ConfigurationDataBase cdb = message->Find(0);
        CDBExtended cdbe(cdb);
        if(!cdbe.IsValid()){
            AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: CDBExtended invalid", Name());
            return false;
        }
        MDSplus::TreeNode *node;
        try{
            node = tree->getNode(content.Buffer());
        }
        catch(MDSplus::MdsException &exc){
            AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: tree->getNode failed", Name());
            return False;
        }

        int32 append;
        cdbe.ReadInt32(append, "Append", 0);
        FString type;
        if(!cdbe.ReadFString(type, "Type")){
            AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Type not specifed", Name());
            return False;
        }

        MDSplus::Data *data = NULL;
        if(type == "int32"){
            int32 value;
            if(!cdbe.ReadInt32(value, "Value")){
                AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Value not specifed", Name());
                return False;
            }
            data = new MDSplus::Int32(value);
        }
        else if(type == "float"){
            float value;
            if(!cdbe.ReadFloat(value, "Value")){
                AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Value not specifed", Name());
                return False;
            }
            data = new MDSplus::Float32(value);
        }
        else if(type == "double"){
            double value;
            if(!cdbe.ReadDouble(value, "Value")){
                AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Value not specifed", Name());
                return False;
            }
            data = new MDSplus::Float64(value);
        }
        else if(type == "string"){
            FString msg;
            if(!cdbe.ReadFString(msg, "Value")){
                AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Value not specifed", Name());
                return False;
            }
            FString oldMsg;
            if(append){
                try{
                    oldMsg = node->getData()->getString();
                    oldMsg += "\n";
                }
                catch(MDSplus::MdsException &exc){
                    oldMsg = "";
                }
            }
            oldMsg += msg;
            data = new MDSplus::String(oldMsg.Buffer());
        }
        else{
            AssertErrorCondition(FatalError, "MDSWriterDrv::ProcessMessage: %s: Unknown Type specifed", Name());
            return False;
        }
        node->putData(data);
        MDSplus::deleteData(data);
        delete node;
    }
    return true;
}

OBJECTLOADREGISTER(MDSWriterDrv,"$Id: MDSWriterDrv.cpp 3 2012-01-15 16:26:07Z aneto $")

