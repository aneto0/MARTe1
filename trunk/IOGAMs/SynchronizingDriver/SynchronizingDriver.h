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

#ifndef SYNCHRONIZINGDRIVER_H_
#define SYNCHRONIZINGDRIVER_H_

#include "System.h"
#include "GenericAcqModule.h"

OBJECT_DLL(SynchronizingDriver)

class SynchronizingDriver: public GenericAcqModule{

OBJECT_DLL_STUFF(SynchronizingDriver)

private:
    
    /** Number of down sampling to perform. */
    int32  downSamplingSize;
    
    ////////////////////////
    // Buffer Information // 
    ////////////////////////
    
    /** Memory Buffer (NumberOfInputs()*downSamplingSize*2)*/
    int32  *memoryBuffer;
    
    /** Pointer to the end of the memory buffer */
    int32  *endOfMemoryBuffer;
    
    /** Pointer to the last received buffer */
    int32  *readBuffer;

    /** Pointer to the last received buffer when GetData is called with bufferNumber 0*/
    int32  *readBufferGetData;
    
    /** Pointer to the writing position */
    int32  *writeBuffer;

    ////////////////////////
    // Timing information //
    ////////////////////////
    
    /** Last time mark */
    int32   lastUsecTime;
    
    /** Return the actual Time as microseconds */
    virtual int64 GetUsecTime(){
        int64 time = (uint32)lastUsecTime;
        return time;
    }
    
public:
    
    /** Constructor. */
    SynchronizingDriver();

    /** Destructor */
    virtual ~SynchronizingDriver();
    
public:

    /** ObjectLoadSetup */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
    
    /** Save parameter */
    virtual bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL){
        return True;
    }

public:
    
    /** Set board used as input */
    virtual bool SetInputBoardInUse(bool on = True){

        if(inputBoardInUse && on) {
            AssertErrorCondition(InitialisationError, "SynchronizingDriver::SetInputBoardInUse: Board %s is already in use", Name());
            return False;
        }

        inputBoardInUse  = on;
        return True;
    }

    virtual bool SetOutputBoardInUse(bool on = True){

        if(outputBoardInUse && on){
            AssertErrorCondition(InitialisationError, "SynchronizingDriver::SetOutputBoardInUse: Board %s is already in use", Name());
            return False;
        }

        outputBoardInUse = on;
        return True;
    }

public:
    
    /////////////////////////////////////////////////////////////////////////////////
    //                            Output Modules Methods                           //
    /////////////////////////////////////////////////////////////////////////////////

    /** Update the output of the module using n = numberOfOutputChannels data word
        of the source buffer starting from absoluteOutputPosition */
    virtual bool WriteData(uint32 usecTime, const int32 *buffer);

    
    /////////////////////////////////////////////////////////////////////////////////
    //                             Input Modules Methods                           //
    /////////////////////////////////////////////////////////////////////////////////
    
    /** Copy local input buffer in destination buffer:
        return  0 if data not ready
        return <0 if error
        return >0 if OK
    */
    virtual int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);
    
    /** Number Of Downsampling */
    virtual int32  NumberOfBuffers(){return downSamplingSize;}

};

#endif /*SYNCHRONIZINGDRIVER_H_*/
