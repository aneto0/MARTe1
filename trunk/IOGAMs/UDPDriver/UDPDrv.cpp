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

#include "UDPDrv.h"

#include "Endianity.h"
#include "CDBExtended.h"
#include "FastPollingMutexSem.h"

struct UDPMsgHeaderStruct{
    unsigned int nSampleNumber; // the sample number since the last t=0
    unsigned int nSampleTime;   // the sample time
};

/**
 * Enable System Acquisition
 */
bool UDPDrv::EnableAcquisition(){
    // Initialise lastPacketID equal to 0xFFFFFFFF
    lastPacketID = 0xFFFFFFFF;
    // open UDP socket
    if (liveness != -1){
        AssertErrorCondition(Warning, "UDPDrv::EnableAcquisition: UDP socket already alive");
        return False;
    }


    liveness++;
    return True;
}

/**
 * Disable System Acquisition
 */
bool UDPDrv::DisableAcquisition(){
    // close UDP socket
    if (liveness==-1){
        AssertErrorCondition(Warning, "UDPDrv::DisableAcquisition: UDP socket not alive");
        return False;
    }
    socket.Close();
    liveness=-1;
    return True;
}

/*
 * Constructors
 */
bool  UDPDrv::Init(){
    // Init general parameters
    moduleType                          = UDP_MODULE_UNDEFINED;
    mux.Create();
    packetByteSize                      = 0;
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

    destinationServerAddress.SetSize(0);
    destinationServerAddress            = -1;
    destinationServerPort               = -1;
    serverPort                          = -1;

    // reset all buffers pointers 
    for(int i = 0 ; i < nOfDataBuffers ; i++) {
        dataBuffer[i] = NULL;
    }
    return True;
}

UDPDrv::UDPDrv(){
    Init();
}

/*
 * Destructor
 */
UDPDrv::~UDPDrv(){
    keepRunning = False;
    DisableAcquisition();
    if(moduleType == UDP_MODULE_RECEIVER) {
        int counter = 0;
        while((!keepRunning) && (counter++ < 100)) SleepMsec(1);
        if(Threads::IsAlive(threadID)) {
            AssertErrorCondition(Warning,"UDPDrv::~UDPDrv: %s: Had To Kill Thread %d",Name(), threadID);
            Threads::Kill(threadID);
            threadID = 0;
        }
        else{
            AssertErrorCondition(Information,"UDPDrv::~UDPDrv: %s: Successfully waited for Thread %d to die on its own",Name(), threadID);
        }
    }

    // Free memory
    for(int i = 0 ; i < nOfDataBuffers ; i++) {
        if(dataBuffer[i] != NULL)free((void *&)dataBuffer[i]);
    }        	

}

/**
 * ObjectLoadSetup
 */
bool UDPDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){    
    // Disable previous opened connections
    DisableAcquisition();
    // Parent class Object load setup
    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"UDPDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }
    // Read ModuleType IN/OUT
    FString module;
    if(!cdb.ReadFString(module,"ModuleType")){
        AssertErrorCondition(InitialisationError,"UDPDrv::ObjectLoadSetup: %s did not specify ModuleType entry",Name());
        return False;
    }
    if(module == "InputModule"){
        moduleType = UDP_MODULE_RECEIVER;
    }else if(module == "OutputModule"){
        moduleType = UDP_MODULE_TRANSMITTER;
    }else{
        AssertErrorCondition(InitialisationError,"UDPDrv::ObjectLoadSetup: %s unknown module type %s",Name(),module.Buffer());
        return False;
    }
    cpuMask = 0xFFFF;

    if(!socket.Open()){
        AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: failed to open socket");
        return False;
    }

    //Read destinationServerAddress destinationServerPort serverPort....

    if(moduleType == UDP_MODULE_TRANSMITTER){
        if(!cdb.ReadFString(destinationServerAddress, "DestinationServerAddress")){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: DestinationServerAddress was not specified");
            socket.Close();
            return False;
        }
        if(!cdb.ReadInt32((int32 &)destinationServerPort, "DestinationServerPort")){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: DestinationServerPort was not specified");
            socket.Close();
            return False;
        }
        if(!socket.Connect(destinationServerAddress.Buffer(), destinationServerPort)){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: Failed to connect socket to %s:%d", destinationServerAddress.Buffer(), destinationServerPort);
            socket.Close();
            return False;
        }
        // UDP Packet. Compute total size and allocate storing buffer
        //Take into account that the header is automatically sent as the first two channels
        packetByteSize = (numberOfOutputChannels + 2) * sizeof(int32);
        // Initialise the supporting packet without using the AtmPacket class
        outputPacket = malloc(packetByteSize);
        if(outputPacket == NULL) {
            AssertErrorCondition(InitialisationError,"UDPDrv::ObjectLoadSetup: malloc outputPacket");
            return False;        	
        }
    }
    else{
        if(numberOfInputChannels < 2){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: At least 2 input channels must be specified for the header");
            socket.Close();
            return False;
        }
        if(!cdb.ReadInt32((int32 &)serverPort, "ServerPort")){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: ServerPort was not specified");
            socket.Close();
            return False;
        }
        // Read cpu mask
        if(!cdb.ReadInt32(cpuMask, "CpuMask", 0xFFFF)){
            AssertErrorCondition(Warning,"UDPDrv::ObjectLoadSetup: %s CpuMask was not specified. Using default: %d",Name(),cpuMask);
        }

        // Read MaxDataAgeUsec param
        if (!cdb.ReadInt32(maxDataAgeUsec, "MaxDataAgeUsec", 20000)){
            AssertErrorCondition(Warning,"UDPDrv::ObjectLoadSetup: %s did not specify MaxDataAgeUsec entry. Assuming %i usec" ,Name(),maxDataAgeUsec);
        }
        // Read MacNOfLostPackets
        cdb.ReadInt32(maxNOfLostPackets, "MaxNOfLostPackets", 4);

        if(cdb.ReadInt32(receiverThreadPriority, "ThreadPriority", 0)) {
            if(receiverThreadPriority > 32 || receiverThreadPriority < 0) {
                AssertErrorCondition(InitialisationError, "UDPDrv::ObjectLoadSetup: %s ThreadPriority parameter must be <= 32 and >= 0", Name());
                return False;
            }
        } else {
            AssertErrorCondition(Warning, "UDPDrv::ObjectLoadSetup: %s ThreadPriority parameter not specified", Name());
        }

        /// Read the UsecPeriod of the UDP packet producer
        cdb.ReadInt32(producerUsecPeriod, "ProducerUsecPeriod", -1);

        // Create Data Buffers. Compute total size and allocate storing buffer 
        for(int i=0 ; i < nOfDataBuffers ; i++){
            dataBuffer[i] = (uint32 *)malloc(numberOfInputChannels*sizeof(int));
            if(dataBuffer[i] == NULL){
                AssertErrorCondition(InitialisationError,"UDPDrv::ObjectLoadSetup: %s UDP dataBuffer allocation failed",Name());
                socket.Close();
                return False;
            }
            // Initialize the triple buffer
            uint32 *tempData = dataBuffer[i];
            for(int j = 0 ; j < numberOfInputChannels ; j++) {
                tempData[j] = 0;
            }
        }
        packetByteSize = numberOfInputChannels*sizeof(int32);
        if(!socket.SetBlocking(True)){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: Failed to set UDP socket as blocking");
            socket.Close();
            return False;
        }
        if(!socket.Listen(serverPort)){
            AssertErrorCondition(InitialisationError, "UDPDrv::EnableAcquisition: Failed to set UDP socket listening in port %d", serverPort);
            socket.Close();
            return False;
        }
    }

    if(!EnableAcquisition()) {
        AssertErrorCondition(InitialisationError, "UDPDrv::ObjectLoadSetup Failed Enabling Acquisition");
        return False;
    }

    keepRunning = False;
    FString threadName = Name();
    threadName += "UDPHandler";
    if(moduleType == UDP_MODULE_RECEIVER) {
        threadID = Threads::BeginThread((ThreadFunctionType)ReceiverCallback, (void*)this, THREADS_DEFAULT_STACKSIZE, threadName.Buffer(), XH_NotHandled, cpuMask);
        int counter = 0;
        while((!keepRunning)&&(counter++ < 100)) SleepMsec(1);
        if(!keepRunning) {
            AssertErrorCondition(InitialisationError, "UDPDrv::ObjectLoadSetup: ReceiverCallback failed to start");
            return False;
        }
    }

    // Tell user the initialization phase is done
    AssertErrorCondition(Information,"UDPDrv::ObjectLoadSetup:: UDP Module %s Correctly Initialized type %d",Name(), moduleType);

    return True;
}

/**
 * GetData
 */
int32 UDPDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {

    // Check module type
    if(moduleType != UDP_MODULE_RECEIVER) {
        AssertErrorCondition(FatalError, "UDPDrv::GetData: %s is not a receving module",Name());
        return -1;
    }

    // check if buffer is allocated
    if(buffer == NULL) {
        AssertErrorCondition(FatalError,"UDPDrv::GetData: %s. The DDInterface buffer is NULL.",Name());
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
    UDPMsgHeaderStruct *header = (UDPMsgHeaderStruct *)lastReadBuffer;
    // Check data age
    uint32 sampleNo = header->nSampleNumber;
    if(freshPacket) {
        if(abs(usecTime-header->nSampleTime) > maxDataAgeUsec) {
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
    uint32 *destinationEnd = (uint32 *)(buffer + packetByteSize/sizeof(int32));
    uint32 *source = lastReadBuffer;
    while(destination < destinationEnd) {
        *destination++ = *source++;
    }

    UDPMsgHeaderStruct *p = (UDPMsgHeaderStruct *)buffer;
    p->nSampleNumber = sampleNo;

    return 1;
}

/**
 * WriteData
 */
bool UDPDrv::WriteData(uint32 usecTime, const int32 *buffer){
    // check module type
    if(moduleType != UDP_MODULE_TRANSMITTER){
        AssertErrorCondition(FatalError,"UDPDrv::WriteData: %s is not a transmitter module",Name());
        return False;
    }
    // check if buffer is not allocated
    if(buffer == NULL){
        AssertErrorCondition(FatalError,"UDPDrv::WriteData: %s. The DDInterface buffer is NULL.",Name());
        return False;
    }
    uint32  size         = packetByteSize;
    uint32 *packetToSend = (uint32 *)outputPacket;
    // Set the packet content
    memcpy(packetToSend + 2, buffer, numberOfOutputChannels * sizeof(uint32));
    // Add the header
    *packetToSend       = packetNumber++;
    *(packetToSend + 1) = usecTime;
    int32 i             = 0;
    for(i=0; i<(numberOfOutputChannels + 2); i++){
        Endianity::ToMotorola(packetToSend[i]);
    }
    // Send packet
    if(!socket.Write(packetToSend, size)){
        AssertErrorCondition(FatalError,"UDPDrv::WriteData: %s. Send socket error",Name());
        return False;
    }
    return True;
}

/**
 * InputDump
 */ 
bool UDPDrv::InputDump(StreamInterface &s) const {
    // Checks for the I/O type
    if(moduleType != UDP_MODULE_RECEIVER) {
        s.Printf("%s is not an Input module\n", Name());
        return False;
    }
    return True;
}

/**
 * OutputDump
 */ 
bool UDPDrv::OutputDump(StreamInterface &s) const{
    // Checks for the I/O type
    if(moduleType != UDP_MODULE_TRANSMITTER){
        s.Printf("%s is not an Output module\n",Name());
        return False;
    }
    return True;
}

/**
 * GetUsecTime
 */
int64 UDPDrv::GetUsecTime(){

    // Check module type
    if (moduleType!=UDP_MODULE_RECEIVER){
        AssertErrorCondition(FatalError,"GetUsecTime:This method can only be called an input UDP channel");
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
bool UDPDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("%s %s\n",ClassName(),Version());
    // Module name
    s.Printf("Module Name --> %s\n",Name());
    s.Printf("MaxDataAgeUsec           = %d\n",maxDataAgeUsec);
    s.Printf("MaxNOfLostPackets        = %d\n",maxNOfLostPackets);
    // Module Type
    switch (moduleType){
        case UDP_MODULE_RECEIVER:
            s.Printf("UDP Type --> RECEIVER");
            break;
        case UDP_MODULE_TRANSMITTER:
            s.Printf("UDP Type --> TRANSMITTER");
            break;
        default:
            s.Printf("UDP Type --> UNDEFINED");
    }
    return True;
}

/**
 * Receiver CallBack
 */
void ReceiverCallback(void* userData){
    UDPDrv *p = (UDPDrv*)userData;   
    p->RecCallback(userData);
}


/**
 * RecCallback
 */
void UDPDrv::RecCallback(void* arg){

    // Set Thread priority
    if(receiverThreadPriority) {
        Threads::SetRealTimeClass();
        Threads::SetPriorityLevel(receiverThreadPriority);
    }

    // Allocate
    int32 *dataSource;
    if((dataSource = (int32 *)malloc(packetByteSize)) == NULL) {
        AssertErrorCondition(FatalError, "UDPDrv::RecCallback: unable to allocate buffer. Receiver thread exiting");
        return;
    }
    keepRunning = True;	

    int64 lastCounterTime = 0;
    int64 oneMinCounterTime = HRT::HRTFrequency()*60;
    while(keepRunning) {
        // Print statistics of errors and warnings every minute
        int64 currentCounterTime = HRT::HRTCounter();
        if(currentCounterTime - lastCounterTime > oneMinCounterTime) {
            if(sizeMismatchErrorCounter > 0) {
                AssertErrorCondition(FatalError, "UDPDrv::RecCallback: %s Wrong packet size [occured %i times]", Name(), sizeMismatchErrorCounter);
                sizeMismatchErrorCounter = 0;
            }
            if(deviationErrorCounter > 0) {
                AssertErrorCondition(Warning, "UDPDrv::RecCallback: %s: Data arrival period mismatch with specified producer period [occured %i times]", Name(), deviationErrorCounter);
                deviationErrorCounter = 0;
            }
            if(rolloverErrorCounter > 0) {
                AssertErrorCondition(Warning,"UDPDrv::RecCallback: %s: Lost more than %d packets after a reset [occured %i times]", Name(), maxNOfLostPackets, rolloverErrorCounter);
                rolloverErrorCounter = 0;
            }
            if(lostPacketErrorCounterAux > 0) {
                AssertErrorCondition(Warning, "UDPDrv::RecCallback: %s: packets lost [occured %i times]", Name(), lostPacketErrorCounterAux);
                lostPacketErrorCounterAux = 0;
            }
            if(samePacketErrorCounter > 0) {
                AssertErrorCondition(Warning, "UDPDrv::RecCallback: %s: nSampleNumber unchanged [occured %i times]", Name(), samePacketErrorCounter);
                samePacketErrorCounter = 0;
            }
            if(recoveryCounter > 0) {
                AssertErrorCondition(Information, "UDPDrv::RecCallback: %s: Re-established correct packets sequence [occured %i times]", Name(), recoveryCounter);
                recoveryCounter = 0;
            }
            if(previousPacketTooOldErrorCounter > 0) {
                AssertErrorCondition(Warning,"UDPDrv::RecCallback: %s: Data too old [occured %i times]", Name(), previousPacketTooOldErrorCounter);
                previousPacketTooOldErrorCounter = 0;
            }
            lastCounterTime = currentCounterTime;
        }

        // Read the data
        uint32 readBytes = packetByteSize;
        if(!socket.Read(dataSource, readBytes)){
            AssertErrorCondition(FatalError, "UDPDrv::RecCallback: Failed reading data. Thread is going to return");
            return;
        }

        // Check the buffer length
        if(readBytes != packetByteSize) {
            sizeMismatchErrorCounter++;
            continue;
        }

        // Copy dataSource in the write only buffer; does endianity swap
        Endianity::MemCopyFromMotorola((uint32 *)dataBuffer[writeBuffer],(uint32 *)dataSource,packetByteSize/sizeof(int32));

        // Checks if packets have been lost
        UDPMsgHeaderStruct *header = (UDPMsgHeaderStruct *)dataBuffer[writeBuffer];

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
            if(abs((uint32)((counter-lastCounter)*HRT::HRTPeriod()*1000000)-(uint32)((header->nSampleNumber-originalNSampleNumber)*producerUsecPeriod)) > 0.1*producerUsecPeriod) {
                deviationErrorCounter++;
            }
            originalNSampleNumber = header->nSampleNumber;
            lastCounter = counter;
        }

        // If is the first packet doesn't do any check on nSampleNumber
        if(lastPacketID != 0xFFFFFFFF) {
            // Checks nSampleNumber
            // Warning if a reset has happened and too much packet had been lost
            // nSampleNumber has been casted to int32 to prevent wrong casting from compiler
            if((int32)(header->nSampleNumber)-lastPacketID < 0) {
                if((int32)(header->nSampleNumber) > maxNOfLostPackets) {
                    rolloverErrorCounter++;
                }
            } else {
                // nSampleNumber has been casted to int32 to prevent wrong casting from compiler
                if(((int32)(header->nSampleNumber)-lastPacketID) > 1) {
                    // Warning if a packet is lost
                    lostPacketErrorCounter++;
                    lostPacketErrorCounterAux++;
                } else if(((int32)(header->nSampleNumber)-lastPacketID) == 0) {
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
        lastPacketID       = header->nSampleNumber;
        lastPacketUsecTime = header->nSampleTime;

        // If the module is also a timing source call the Trigger() method of
        // the time service object
        for(int i = 0 ; i < nOfTriggeringServices ; i++) {
            triggerService[i].Trigger();
        }
    }
    keepRunning = True;	
}

bool UDPDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf("<html><head><title>UDPDrv</title></head>\n");
    hStream.Printf("<body>\n");

    hStream.Printf("<h1 align=\"center\">%s</h1>\n", Name());
    if(moduleType == UDP_MODULE_RECEIVER) {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Receiver");
    } else if(moduleType == UDP_MODULE_TRANSMITTER) {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Transmitter");
    } else {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Undefined");
    }
    if(moduleType == UDP_MODULE_RECEIVER) {
        hStream.Printf("<h2 align=\"center\">Input channels = %d</h2>\n", numberOfInputChannels);
        hStream.Printf("<h2 align=\"center\">MaxDataAgeUsec = %d</h2>\n", maxDataAgeUsec);
        hStream.Printf("<h2 align=\"center\">MaxNOfLostPackets = %d</h2>\n", maxNOfLostPackets);
        hStream.Printf("<h2 align=\"center\">CPU mask = 0x%x</h2>\n", cpuMask);
        hStream.Printf("<h2 align=\"center\">Thread priority = %d</h2>\n", receiverThreadPriority);
    } else if(moduleType == UDP_MODULE_TRANSMITTER) {
        hStream.Printf("<h2 align=\"center\">Output channels = %d</h2>\n", numberOfOutputChannels);
    }

    if(moduleType == UDP_MODULE_RECEIVER) {
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
                hStream.Printf("<td>%u</td>", *(dataBuffer[idx]+packetByteSize/sizeof(int32)-1));
            }
        }
        hStream.Printf("</tr>\n");
    }

    hStream.Printf("</table>\n");
    hStream.Printf("</body></html>\n");

}
OBJECTLOADREGISTER(UDPDrv, "$Id$")
