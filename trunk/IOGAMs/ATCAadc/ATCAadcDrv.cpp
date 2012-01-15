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
#include "ATCAadcDrv.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "HRT.h"
#include "Sleep.h"
#include "Console.h"

int32 SingleATCAModule::currentDMABufferIndex     = 0;
int32 SingleATCAModule::currentMasterHeader       = 0;

#ifdef _LINUX
int32 ATCAadcDrv::pageSize                        = 0;
int32 ATCAadcDrv::fileDescriptor                  = 0;
#endif

SingleATCAModule::SingleATCAModule(){
    moduleIdentifier                        = 0;
    numberOfAnalogueInputChannels           = 0;
    numberOfDigitalInputChannels            = 0;
    numberOfAnalogueOutputChannels          = 0;
    numberOfDigitalOutputChannels           = 0;
    int i = 0;
    for(i = 0; i < 8; i++)outputMap[i]      = 0;

    // Input Section //
    isMaster                                = False;
    for(i = 0; i < 4; i++)dmaBuffers[i]     = NULL;
    nextExpectedAcquisitionCPUTicks         = 0;
    boardInternalCycleTicks                 = 0;
    datagramArrivalFastMonitorSecSleep      = 0.0;
    boardInternalCycleTime                  = 0;

    lastCycleUsecTime                       = 0;
    packetCounter                           = 0;
    synchronizing                           = False;
    channelStatistics                       = NULL;

    allowPollSleeping                       = True;
    worstPollSleepJitter                    = 0;
    worstPollSleepJitterDecayRate           = (1 - 5e-6);
    pollSleepTime                           = 0;
    pollSleepTimeWakeBeforeUs               = 20;
}


bool SingleATCAModule::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);
    FString moduleName;
    cdb->NodeName(moduleName);
    if(!cdb.ReadInt32(moduleIdentifier, "ModuleIdentifier")){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: ModuleIdentifier has not been specified.");
        return False;
    }

    int32 master = 0;
    cdb.ReadInt32(master, "IsMaster",0);
    isMaster = (master != 0);
    if(isMaster){
        CStaticAssertErrorCondition(Information,"SingleATCAModule::ObjectLoadSetup: Module with identifier %d has been specified as master.", moduleIdentifier);
    }
    
    synchronizing = False;
    if (isMaster) {
        FString syncMethod;
        if(!cdb.ReadFString(syncMethod, "SynchronizationMethod")){
            CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: SynchronizationMethod has not been specified.");
            return False;
        }
        if (syncMethod == "GetLatest") {
            synchronizing = False;
            CStaticAssertErrorCondition(Information,"SingleATCAModule::ObjectLoadSetup: synchronization method: GetLatest");
        } else {
            synchronizing = True;
            CStaticAssertErrorCondition(Information,"SingleATCAModule::ObjectLoadSetup: synchronization method: Synchronous on input");
        }
    }

    if(!cdb.ReadInt32(numberOfAnalogueInputChannels, "NumberOfAnalogueInput")){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueInput has not been specified.");
        return False;
    }

    if(!cdb.ReadInt32(numberOfDigitalInputChannels, "NumberOfDigitalInput")){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfDigitalInput has not been specified.");
        return False;
    }
    
    if(!cdb.ReadInt32(numberOfAnalogueOutputChannels, "NumberOfAnalogueOutput")){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueOutput has not been specified.");
        return False;
    }

    if(!cdb.ReadInt32(numberOfDigitalOutputChannels, "NumberOfDigitalOutput")){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfDigitalOutput has not been specified.");
        return False;
    }

    int32 detectedNumberOfInputAnalogChannels = 0;    
#ifdef _RTAI
    detectedNumberOfInputAnalogChannels = GetNumberOfInputAnalogChannels(moduleIdentifier);
#elif defined(_LINUX)
    int32 temp = moduleIdentifier;
    int32 ret  = ioctl(ATCAadcDrv::fileDescriptor, PCIE_ATCA_ADC_IOCT_N_IN_ANA_CHANNELS, &temp);
    if(ret != 0){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not query number of analog input channels. ioctl returned : %d", ret);
        return False;
    }
    detectedNumberOfInputAnalogChannels = temp;
#endif
    if(numberOfAnalogueInputChannels > detectedNumberOfInputAnalogChannels){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueInputs is at most %d. Specified %d.", detectedNumberOfInputAnalogChannels, numberOfAnalogueInputChannels);
        return False;
    }
    

    int32 detectedNumberOfInputDigitalChannels = 0;
#ifdef _RTAI
    detectedNumberOfInputDigitalChannels = GetNumberOfInputDigitalChannels(moduleIdentifier);
#elif defined(_LINUX)
    temp = moduleIdentifier;
    ret  = ioctl(ATCAadcDrv::fileDescriptor, PCIE_ATCA_ADC_IOCT_N_IN_DIG_CHANNELS, &temp);
    if(ret != 0){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not query number of digital input channels. ioctl returned : %d", ret);
        return False;
    }
    detectedNumberOfInputDigitalChannels = temp;
#endif
    if(numberOfDigitalInputChannels > detectedNumberOfInputDigitalChannels){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfDigitalInputs is at most %d. Specified %d.", detectedNumberOfInputDigitalChannels, numberOfDigitalInputChannels);
        return False;
    }

    int32 detectedNumberOfOutputAnalogChannels = 0;
#ifdef _RTAI
    detectedNumberOfOutputAnalogChannels = GetNumberOfAnalogueOutputChannels(moduleIdentifier);
#elif defined(_LINUX)
    temp = moduleIdentifier;
    ret  = ioctl(ATCAadcDrv::fileDescriptor, PCIE_ATCA_ADC_IOCT_N_OUT_ANA_CHANNELS, &temp);
    if(ret != 0){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not query number of analog output channels. ioctl returned : %d", ret);
        return False;
    }
    detectedNumberOfOutputAnalogChannels = temp;
#endif
    if(numberOfAnalogueOutputChannels > detectedNumberOfOutputAnalogChannels){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueOutputs is at most %d. Specified %d.", detectedNumberOfOutputAnalogChannels, numberOfAnalogueOutputChannels);
        return False;
    }

    int32 detectedNumberOfDigitalOutputChannels = 0;
#ifdef _RTAI
    detectedNumberOfDigitalOutputChannels = GetNumberOfDigitalOutputChannels(moduleIdentifier);
#elif defined(_LINUX)
    temp = moduleIdentifier;
    ret  = ioctl(ATCAadcDrv::fileDescriptor, PCIE_ATCA_ADC_IOCT_N_OUT_DIG_CHANNELS, &temp);
    if(ret != 0){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not query number of digital output channels. ioctl returned : %d", ret);
        return False;
    }
    detectedNumberOfDigitalOutputChannels = temp;
#endif
    if(numberOfDigitalOutputChannels > detectedNumberOfDigitalOutputChannels){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueOutputs is at most %d. Specified %d.", detectedNumberOfDigitalOutputChannels, numberOfAnalogueOutputChannels);
        return False;
    }

    if(numberOfAnalogueOutputChannels > 0){
        bool hasRTM = False;
#ifdef _RTAI
        hasRTM = IsRTMPresent(moduleIdentifier);
#elif defined(_LINUX)
        int rtm = moduleIdentifier;
        ret  = ioctl(ATCAadcDrv::fileDescriptor, PCIE_ATCA_ADC_IOCT_IS_RTM_PRESENT, &rtm);
        if(ret != 0){
            CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not query IsRTMPresent. ioctl returned : %d", ret);
            return False;
        }
        hasRTM = (rtm == 1);
#endif
        if(!hasRTM){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: Module %d specifies %d outputs but does not have RTM module", moduleIdentifier, numberOfAnalogueOutputChannels);
            return False;
        }
        int dims    = 1;
        int size[2] = {numberOfAnalogueOutputChannels,1};
        if(!cdb.ReadInt32Array(outputMap, size, dims, "OutputMap")){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: OutputMap not specified. Assuming sequential order.");
            for(int i = 0; i < numberOfAnalogueOutputChannels; i++)
                outputMap[i] = i+1;
        }

        // output order starts from 0 for convenience.
        for(int i = 0; i < numberOfAnalogueOutputChannels; i++) outputMap[i]--;
    }
    
    if(channelStatistics != NULL){
        delete[] channelStatistics;
        channelStatistics = NULL;
    }

    channelStatistics = new StatSignalInfo[NumberOfInputChannels()];
    if(channelStatistics == NULL){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not create ChannelStatistics for %d channels", NumberOfInputChannels());
        return False;
    }

    for(int i=0; i<NumberOfInputChannels(); i++){
        channelStatistics[i].Init();
        channelStatistics[i].decayRate = 0.999;
    }

    if(isMaster){
        if(!cdb.ReadInt32(boardInternalCycleTime, "BoardInternalCycleTime",50)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: BoardInternalCycleTime has not been specified. Assuming %d usec triggering period", boardInternalCycleTime);
        }        
        boardInternalCycleTicks = (int64)(boardInternalCycleTime * 1e-6 * HRT::HRTFrequency());

        int32 temp = 0;
        if(!cdb.ReadInt32(temp, "DatagramMonitoringFastSleep",2)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: DataArrivalUsecSleep has not been specified. Assuming %d usec sleeping time.", temp);
        }
        datagramArrivalFastMonitorSecSleep = temp*1e-6;
        if(!cdb.ReadInt32(temp, "DataAcquisitionUsecTimeOut",1000)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: DataAcquisitionUsecTimeOut has not been specified. Assuming %d usec of timeout.", temp);
        }

        double deltaT = temp*1e-6;
        dataAcquisitionUsecTimeOut = (int64)(deltaT*HRT::HRTFrequency());

        if(!cdb.ReadInt32(temp, "AllowPollSleeping", 1)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: AllowPollSleeping has not been specified. Assuming %d ", allowPollSleeping);
	}
        allowPollSleeping = (temp == 1);

        if(!cdb.ReadFloat(worstPollSleepJitterDecayRate, "WorstPollSleepJitterDecayRate", 5e-6)){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: WorstPollSleepJitterDecayRate has not been specified. Assuming %f ", worstPollSleepJitterDecayRate);
        }
        worstPollSleepJitterDecayRate = 1 - worstPollSleepJitterDecayRate;
        if(!cdb.ReadFloat(pollSleepTimeWakeBeforeUs, "PollSleepTimeWakeBeforeUs", 20)){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: PollSleepTimeWakeBeforeUs has not been specified. Assuming %f ", pollSleepTimeWakeBeforeUs);
        }
        pollSleepTimeWakeBeforeUs *= 1e-6;
    }
    return True;
}

/** Copies the pointers to the DMA Buffers */
#ifdef _LINUX
bool SingleATCAModule::InstallDMABuffers(int32 *mappedDmaMemoryLocation){
    int32 boardSlotNums[12];
    int32 boardIdx = 0;
    int ret = ioctl(ATCAadcDrv::fileDescriptor, PCIE_ATCA_ADC_IOCT_GET_BOARD_SLOT_NS, boardSlotNums);
    if(ret != 0){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: Could not query the number of boards. ioctl returned : %d",ret);
        return False;
    }

    for(boardIdx = 0; boardIdx < 12; boardIdx++){
        if(boardSlotNums[boardIdx] == moduleIdentifier){
            break;
        }
    }

    int32 pageInc = ATCAadcDrv::pageSize / sizeof(int32);
    for(int32 i = 0; i < DMA_BUFFS; i++){
        dmaBuffers[i] = mappedDmaMemoryLocation + pageInc * (DMA_BUFFS * boardIdx + i);
        CStaticAssertErrorCondition(Information,"SingleATCAModule::InstallDMABuffers: DMABuffer[%d] %p ",i, dmaBuffers[i]);
    }

    CStaticAssertErrorCondition(Information,"SingleATCAModule::InstallDMABuffers: dmaBuffers: %p %p %p %p", dmaBuffers[0], dmaBuffers[1], dmaBuffers[2], dmaBuffers[3]);
    return True;
}
#else
/** Copies the pointers to the DMA Buffers */
bool SingleATCAModule::InstallDMABuffers(){
    int *boardDMABufferPointers = GetBoardBufferAddress(moduleIdentifier);
    if(boardDMABufferPointers == NULL) return False;
    for(int32 i = 0; i < DMA_BUFFS; i++){
        dmaBuffers[i]  = (int32 *)boardDMABufferPointers[i];
        CStaticAssertErrorCondition(Information,"SingleATCAModule::InstallDMABuffers: DMABuffer[%d] %p ",i, dmaBuffers[i]);
    }

    CStaticAssertErrorCondition(Information,"SingleATCAModule::InstallDMABuffers: dmaBuffers: %p %p %p %p", dmaBuffers[0], dmaBuffers[1], dmaBuffers[2], dmaBuffers[3]);
    return True;
}
#endif

int32 SingleATCAModule::GetLatestBufferIndex(){

    uint32 *latestBufferHeader = (uint32 *)dmaBuffers[0];
    uint32 latestBufferIndex  = 0;

    // check which one is the oldest buffer
    for (int dmaIndex = 1; dmaIndex < DMA_BUFFS; dmaIndex++) {
        // Pointer to the header
        uint32 *header     = (uint32 *)dmaBuffers[dmaIndex];
        //uint32 *footer     = header + NumberOfInputChannels() + HEADER_LENGTH;

        if ((*header > *latestBufferHeader)) {
            latestBufferHeader = header;
            latestBufferIndex  = dmaIndex;
        }
    }
    return latestBufferIndex;
}


int32 SingleATCAModule::CurrentBufferIndex(){

    uint32 *oldestBufferHeader = (uint32 *)dmaBuffers[0];
    uint32 oldestBufferIndex  = 0;

    int64  stopAcquisition    = HRT::HRTCounter()  + dataAcquisitionUsecTimeOut;

    // check which one is the oldest buffer
    int dmaIndex = 0;
    for (dmaIndex = 1; dmaIndex < DMA_BUFFS; dmaIndex++) {
        // Pointer to the header
        uint32 *header     = (uint32 *)dmaBuffers[dmaIndex];
        if (*header < *oldestBufferHeader) {
            oldestBufferHeader = header;
            oldestBufferIndex  = dmaIndex;
        }
    }

    uint32 *oldestBufferFooter = oldestBufferHeader + NumberOfInputChannels() + HEADER_LENGTH;
    uint32 oldestTimeMark      = *oldestBufferFooter;

    // If the data transfer is not in progress it means that the new data will
    // be stored in the oldest buffer. 
    int64 actualTime = HRT::HRTCounter();
    while (oldestTimeMark == *oldestBufferFooter) {
        if(actualTime > stopAcquisition) {
            return -1;
        }
        actualTime = HRT::HRTCounter();
    }

    if(*oldestBufferHeader == *oldestBufferFooter) return oldestBufferIndex;
    return -2;
}

bool SingleATCAModule::WriteData(const int32 *&buffer){    
    for(int i = 0; i < numberOfAnalogueOutputChannels; i++){
#ifdef _LINUX    
        int32 toWrite[4];
        toWrite[0] = moduleIdentifier;
        toWrite[1] = outputMap[i];
        toWrite[2] = *buffer++;
        toWrite[3] = 0;
        write(ATCAadcDrv::fileDescriptor, toWrite, 4 * sizeof(int32));
#else
        WriteToDAC(moduleIdentifier, outputMap[i], *buffer++);
#endif
    }
    for(int i = 0; i < numberOfDigitalOutputChannels; i++){
#ifdef _LINUX
        int32 toWrite[4];
        toWrite[0] = moduleIdentifier;
        toWrite[1] = 0;
        toWrite[2] = *buffer++;
        toWrite[3] = 1;
        if(write(ATCAadcDrv::fileDescriptor, toWrite, 4 * sizeof(int32)) < 0){
            CStaticAssertErrorCondition(FatalError,"SingleATCAModule::WriteData: Could not write the value : %d to module %d", toWrite[2], toWrite[0]);
            return False;
	}
#else
        WriteToDIO(moduleIdentifier, 0, *buffer++);
#endif
    }

    return True;
}

bool SingleATCAModule::Poll(){
#ifdef _LINUX
    static int firstTime = 1;
    if(firstTime == 1){
        SleepSec(1e-3);    
        firstTime = 0;
    }
#endif
    if(isMaster){
        if (synchronizing) {
            if(allowPollSleeping) {
                //Allow the worstPollSleepJitter to decay -> 0
                worstPollSleepJitter *= worstPollSleepJitterDecayRate;
                int64 tStart  = HRT::HRTCounter();
                pollSleepTime = (nextExpectedAcquisitionCPUTicks - tStart) * HRT::HRTPeriod() - pollSleepTimeWakeBeforeUs - worstPollSleepJitter;
                if(pollSleepTime > 0){
                    SleepNoMore(pollSleepTime);
                    float jitter = ((HRT::HRTCounter() - tStart) * HRT::HRTPeriod()) - pollSleepTime;
                    if(jitter < 0) jitter = -jitter;

                    if(jitter > worstPollSleepJitter){
                        worstPollSleepJitter = jitter;
                    }
                }
            }
            int32 previousAcquisitionIndex = currentDMABufferIndex;
            int32 currentDMA = CurrentBufferIndex();
            if(currentDMA < 0){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::GetData: Returned -1");
                return False;
            }

            currentDMABufferIndex = currentDMA;

            // Update NextExecTime with a guess
            nextExpectedAcquisitionCPUTicks = HRT::HRTCounter() + boardInternalCycleTicks;
            int deltaBuffer    = *dmaBuffers[currentDMABufferIndex] - *dmaBuffers[previousAcquisitionIndex];
            int nOfLostPackets = deltaBuffer - boardInternalCycleTime;
            if (( nOfLostPackets > 0)){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::GetData: Lost %d Packets", nOfLostPackets / boardInternalCycleTime);
            }
            currentMasterHeader = *(dmaBuffers[currentDMABufferIndex]);
            lastCycleUsecTime = (uint32)currentMasterHeader;
        }       
        return True;
    }
    return False;
}

bool SingleATCAModule::GetData(int32 *&buffer){

    /** Perform synchronisation */
    if(isMaster){
        if (synchronizing) {
            buffer[0] = *dmaBuffers[currentDMABufferIndex];
            buffer[1] = buffer[0];
            
            // Skip the packet sample number and sample time
            buffer += 2;
	    currentMasterHeader   = *(dmaBuffers[currentDMABufferIndex]);
	    lastCycleUsecTime     = (uint32)currentMasterHeader;
        } else {
            currentDMABufferIndex = GetLatestBufferIndex();
        }
    }else{
        int32 *header = dmaBuffers[currentDMABufferIndex];
        int32 *footer = header + NumberOfInputChannels() + HEADER_LENGTH;
        if(*header != currentMasterHeader){
            CStaticAssertErrorCondition(FatalError, "SingleATCAModule (slot=%d)::GetData: h (=%d) different from master h(=%d)", moduleIdentifier, *header, currentMasterHeader);
        return False;
        }
        if(*header != *footer){
            CStaticAssertErrorCondition(FatalError, "SingleATCAModule (slot=%d)::GetData: The header (=%d) is different from the footer(=%d)", moduleIdentifier, *header, *footer);
            return False;
        }
    }

    
    // Skip the Header in the DMA Buffer
    int32 *src           = (int32 *)dmaBuffers[currentDMABufferIndex] + 1;
    int32 *dest          = buffer;
    memcpy(dest, src, NumberOfInputChannels()*sizeof(int32));

    //This is introducing a huge delay (~1.5us per board). To be solved.
    /*for(int i=0; i<NumberOfInputChannels(); i++){
        channelStatistics[i].Update((float)(buffer[i] * 1.49e-8));
    }*/
    buffer += NumberOfInputChannels();

    return True;
}

bool SingleATCAModule::ProcessHttpMessage(HttpStream &hStream) {
    hStream.Printf("<table class=\"bltable\">\n");
    hStream.Printf("<tr>\n");
    hStream.Printf("<td>Module Identifier</td><td>%d</td>\n", moduleIdentifier);
    hStream.Printf("</tr>\n");
    hStream.Printf("<tr>\n");
    hStream.Printf("<td>Master</td><td>%s</td>\n", isMaster ? "True" : "False");
    hStream.Printf("</tr>\n");
    hStream.Printf("</table>\n"); 
    hStream.Printf("<table class=\"bltable\">\n");
    hStream.Printf("<tr><th>Channel</th><th>Last</th><th>Mean</th><th>Variance</th><th>Abs Max</th><th>Abs Min</th><th>Rel Max</th><th>Rel Min</th></tr>\n");
    int i=0;
    for(i=0; i<NumberOfInputChannels(); i++){
        hStream.Printf("<tr><td>%d</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td></tr>\n", i + 1, channelStatistics[i].LastValue(), channelStatistics[i].Mean(10), channelStatistics[i].Variance(10), channelStatistics[i].AbsMax(), channelStatistics[i].AbsMin(), channelStatistics[i].RelMax(), channelStatistics[i].RelMin());
    }
    hStream.Printf("</table>");
    return True;              
}

bool SingleATCAModule::ResetStatistics(){
    int i=0;
    for(i=0; i<NumberOfInputChannels(); i++){
        channelStatistics[i].Init();
    }
    return True;
}

ATCAadcDrv::ATCAadcDrv(){
    numberOfBoards              = 0;
    lastCycleUsecTime           = 0;
    modules                     = NULL;
    synchronizing               = False;
    softwareTrigger             = 0;    
    masterBoardIdx              = -1;
    autoSoftwareTriggerAfterUs  = -1;
    autoSoftwareTrigger         = False;
#ifdef _LINUX
    fileDescriptor              = 0;
    pageSize                    = sysconf(_SC_PAGE_SIZE);
#endif
    css = "table.bltable {"
         "margin: 1em 1em 1em 2em;"
         "background: whitesmoke;"
         "border-collapse: collapse;"
      "}"
      "table.bltable th, table.bltable td {"
          "border: 1px silver solid;"
          "padding: 0.2em;"
      "}"
      "table.bltable th {"
          "background: gainsboro;"
          "text-align: left;"
      "}"
      "table.bltable caption {"
          "margin-left: inherit;"
          "margin-right: inherit;"
      "}";
}

bool ATCAadcDrv::EnableAcquisition(){
    if(modules == NULL) return False;
#ifdef _RTAI
    return (EnableATCApcieAcquisition() == 0);
#elif defined(_LINUX)
    int ret = ioctl(fileDescriptor, PCIE_ATCA_ADC_IOCT_ACQ_ENABLE);
    if(ret != 0){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Could not enable acquisition. ioctl returned : %d",Name(), ret);
        return False;
    }
#endif
    return True;
}

bool ATCAadcDrv::DisableAcquisition(){
    if(modules == NULL) return False;
#ifdef _RTAI
    return (DisableATCApcieAcquisition() == 0);
#elif defined(_LINUX)
    int ret = ioctl(fileDescriptor, PCIE_ATCA_ADC_IOCT_ACQ_DISABLE);
    if(ret != 0){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Could not disable acquisition. ioctl returned : %d",Name(), ret);
        return False;
    }
#endif
    return True;
}

bool ATCAadcDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
#ifdef _LINUX
    fileDescriptor = open("/dev/pcieATCAAdc0", O_RDWR);
    if(fileDescriptor < 1){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Could not open device driver at: %s",Name(), "/dev/pcieATCAAdc0");
        return False;
    }
#endif
    DisableAcquisition();
    
    CDBExtended cdb(info);
    
    if(!cdb.ReadInt32(softwareTrigger, "UseSoftwareTrigger",0)){
        CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: UseSoftwareTrigger has not been specified. Assuming hardware trigger");
    }
        
    if(!GenericAcqModule::ObjectLoadSetup(cdb,err)){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: GenericAcqModule::ObjectLoadSetup failed",Name());
        return False;
    }
    
    FString syncMethod;
    if(!cdb.ReadFString(syncMethod, "SynchronizationMethod")){
        CStaticAssertErrorCondition(InitialisationError,"ATCAAdcDrv::ObjectLoadSetup: SynchronizationMethod has not been specified.");
        return False;
    }
    if (syncMethod == "GetLatest") synchronizing = False;
    else synchronizing = True;

    cdb.ReadInt32(autoSoftwareTriggerAfterUs, "AutoSoftwareTriggerAfterUs", -1);
    autoSoftwareTrigger = (autoSoftwareTriggerAfterUs > 0);
    if(autoSoftwareTrigger){
        CStaticAssertErrorCondition(Information, "ATCAadcDrv::ObjectLoadSetup: %s the system will be automatically triggered after %d us", Name(), autoSoftwareTriggerAfterUs);
    }

    // Get buffer address from the driver exported function GetBufferAddress (only works in RTAI!)
#ifdef _RTAI
    numberOfBoards             = GetNumberOfBoards();
#elif defined(_LINUX)
    int ret = ioctl(fileDescriptor, PCIE_ATCA_ADC_IOCT_NUM_BOARDS, &numberOfBoards);
    if(ret != 0){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Could not query the number of boards. ioctl returned : %d",Name(), ret);
        return False;
    }

    mappedDmaMemorySize = numberOfBoards * DMA_BUFFS * pageSize;
    mappedDmaMemoryLocation = (int32 *)mmap(0, mappedDmaMemorySize, PROT_READ, MAP_FILE | MAP_SHARED | MAP_LOCKED | MAP_POPULATE | MAP_NONBLOCK, fileDescriptor, 0);
    if(mappedDmaMemoryLocation == MAP_FAILED) {
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: MAP_FAILED",Name());
        return False;
    }
#else
    numberOfBoards             = -1;
#endif  

    if(!cdb->Move( "Modules")){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: No Module has been specified",Name());
        return False;
    }
    
    int32 nOfATCAModules = cdb->NumberOfChildren();
    if(nOfATCAModules!= numberOfBoards){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Number of installed boards [%d] differs from the number of specified boards [%d].",Name(),numberOfBoards,nOfATCAModules);
        return False;
    }

    if(modules != NULL) delete[] modules;
    modules = new SingleATCAModule[nOfATCAModules];
    if(modules == NULL){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Failed allocating space for %d modules.",Name(),nOfATCAModules);
        return False;
    }

    masterBoardIdx = -1;
    for(int i = 0; i < nOfATCAModules; i++){
        cdb->MoveToChildren(i);
        cdb.WriteFString(syncMethod,"SynchronizationMethod");
        
        if(!modules[i].ObjectLoadSetup(cdb,err)){
            AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Failed initialising module %d.",Name(),i);
            delete[] modules; 
            return False;
        }
        
        if(modules[i].isMaster){
            if(masterBoardIdx == -1){
                masterBoardIdx = i;
            }else{
                AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Failed initialising module %d. A master board was already specified at index: %d",Name(),i,masterBoardIdx);
                return False;
            }
        }
        cdb->MoveToFather();
    }

    for(int i = 0; i < nOfATCAModules; i++){
#ifdef _LINUX
        if(!modules[i].InstallDMABuffers(mappedDmaMemoryLocation)){
#else
        if(!modules[i].InstallDMABuffers()){
#endif
            AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Board %d failed to initialise DMA buffers",Name(), i);
            return False;
        }
    }

    cdb->MoveToFather();

    int32  extTriggerAndClock = 0;
    if(!cdb.ReadInt32(extTriggerAndClock, "ExtTriggerAndClock",1)){
        AssertErrorCondition(Warning,"ATCAadcDrv::ObjectLoadSetup: %s: ExtTriggerAndClock not specified, using default = %d",Name(), extTriggerAndClock);
    }

#ifdef _LINUX
    ret = ioctl(fileDescriptor, PCIE_ATCA_ADC_IOCT_SET_EXT_CLK_TRG, &extTriggerAndClock);
    if(ret != 0){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Could not SetATCApcieExternalTriggerAndClock. ioctl returned : %d",Name(), ret);
    return False;
    }
#else
    SetATCApcieExternalTriggerAndClock(extTriggerAndClock);
#endif    
    // Setup OK

    AssertErrorCondition(Information,"ATCAadcDrv::ObjectLoadSetup: %s: initialized correctly ",Name());
    EnableAcquisition();
    return True;
}

bool ATCAadcDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){

    s.Printf("%s %s\n",ClassName(),Version());

    return True;
}

bool ATCAadcDrv::WriteData(uint32 usecTime, const int32 *buffer){
    if(buffer == NULL) return False;
    const int32 *lBuffer = buffer;
    for(int i = 0; i < numberOfBoards; i++){
        modules[i].WriteData(lBuffer);
    }
    return True;
}

int32 ATCAadcDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNum){

    //Check buffer existence
    if(buffer == NULL){
        AssertErrorCondition(FatalError,"ATCAadcDrv::GetData: %s. The DDInterface buffer is NULL.",Name());
        return -1;
    }

    int32 *lBuffer = buffer;
    for(int i = 0; i < numberOfBoards; i++){
        if(!modules[i].GetData(lBuffer)){
            AssertErrorCondition(FatalError,"ATCAadcDrv::GetData: %s. Module %d failed acquiring data",Name(),i);
            return -1;
        }
    }


    return 1;
}

bool ATCAadcDrv::Poll(){
    if(autoSoftwareTrigger){
        if(lastCycleUsecTime > autoSoftwareTriggerAfterUs){
            SoftwareTrigger();
        }
    }
    bool ok = modules[masterBoardIdx].Poll();
    if(!ok){
        return ok;
    }
    // If the module is the timingATMDrv call the Trigger() method of
    // the time service object
    lastCycleUsecTime = modules[masterBoardIdx].lastCycleUsecTime;
    
    for(int i = 0; i < nOfTriggeringServices; i++){
        triggerService[i].Trigger();
    }
    return ok;
}

bool ATCAadcDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style></head><body>\n" );
    hStream.Printf("<table class=\"bltable\">\n");
    int i=0;
    hStream.Printf("<tr>\n");
    for(i=0; i<numberOfBoards; i++){
        hStream.Printf("<td>%d</td>\n", modules[i].BoardIdentifier());
    }
    hStream.Printf("</tr>\n");
    hStream.Printf("<tr>\n");
    for(i=0; i<numberOfBoards; i++){
        hStream.Printf("<td>%s</td>\n", modules[i].isMaster ? "M" : "");
    }
    hStream.Printf("</tr>\n");
    hStream.Printf("<tr>\n");
    for(i=0; i<numberOfBoards; i++){
        hStream.Printf("<td>%s</td>\n", modules[i].NumberOfOutputChannels() > 0 ? "R" : "");
    }
    hStream.Printf("</tr>\n");
    hStream.Printf("<tr>\n");
    hStream.Printf("<form>\n");
    for(i=0; i<numberOfBoards; i++){
        hStream.Printf("<td><button type=\"submit\" name=\"boardID\" value=\"%d\">.</button></td>\n", modules[i].BoardIdentifier());
    }
    hStream.Printf("</tr>\n");
    hStream.Printf("</form>\n");
    hStream.Printf("</table>\n");
    hStream.Printf("<form>\n");
    hStream.Printf("<td><button type=\"submit\" name=\"reset\" value=\"true\">Reset statistics</button></td>\n");
    hStream.Printf("</form>\n");

    FString reqBoardID;
    reqBoardID.SetSize(0);
    if (hStream.Switch("InputCommands.boardID")){
        hStream.Seek(0);
        hStream.GetToken(reqBoardID, "");
        hStream.Switch((uint32)0);
    }
    FString reqReset;
    reqReset.SetSize(0);
    if (hStream.Switch("InputCommands.reset")){
        hStream.Seek(0);
        hStream.GetToken(reqReset, "");
        hStream.Switch((uint32)0);
    }
    if(reqReset.Size() > 0){
        for(i=0; i<numberOfBoards; i++){
            modules[i].ResetStatistics();
        }
    }


    if(reqBoardID.Size() > 0){
        int32 boardID = atoi(reqBoardID.Buffer());
        for(i=0; i<numberOfBoards; i++){
            if(modules[i].BoardIdentifier() == boardID){
                modules[i].ProcessHttpMessage(hStream);
                break;
            }
        }
    }

    hStream.Printf("<p>Worst polling jitter (us): %f\n", modules[masterBoardIdx].worstPollSleepJitter * 1e6);
    hStream.Printf("<p>Last polling sleep time (us): %f", modules[masterBoardIdx].pollSleepTime * 1e6);
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}

OBJECTLOADREGISTER(ATCAadcDrv,"$Id$")
