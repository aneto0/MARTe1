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

#include "ATMDrv.h"

#include "Endianity.h"
#include "CDBExtended.h"
#include "FastPollingMutexSem.h"

#ifndef _RTAI
#include <linux/atm.h>
#else
#include "atm_rtai.h"
#endif

struct AtmMsgHeaderStruct{
    unsigned int nSampleNumber; // the sample number since the last t=0
    unsigned int nSampleTime;   // the time since t=0, going to PRE as microseconds
};

// Timing ATM module
static const int32 timingATMDrv = 400;

/**
 * Enable System Acquisition
 */
bool ATMDrv::EnableAcquisition(){
    // Initialise lastPacketID equal to 0xFFFFFFFF
    lastPacketID = 0xFFFFFFFF;
    // Set the chip on line
    // open ATM socket
    if (liveness!=-1){
        AssertErrorCondition(Warning, "ATMDrv::EnableAcquisition: ATM socket already alive");
        return False;
    }
    atmSocket = socket(PF_ATMPVC, SOCK_DGRAM, 0);
    if(atmSocket < 0){
        AssertErrorCondition(InitialisationError, "ATMDrv::EnableAcquisition: Cannot create ATM socket");
        return False;
    }
    liveness++;
    return True;
}

/**
 * Disable System Acquisition
 */
bool ATMDrv::DisableAcquisition(){
    // Set the chip off line
    // close ATM socket
    if (liveness==-1){
        AssertErrorCondition(Warning, "ATMDrv::DisableAcquisition: ATM socket not alive");
        return False;
    }
    if(atmSocket < 0){
        AssertErrorCondition(Warning, "ATMDrv::DisableAcquisition: File Descriptor < 0");
        return False;
    }

    close(atmSocket);
    liveness=-1;
    return True;
}

/*
 * Constructors
 */
bool  ATMDrv::Init(){
    // Init general parameters
    vci                                 = 0;
    moduleType                          = ATMMODULE_UNDEFINED;
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
    freshPacket                         = False;
    // reset all buffers pointers 
    for(int i = 0 ; i < nOfDataBuffers ; i++) {
        dataBuffer[i] = NULL;
    }
    return True;
}

ATMDrv::ATMDrv(){
    Init();
}

/*
 * Destructor
 */
ATMDrv::~ATMDrv(){
    keepRunning = False;
    DisableAcquisition();
    if(moduleType == ATMMODULE_RECEIVER) {
        int counter = 0;
        while((!keepRunning) && (counter++ < 100)) SleepMsec(1);
        if(Threads::IsAlive(threadID)) {
            AssertErrorCondition(Warning,"ATMDrv::~ATMDrv: %s: Had To Kill Thread %d",Name(), threadID);
            Threads::Kill(threadID);
            threadID = 0;
        }
        else{
            AssertErrorCondition(Information,"ATMDrv::~ATMDrv: %s: Successfully waited for Thread %d to die on its own",Name(), threadID);
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
bool ATMDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){    
    // Disable previous opened connections
    DisableAcquisition();
    // Parent class Object load setup
    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"ATMDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }
    // Read VCI parameter
    if(!cdb.ReadInt32(vci,"VCI")){
        AssertErrorCondition(InitialisationError,"ATMDrv::ObjectLoadSetup: %s did not specify VCI entry",Name());
        return False;
    }
    // Read ModuleType IN/OUT
    FString module;
    if(!cdb.ReadFString(module,"ModuleType")){
        AssertErrorCondition(InitialisationError,"ATMDrv::ObjectLoadSetup: %s did not specify ModuleType entry",Name());
        return False;
    }
    if(module == "InputModule"){
        moduleType = ATMMODULE_RECEIVER;
    }else if(module == "OutputModule"){
        moduleType = ATMMODULE_TRANSMITTER;
    }else{
        AssertErrorCondition(InitialisationError,"ATMDrv::ObjectLoadSetup: %s unknown module type %s",Name(),module.Buffer());
        return False;
    }

    cpuMask = 0xFFFF;

    // Based on mudule type, init a recv or a send ATM channel
    if(moduleType == ATMMODULE_RECEIVER){
        /////////////////////////
        // Input Module (recv) //
        /////////////////////////

        // Read cpu mask
        if(!cdb.ReadInt32(cpuMask, "CpuMask", 0xFFFF)){
            AssertErrorCondition(Warning,"ATMDrv::ObjectLoadSetup: %s CpuMask was not specified. Using default: %d",Name(),cpuMask);
        }

        // Read MaxDataAgeUsec param
        if (!cdb.ReadInt32(maxDataAgeUsec, "MaxDataAgeUsec", 20000)){
            AssertErrorCondition(Warning,"ATMDrv::ObjectLoadSetup: %s did not specify MaxDataAgeUsec entry. Assuming %i usec" ,Name(),maxDataAgeUsec);
        }
        // Read MacNOfLostPackets
        cdb.ReadInt32(maxNOfLostPackets, "MaxNOfLostPackets", 4);

        if(cdb.ReadInt32(receiverThreadPriority, "ThreadPriority", 0)) {
            if(receiverThreadPriority > 32 || receiverThreadPriority < 0) {
                AssertErrorCondition(InitialisationError, "ATMDrv::ObjectLoadSetup: %s ThreadPriority parameter must be <= 32 and >= 0", Name());
                return False;
            }
        } else {
            AssertErrorCondition(Warning, "ATMDrv::ObjectLoadSetup: %s ThreadPriority parameter not specified", Name());
        }

        /// Read the UsecPeriod of the ATM packet producer
        cdb.ReadInt32(producerUsecPeriod, "ProducerUsecPeriod", -1);

        // Create Data Buffers. Compute total size and allocate storing buffer 
        for(int i=0 ; i < nOfDataBuffers ; i++){
            dataBuffer[i] = (uint32 *)malloc(numberOfInputChannels*sizeof(int));
            if(dataBuffer[i] == NULL){
                AssertErrorCondition(InitialisationError,"ATMDrv::ObjectLoadSetup: %s ATM dataBuffer allocation failed",Name());
                return False;
            }
            // Initialize the triple buffer
            uint32 *tempData = dataBuffer[i];
            for(int j = 0 ; j < numberOfInputChannels ; j++) {
                tempData[j] = 0;
            }
        }
        packetByteSize = numberOfInputChannels*sizeof(int32);

    } else if(moduleType == ATMMODULE_TRANSMITTER) {
        //////////////////////////
        // Output Module (send) //
        //////////////////////////

        // ATM Packet. Compute total size and allocate storing buffer
        packetByteSize = numberOfOutputChannels*sizeof(int32);
        // Initialise the supporting packet without using the AtmPacket class
        outputPacket = malloc(packetByteSize);
        if(outputPacket == NULL) {
            AssertErrorCondition(InitialisationError,"ATMDrv::ObjectLoadSetup: malloc outputPacket");
            return False;        	
        }
    }

    if(!EnableAcquisition()) {
        AssertErrorCondition(InitialisationError, "ATMDrv::ObjectLoadSetup Failed Enabling Acquisition");
        return False;
    }

    struct atm_qos qos;
    memset(&qos, 0, sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.txtp.traffic_class = ATM_UBR;
    qos.txtp.max_sdu = MAX_SDU;
    qos.rxtp = qos.txtp;
    qos.rxtp.spare = 1;
    if(setsockopt(atmSocket, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) == -1) {
        AssertErrorCondition(InitialisationError, "ATMDrv::ObjectLoadSetup Cannot setsocketopt(%d, %d, %d, 0x%p, %d)", atmSocket, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos));
        return False;
    }
    struct sockaddr_atmpvc addr;
    memset(&addr, 0, sizeof(addr));
    addr.sap_family   = AF_ATMPVC;
    addr.sap_addr.itf = ATM_ITF_ANY;
    addr.sap_addr.vpi = 0;
    addr.sap_addr.vci = vci;
    if(bind(atmSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        AssertErrorCondition(InitialisationError, "ATMDrv::ObjectLoadSetup Cannot bind(%d, 0x%p, %d)", atmSocket, (struct sockaddr*)&addr, sizeof(addr));
        return False;
    }

    keepRunning = False;
    FString threadName = Name();
    threadName += "ATMHandler";
    if(moduleType == ATMMODULE_RECEIVER) {
        threadID = Threads::BeginThread((ThreadFunctionType)ReceiverCallback, (void*)this, THREADS_DEFAULT_STACKSIZE, threadName.Buffer(), XH_NotHandled, cpuMask);
        int counter = 0;
        while((!keepRunning)&&(counter++ < 100)) SleepMsec(1);
        if(!keepRunning) {
            AssertErrorCondition(InitialisationError, "ATMDrv::ObjectLoadSetup: ReceiverCallback failed to start");
            return False;
        }
    }

    // Tell user the initialization phase is done
    AssertErrorCondition(Information,"ATMDrv::ObjectLoadSetup:: ATM Module %s Correctly Initialized - VCI --> %d, type %d",Name(), vci, moduleType);

    return True;
}

/**
 * GetData
 */
int32 ATMDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {

    // Check module type
    if(moduleType!=ATMMODULE_RECEIVER) {
        AssertErrorCondition(FatalError,"ATMDrv::GetData: %s is not a receving module on vci %d",Name(),vci);
        return -1;
    }

    // check if buffer is allocated
    if(buffer == NULL) {
        AssertErrorCondition(FatalError,"ATMDrv::GetData: %s. The DDInterface buffer is NULL.",Name());
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
    AtmMsgHeaderStruct *header = (AtmMsgHeaderStruct *)lastReadBuffer;
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

    AtmMsgHeaderStruct *p = (AtmMsgHeaderStruct *)buffer;
    p->nSampleNumber = sampleNo;

    return 1;
}

/**
 * WriteData
 */
bool ATMDrv::WriteData(uint32 usecTime, const int32 *buffer){
    // check module type
    if(moduleType!=ATMMODULE_TRANSMITTER){
        AssertErrorCondition(FatalError,"ATMDrv::WriteData: %s is not a transmitter module on vci %d",Name(),vci);
        return False;
    }
    // check if buffer is not allocated
    if(buffer == NULL){
        AssertErrorCondition(FatalError,"ATMDrv::WriteData: %s. The DDInterface buffer is NULL.",Name());
        return False;
    }
    // Set the packet content - does endianity swap
    int size = packetByteSize;
    Endianity::MemCopyToMotorola((uint32*)outputPacket, (uint32*)buffer, numberOfOutputChannels);
    // Send packet
    int _ret;
    _ret = send(atmSocket, outputPacket, size, 0);
    if(_ret == -1){
        AssertErrorCondition(FatalError,"ATMDrv::WriteData: %s. Send socket error",Name());
        return False;
    }
    return True;
}

/**
 * InputDump
 */ 
bool ATMDrv::InputDump(StreamInterface &s) const {
    // Checks for the I/O type
    if(moduleType != ATMMODULE_RECEIVER) {
        s.Printf("%s is not an Input module\n", Name());
        return False;
    }
    // Prints some usefull informations about the input board
    s.Printf("%s - ATM board attached at VCI #%d\n",Name(),vci);
    return True;
}

/**
 * OutputDump
 */ 
bool ATMDrv::OutputDump(StreamInterface &s) const{
    // Checks for the I/O type
    if(moduleType != ATMMODULE_TRANSMITTER){
        s.Printf("%s is not an Output module\n",Name());
        return False;
    }
    // Prints some usefull informations about the input board
    s.Printf("%s - ATM board attached at VCI #%d\n\n",Name(),vci);
    return True;
}

/**
 * GetUsecTime
 */
int64 ATMDrv::GetUsecTime(){

    // Check module type
    if (moduleType!=ATMMODULE_RECEIVER){
        AssertErrorCondition(FatalError,"GetUsecTime:This method can only be called an input ATM channel");
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
bool ATMDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("%s %s\n",ClassName(),Version());
    // Module name
    s.Printf("Module Name --> %s\n",Name());
    // VCI Parameters
    s.Printf("VCI No                   = %d\n",vci);
    s.Printf("MaxDataAgeUsec           = %d\n",maxDataAgeUsec);
    s.Printf("MaxNOfLostPackets        = %d\n",maxNOfLostPackets);
    // VCI Type
    switch (moduleType){
        case 0:
            s.Printf("VCI Type --> RECEIVER");
            break;
        case 1:
            s.Printf("VCI Type --> TRANSMITTER");
            break;
        default:
            s.Printf("VCI Type --> UNDEFINED");
    }
    if(vci == timingATMDrv) s.Printf("\nThis is even a Time Module");
    return True;
}

/**
 * Receiver CallBack
 */
void ReceiverCallback(void* userData){
    ATMDrv *p = (ATMDrv*)userData;   
    p->RecCallback(userData);
}


/**
 * RecCallback
 */
void ATMDrv::RecCallback(void* arg){

    // Set Thread priority
    if(receiverThreadPriority) {
        Threads::SetRealTimeClass();
        Threads::SetPriorityLevel(receiverThreadPriority);
    }

    // Allocate
    int32 *dataSource;
    if((dataSource = (int32 *)malloc(packetByteSize)) == NULL) {
        AssertErrorCondition(FatalError, "ATMDrv::RecCallback: unable to allocate buffer on vci %d receiver thread exiting", vci);
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
                AssertErrorCondition(FatalError, "ATMDrv::RecCallback: ATM Wrong packet size on vci %i [occured %i times]", vci, sizeMismatchErrorCounter);
                sizeMismatchErrorCounter = 0;
            }
            if(deviationErrorCounter > 0) {
                AssertErrorCondition(Warning, "ATMDrv::RecCallback: %s: Data arrival period mismatch with specified producer period on VCI %d [occured %i times]", Name(), vci, deviationErrorCounter);
                deviationErrorCounter = 0;
            }
            if(rolloverErrorCounter > 0) {
                AssertErrorCondition(Warning,"Lost more than %d packets after a reset on vci %d [occured %i times]",maxNOfLostPackets, vci, rolloverErrorCounter);
                rolloverErrorCounter = 0;
            }
            if(lostPacketErrorCounterAux > 0) {
                AssertErrorCondition(Warning, "packets lost on vci = %d [occured %i times]", vci, lostPacketErrorCounterAux);
                lostPacketErrorCounterAux = 0;
            }
            if(samePacketErrorCounter > 0) {
                AssertErrorCondition(Warning, "nSampleNumber unchanged in vci = %d [occured %i times]", vci, samePacketErrorCounter);
                samePacketErrorCounter = 0;
            }
            if(recoveryCounter > 0) {
                AssertErrorCondition(Information, "Re-established correct packets sequence on vci %d [occured %i times]", vci, recoveryCounter);
                recoveryCounter = 0;
            }
            if(previousPacketTooOldErrorCounter > 0) {
                AssertErrorCondition(Warning,"ATMDrv::GetData: %s: VCI %d: Data too old [occured %i times]",Name(),vci, previousPacketTooOldErrorCounter);
                previousPacketTooOldErrorCounter = 0;
            }
            lastCounterTime = currentCounterTime;
        }

        // recv data from ATM card
        int _ret = recv(atmSocket, dataSource, packetByteSize, 0);
        if(_ret == -1) {
            AssertErrorCondition(FatalError,"ATMDrv::RecCallback: Socket recv error");
            return;
        }

        // Check the buffer length
        if(_ret != packetByteSize) {
            sizeMismatchErrorCounter++;
            continue;
        }

        // Copy dataSource in the write only buffer; does endianity swap
        Endianity::MemCopyFromMotorola((uint32 *)dataBuffer[writeBuffer],(uint32 *)dataSource,packetByteSize/sizeof(int32));

        // Checks if packets have been lost
        AtmMsgHeaderStruct *header = (AtmMsgHeaderStruct *)dataBuffer[writeBuffer];

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

        // If the module is the timingATMDrv call the Trigger() method of
        // the time service object
        for(int i = 0 ; i < nOfTriggeringServices ; i++) {
            triggerService[i].Trigger();
        }
    }
    keepRunning = True;	
}

bool ATMDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf("<html><head><title>LinuxATMDrv</title></head>\n");
    hStream.Printf("<body>\n");

    hStream.Printf("<h1 align=\"center\">%s</h1>\n", Name());
    if(moduleType == ATMMODULE_RECEIVER) {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Receiver");
    } else if(moduleType == ATMMODULE_TRANSMITTER) {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Transmitter");
    } else {
        hStream.Printf("<h2 align=\"center\">Type = %s</h2>\n", "Undefined");
    }
    hStream.Printf("<h2 align=\"center\">VCI = %d</h2>\n", vci);
    if(moduleType == ATMMODULE_RECEIVER) {
        hStream.Printf("<h2 align=\"center\">Input channels = %d</h2>\n", numberOfInputChannels);
        hStream.Printf("<h2 align=\"center\">MaxDataAgeUsec = %d</h2>\n", maxDataAgeUsec);
        hStream.Printf("<h2 align=\"center\">MaxNOfLostPackets = %d</h2>\n", maxNOfLostPackets);
        hStream.Printf("<h2 align=\"center\">CPU mask = 0x%x</h2>\n", cpuMask);
        hStream.Printf("<h2 align=\"center\">Thread priority = %d</h2>\n", receiverThreadPriority);
    } else if(moduleType == ATMMODULE_TRANSMITTER) {
        hStream.Printf("<h2 align=\"center\">Output channels = %d</h2>\n", numberOfOutputChannels);
    }

    if(moduleType == ATMMODULE_RECEIVER) {
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
OBJECTLOADREGISTER(ATMDrv, "$Id: ATMDrv.cpp,v 1.25 2011/09/16 10:30:32 ppcc_dev Exp $")
