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
 * $Id: FileWriterDrv.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/

#include "FileWriterDrv.h"
#include "GlobalObjectDataBase.h"

/**
 * Receiver CallBack
 */
void ThreadWriterCallback(void* userData){
    FileWriterDrv *p = (FileWriterDrv*)userData;   
    p->WriteDataToDisk();
}


void FileWriterDrv::WriteDataToDisk(){
    //Counter for shared buffers
    int32 i = 0;
    //Counter for number of signals 
    int32 j = 0;
    uint32 size  = 0;
    bool ok = False;
    running = True;
    int64 lastWriteCounter = HRT::HRTCounter();
    int64 ellapsedWriteCounter = lastWriteCounter;
    int64 lastWriteSize = 0;
    while(running){
        //Wait for the next buffer to be free
        sharedBufferMux.FastLock();
        sharedBufferSem.Reset();
        sharedBufferMux.FastUnLock();
        sharedBufferSem.Wait();
        while(sharedBuffer[i][0] == 1){
            //Write to disk
            for(j=0; j<numberOfOutputFiles; j++){
                size = numberOfBytesPerSignal;
                lastWriteSize += size;
                ok = outputFiles[j].Write(&sharedBuffer[i][1] + j * numberOfWordsPerSignal, size);
                if(!ok){
                    AssertErrorCondition(FatalError,"FileWriterDrv::WriteDataToDisk: %s Could not write data for file: %s",Name(),outputFilenames[j].Buffer());
                }
                if(size != numberOfBytesPerSignal){
                    AssertErrorCondition(FatalError,"FileWriterDrv::WriteDataToDisk: %s size != numberOfBytesPerSignal: %d != %d for file: %s",Name(),size,numberOfBytesPerSignal,outputFilenames[j].Buffer());
                }
            }
            sharedBufferMux.FastLock();
            sharedBuffer[i][0] = 0;
            numberOfFreeBuffers++;
            sharedBufferMux.FastUnLock();
            ellapsedWriteCounter = HRT::HRTCounter() - lastWriteCounter;
            bandwidthToDisk = lastWriteSize / (ellapsedWriteCounter * HRT::HRTPeriod());
            lastWriteCounter = HRT::HRTCounter();
            lastWriteSize = 0;
            //Look for the next buffer
            i++;
            if(i == numberOfBuffers){
                i = 0;
            }
        } 
    }
    for(j=0; j<numberOfOutputFiles; j++){
        AssertErrorCondition(Information,"FileWriterDrv::WriteDataToDisk: %s Closing file: %s",Name(),outputFilenames[j].Buffer());
        if(!outputFiles[j].Close()){
            AssertErrorCondition(FatalError,"FileWriterDrv::WriteDataToDisk: %s Failed closing file: %s",Name(),outputFilenames[j].Buffer());
        }
    }
    running = True;
}

bool FileWriterDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    AssertErrorCondition(Information, "FileWriterDrv::ObjectLoadSetup: %s Loading signals", Name());

    CDBExtended cdb(info);
    if(!GenericAcqModule::ObjectLoadSetup(info,err)){
        AssertErrorCondition(InitialisationError,"FileWriterDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed",Name());
        return False;
    }

    if(numberOfInputChannels != 2) {
        AssertErrorCondition(InitialisationError,"FileWriterDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed. Please set NumberOfInputs=2 to account for the statistics channels",Name());
        return False;
    }

    //Retrieve the number of files to write (each channel will be stored in a separate file)
    int32 arrayDimension = 1;
    int32 arraySize[1];
    arraySize[0] = 0;
    if(cdb->GetArrayDims(arraySize, arrayDimension, "OutputFiles")){
        numberOfOutputFiles = arraySize[0];
        AssertErrorCondition(Information, "FileWriterDrv::ObjectLoadSetup: %s OutputFiles has %d elements", Name(), numberOfOutputFiles);
        //Try to allocate memory
        outputFilenames = new FString[numberOfOutputFiles];
        if(outputFilenames == NULL){
            AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup: %s Failed to allocate %d elements for outputFilenames", Name(), numberOfOutputFiles);
            return False;
        }
        outputFiles = new File[numberOfOutputFiles];
        if(outputFiles == NULL){
            AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup: %s Failed to allocate %d elements for outputFiles", Name(), numberOfOutputFiles);
            return False;
        }
        //Do the actual reading of values
        if(!cdb.ReadFStringArray(outputFilenames, arraySize, arrayDimension, "OutputFiles")){
            AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup: %s Failed reading data to OutputFiles", Name());
            return False;
        }
    } 
    else{
        AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup: %s Failed OutputFiles Array", Name());
        return False;
    }

    if(!cdb.ReadInt32(numberOfBuffers, "NumberOfBuffers", -1)){
        AssertErrorCondition(InitialisationError,"FileWriterDrv::ObjectLoadSetup: %s NumberOfBuffers was not specified",Name());
        return False;
    }
    if(numberOfBuffers < 1){
        AssertErrorCondition(InitialisationError,"FileWriterDrv::ObjectLoadSetup: %s at least one buffer must be specified. NumberOfBuffers = %d",Name(), numberOfBuffers);
        return False;
    }
    numberOfFreeBuffers = numberOfBuffers;
    numberOfBytesPerSignal = numberOfOutputChannels / numberOfOutputFiles * sizeof(int32);
    numberOfWordsPerSignal = numberOfBytesPerSignal / sizeof(int32);
    //Allocate the shared memory
    int32 i=0;
    sharedBuffer = new int32*[numberOfBuffers];
    if(sharedBuffer == NULL){
        AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup: %s Failed to allocated sharedBuffer for %d buffers", Name(), numberOfBuffers);
        return False;
    }
    for(i=0; i<numberOfBuffers; i++){
        //First item is used to mark if the buffer should be consumed, this is why we have +1
        //Each buffer holds all the samples (this also supports arrays of data) for all the channels per control cycle 
        sharedBuffer[i] = new int32[numberOfOutputChannels + 1];
        if(sharedBuffer[i] == NULL){
            AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup: %s Failed to allocated sharedBuffer for %d bytes", Name(), numberOfBytesPerSignal + sizeof(int32));
            return False;
        }
        sharedBuffer[i][0] = 0;
    }

    if(!cdb.ReadInt32(cpuMask, "CpuMask", 0xFFFF)){
        AssertErrorCondition(Warning,"UDPDrv::ObjectLoadSetup: %s CpuMask was not specified. Using default: %d",Name(),cpuMask);
    }
    if(cdb.ReadInt32(threadPriority, "ThreadPriority", 31)) {
        if(threadPriority > 32 || threadPriority < 0) {
            AssertErrorCondition(InitialisationError, "UDPDrv::ObjectLoadSetup: %s ThreadPriority parameter must be <= 32 and >= 0", Name());
            return False;
        }
    }
    int32 j=0;
    for(j=0; j<numberOfOutputFiles; j++){
        AssertErrorCondition(Information,"FileWriterDrv::WriteDataToDisk: %s Opening file: %s",Name(),outputFilenames[j].Buffer());
        if(!outputFiles[j].OpenWrite(outputFilenames[j].Buffer())){
            AssertErrorCondition(InitialisationError,"FileWriterDrv::WriteDataToDisk: %s Failed opening file: %s",Name(),outputFilenames[j].Buffer());
        }
    }

    running = False;
    FString threadName = Name();
    threadName += "_WriteDataToDisk";
    threadID = Threads::BeginThread((ThreadFunctionType)ThreadWriterCallback, (void*)this, THREADS_DEFAULT_STACKSIZE, threadName.Buffer(), XH_NotHandled, cpuMask);
    int counter = 0;
    while((!running) && (counter++ < 100)) {
        SleepMsec(1);
    }
    if(!running) {
        AssertErrorCondition(InitialisationError, "FileWriterDrv::ObjectLoadSetup:%s WriteDataToDisk failed to start", Name());
        return False;
    }
    return True;
}

bool FileWriterDrv::StopWriteDataToDiskThread(){
    running = False;
    uint32 counter = 0;
    while(!running){
        if(counter++ > 100){
            AssertErrorCondition(FatalError, "FileWriterDrv::ObjectLoadSetup:%s WriteDataToDisk failed to stop after 10 s", Name());
            running = False;
            return False;
        }
        SleepMsec(100);
    } 
    running = False;
    return True;
}

bool FileWriterDrv::ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
    if(!envelope.IsValid()){
        AssertErrorCondition(FatalError, "FileWriterDrv::ProcessMessage: %s: Received invalid envelope", Name());
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    FString sender               = envelope->Sender();
    bool    replyExpected        = envelope->ManualReplyExpected();
    if (message.IsValid()){
        int32   code    = message->GetMessageCode().Code();
        FString content = message->Content();
        AssertErrorCondition(Information,"FileWriterDrv::ProcessMessage: %s: Received Message %s from %s", Name(), content.Buffer(), sender.Buffer());
        bool ret        = False;
        if(content == MSG_STOP){
            ret = StopWriteDataToDiskThread();
        }
        else{
            AssertErrorCondition(FatalError, "FileWriterDrv::ProcessMessage: %s: received invalid message content %s with code: %d", Name(), content.Buffer(), code);
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

/**
 * GetData
 */
bool FileWriterDrv::WriteData(uint32 usecTime, const int32 *ibuffer){
    if(!running){
        AssertErrorCondition(FatalError,"FileWriterDrv::WriteData: %s consumer thread is not running. For each run you are forced to upload a new configuration...",Name());
        return False;
    }
    //Buffer overrun! Fully cycle of writing in the shared buffer performed and the consumer thread
    //has still not consumed the latest buffer...
    sharedBufferMux.FastLock();
    if(sharedBuffer[lastWriteIdx][0] != 0){
        AssertErrorCondition(FatalError,"FileWriterDrv::WriteData: %s sharedbuffer overwritten for index: %d. Data lost. Try increasing number of buffers.",Name(),lastWriteIdx);
        sharedBufferMux.FastUnLock();
        return False;
    }
    sharedBufferMux.FastUnLock();
    memcpy(&sharedBuffer[lastWriteIdx][1], ibuffer, numberOfBytesPerSignal * numberOfOutputFiles);
    sharedBufferMux.FastLock();
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

bool FileWriterDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

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
    hStream.Printf("Bandwidth to disk (MB/s) = %f\n<br>\n", bandwidthToDisk/1e6);
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

int32 FileWriterDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) {
    memcpy(buffer, &numberOfFreeBuffers, sizeof(int32));
    memcpy(buffer+1, &bandwidthToDisk, sizeof(float));
    return 0;
}

OBJECTLOADREGISTER(FileWriterDrv,"$Id: FileWriterDrv.cpp 3 2012-01-15 16:26:07Z aneto $")

