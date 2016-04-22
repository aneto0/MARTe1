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
 * $Id: CircularBufferSynchDrv.cpp 3 2012-01-15 16:26:07Z aneto $
 *
 **/

#include "CircularBufferSynchDrv.h"
#include "GlobalObjectDataBase.h"

bool CircularBufferSynchDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    AssertErrorCondition(Information, "CircularBufferSynchDrv::ObjectLoadSetup: %s Loading signals", Name());

    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"CircularBufferSynchDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return false;
    }

    if(numberOfInputChannels < 1){
        AssertErrorCondition(InitialisationError,"CircularBufferSynchDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed. Set NumberOfInputs>0",Name());
    }
    if(numberOfInputChannels != numberOfOutputChannels) {
        AssertErrorCondition(InitialisationError,"CircularBufferSynchDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed. Set NumberOfInputs != numberOfOutputChannels ",Name());
        return false;
    }
    if(!cdb.ReadInt32(numberOfBuffers, "NumberOfBuffers", -1)){
        AssertErrorCondition(InitialisationError,"CircularBufferSynchDrv::ObjectLoadSetup: %s NumberOfBuffers was not specified",Name());
        return false;
    }
    if(numberOfBuffers < 1){
        AssertErrorCondition(InitialisationError,"CircularBufferSynchDrv::ObjectLoadSetup: %s at least one buffer must be specified. NumberOfBuffers = %d",Name(), numberOfBuffers);
        return false;
    }
    numberOfFreeBuffers = numberOfBuffers;
    //Allocate the shared memory
    sharedBuffer = new int32*[numberOfBuffers];
    if(sharedBuffer == NULL){
        AssertErrorCondition(FatalError, "CircularBufferSynchDrv::ObjectLoadSetup: %s Failed to allocated sharedBuffer for %d buffers", Name(), numberOfBuffers);
        return false;
    }
    uint32 i;
    for(i=0; i < numberOfBuffers; i++){
        sharedBuffer[i] = new int32[numberOfInputChannels];
    }
    return true;
}

bool CircularBufferSynchDrv::WriteData(uint32 usecTime, const int32 *ibuffer){
    memcpy(&sharedBuffer[currentBuffer][0], ibuffer, numberOfBuffers * sizeof(int32));
    currentBuffer++;
    if(currentBuffer == numberOfBuffers){
        currentBuffer = 0;
    }
    if(currentBuffer == currentReadBuffer){
        AssertErrorCondition(FatalError,"CircularBufferSynchDrv::WriteData: %s sharedBuffer overwritten for index: %d. Data lost. Try increasing number of buffers.", Name(), currentBuffer);
        return false;
    }
    numberOfFreeBuffers--;
    synchSem.Post();
    for(int i = 0 ; i < nOfTriggeringServices ; i++) {
        triggerService[i].Trigger();
    }

    return true;
}

bool CircularBufferSynchDrv::ProcessHttpMessage(HttpStream &hStream) {
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
    hStream.Printf("Current buffer = %d\n<br>\n", currentBuffer);
    hStream.Printf("Current read buffer = %d\n<br>\n", currentReadBuffer);
    hStream.Printf("</body></html>\n");

}

int32 CircularBufferSynchDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {
    memcpy(buffer, &sharedBuffer[currentReadBuffer][0], numberOfInputChannels * sizeof(int32));
    currentReadBuffer++;
    if(currentReadBuffer == numberOfBuffers){
        currentReadBuffer = 0;
    }
    numberOfFreeBuffers++;
    return 0;
}

bool CircularBufferSynchDrv::Poll(){
    if(currentReadBuffer != currentBuffer){
        return true;
    }
    synchSem.Wait();
    synchSem.Reset();
    return true;
}

OBJECTLOADREGISTER(CircularBufferSynchDrv,"$Id: CircularBufferSynchDrv.cpp 3 2012-01-15 16:26:07Z aneto $")

