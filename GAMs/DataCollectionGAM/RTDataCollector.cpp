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

#include "ConfigurationDataBase.h"
#include "RTDataCollector.h"
#include "Endianity.h"
#include "FString.h"
#include "CDBExtended.h"
#include "DDBInterface.h"


bool ModifyTimeBase(StreamInterface &in,StreamInterface &out,void *userData){

    RTDataCollector  *collector = (RTDataCollector  *)userData;
    if(collector == NULL) return False;

    collector->TimeWindowsMenu(in,out);

    return True;
}


bool RTDataCollector::PrepareForNextPulse(){
    // Empty this container
    dataCollectionDelaySystem.PrepareForNextPulse();
    // Empty this container
    dataStorage.PrepareForNextPulse();
    // rebuild list of buffers from memory pool
    freeDataBuffersPool.RebuildList();
    return True;
}

void RTDataCollector::CleanUp(){
    // Empty this container
    dataCollectionDelaySystem.Reset();
    // Empty this container
    dataStorage.Reset();
    // free memory!
    freeDataBuffersPool.CleanUp();
}

bool RTDataCollector::CompleteDataCollection(){

    // Flush the delaySys
    RTCollectionBuffer *p = NULL;
    while((p = dataCollectionDelaySystem.QueueExtract()) != NULL){
        dataStorage.StoreData(*p,False);
    }
    return True;
}

bool RTDataCollector::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err, const DDBInterface *jpfInterface){

    // CleanUp
    CleanUp();

    CDBExtended cdb(info);

    // Set Object Name
    FString name;
    cdb->NodeName(name);
    SetObjectName(name.Buffer());

    if(jpfInterface == NULL){
        AssertErrorCondition(InitialisationError,"RTDataCollector::ObjectLoadSetup: %s: jpfInterface to DDB is NULL",Name());
        return False;
    }

    //Check if the user wants the signal names to be forced to upper case
    FString tmp;
    cdb.ReadFString(tmp, "ForceSignalNamesUpperCase", "True");
    signalTable.forceSignalNamesUpperCase = ((tmp == "True") || (tmp == "true"));
    ///////////////////////
    // Init Signal Table //
    ///////////////////////

    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"RTDataCollector::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    if(!signalTable.Initialize(*jpfInterface, cdb)){
        AssertErrorCondition(InitialisationError,"RTDataCollector::Initialise: %s: Failed to initialize signal Table",Name());
        return False;
    }

    cdb->MoveToFather();

    //////////////////////////
    // Init Collector Class //
    //////////////////////////

    nOfChannels = jpfInterface->BufferWordSize();
    if(nOfChannels <= 0){
        AssertErrorCondition(InitialisationError,"RTDataCollector::ObjectLoadSetup: %s: nOfChannels <= 0",Name());
        return False;
    }

    // Max number of samples available in JPF
    int32    nOfSamples = 0;
    if(!cdb.ReadInt32(nOfSamples,"NOfAcquisitionSamples")){
        AssertErrorCondition(InitialisationError,"RTDataCollector::ObjectLoadSetup: %s: Failed reading entry NOfAcquisitionSamples",Name());
        return False;
    }

    if (!freeDataBuffersPool.Init(nOfChannels,nOfSamples)){
        AssertErrorCondition(InitialisationError,"RTDataCollector::ObjectLoadSetup: %s: Failed Initialising freeDataBuffersPool for %d channels of %d samples",Name(),nOfChannels,nOfSamples);
        return False;
    }

    int32 preTrigger;
    if(!cdb.ReadInt32(preTrigger,"PreTrigger",0)){
        AssertErrorCondition(Warning,"RTDataCollector::ObjectLoadSetup: %s: Failed reading entry PreTrigger",Name());
    }

    dataCollectionDelaySystem.Init(preTrigger);

    // Check if, when calling CopyData() in Level5/Signal.cpp, standard (lower) or extra (upper) memory allcation is required
    int32 tempUseUpperMemory2CopySignal = 0;
    if(!cdb.ReadInt32(tempUseUpperMemory2CopySignal, "UseUpperMemory2CopySignal", 0)){
        AssertErrorCondition(Warning,"RTDataCollector::ObjectLoadSetup: %s: UseUpperMemory2CopySignal not specified -> using standard memory allocation",Name());
    }
    if(!signalTable.SetUseUpperMemory2CopySignal((MemoryAllocationFlags)tempUseUpperMemory2CopySignal)){
        AssertErrorCondition(InitialisationError, "RTDataCollector::ObjectLoadSetup: %s: failed setting memory partition.", Name());
	return False;
    }else{
        AssertErrorCondition(Information, "RTDataCollector::ObjectLoadSetup: %s: Successfully set memory partition 0x%x for copying signals.", Name(), signalTable.GetUseUpperMemory2CopySignal());
    }

    // Initialize the RT Data Storage System member object
    if(!cdb->Move("EventTrigger")){
        AssertErrorCondition(InitialisationError,"RTDataCollector::ObjectLoadSetup: %s: did not specify node EventTrigger",Name());
	return False;
    }

    if(!dataStorage.ObjectLoadSetup(cdb,err)){
        AssertErrorCondition(InitialisationError,"RTDataCollector::ObjectLoadSetup: %s: dataStorage.ObjectLoadSetup Failed",Name());
        return False;
    }

    cdb->MoveToFather();

    AssertErrorCondition(Information,"RTDataCollector::ObjectLoadSetup: %s: RTDataCollector Initialized Correctly",Name());

    return True;
}

bool RTDataCollector::StoreData(const uint32 *ddbInterfaceDataBuffer,uint32 usecTime, bool fastTrigger){

    // Check if the collector pool is empty
    bool delayedFastTrigger = False;

    RTCollectionBuffer *newBuffer = freeDataBuffersPool.GetFreeBuffer();
    if(newBuffer != NULL){
        newBuffer->Copy(ddbInterfaceDataBuffer,nOfChannels,usecTime, fastTrigger);
        dataCollectionDelaySystem.QueueAdd(newBuffer);
        delayedFastTrigger = (newBuffer->PacketFastTriggerRequest() != 0);
    }

    // Pops a data from the delaySys and push it in the storageSys
    if((dataCollectionDelaySystem.DelayIsFull()) || (newBuffer == NULL)){
        RTCollectionBuffer *buf = dataCollectionDelaySystem.QueueExtract();
        // the fastTrigger bypasses the delay system
        if (buf != NULL){
            RTCollectionBuffer *rejected = dataStorage.StoreData(*buf,delayedFastTrigger);
            // If the collection buffer isn't stored delete it
            if(rejected != NULL) freeDataBuffersPool.ReturnUnusedBuffer(rejected);
        }
    }

    return True;
}


bool RTDataCollector::ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err){

    FString cdbS;
    cdbS.Printf("NOfChannels = %d \n", nOfChannels);

    if(!dataCollectionDelaySystem.ObjectDescription(cdbS)){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: failed to save dataCollectionDelaySystem information",Name());
	return False;
    }

    if(!freeDataBuffersPool.ObjectDescription(cdbS)){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: failed to save freeDataBuffersPool information",Name());
	return False;
    }

    if(!dataStorage.ObjectDescription(cdbS)){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: failed to save freeDataBuffersPool information",Name());
	return False;
    }

    if(!signalTable.ObjectDescription(cdbS)){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: failed to save signalTable information",Name());
	return False;
    }

    cdbS.Seek(0);

    if(!info->ReadFromStream(cdbS)){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: failed to reading CDB info from stream",Name());
	return False;
    }

    int32 maxAcquisitionSamples = 0;
    CDBExtended cdb(info);

    if(!cdb.ReadInt32(maxAcquisitionSamples, "PointsForAcquisition")){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: failed to reading PointsForAcquisition entry from CDB",Name());
	return False;
    }

    cdb->Move("Signals");
    if(cdb->NumberOfChildren() != nOfChannels){
	AssertErrorCondition(Information,"RTDataCollector::ObjectSaveSetup: %s: Number Of signals %d differs from that in the CDB (%d)",Name(), nOfChannels, cdb->NumberOfChildren());
	return False;
    }

    for(int i = 0; i < nOfChannels; i++){
	cdb->MoveToChildren(i);
	cdb.WriteInt32(maxAcquisitionSamples, "MaxSize");
	cdb->MoveToFather();
    }

    cdb->MoveToFather();
    return True;
}


bool RTDataCollector::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("%s %s\n" ,ClassName(),Version());
    s.Printf("storing %i data \n",nOfChannels);
    bool ret = True;
    ret &= freeDataBuffersPool.ObjectDescription(s,full,err);
    s.Printf("%i buffers delayed\n",dataCollectionDelaySystem.Size());
    ret &= dataStorage.ObjectDescription(s,full,err);
    return ret;
}

void RTDataCollector::HTMLInfo(HttpStream &hStream) {
    dataStorage.HTMLInfo(hStream);
}

OBJECTREGISTER(RTDataCollector,"$Id: RTDataCollector.cpp,v 1.11 2011/07/01 14:55:30 aneto Exp $")
