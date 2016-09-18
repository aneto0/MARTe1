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
 * $Id: SDNDrv.cpp 456 2016-04-14 10:56:48Z aneto $
 *
**/

#include "SDNDrv.h"

#include "Endianity.h"
#include "CDBExtended.h"
#include "FastPollingMutexSem.h"

/**
 * Enable System Acquisition
 */
bool SDNDrv::EnableAcquisition(){
    // Initialise lastPacketID equal to 0xFFFFFFFF
    lastPacketID = 0xFFFFFFFF;
    if (liveness != -1){
        AssertErrorCondition(Warning, "SDNDrv::EnableAcquisition: SDN socket already alive");
        return False;
    }


    liveness++;
    return True;
}

/**
 * Disable System Acquisition
 */
bool SDNDrv::DisableAcquisition(){
    if (liveness==-1){
        AssertErrorCondition(Warning, "SDNDrv::DisableAcquisition: SDN socket not alive");
        return False;
    }
    liveness=-1;
    return True;
}

/*
 * Constructors
 */
bool  SDNDrv::Init(){
    // Init general parameters
    moduleType                          = SDN_MODULE_UNDEFINED;
    mux.Create();
    payloadByteSize                      = 0;
    // Init receiver parameters
    writeBuffer                         = 0;
    globalReadBuffer                    = 0;
    sizeMismatchErrorCounter            = 0;
    previousPacketTooOldErrorCounter    = 0;
    deviationErrorCounter               = 0;
    lostPacketErrorCounter              = 0;
    lostPacketErrorCounterAux           = 0;
    samePacketErrorCounter              = 0;
    recoveryCounter                     = 0;
    maxDataAgeUsec                      = 20000;
    lastPacketID                        = 0xFFFFFFFF;
    lastPacketUsecTime                  = 0;
    liveness                            = -1;
    keepRunning                         = False;
    producerUsecPeriod                  = -1;
    originalNSampleNumber               = 0;
    lastCounter                         = 0;
    cpuMask                             = 0xFFFF;
    receiverThreadPriority              = 0;
    packetNumber                        = 0;
    freshPacket                         = False;
    applyEndianity                      = True;

    interfaceName.SetSize(0);
    topicName.SetSize(0);
    multicastGroup.SetSize(0);
    multicastPort = 0;

    publisher = NULL;
    subscriber = NULL;

    // reset all buffers pointers 
    for(int i = 0 ; i < nOfDataBuffers ; i++) {
        dataBuffer[i] = NULL;
    }
    return True;
}

SDNDrv::SDNDrv(){
    Init();
}

/*
 * Destructor
 */
SDNDrv::~SDNDrv(){
    keepRunning = False;
    DisableAcquisition();
    if(moduleType == SDN_MODULE_RECEIVER) {
        int counter = 0;
        while((!keepRunning) && (counter++ < 100)) SleepMsec(1);
        if(Threads::IsAlive(threadID)) {
            AssertErrorCondition(Warning,"SDNDrv::~SDNDrv: %s: Had To Kill Thread %d",Name(), threadID);
            Threads::Kill(threadID);
            threadID = 0;
        }
        else{
            AssertErrorCondition(Information,"SDNDrv::~SDNDrv: %s: Successfully waited for Thread %d to die on its own",Name(), threadID);
        }
    }

    // Free memory
    for(int i = 0 ; i < nOfDataBuffers ; i++) {
        if(dataBuffer[i] != NULL)free((void *&)dataBuffer[i]);
    }        	

    if(publisher != NULL){
        delete publisher;
    }

    if(subscriber != NULL){
        delete subscriber;
    }
}

/**
 * ObjectLoadSetup
 */
bool SDNDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){    
    // Disable previous opened connections
    DisableAcquisition();
    // Parent class Object load setup
    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"SDNDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }
    // Read ModuleType IN/OUT
    FString module;
    if(!cdb.ReadFString(module,"ModuleType")){
        AssertErrorCondition(InitialisationError,"SDNDrv::ObjectLoadSetup: %s did not specify ModuleType entry",Name());
        return False;
    }
    if(module == "InputModule"){
        moduleType = SDN_MODULE_RECEIVER;
    }else if(module == "OutputModule"){
        moduleType = SDN_MODULE_TRANSMITTER;
    }else{
        AssertErrorCondition(InitialisationError,"SDNDrv::ObjectLoadSetup: %s unknown module type %s",Name(),module.Buffer());
        return False;
    }
    cpuMask = 0xFFFF;

    //Read connection parameters
    if(!cdb.ReadFString(interfaceName, "InterfaceName")){
        AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s InterfaceName was not specified", Name());
        return false;
    }
    if(!cdb.ReadFString(topicName, "TopicName")){
        AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s TopicName was not specified", Name());
        return false;
    }
    if(!cdb.ReadFString(multicastGroup, "MulticastGroup")){
        AssertErrorCondition(Information, "SDNDrv::ObjectLoadSetup: %s MulticastGroup was not specified. It will be automatically computed.", Name());
    }
    if(!cdb.ReadInt32((int32 &)multicastPort, "MulticastPort")){
        if(multicastGroup.Size() > 0){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s MulticastPort was not specified but MulticastGroup was.", Name());
            return false;
        }
        else{
            AssertErrorCondition(Information, "SDNDrv::ObjectLoadSetup: %s MulticastPort was not specified. It will be automatically computed.", Name());
        }
    }

    sdn::Metadata_t mdata; 
    sdn::Topic_InitializeMetadata(mdata); 
    sstrncpy(mdata.name, (char *)topicName.Buffer(), STRING_MAX_LENGTH);
    mdata.mcast_port = multicastPort;
    if(multicastGroup.Size() > 0) {
        sstrncpy(mdata.mcast_group, (char *)multicastGroup.Buffer(), MAX_IP_ADDR_LENGTH);
        mdata.mcast_port = multicastPort;
    }
    else {
        if(Topic_GenerateMCastAddress(mdata) != STATUS_SUCCESS){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s Topic_GenerateMCastAddress.", Name());
            return false;
        }
    }
    AssertErrorCondition(Information, "SDNDrv::ObjectLoadSetup: %s Topic: %s Group:%s:%d.", Name(), topicName.Buffer(), mdata.mcast_group, mdata.mcast_port);
    //Compute the signals
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s No Signals declared.", Name());
        return false;
    }

    uint32 numberOfSignals = cdb->NumberOfChildren();
    uint32 n;
    for(n = 0; n<numberOfSignals; n++){
        cdb->MoveToChildren(n);
        FString signalName;
        cdb->NodeName(signalName);
        FString signalType;
        if(!cdb.ReadFString(signalType, "SignalType")){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: SignalType not declared for signal %s.", signalName.Buffer());
            return false;
        }
        TypeIdentifier_t type;
        if(signalType == "int32"){
            type = ANYTYPE_ATTR_SINT32;
        }
        else if(signalType == "uint32"){
            type = ANYTYPE_ATTR_UINT32;
        }
        else if(signalType == "float"){
            type = ANYTYPE_ATTR_FLOAT;
        }
        else{
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: SignalType not supported for signal %s.", signalName.Buffer());
            return false;
        }
        payload.AddAttribute((uint_t)n, (char *)signalName.Buffer(), type);
        cdb->MoveToFather();
    }
    cdb->MoveToFather();
    payloadByteSize = numberOfSignals * sizeof(uint32);
    mdata.size = payloadByteSize;

    if(moduleType == SDN_MODULE_TRANSMITTER){
        numberOfOutputChannels = numberOfSignals;
        publisher = new sdn::Publisher(mdata);
        if(publisher->SetInterface((char *)interfaceName.Buffer()) != STATUS_SUCCESS){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s SetInterface failed for interface %s.", Name(), interfaceName.Buffer());
            return false;
        }
        if(publisher->Configure() != STATUS_SUCCESS){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s subscriber->Configure() failed.", Name());
            return false;
        }
        payload.SetInstance(publisher->GetTopicInstance());

    }
    else{
        numberOfInputChannels = numberOfSignals;
        subscriber = new sdn::Subscriber(mdata);

        // Read cpu mask
        if(!cdb.ReadInt32(cpuMask, "CpuMask", 0xFFFF)){
            AssertErrorCondition(Warning,"SDNDrv::ObjectLoadSetup: %s CpuMask was not specified. Using default: %d",Name(),cpuMask);
        }

        // Read MaxDataAgeUsec param
        if (!cdb.ReadInt32(maxDataAgeUsec, "MaxDataAgeUsec", 20000)){
            AssertErrorCondition(Warning,"SDNDrv::ObjectLoadSetup: %s did not specify MaxDataAgeUsec entry. Assuming %i usec" ,Name(),maxDataAgeUsec);
        }
        // Read MacNOfLostPackets
        cdb.ReadInt32(maxNOfLostPackets, "MaxNOfLostPackets", 4);

        if(cdb.ReadInt32(receiverThreadPriority, "ThreadPriority", 0)) {
            if(receiverThreadPriority > 32 || receiverThreadPriority < 0) {
                AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s ThreadPriority parameter must be <= 32 and >= 0", Name());
                return False;
            }
        } else {
            AssertErrorCondition(Warning, "SDNDrv::ObjectLoadSetup: %s ThreadPriority parameter not specified", Name());
        }

        /// Read the UsecPeriod of the SDN packet producer
        cdb.ReadInt32(producerUsecPeriod, "ProducerUsecPeriod", -1);

        // Create Data Buffers. Compute total size and allocate storing buffer 
        for(int i=0 ; i < nOfDataBuffers ; i++){
            dataBuffer[i] = (uint32 *)malloc(numberOfInputChannels*sizeof(int));
            if(dataBuffer[i] == NULL){
                AssertErrorCondition(InitialisationError,"SDNDrv::ObjectLoadSetup: %s SDN dataBuffer allocation failed",Name());
                return False;
            }
            // Initialize the triple buffer
            uint32 *tempData = dataBuffer[i];
            for(int j = 0 ; j < numberOfInputChannels ; j++) {
                tempData[j] = 0;
            }
        }
        if(subscriber->SetInterface((char *)interfaceName.Buffer()) != STATUS_SUCCESS){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s SetInterface failed for interface %s.", Name(), interfaceName.Buffer());
            return false;
        }
        if(subscriber->Configure() != STATUS_SUCCESS){
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: %s subscriber->Configure() failed.", Name());
            return false;
        }
        payload.SetInstance(subscriber->GetTopicInstance());

    }
    
    if(!EnableAcquisition()) {
        AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup Failed Enabling Acquisition");
        return False;
    }

    keepRunning = False;
    FString threadName = Name();
    threadName += "SDNHandler";
    if(moduleType == SDN_MODULE_RECEIVER) {
        threadID = Threads::BeginThread((ThreadFunctionType)ReceiverCallback, (void*)this, THREADS_DEFAULT_STACKSIZE, threadName.Buffer(), XH_NotHandled, cpuMask);
        int counter = 0;
        while((!keepRunning)&&(counter++ < 100)) SleepMsec(1);
        if(!keepRunning) {
            AssertErrorCondition(InitialisationError, "SDNDrv::ObjectLoadSetup: ReceiverCallback failed to start");
            return False;
        }
    }

    // Tell user the initialization phase is done
    AssertErrorCondition(Information,"SDNDrv::ObjectLoadSetup:: SDN Module %s Correctly Initialized type %d",Name(), moduleType);

    return True;
}

/**
 * GetData
 */
int32 SDNDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {

    // Check module type
    if(moduleType != SDN_MODULE_RECEIVER) {
        AssertErrorCondition(FatalError, "SDNDrv::GetData: %s is not a receving module",Name());
        return -1;
    }

    // check if buffer is allocated
    if(buffer == NULL) {
        AssertErrorCondition(FatalError,"SDNDrv::GetData: %s. The DDInterface buffer is NULL.",Name());
        return -1;
    }

    // Make sure that, while writeBuffer is being
    // used to update globalReadBuffer, it is not being
    // changed in the receiver thread callback
    while(!mux.FastTryLock());
    // Update readBuffer index
    globalReadBuffer = writeBuffer - 1;
    if(globalReadBuffer < 0) {
        globalReadBuffer = nOfDataBuffers-1;
    }
    // Gets the last acquired data buffer
    uint32 *lastReadBuffer     = dataBuffer[globalReadBuffer];
    SDNHeader_t* header = (SDNHeader_t*) subscriber->GetTopicHeader();
    // Check data age
    uint32 sampleNo = header->topic_counter;
    if(freshPacket) {
        if(abs(usecTime-header->send_time / 1000) > maxDataAgeUsec) {
            // Packet too old
            // return the last received data and put 0xFFFFFFFF as nSampleNumber
            sampleNo = 0xFFFFFFFF;
            previousPacketTooOldErrorCounter++;
            freshPacket = False;
        }
    } else {
        sampleNo = 0xFFFFFFFF;
        previousPacketTooOldErrorCounter++;
    }
    // Give back lock
    mux.FastUnLock();

    // Copy the data from the internal buffer to
    // the one passed in GetData
    uint32 *destination = (uint32 *)buffer;
    uint32 *destinationEnd = (uint32 *)(buffer + payloadByteSize/sizeof(int32));
    uint32 *source = lastReadBuffer;
    while(destination < destinationEnd) {
        *destination++ = *source++;
    }

    return 1;
}

/**
 * WriteData
 */
bool SDNDrv::WriteData(uint32 usecTime, const int32 *buffer){
    // check module type
    if(moduleType != SDN_MODULE_TRANSMITTER){
        AssertErrorCondition(FatalError,"SDNDrv::WriteData: %s is not a transmitter module",Name());
        return False;
    }
    // check if buffer is not allocated
    if(buffer == NULL){
        AssertErrorCondition(FatalError,"SDNDrv::WriteData: %s. The DDBInterface buffer is NULL.",Name());
        return False;
    }
    
    // Set the packet content
    memcpy(publisher->GetTopicInstance(), buffer, payloadByteSize);
    if(publisher->Publish() != STATUS_SUCCESS){
        AssertErrorCondition(FatalError,"SDNDrv::WriteData: %s. Publish() error",Name());
        return False;
    }
    return True;
}

/**
 * InputDump
 */ 
bool SDNDrv::InputDump(StreamInterface &s) const {
    // Checks for the I/O type
    if(moduleType != SDN_MODULE_RECEIVER) {
        s.Printf("%s is not an Input module\n", Name());
        return False;
    }
    return True;
}

/**
 * OutputDump
 */ 
bool SDNDrv::OutputDump(StreamInterface &s) const{
    // Checks for the I/O type
    if(moduleType != SDN_MODULE_TRANSMITTER){
        s.Printf("%s is not an Output module\n",Name());
        return False;
    }
    return True;
}

/**
 * GetUsecTime
 */
int64 SDNDrv::GetUsecTime(){

    // Check module type
    if (moduleType!=SDN_MODULE_RECEIVER){
        AssertErrorCondition(FatalError,"GetUsecTime:This method can only be called an input SDN channel");
        return 0xFFFFFFFF;
    }

    if(producerUsecPeriod != -1) {
        return ((int64)originalNSampleNumber*(int64)producerUsecPeriod);
    } else {
        return lastPacketUsecTime;
    }
}

/**
 * ObjectDescription
 */
bool SDNDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("%s %s\n",ClassName(),Version());
    // Module name
    s.Printf("Module Name --> %s\n",Name());
    s.Printf("MaxDataAgeUsec           = %d\n",maxDataAgeUsec);
    s.Printf("MaxNOfLostPackets        = %d\n",maxNOfLostPackets);
    // Module Type
    switch (moduleType){
        case SDN_MODULE_RECEIVER:
            s.Printf("SDN Type --> RECEIVER");
            break;
        case SDN_MODULE_TRANSMITTER:
            s.Printf("SDN Type --> TRANSMITTER");
            break;
        default:
            s.Printf("SDN Type --> UNDEFINED");
    }
    return True;
}

/**
 * Receiver CallBack
 */
void ReceiverCallback(void* userData){
    SDNDrv *p = (SDNDrv*)userData;   
    p->RecCallback(userData);
}


/**
 * RecCallback
 */
void SDNDrv::RecCallback(void* arg){

    // Set Thread priority
    if(receiverThreadPriority) {
        Threads::SetRealTimeClass();
        Threads::SetPriorityLevel(receiverThreadPriority);
    }

    keepRunning = True;	

    int64 lastCounterTime = 0;
    int64 oneMinCounterTime = HRT::HRTFrequency()*60;
    while(keepRunning) {
        // Print statistics of errors and warnings every minute
        int64 currentCounterTime = HRT::HRTCounter();
        if(currentCounterTime - lastCounterTime > oneMinCounterTime) {
            if(sizeMismatchErrorCounter > 0) {
                AssertErrorCondition(FatalError, "SDNDrv::RecCallback: %s Wrong packet size [occured %i times]", Name(), sizeMismatchErrorCounter);
                sizeMismatchErrorCounter = 0;
            }
            if(deviationErrorCounter > 0) {
                AssertErrorCondition(Warning, "SDNDrv::RecCallback: %s: Data arrival period mismatch with specified producer period [occured %i times]", Name(), deviationErrorCounter);
                deviationErrorCounter = 0;
            }
            if(rolloverErrorCounter > 0) {
                AssertErrorCondition(Warning,"SDNDrv::RecCallback: %s: Lost more than %d packets after a reset [occured %i times]", Name(), maxNOfLostPackets, rolloverErrorCounter);
                rolloverErrorCounter = 0;
            }
            if(lostPacketErrorCounterAux > 0) {
                AssertErrorCondition(Warning, "SDNDrv::RecCallback: %s: packets lost [occured %i times]", Name(), lostPacketErrorCounterAux);
                lostPacketErrorCounterAux = 0;
            }
            if(samePacketErrorCounter > 0) {
                AssertErrorCondition(Warning, "SDNDrv::RecCallback: %s: nSampleNumber unchanged [occured %i times]", Name(), samePacketErrorCounter);
                samePacketErrorCounter = 0;
            }
            if(recoveryCounter > 0) {
                AssertErrorCondition(Information, "SDNDrv::RecCallback: %s: Re-established correct packets sequence [occured %i times]", Name(), recoveryCounter);
                recoveryCounter = 0;
            }
            if(previousPacketTooOldErrorCounter > 0) {
                AssertErrorCondition(Warning,"SDNDrv::RecCallback: %s: Data too old [occured %i times]", Name(), previousPacketTooOldErrorCounter);
                previousPacketTooOldErrorCounter = 0;
            }
            lastCounterTime = currentCounterTime;
        }

        // Read the data
        uint32 readBytes = payloadByteSize;
        if(subscriber->Receive() != STATUS_SUCCESS){
            AssertErrorCondition(FatalError,"SDNDrv::RecCallback: %s: Receive failed. Going to return...", Name()); 
            return;
        }

        memcpy((uint32 *)dataBuffer[writeBuffer], (uint32 *)subscriber->GetTopicInstance(), payloadByteSize);
        SDNHeader_t* header = (SDNHeader_t*) subscriber->GetTopicHeader();
        // Check the buffer length
        if(readBytes != payloadByteSize) {
            sizeMismatchErrorCounter++;
            continue;
        }

        // Make sure that while writeBuffer is being
        // updated it is not being read elsewhere
        while(!mux.FastTryLock());
        // Update buffer index
        writeBuffer++;
        if(writeBuffer >= nOfDataBuffers) {
            writeBuffer = 0;
        }
        freshPacket = True;
        // Unlock resource
        mux.FastUnLock();

        if(producerUsecPeriod != -1) {
            int64 counter = HRT::HRTCounter();
            /// Allow for a 10% deviation from the specified producer usec period
            if(abs((uint32)((counter-lastCounter)*HRT::HRTPeriod()*1000000)-(uint32)((header->topic_counter-originalNSampleNumber)*producerUsecPeriod)) > 0.1*producerUsecPeriod) {
                deviationErrorCounter++;
            }
            originalNSampleNumber = header->topic_counter;
            lastCounter = counter;
        }

        // If is the first packet doesn't do any check on nSampleNumber
        if(lastPacketID != 0xFFFFFFFF) {
            // Checks nSampleNumber
            // Warning if a reset has happened and too much packet had been lost
            // nSampleNumber has been casted to int32 to prevent wrong casting from compiler
            if((int32)(header->topic_counter)-lastPacketID < 0) {
                if((int32)(header->topic_counter) > maxNOfLostPackets) {
                    rolloverErrorCounter++;
                }
            } else {
                // nSampleNumber has been casted to int32 to prevent wrong casting from compiler
                if(((int32)(header->topic_counter)-lastPacketID) > 1) {
                    // Warning if a packet is lost
                    lostPacketErrorCounter++;
                    lostPacketErrorCounterAux++;
                } else if(((int32)(header->topic_counter)-lastPacketID) == 0) {
                    // Warning if nSampleNumber in packet hasn't changed
                    samePacketErrorCounter++;
                } else if(lostPacketErrorCounter > 0) {
                    recoveryCounter++;
                    // Reset error counter
                    lostPacketErrorCounter = 0;
                }
            }
        }

        // Update lastPacketID
        lastPacketID       = header->topic_counter;
        lastPacketUsecTime = header->send_time / 1000;

        // If the module is also a timing source call the Trigger() method of
        // the time service object
        for(int i = 0 ; i < nOfTriggeringServices ; i++) {
            triggerService[i].Trigger();
        }
    }
    keepRunning = True;	
}

bool SDNDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf("<html><head><title>SDNDrv</title></head>\n");
    hStream.Printf("<body>\n");

    hStream.Printf("<h1 align=\"center\">%s</h1>\n", Name());
    if(moduleType == SDN_MODULE_RECEIVER) {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Receiver");
    } else if(moduleType == SDN_MODULE_TRANSMITTER) {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Transmitter");
    } else {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Undefined");
    }
    if(moduleType == SDN_MODULE_RECEIVER) {
        hStream.Printf("<h2 align=\"center\">Input channels = %d</h2>\n", numberOfInputChannels);
        hStream.Printf("<h2 align=\"center\">MaxDataAgeUsec = %d</h2>\n", maxDataAgeUsec);
        hStream.Printf("<h2 align=\"center\">MaxNOfLostPackets = %d</h2>\n", maxNOfLostPackets);
        hStream.Printf("<h2 align=\"center\">CPU mask = 0x%x</h2>\n", cpuMask);
        hStream.Printf("<h2 align=\"center\">Thread priority = %d</h2>\n", receiverThreadPriority);
    } else if(moduleType == SDN_MODULE_TRANSMITTER) {
        hStream.Printf("<h2 align=\"center\">Output channels = %d</h2>\n", numberOfOutputChannels);
    }

    if(moduleType == SDN_MODULE_RECEIVER) {
        /* Data table */
        hStream.Printf("<table border=\"1\" align=\"center\">\n");
        hStream.Printf("<tr>\n");
        hStream.Printf("<th></th>\n");
        hStream.Printf("<th>Buffer(k)</th>\n");
        hStream.Printf("<th>Buffer(k-1)</th>\n");
        hStream.Printf("<th>Buffer(k-2)</th>\n");
        hStream.Printf("</tr>\n");

        int32 idx;

        hStream.Printf("<tr>\n");
        hStream.Printf("<td>Sample Number</td>\n");
        for(int32 i = 0 ; i < nOfDataBuffers ; i++) {
            idx = writeBuffer-1-i;
            if(idx < 0) idx = nOfDataBuffers-(int32)fabs(idx);
            if(dataBuffer[idx] != NULL) {
                hStream.Printf("<td>%u</td>", *(dataBuffer[idx]+0));
            }
        }
        hStream.Printf("</tr>\n");

        hStream.Printf("<tr>\n");
        hStream.Printf("<td>Sample Time (usec)</td>\n");
        for(int32 i = 0 ; i < nOfDataBuffers ; i++) {
            idx = writeBuffer-1-i;
            if(idx < 0) idx = nOfDataBuffers-(int32)fabs(idx);
            if(dataBuffer[idx] != NULL) {
                hStream.Printf("<td>%u</td>", *(dataBuffer[idx]+1));
            }
        }
        hStream.Printf("</tr>\n");

        hStream.Printf("<tr>\n");
        hStream.Printf("<td>Packet Id</td>\n");
        for(int32 i = 0 ; i < nOfDataBuffers ; i++) {
            idx = writeBuffer-1-i;
            if(idx < 0) idx = nOfDataBuffers-(int32)fabs(idx);
            if(dataBuffer[idx] != NULL) {
                hStream.Printf("<td>%u</td>", *(dataBuffer[idx]+payloadByteSize/sizeof(int32)-1));
            }
        }
        hStream.Printf("</tr>\n");
    }

    hStream.Printf("</table>\n");
    hStream.Printf("</body></html>\n");

}
OBJECTLOADREGISTER(SDNDrv, "$Id: SDNDrv.cpp 456 2016-04-14 10:56:48Z aneto $")
