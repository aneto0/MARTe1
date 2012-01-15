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
    periodCPUTicksSleep                     = 0;
    datagramArrivalFastMonitorSecSleep      = 0.0;
    usecCycleTimeForSoftTrigger             = 0;
    softwareTrigger                         = 0;

    lastCycleUsecTime                       = 0;
    packetCounter                           = 0;
    synchronizing                           = False;
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

    if(numberOfAnalogueInputChannels > 32){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueInputs is at most 32. Specified %d.",numberOfAnalogueInputChannels);
        return False;
    }

    if(numberOfDigitalInputChannels > 1){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfDigitalInputs is at most 1. Specified %d.",numberOfDigitalInputChannels);
        return False;
    }

    if(numberOfAnalogueOutputChannels > 8){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfAnalogueOutputs is at most 8. Specified %d.",numberOfAnalogueOutputChannels);
        return False;
    }

    if(numberOfAnalogueOutputChannels > 0){
    	bool hasRTM = False;
#ifdef _RTAI
	hasRTM = IsRTMPresent(moduleIdentifier);
#endif
	if(!hasRTM){
	    CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: Module %d specifies %d outputs but does not have RTM module", moduleIdentifier, numberOfAnalogueOutputChannels);
	    //return False;
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
    
    // Input Section //
    if(NumberOfInputChannels() != BUFFER_LENGTH){
        CStaticAssertErrorCondition(InitialisationError,"SingleATCAModule::ObjectLoadSetup: NumberOfInputChannels [%d] differs from the module settings [%d].",NumberOfInputChannels(), BUFFER_LENGTH);
        return False;
    }

    if(isMaster){

        if(!cdb.ReadInt32(softwareTrigger, "UseSoftwareTrigger",0)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: UseSoftwareTrigger has not been specified. Assuming hardware trigger");
        }

        if(softwareTrigger != 0){
            if(!cdb.ReadInt32(usecCycleTimeForSoftTrigger, "SoftwareTriggerUsecCycleTime",50)){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: SoftwareTriggerUsecCycleTime has not been specified. Assuming 50 usec triggering period");
            }
        }

        int32 temp = 0;
        if(!cdb.ReadInt32(temp, "DataArrivalUsecSleep",45)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: DataArrivalUsecSleep has not been specified. Assuming %d usec sleeping time.", temp);
        }

        double deltaT = temp*1e-6;
        periodCPUTicksSleep = (int64)(deltaT*HRT::HRTFrequency());

        if(!cdb.ReadInt32(temp, "DatagramMonitoringFastSleep",2)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: DataArrivalUsecSleep has not been specified. Assuming %d usec sleeping time.", temp);
        }
        
        datagramArrivalFastMonitorSecSleep = temp*1e-6;

        if(!cdb.ReadInt32(temp, "DataAcquisitionUsecTimeOut",1000)){
            CStaticAssertErrorCondition(Warning,"SingleATCAModule::ObjectLoadSetup: DataAcquisitionUsecTimeOut has not been specified. Assuming %d usec of timeout.", temp);
        }

        deltaT = temp*1e-6;
        dataAcquisitionUsecTimeOut = (int64)(deltaT*HRT::HRTFrequency());

    }
    return True;
}


int32 SingleATCAModule::GetLatestBufferIndex(){

    uint32 *latestBufferHeader = (uint32 *)dmaBuffers[0];
    uint32 latestBufferIndex  = 0;

    // check which one is the oldest buffer
    for (int dmaIndex = 1; dmaIndex < DMA_BUFFS; dmaIndex++) {
        // Pointer to the header
        uint32 *header     = (uint32 *)dmaBuffers[dmaIndex];
        uint32 *footer     = header + NumberOfInputChannels() + HEADER_LENGTH;

        if ((*header > *latestBufferHeader) || (*header == *footer)) {
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
        if(actualTime > stopAcquisition) return -1;
        SleepNoMore(datagramArrivalFastMonitorSecSleep);
	actualTime = HRT::HRTCounter();
    }

    if(*oldestBufferHeader == *oldestBufferFooter) return oldestBufferIndex;
    return -2;
}

bool SingleATCAModule::WriteData(const int32 *&buffer){

    for(int i = 0; i < numberOfAnalogueOutputChannels; i++){
        WriteToDAC(moduleIdentifier, outputMap[i], *buffer++);
    }
    return True;
}


bool SingleATCAModule::GetData(int32 *&buffer){

    /** Perform synchronisation */
    if(isMaster){
        if (synchronizing) {
            int64 tStart = HRT::HRTCounter();
            if (tStart < nextExpectedAcquisitionCPUTicks) {
                float sleepTime = (nextExpectedAcquisitionCPUTicks - tStart)*HRT::HRTPeriod();
                SleepNoMore(sleepTime);
            }

            int32 previousAcquisitionIndex = currentDMABufferIndex;
            int32 currentDMA = CurrentBufferIndex();
            if(currentDMA < 0){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::GetData: Returned -1");
                return False;
            }

            currentDMABufferIndex = currentDMA;

            // Update NextExecTime with a guess
            nextExpectedAcquisitionCPUTicks = HRT::HRTCounter() + periodCPUTicksSleep;
            int nOfLostPackets = *dmaBuffers[currentDMABufferIndex] - *dmaBuffers[previousAcquisitionIndex] - 1;
            if (( nOfLostPackets > 0)){
                CStaticAssertErrorCondition(Warning,"SingleATCAModule::GetData: Lost %d Packets", nOfLostPackets);
            }
            
            if(softwareTrigger == 1){
                buffer[0] = (packetCounter++ + nOfLostPackets);
            }else{
                buffer[0] = *dmaBuffers[currentDMABufferIndex];
            }
            buffer[1] = buffer[0]*usecCycleTimeForSoftTrigger;
            lastCycleUsecTime = buffer[1];
            // Skip the packet sample number and sample time
            buffer += 2;
            
        } else {
            currentDMABufferIndex = GetLatestBufferIndex();
        }
        
    }

    // Skip the Header in the DMA Buffer
    int32 *src           = (int32 *)dmaBuffers[currentDMABufferIndex] + 1;
    int32 *dest          = buffer;
    memcpy(dest, src, NumberOfInputChannels()*sizeof(int32));
    buffer += NumberOfInputChannels();

    return True;
}

ATCAadcDrv::ATCAadcDrv(){
    numberOfBoards    = 0;
    lastCycleUsecTime = 0;
    modules           = NULL;
    synchronizing     = False;
}

bool ATCAadcDrv::EnableAcquisition(){
    if(modules == NULL) return False;
    enableATCApcieAcquisition(modules[0].BoardIdentifier(), modules[0].SoftwareTrigger());
    return True;
}

bool ATCAadcDrv::DisableAcquisition(){
    if(modules == NULL) return False;
    disableATCApcieAcquisition(modules[0].BoardIdentifier());
    return True;
}

bool ATCAadcDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    DisableAcquisition();
    
    CDBExtended cdb(info);
	
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


    // Get buffer address from the driver exported function GetBufferAddress (only works in RTAI!)
#ifdef _RTAI
    numberOfBoards             = GetNumberOfBoards();
    int32  masterBoardIndex    = GetMasterBoard();
    int32 *dmaBufferAddress    = (int32 *)GetBufferAddress();
#else
    numberOfBoards             = -1;
    int32  masterBoardIndex    = -1;
    int32 *dmaBufferAddress    = NULL;
#endif	

    if(numberOfBoards > 2){
        AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: This Driver cannot work with more than 2 ATCA modules",Name());
        return False;
    }
    
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

    for(int i = 0; i < nOfATCAModules; i++){
        cdb->MoveToChildren(i);
        if(i == 0) {
            cdb.WriteInt32(masterBoardIndex , "ModuleIdentifier");
            cdb.WriteFString(syncMethod,"SynchronizationMethod");
        }
        else      {
            if(masterBoardIndex == 0)cdb.WriteInt32(1 , "ModuleIdentifier");
            else                     cdb.WriteInt32(0 , "ModuleIdentifier");
        }
        
        if(!modules[i].ObjectLoadSetup(cdb,err)){
            AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Failed initialising module %d.",Name(),i);
            delete[] modules; 
            return False;
        }
        cdb->MoveToFather();
    }

    for(int i = 0; i < nOfATCAModules; i++){
        for(int j = 0; j < DMA_BUFFS; j++)AssertErrorCondition(Information,"DMABuffer[%d] %p ",j, dmaBufferAddress[DMA_BUFFS*i + j]);
        if(!modules[i].InstallDMABuffers(dmaBufferAddress)){
            AssertErrorCondition(InitialisationError,"ATCAadcDrv::ObjectLoadSetup: %s: Board %d failed to initialise DMA buffers",Name(), i);
            return False;
        }
    }

    cdb->MoveToFather();

    int32  extTriggerAndClock = 0;
    if(!cdb.ReadInt32(extTriggerAndClock, "ExtTriggerAndClock",1)){
        AssertErrorCondition(Warning,"ATCAadcDrv::ObjectLoadSetup: %s: ExtTriggerAndClock not specified, using default = %d",Name(), extTriggerAndClock);
    }

    setATCApcieExternalTriggerAndClock(masterBoardIndex, extTriggerAndClock);
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

int32 ATCAadcDrv::GetData(uint32 usecTime, int32 *buffer){

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

    lastCycleUsecTime = modules[0].lastCycleUsecTime;
	
    // If the module is the timingATMDrv call the Trigger() method of
    // the time service object
    if(synchronizing){
        for(int i = 0; i < nOfTriggeringServices; i++){
            triggerService[i].Trigger();
        }
    }

    return 1;
}


OBJECTLOADREGISTER(ATCAadcDrv,"$Id: ATCAadcDrvNoExtClkTrg.cpp,v 1.1 2008/11/21 14:12:49 ppcc_dev Exp $")
