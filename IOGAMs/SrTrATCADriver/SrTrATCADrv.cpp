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

#include "SrTrATCADrv.h"
#include "Directory.h"


SrTrATCADrv::SrTrATCADrv(){
    device                       = 0;
    memoryDownloadThreadPriority = 31;
    memoryDownloadThreadCpuMask  = 0xff;
    memoryDownloadThreadId       = 0;
    rtThreadPriority             = 31;
    rtThreadCpuMask              = 0xff;
    rtThreadId                   = 0;
    slotNumber                   = 0;
    acquisitionType              = SINGLE;
    acquisitionMode              = RAW;
    frequency                    = FREQUENCY_250_400;
    triggerType                  = HARDWARE;
    triggerMode                  = LEVEL;
    triggerFilter                = NONE;
    inputFilter                  = NONE;
    preTriggerSamples            = 0;
    pulseWidth                   = 0;
    triggerAccuracy              = 0;
    triggerDelay                 = 0;
    processK                     = 0;
    processL                     = 0;
    processM                     = 0;
    processT                     = 0;
    acquisitionByteSize          = 0;
    readByteSize                 = 0;
    dmaByteSize                  = 0;
    invertInputSignal            = 0;
    numberOfConnectedChannels    = 0;
    sizeMismatchErrorCounter     = 0;
    lostPacketErrorCounter       = 0;
    rtUsecPeriod                 = 0;
    lastPacketUsecTime           = 0;
    offlineMSecTick              = 0;
    postFrequencyToTicksFactor   = 0;

    memoryDownloadThreadKeepRunning = False;
    rtThreadKeepRunning             = False;
    acquisitionInProgress           = False;
    rtAcquisitionInProgress         = False;
    acquisitionAborted              = False;
    realtimeEnabled                 = False;

    externalCalibrationFile      = "";
    baseDirectoryRamData         = "";
    relativeDirectoryRamData     = "";

    int32 i=0;
    for(i=0; i<N_CHANNELS; i++){
        channelConnected[i]       = False;
        channelRamDataFilename[i] = "";
    }

    memoryDownloadEventSem.Create();
    rtThreadEventSem.Create();
    dmaPathMux.Create();
}

SrTrATCADrv::~SrTrATCADrv(){
    memoryDownloadThreadKeepRunning = False;
    rtThreadKeepRunning             = False;
    DisableAcquisition();
    int32 counter = 0;
    memoryDownloadEventSem.Post();
    while((!rtThreadKeepRunning) && (counter++ < 200)){
        SleepMsec(1);
    }
    counter = 0;
    while((!memoryDownloadThreadKeepRunning) && (counter++ < 200)){
        SleepMsec(1);
    }
    if(Threads::IsAlive(memoryDownloadThreadId)) {
        AssertErrorCondition(Warning,"SrTrATCADrv::~SrTrATCADrv: %s: Had To Kill Thread %d",Name(), memoryDownloadThreadId);
        Threads::Kill(memoryDownloadThreadId);
        memoryDownloadThreadId = 0;
    }
    else{
        AssertErrorCondition(Information,"SrTrATCADrv::~SrTrATCADrv: %s: Successfully waited for Thread %d to die on its own",Name(), memoryDownloadThreadId);
    }
    if(Threads::IsAlive(rtThreadId)) {
        AssertErrorCondition(Warning,"SrTrATCADrv::~SrTrATCADrv: %s: Had To Kill Thread %d",Name(), rtThreadId);
        Threads::Kill(memoryDownloadThreadId);
        rtThreadId = 0;
    }
    else{
        AssertErrorCondition(Information,"SrTrATCADrv::~SrTrATCADrv: %s: Successfully waited for Thread %d to die on its own",Name(), rtThreadId);
    }
    memoryDownloadEventSem.Close();
    dmaPathMux.Close();

    //Close the device
    if(device > 1){
        if(close(device) != 0){
            AssertErrorCondition(FatalError,"SrTrATCADrv::DisableAcquisition: %s Failed to close device",Name());
        }
    }
}

void MemoryDownloadThreadCallback(void* userData){
    SrTrATCADrv *p = (SrTrATCADrv*)userData;   
    p->MemoryDownloadCallback(userData);
}

void RTDataThreadCallback(void* userData){
    SrTrATCADrv *p = (SrTrATCADrv*)userData;   
    p->RTDataCallback(userData);
}

void SrTrATCADrv::MemoryDownloadCallback(void* arg){

    // Set Thread priority
    if(memoryDownloadThreadPriority) {
        Threads::SetRealTimeClass();
        Threads::SetPriorityLevel(memoryDownloadThreadPriority);
    }

    memoryDownloadThreadKeepRunning = True;	
    while(memoryDownloadThreadKeepRunning) {
        memoryDownloadEventSem.ResetWait();
        if(!memoryDownloadThreadKeepRunning){
            break;
        }
        if(rtAcquisitionInProgress){
            AssertErrorCondition(Warning, "SrTrATCADrv::MemoryDownloadThreadCallback: %s Tried to store data with acquisition in progress. Stopping acquisition.", Name());
            if(!StopAcquisition()){
                continue;
            }
        }
        dmaPathMux.Lock();
        StoreRamData();
        dmaPathMux.UnLock();
    }

    memoryDownloadThreadKeepRunning = True;	
}

void SrTrATCADrv::RTDataCallback(void* arg){

    // Set Thread priority
    if(rtThreadPriority) {
        Threads::SetRealTimeClass();
        Threads::SetPriorityLevel(memoryDownloadThreadPriority);
    }
    int64 lastCounterTime = 0;
    int64 oneMinCounterTime = HRT::HRTFrequency()*60;

    rtThreadKeepRunning = True;	
    while(rtThreadKeepRunning) {
        //Give a non real-time tick
        if(!rtThreadEventSem.ResetWait(offlineMSecTick)){
            int32 i = 0;
            latestRTDataPacket.usecTime += 2000;
            if(latestRTDataPacket.usecTime >= 4000000000u){
                latestRTDataPacket.usecTime = 0;
            }
            for(i = 0; i < nOfTriggeringServices; i++) {
                triggerService[i].Trigger();
            }
        }
        while(rtAcquisitionInProgress){
            if(!rtThreadKeepRunning){
                break;
            }
            //RT logic
            dmaPathMux.Lock();
            int32 ret = read(device, &latestRTDataPacket, sizeof(RTDataPacket));
            if(ret == -1) {
                AssertErrorCondition(FatalError, "SrTrATCADrv::RTDataCallback: read error");
                dmaPathMux.UnLock();
                return;
            }
            //Check statistics
            if(ret != sizeof(RTDataPacket)) {
                sizeMismatchErrorCounter++;
            }
            latestRTDataPacket.usecTime *= HEADER_COUNTS_TO_TIME;
            if((latestRTDataPacket.usecTime - lastPacketUsecTime) > rtUsecPeriod){
                lostPacketErrorCounter++;
            }
            lastPacketUsecTime = latestRTDataPacket.usecTime;
            dmaPathMux.UnLock();
            int32 i = 0;
            for(i = 0; i < nOfTriggeringServices; i++) {
                triggerService[i].Trigger();
            }
            int64 currentCounterTime = HRT::HRTCounter();
            if(currentCounterTime - lastCounterTime > oneMinCounterTime) {
                if(sizeMismatchErrorCounter > 0) {
                    AssertErrorCondition(FatalError, "SrTrATCADrv::RTDataCallback: Wrong packet occured %d times", sizeMismatchErrorCounter);
                    sizeMismatchErrorCounter = 0;
                }
                if(lostPacketErrorCounter > 0) {
                    AssertErrorCondition(Warning, "SrTrATCADrv::RTDataCallback: packets lost occured %d times", lostPacketErrorCounter);
                    lostPacketErrorCounter = 0;
                }
                lastCounterTime = currentCounterTime;
            }
        }
        //rtThreadEventSem.Post();
    }
    rtThreadKeepRunning = True;	
}

bool SrTrATCADrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }

    if(!cdb.ReadInt32(slotNumber, "SlotNumber")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s SlotNumber must be specified",Name());
        return False;
    }

    int32 temp;
    //Check if a channel is connected (this can be overriden by the interleave settings)
    if(!cdb.ReadInt32(temp, "Channel1Connected")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel1Connected must be specified",Name());
        return False;
    }
    channelConnected[0] = (temp == 1);

    if(!cdb.ReadInt32(temp, "Channel2Connected")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel2Connected must be specified",Name());
        return False;
    }
    channelConnected[1] = (temp == 1);

    if(!cdb.ReadInt32(temp, "Channel3Connected")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel3Connected must be specified",Name());
        return False;
    }
    channelConnected[2] = (temp == 1);

    if(!cdb.ReadInt32(temp, "Channel4Connected")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel4Connected must be specified",Name());
        return False;
    }
    channelConnected[3] = (temp == 1);

    //Check the acquisition type (the interleave setting are set later with the Interleaved parameter
    if(!cdb.ReadInt32(temp, "AcquisitionType")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s AcquisitionType must be specified",Name());
        return False;
    }
    //0 is SINGLE 1 is INTERLEAVED
    if(temp == 0){
        acquisitionType == SINGLE;
    }
    else if(temp != 1){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported AcquisitionType requested: %d",Name(), temp);
        return False;
    }

    //Check the Interleaved parameter (these might override the connection of channels)
    if(!cdb.ReadInt32(temp, "Interleaved")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Interleaved must be specified",Name());
        return False;
    }
    //2 -> INTERLEAVED_2, 4 -> INTERLEAVED_4
    if(acquisitionType != SINGLE){
        if(temp == 2){
            acquisitionType = INTERLEAVED_2;
            channelConnected[0] = True;
            channelConnected[1] = True;
            channelConnected[2] = False;
            channelConnected[3] = False;
        }
        else if(temp == 4){
            acquisitionType = INTERLEAVED_4;
            channelConnected[0] = True;
            channelConnected[1] = False;
            channelConnected[2] = False;
            channelConnected[3] = False;
        }
        else{
            AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported Interleaved requested: %d",Name(),temp);
            return False;
        }
    }

    if(!cdb.ReadInt32(temp, "TriggerFilter")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s TriggerFilter must be specified",Name());
        return False;
    }
    if(temp == 0){
        triggerFilter = NONE;
    }
    else if(temp == 1){
        triggerFilter = FIR;
    }
    else{
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported TriggerFilter requested: %d",Name(),temp);
        return False;
    }

    if(!cdb.ReadInt32(temp, "InputFilter")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s InputFilter must be specified",Name());
        return False;
    }
    if(temp == 0){
        inputFilter = NONE;
    }
    else if(temp == 1){
        inputFilter = FIR;
    }
    else{
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported InputFilter requested: %d",Name(),temp);
        return False;
    }

    if(!cdb.ReadInt32(invertInputSignal, "InvertInputSignal")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s InvertInputSignal must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(temp, "AcquisitionMode")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s AcquisitionMode must be specified",Name());
        return False;
    }
    if(temp == 0){
        acquisitionMode = RAW;
    }
    else if(temp == 2){
        acquisitionMode = PROCESSED;
    }
    else if(temp == 1){
        acquisitionMode = SEGMENTED;
    }
    else if(temp == 3){
        acquisitionMode = CALIBRATION;
    }
    else{
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported AcquisitionMode requested: %d",Name(),temp);
        return False;
    }

    float tempf;
    if(!cdb.ReadFloat(tempf, "Frequency")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Frequency must be specified",Name());
        return False;
    }
    if(tempf == 250 || tempf == 400){
        frequency = FREQUENCY_250_400;
    }
    else if(tempf == 200){
        frequency = FREQUENCY_200;
    }
    else if(tempf == 125 || tempf == 160){
        frequency = FREQUENCY_125_160;
    }
    else if(tempf == 100){
        frequency = FREQUENCY_100;
    }
    else if(tempf == 62.5 || tempf == 80){
        frequency = FREQUENCY_62_5_80;
    }
    else if(tempf == 50){
        frequency = FREQUENCY_50;
    }
    else{
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported Frequency requested: %d",Name(),temp);
        return False;
    }

    //Parse the trigger type
    if(!cdb.ReadInt32(temp, "TriggerType")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s TriggerType must be specified",Name());
        return False;
    }
    if(temp == 0){
        triggerType = SOFTWARE;
    }
    else if(temp == 1){
        triggerType = HARDWARE;
    }
    else{
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported TriggerType requested: %d",Name(),temp);
        return False;
    }

    //Parse the trigger mode 
    if(!cdb.ReadInt32(temp, "TriggerMode")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s TriggerMode must be specified",Name());
        return False;
    }
    if(temp == 0){
        triggerMode = LEVEL;
    }
    else if(temp == 1){
        triggerMode = FLANK;
    }
    else{
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Unsupported TriggerMode requested: %d",Name(),temp);
        return False;
    }

    //For the current time ignore ExternalCalibrationFile
/*    if(!cdb.ReadFString(externalCalibrationFile, "ExternalCalibrationFile")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s must be specified",Name());
        return False;
    }*/

    if(!cdb.ReadInt32(preTriggerSamples, "PreTriggerSamples")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s PreTriggerSamples must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(pulseWidth, "PulseWidth")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s PulseWidth must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(triggerAccuracy, "TriggerAccuracy")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s TriggerAccuracy must be specified",Name());
        return False;
    }
    //The actual ioctl value starts in zero
    triggerAccuracy--;

    //This trigger delay is in us
    if(!cdb.ReadUint32(triggerDelay, "TriggerDelay")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s TriggerDelay must be specified",Name());
        return False;
    }
    if(!cdb.ReadUint32(postFrequencyToTicksFactor, "PostFrequencyToTicksFactor")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s PostFrequencyToTicksFactor must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(processK, "ProcessK")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ProcessK must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(processL, "ProcessL")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ProcessL must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(processM, "ProcessM")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ProcessM must be specified",Name());
        return False;
    }

    if(!cdb.ReadInt32(processT, "ProcessT")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ProcessT must be specified",Name());
        return False;
    }

    if(!cdb.ReadUint32(acquisitionByteSize, "AcqByteSize")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s AcqByteSize must be specified",Name());
        return False;
    }

    if(!cdb.ReadUint32(readByteSize, "ReadByteSize")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ReadByteSize must be specified",Name());
        return False;
    }

    if(!cdb.ReadUint32(dmaByteSize, "DmaByteSize")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s DmaByteSize must be specified",Name());
        return False;
    }

    //Check the correct settings for readByteSize and dmaByteSize
    if(readByteSize < 1){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ReadByteSize must be > 1 (and not %d)",Name(),readByteSize);
        return False;
    }
    if((readByteSize % dmaByteSize) != 0){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s ReadByteSize must be a multiple of DmaByteSize (and not %d, with DmaByteSize=%d)",Name(),readByteSize,dmaByteSize);
        return False;
    }

    //Read the ram data file names
    if(!cdb.ReadFString(channelRamDataFilename[0], "Channel1RamDataFilename")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel1RamDataFilename must be specified",Name());
        return False;
    }
    if(!cdb.ReadFString(channelRamDataFilename[1], "Channel2RamDataFilename")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel2RamDataFilename must be specified",Name());
        return False;
    }
    if(!cdb.ReadFString(channelRamDataFilename[2], "Channel3RamDataFilename")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel3RamDataFilename must be specified",Name());
        return False;
    }
    if(!cdb.ReadFString(channelRamDataFilename[3], "Channel4RamDataFilename")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s Channel4RamDataFilename must be specified",Name());
        return False;
    }
    if(!cdb.ReadFString(baseDirectoryRamData, "BaseDirectoryRamData")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s BaseDirectoryRamData must be specified",Name());
        return False;
    }

    //Check the number of connected channels
    numberOfConnectedChannels = 0;
    int32 i=0;
    for(i=0; i<N_CHANNELS; i++){
        if(channelConnected[i]){
            numberOfConnectedChannels++;
        }
    }

    //The number of acquisitionByteSize, divided by the number of channels must be a multiple of readByteSize
    while(((acquisitionByteSize/ numberOfConnectedChannels) % readByteSize) != 0){
        acquisitionByteSize++;
    }
    if(acquisitionByteSize > 0x7FFFFFFF){
        acquisitionByteSize = 0x7FFFFFFF;
    }
    AssertErrorCondition(Information,"SrTrATCADrv::ObjectLoadSetup: %s acquisitionByteSize set to %d", Name(), acquisitionByteSize);


    //Thread parameters
    if(!cdb.ReadInt32(memoryDownloadThreadPriority, "MemoryDownloadThreadPriority")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s MemoryDownloadThreadPriority must be specified", Name());
        return False;
    }
    if(!cdb.ReadInt32(memoryDownloadThreadCpuMask, "MemoryDownloadThreadCpuMask")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s MemoryDownloadThreadCpuMask must be specified", Name());
        return False;
    }
    FString memoryDownloadThreadName = Name();
    memoryDownloadThreadName        += "MemoryDownloadThreadHandler";
    memoryDownloadThreadId           = Threads::BeginThread((ThreadFunctionType)MemoryDownloadThreadCallback, (void*)this, THREADS_DEFAULT_STACKSIZE, memoryDownloadThreadName.Buffer(), XH_NotHandled, memoryDownloadThreadCpuMask);
    //Check if the thread started as expected...
    int32 counter = 0;
    while((!memoryDownloadThreadKeepRunning) && (counter++ < 100)){
        SleepMsec(1);
    }
    if(!memoryDownloadThreadKeepRunning) {
        AssertErrorCondition(InitialisationError, "SrTrATCADrv::ObjectLoadSetup: MemoryDownloadThreadCallback failed to start");
        return False;
    }

    //Check if the RT part is to be used
    int32 tmp = 0;
    cdb.ReadInt32(tmp, "RealtimeEnabled", 1);
    realtimeEnabled = (tmp == 1);
    if(!cdb.ReadInt32(rtThreadPriority, "RTThreadPriority")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s RTThreadPriority must be specified", Name());
        return False;
    }
    if(!cdb.ReadInt32(rtThreadCpuMask , "RTThreadCpuMask")){
        AssertErrorCondition(InitialisationError,"SrTrATCADrv::ObjectLoadSetup: %s RTThreadCpuMask must be specified", Name());
        return False;
    }
    FString rtThreadName = Name();
    rtThreadName        += "RTDataThreadHandler";
    rtThreadId           = Threads::BeginThread((ThreadFunctionType)RTDataThreadCallback, (void*)this, THREADS_DEFAULT_STACKSIZE, rtThreadName.Buffer(), XH_NotHandled, rtThreadCpuMask);
    //Check if the thread started as expected...
    counter = 0;
    while((!rtThreadKeepRunning) && (counter++ < 1000)){
        SleepMsec(1);
    }
    if(!rtThreadKeepRunning) {
        AssertErrorCondition(InitialisationError, "SrTrATCADrv::ObjectLoadSetup: RTDataCallback failed to start");
        return False;
    }

    cdb.ReadInt32(rtUsecPeriod, "RTUsecPeriod", 2000);
    cdb.ReadInt32(offlineMSecTick, "OfflineMSecTick", 2);
    EnableAcquisition();
    return True;
}

bool SrTrATCADrv::EnableAcquisition(){
    //Try open the device in this slot
    FString devLocation;
    devLocation.Printf("/dev/pcie%d", slotNumber);
    device = open(devLocation.Buffer(), O_RDWR);
    if(device < 1){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to open device in %s",Name(),devLocation.Buffer());
        return False;
    }

    //The trigger delay is in ticks
    uint32 triggerDelayTicks = (int)(triggerDelay / 1e6 * postFrequencyToTicksFactor);
    //ioctl all the properties
    if(ioctl(device, PCIE_SRTR_IOCS_DQTP,        &acquisitionMode) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set acquisitionMode = %d in %s", Name(), acquisitionMode, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_ILVM,        &acquisitionType) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set acquisitionType = %d in %s", Name(), acquisitionType, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_PLLCFG,      &frequency) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set frequency = %d in %s", Name(), frequency, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_PTRG,        &preTriggerSamples) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set preTriggerSamples = %d in %s", Name(), preTriggerSamples, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_PWIDTH,      &pulseWidth) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set pulseWidth = %d in %s", Name(), pulseWidth, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_Trg_Acc,     &triggerAccuracy) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set triggerAccuracy = %d in %s", Name(), triggerAccuracy, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_DATA_PROC_K, &processK) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set processK = %d in %s", Name(), processK, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_DATA_PROC_L, &processL) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set processL = %d in %s", Name(), processL, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_DATA_PROC_M, &processM) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set processM = %d in %s", Name(), processM, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_THRESHOLD,   &processT) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set processT = %d in %s", Name(), processT, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_COMPL,       &invertInputSignal) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set invertInputSignal = %d in %s", Name(), invertInputSignal, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_TRGM,        &triggerMode) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set triggerMode = %d in %s", Name(), triggerMode, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_INFIR,       &inputFilter) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set inputFilter = %d in %s", Name(), inputFilter, devLocation.Buffer());
        return False;
    }
    int32 channelConnectedMask = 0x0;
    channelConnectedMask ^= channelConnected[0];
    channelConnectedMask ^= (channelConnected[1] << 1);
    channelConnectedMask ^= (channelConnected[2] << 2);
    channelConnectedMask ^= (channelConnected[3] << 3);

    if(ioctl(device, PCIE_SRTR_IOCS_CHAN_ON_OFF, &channelConnectedMask) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set inputFilter = %d in %s", Name(), inputFilter, devLocation.Buffer());
        return False;
    }
/*    if(ioctl(device, PCIE_SRTR_IOCS_TFIR,        &triggerFilter) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set triggerFilter = %d in %s", Name(), triggerFilter, devLocation.Buffer());
        return False;
    }*/
    if(ioctl(device, PCIE_SRTR_IOCS_ACQBYTESIZE, &acquisitionByteSize) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set acquisitionByteSize = %d in %s", Name(), acquisitionByteSize, devLocation.Buffer());
        return False;
    }
    if(ioctl(device, PCIE_SRTR_IOCS_POST_TRG,    &triggerDelayTicks) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::EnableAcquisition: %s Failed to set triggerDelayTicks = %d in %s", Name(), triggerDelayTicks, devLocation.Buffer());
        return False;
    }
    
    return True;
}

bool SrTrATCADrv::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
    if(!envelope.IsValid()){
        AssertErrorCondition(FatalError, "SrTrATCADrv::ProcessMessage: %s: Received invalid envelope", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    FString sender               = envelope->Sender();
    bool    replyExpected        = envelope->ManualReplyExpected();
    if (message.IsValid()){
        int32   code    = message->GetMessageCode().Code();
        FString content = message->Content();
        AssertErrorCondition(Information,"SrTrATCADrv::ProcessMessage: %s: Received Message %s from %s", Name(), content.Buffer(), sender.Buffer());
        bool ret        = False;
        if(content == MSG_START_ACQUISITION){
            ret = StartAcquisition();
        }
        else if(content == MSG_STOP_ACQUISITION){
            ret = StopAcquisition();
        }
        else if(content == MSG_ABORT_ACQUISITION){
            acquisitionAborted = True;
            ret = StopAcquisition();
        }
        else if(content == MSG_STORE_ACQUISITION){
            if(!acquisitionAborted){
                ret = memoryDownloadEventSem.Post();
            }
            else{
                AssertErrorCondition(Warning, "SrTrATCADrv::ProcessMessage: %s: data will not be downloaded since acquisition was aborted", Name());
                ret = True;
            }
        }
        else if(code == MSG_RELATIVE_DIR_CODE){
            relativeDirectoryRamData = content; 
            AssertErrorCondition(Information, "SrTrATCADrv::ProcessMessage: %s: data will be stored in relative directory with name: %s", relativeDirectoryRamData.Buffer());
            //Try to create the directory
            FString directory;
            directory.Printf("/%s/%s/", baseDirectoryRamData.Buffer(), relativeDirectoryRamData.Buffer());
            Directory::Create(directory.Buffer());
            ret = Directory::DirectoryExists(directory.Buffer());
            if(!ret){
                AssertErrorCondition(FatalError, "SrTrATCADrv::ProcessMessage: %s: failed to create directory: %s", Name(), directory.Buffer());
            }
        }
        else{
            AssertErrorCondition(FatalError, "SrTrATCADrv::ProcessMessage: %s: received invalid message content %s with code: %d", Name(), content.Buffer(), code);
        }
        if(replyExpected){
            GCRTemplate<Message> gcrtm(GCFT_Create);
            if(!gcrtm.IsValid()){
                AssertErrorCondition(FatalError, "SrTrATCADrv::ProcessMessage: %s: Failed creating response message", Name());
                return False;
            }
            GCRTemplate<MessageEnvelope> mec(GCFT_Create);
            if (!mec.IsValid()){
                AssertErrorCondition(FatalError,"SrTrATCADrv::ProcessMessage: %s: Failed creating reply message", Name());
                return False;
            }
            if(ret == False){
                gcrtm->Init(0, "ERROR");
            }
            else{
                gcrtm->Init(0, "OK");
            }
            mec->PrepareReply(envelope,gcrtm);
            MessageHandler::SendMessage(mec);
        }
    }
    else{
        return False;
    }
 
    return True;
}

bool SrTrATCADrv::DisableAcquisition(){
    //Stop any acquisition 
    StopAcquisition();
    
    return True;
}

bool SrTrATCADrv::StartAcquisition(){
    //Reset the offset
    int32 offsetRst = 0;
    if(ioctl(device, PCIE_SRTR_IOCS_RDOFF, &offsetRst) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::StartAcquisition: %s Failed to reset the offset", Name());
        return False;
    }
    if(realtimeEnabled){
        //This must be set to 0 so that the DMAs are set to the RT path (it uses the obsolete parameter PCIE_SRTR_IOCS_TFIR)
        int32 dmaPath = 0;
        if(ioctl(device, PCIE_SRTR_IOCS_TFIR, &dmaPath) < 0){
            AssertErrorCondition(FatalError,"SrTrATCADrv::StartAcquisition: %s Failed to set triggerFilter = %d", Name(), triggerFilter);
            return False;
        }
        if(ioctl(device, PCIE_SRTR_IOCT_RT_ENABLE) < 0){
            AssertErrorCondition(FatalError,"SrTrATCADrv::StartAcquisition: %s Failed to set RT enable", Name());
            return False;
        }
        rtAcquisitionInProgress = True;
    }
    else{
        int32 dmaPath = 1;
        if(ioctl(device, PCIE_SRTR_IOCS_TFIR, &dmaPath) < 0){
            AssertErrorCondition(FatalError,"SrTrATCADrv::StartAcquisition: %s Failed to set triggerFilter = %d", Name(), triggerFilter);
            return False;
        }
    }
    if(ioctl(device, PCIE_SRTR_IOCT_ACQ_ENABLE) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::StartAcquisition: %s Failed to start the acquisition", Name());
        return False;
    }
    acquisitionInProgress = True;
    acquisitionAborted    = False;
    rtThreadEventSem.Post();
    return True;
}

bool SrTrATCADrv::StopAcquisition(){
    acquisitionInProgress   = False;
    rtAcquisitionInProgress = False;
    dmaPathMux.Lock();
    if(ioctl(device, PCIE_SRTR_IOCT_ACQ_DISABLE) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::StopAcquisition: %s Failed to stop the acquisition", Name());
        dmaPathMux.UnLock();
        return False;
    }
    if(realtimeEnabled){
        if(ioctl(device, PCIE_SRTR_IOCT_RT_DISABLE) < 0){
            AssertErrorCondition(FatalError,"SrTrATCADrv::StopAcquisition: %s Failed to set RT disable", Name());
            dmaPathMux.UnLock();
            return False;
        }
    }
    dmaPathMux.UnLock();
    return True;
}

bool SrTrATCADrv::StoreRamData(){
    //Offset in the memory
    int32  offsetJump              = 0x80000000 / numberOfConnectedChannels;
    //Current channel offset
    int32  channelOffset           = 0;
    //Number of bytes to read for each channel
    uint32 totalBytesPerChannel    = acquisitionByteSize / numberOfConnectedChannels;
    //An index incremented only if a parameter is connected (required for the offsets)
    int32  connectedParameterIndex = 0;
    //Buffer to read from the driver
    char *buffer = (char *)malloc(readByteSize);
    if(buffer == NULL){
        AssertErrorCondition(FatalError, "SrTrATCADrv::StoreRamData: %s Failed to allocate %d bytes for read buffer", Name(), readByteSize);
        return False;
    }

    //This must be set to 1 so that the DMAs are sent from the RT memory path (it uses the obsolete TFIR parameter)
    int32 dmaPath = 1;
    if(ioctl(device, PCIE_SRTR_IOCS_TFIR, &dmaPath) < 0){
        AssertErrorCondition(FatalError,"SrTrATCADrv::StartAcquisition: %s Failed to set triggerFilter = %d", Name(), triggerFilter);
        return False;
    }
    uint32 i=0;
    for(i=0; i<N_CHANNELS; i++){
        if(!channelConnected[i]){
            AssertErrorCondition(Information, "SrTrATCADrv::StoreRamData: %s Channel %d is not connected, no data will be stored", Name(), i);
        }

        //Try to open the file to sink the data into
        FString absoluteFileLocation = "";
        absoluteFileLocation.Printf("/%s/%s/%s", baseDirectoryRamData.Buffer(), relativeDirectoryRamData.Buffer(), channelRamDataFilename[i].Buffer());
        File channelOutputFile;
        if(!channelOutputFile.OpenWrite(absoluteFileLocation.Buffer())){
            AssertErrorCondition(FatalError, "SrTrATCADrv::StoreRamData: %s Failed to open file %s to store the acquired data", Name(), absoluteFileLocation.Buffer());
            free((void *&)buffer);
            return False;
        }
        //Calculate the correct offset for channel i
        if(i > 0 && numberOfConnectedChannels == 3){
            if(i == 1){
                channelOffset = 0x20000000;
            }
            else if(i == 2){
                channelOffset = 0x40000000;
            }
        }
        else{
            channelOffset = connectedParameterIndex * offsetJump;
        }
        connectedParameterIndex++;
        //set the offset
        if(ioctl(device, PCIE_SRTR_IOCS_RDOFF, &channelOffset) < 0){
            AssertErrorCondition(FatalError, "SrTrATCADrv::StoreRamData: %s Failed to set offset %d for channel %d", Name(), channelOffset, i);
            free((void *&)buffer);
            channelOutputFile.Close();
            return False;
        }

        //while bytes available for channel, read
        uint32 toReadBytes = totalBytesPerChannel;
        uint32 readBytes   = 0;
        while(toReadBytes > 0){
            memset(buffer, readByteSize, 0);
            if(toReadBytes < readByteSize){
                readBytes = read(device, buffer, toReadBytes);
            }
            else{
                readBytes = read(device, buffer, readByteSize);
            }
            if(readBytes < 1){
                AssertErrorCondition(FatalError, "SrTrATCADrv::StoreRamData: %s Failed to read for channel %d", Name(), i);
                break;
            }

            toReadBytes -= readBytes;
            //And sink to file
            channelOutputFile.Write(buffer, readBytes);

        }
        channelOutputFile.Close();
    }
    free((void *&)buffer);
    return True;
}

int32 SrTrATCADrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber){
    //First sample is the time
    buffer[0] = latestRTDataPacket.usecTime;
    //Followed by all the counts and pile ups
    buffer[1] = latestRTDataPacket.ch1Counts;
    buffer[2] = latestRTDataPacket.ch1PileUp;
    buffer[3] = latestRTDataPacket.ch2Counts;
    buffer[4] = latestRTDataPacket.ch2PileUp;
    buffer[5] = latestRTDataPacket.ch3Counts;
    buffer[6] = latestRTDataPacket.ch3PileUp;
    buffer[7] = latestRTDataPacket.ch4Counts;
    buffer[8] = latestRTDataPacket.ch4PileUp;
    //Finally copy all the energies
    int32      i = 0;
    int32      j = 0;
    uint16 *dest = (uint16 *)&buffer[9];
    uint16 *src  = (uint16 *)&latestRTDataPacket.energies[0];
    for(i=0; i<N_CHANNELS; i++){
        for(j=0; j<N_ENERGIES_PER_CHANNEL; j++){
            //Unfortunately the DDB does not support for the moment types < 32 bits
            //In order to have a multiple of 32 bits for each number of energies
            //need to add +1 to the N_ENERGIES_PER_CHANNEL, and this faked last energy is forced to 0
            dest[i * (N_ENERGIES_PER_CHANNEL + 1) + j] = src[j * N_CHANNELS + i];
        }
        dest[i * (N_ENERGIES_PER_CHANNEL + 1) + j] = 0;
    }
    return 0;
}

//Trigger if software trigger is set
bool SrTrATCADrv::PulseStart() {
    if(triggerType == SOFTWARE){
        if(ioctl(device, PCIE_SRTR_IOCT_SOFT_TRIG) < 0){
            AssertErrorCondition(FatalError, "SrTrATCADrv::PulseStart: Board %s failed to send software trigger", Name());
            return False;
        }
        AssertErrorCondition(Information, "SrTrATCADrv::PulseStart: Board %s sent software trigger", Name());
    }
    return True;
}

bool SrTrATCADrv::ProcessHttpMessage(HttpStream &hStream){
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    const char *css = "table.bltable {"
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
    hStream.Printf( "<STYLE type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</STYLE>\n" );

    hStream.Printf("<HTML><HEAD><TITLE>%s</TITLE></HEAD><BODY>",Name());
    hStream.Printf("<H2>Last Received Packet</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TD COLSPAN=2>%d</TD></TR>\n", latestRTDataPacket.usecTime);
    hStream.Printf("<TR><TD>%d</TD><TD>%d</TD></TR>\n", latestRTDataPacket.ch1Counts, latestRTDataPacket.ch1PileUp);
    hStream.Printf("<TR><TD>%d</TD><TD>%d</TD></TR>\n", latestRTDataPacket.ch2Counts, latestRTDataPacket.ch2PileUp);
    hStream.Printf("<TR><TD>%d</TD><TD>%d</TD></TR>\n", latestRTDataPacket.ch3Counts, latestRTDataPacket.ch3PileUp);
    hStream.Printf("<TR><TD>%d</TD><TD>%d</TD></TR>\n", latestRTDataPacket.ch4Counts, latestRTDataPacket.ch4PileUp);
    hStream.Printf("</TABLE>\n");

    hStream.Printf("<H2>Status</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TD>Acquisition in progress</TD><TD>%s</TD></TR>\n", acquisitionInProgress ? "yes" : "no");
    hStream.Printf("<TR><TD>Acquisition aborted</TD><TD>%s</TD></TR>\n",  acquisitionAborted ? "yes" : "no");
    hStream.Printf("<TR><TD>Realtime acquisition in progress?</TD><TD>%s</TD></TR>\n", rtAcquisitionInProgress ? "Yes" : "No");
    hStream.Printf("<TR><TD>Realtime enabled?</TD><TD>%s</TD></TR>\n", realtimeEnabled ? "Yes" : "No");
    hStream.Printf("</TABLE>\n");

    hStream.Printf("<H2>Errors</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TD>Size mismatch</TD><TD>%d</TD></TR>\n", sizeMismatchErrorCounter);
    hStream.Printf("<TR><TD>Lost packets</TD><TD>%d</TD></TR>\n",  lostPacketErrorCounter);
    hStream.Printf("</TABLE>\n");

    hStream.Printf("<H2>Configuration info</H2>\n");
    hStream.Printf("<TABLE CLASS=\"bltable\">\n");
    hStream.Printf("<TR><TH>Parameter</TH><TH>Value</TH></TR>\n");
    hStream.Printf("<TR><TD>SlotNumber</TD><TD>%d</TD></TR>\n", slotNumber);

    FString acquisitionModeStr = "";
    if(acquisitionMode == RAW){
        acquisitionModeStr.Printf("%d (RAW)", RAW);
    }
    else if(acquisitionMode == PROCESSED){
        acquisitionModeStr.Printf("%d (PROCESSED)", PROCESSED);
    }
    else if(acquisitionMode == SEGMENTED){
        acquisitionModeStr.Printf("%d (SEGMENTED)", SEGMENTED);
    }
    else if(acquisitionMode == CALIBRATION){
        acquisitionModeStr.Printf("%d (CALIBRATION)", CALIBRATION);
    }
    else{
        acquisitionModeStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Acquisition Mode</TD><TD>%s</TD></TR>\n", acquisitionModeStr.Buffer());

    FString acquisitionTypeStr;
    if(acquisitionType == SINGLE){
        acquisitionTypeStr.Printf("%d (SINGLE)", SINGLE);
    }
    else if(acquisitionType == INTERLEAVED_2){
        acquisitionTypeStr.Printf("%d (INTERLEAVED_2)", INTERLEAVED_2);
    }
    else if(acquisitionType == INTERLEAVED_4){
        acquisitionTypeStr.Printf("%d (INTERLEAVED_4)", INTERLEAVED_4);
    }
    else{
        acquisitionTypeStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Acquisition Type</TD><TD>%s</TD></TR>\n", acquisitionTypeStr.Buffer());

    FString frequencyStr;
    if(frequency == FREQUENCY_250_400){
        frequencyStr.Printf("%d (FREQUENCY_250_400)", FREQUENCY_250_400);
    }
    else if(frequency == FREQUENCY_200){
        frequencyStr.Printf("%d (FREQUENCY_200)", FREQUENCY_200);
    }
    else if(frequency == FREQUENCY_125_160){
        frequencyStr.Printf("%d (FREQUENCY_125_160)", FREQUENCY_125_160);
    }
    else if(frequency == FREQUENCY_100){
        frequencyStr.Printf("%d (FREQUENCY_100)", FREQUENCY_100);
    }
    else if(frequency == FREQUENCY_62_5_80){
        frequencyStr.Printf("%d (FREQUENCY_62_5_80)", FREQUENCY_62_5_80);
    }
    else if(frequency == FREQUENCY_50){
        frequencyStr.Printf("%d (FREQUENCY_50)", FREQUENCY_50);
    }
    else{
        frequencyStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Frequency</TD><TD>%s</TD></TR>\n", frequencyStr.Buffer());

    FString triggerTypeStr;
    if(triggerType == SOFTWARE){
        triggerTypeStr.Printf("%d (SOFTWARE)", SOFTWARE);
    }
    else if(triggerType == HARDWARE){
        triggerTypeStr.Printf("%d (HARDWARE)", HARDWARE);
    }
    else{
        triggerTypeStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Trigger Type</TD><TD>%s</TD></TR>\n", triggerTypeStr.Buffer());

    FString triggerModeStr;
    if(triggerMode == LEVEL){
        triggerModeStr.Printf("%d (LEVEL)", LEVEL);
    }
    else if(triggerMode == FLANK){
        triggerModeStr.Printf("%d (FLANK)", FLANK);
    }
    else{
        triggerModeStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Trigger Mode</TD><TD>%s</TD></TR>\n", triggerModeStr.Buffer());

    FString triggerFilterStr;
    if(triggerFilter == NONE){
        triggerFilterStr.Printf("%d (NONE)", NONE);
    }
    else if(triggerFilter == FIR){
        triggerFilterStr.Printf("%d (FIR)", FIR);
    }
    else{
        triggerFilterStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Trigger Filter</TD><TD>%s</TD></TR>\n", triggerFilterStr.Buffer());

    FString inputFilterStr;
    if(inputFilter == NONE){
        inputFilterStr.Printf("%d (NONE)", NONE);
    }
    else if(inputFilter == FIR){
        inputFilterStr.Printf("%d (FIR)", FIR);
    }
    else{
        inputFilterStr.Printf("UNKNOWN!"); 
    }
    hStream.Printf("<TR><TD>Input Filter</TD><TD>%s</TD></TR>\n", inputFilterStr.Buffer());

    hStream.Printf("<TR><TD>Pre-Trigger Samples</TD><TD>%d</TD></TR>\n", preTriggerSamples);
    hStream.Printf("<TR><TD>Pulse Width</TD><TD>%d</TD></TR>\n", pulseWidth);
    hStream.Printf("<TR><TD>Trigger Accuracy</TD><TD>%d</TD></TR>\n", triggerAccuracy);
    hStream.Printf("<TR><TD>Trigger Delay</TD><TD>%d</TD></TR>\n", triggerDelay);
    hStream.Printf("<TR><TD>Post time to ticks factor</TD><TD>%d</TD></TR>\n", postFrequencyToTicksFactor);
    hStream.Printf("<TR><TD>Process K</TD><TD>%d</TD></TR>\n", processK);
    hStream.Printf("<TR><TD>Process L</TD><TD>%d</TD></TR>\n", processL);
    hStream.Printf("<TR><TD>Process M</TD><TD>%d</TD></TR>\n", processM);
    hStream.Printf("<TR><TD>Process T</TD><TD>%d</TD></TR>\n", processT);
    hStream.Printf("<TR><TD>AcquisitionByteSize</TD><TD>%d</TD></TR>\n", acquisitionByteSize);
    hStream.Printf("<TR><TD>ReadByteSize</TD><TD>%d</TD></TR>\n", readByteSize);
    hStream.Printf("<TR><TD>DmaByteSize</TD><TD>%d</TD></TR>\n", dmaByteSize);
    hStream.Printf("<TR><TD>Invert Input Signal</TD><TD>%d</TD></TR>\n", invertInputSignal);
    hStream.Printf("<TR><TD>Number Of Connected Channels</TD><TD>%d</TD></TR>\n", numberOfConnectedChannels);

    hStream.Printf("<TR><TD>BaseDirectoryRamData</TD><TD>%s</TD></TR>\n", baseDirectoryRamData.Buffer());
    hStream.Printf("<TR><TD>RelativeDirectoryRamData</TD><TD>%s</TD></TR>\n", relativeDirectoryRamData.Buffer());

    int32 i=0;
    for(i=0; i<N_CHANNELS; i++){
        hStream.Printf("<TR><TD>Channel %d connected?</TD><TD>%s</TD></TR>\n",     i, channelConnected[i] ? "Yes" : "No");
        hStream.Printf("<TR><TD>Channel %d store filename</TD><TD>%s</TD></TR>\n", i, channelRamDataFilename[i].Buffer());
    }

    hStream.Printf("</TABLE>\n");
    hStream.Printf("</BODY></HTML>");
    return True;
}
OBJECTLOADREGISTER(SrTrATCADrv, "$Id: SrTrATCADrv.cpp,v 1.7 2011/12/13 21:01:17 aneto Exp $")

