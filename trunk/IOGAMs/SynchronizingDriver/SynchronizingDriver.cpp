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

#include "System.h"
#include "SynchronizingDriver.h"
#include "CDBExtended.h"

bool SynchronizingDriver::WriteData(uint32 usecTime, const int32 *buffer){
    
    // Get the last time mark
    lastUsecTime = usecTime;
    
    // Store a copy of the writeBuffer;
    int32 *readBufferTemp = writeBuffer;
   
    /* Copy data to local buffer */
    for (int i = 0; i < NumberOfOutputs(); i++)  *writeBuffer++ = *buffer++;
    
    /** Perform circular memory check */
    if(writeBuffer == endOfMemoryBuffer) writeBuffer = memoryBuffer;
    
    // Update the read Buffer ponter
    readBuffer = readBufferTemp;
    /** Trigger External Activities */
    for(int activity = 0; activity < nOfTriggeringServices; activity++){
        triggerService[activity].Trigger();
    }
    
    return True;
}

int32 SynchronizingDriver::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber){
    /** Cannot read future buffers */
    if(bufferNumber  > 0) return -1;
    if(bufferNumber == 0) readBufferGetData = readBuffer;
    
    // If all buffers have been read, update the readBuffer pointer and return False
    int32 *localPointer = readBufferGetData +  bufferNumber*NumberOfInputs();
    if(localPointer < memoryBuffer) {
        int32 difference =  (memoryBuffer - localPointer)/sizeof(int32); 
        localPointer     =   endOfMemoryBuffer - difference; 
    }

    /* Copy data to local buffer */
    for (int i = 0; i < NumberOfInputs() ; i++)  *buffer++ = *localPointer++;
    
    return 1;
}

bool SynchronizingDriver::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdb(info);

    // Common initializations
    if(!GenericAcqModule::ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"SynchronizingDriver::ObjectLoadSetup: %s Generic initialization failed",Name());
        return False;
    }
    
    if(NumberOfInputs() != NumberOfOutputs()){
        AssertErrorCondition(InitialisationError,"SynchronizingDriver::ObjectLoadSetup: %s. Number of Input signals [%d] must equal the number of output signals [%d]",Name(),NumberOfInputs(), NumberOfOutputs());
        return False;
    }
 
    if(!cdb.ReadInt32(downSamplingSize,"DownSamplingSize")){
        AssertErrorCondition(InitialisationError,"SynchronizingDriver::ObjectLoadSetup: %s DownSamplingSize not declared",Name());
        return False;
    }

    if(memoryBuffer     != NULL) free((void *&)memoryBuffer);
    memoryBuffer = (int32 *)malloc(NumberOfInputs()*(downSamplingSize*2)*sizeof(int32));
    if(memoryBuffer == NULL){
        AssertErrorCondition(InitialisationError,"SynchronizingDriver::ObjectLoadSetup: %s Failed allocating %d words for storing data",Name(),NumberOfInputs()*(downSamplingSize*2));        
        return False;
    }
    
    memset(memoryBuffer, 0, NumberOfInputs()*(downSamplingSize*2)*sizeof(int32));

    endOfMemoryBuffer = memoryBuffer + NumberOfInputs()*(downSamplingSize*2);
    readBuffer        = memoryBuffer;
    writeBuffer       = memoryBuffer;
    
    return True;
}

SynchronizingDriver::~SynchronizingDriver(){
   if(memoryBuffer       != NULL) free((void *&)memoryBuffer);
}

SynchronizingDriver::SynchronizingDriver(){

    downSamplingSize   = 0;
    memoryBuffer       = NULL;
    endOfMemoryBuffer  = NULL;
    readBuffer         = NULL;
    writeBuffer        = NULL;
    
    lastUsecTime       = 0;
}


OBJECTLOADREGISTER(SynchronizingDriver,"$Id$")

