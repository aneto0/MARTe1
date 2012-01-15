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

#ifndef ATCAADCDRV_H_
#define ATCAADCDRV_H_

#include "System.h"
#include "GenericAcqModule.h"
#include "FString.h"
#include "module/pcieAdc.h"

class SingleATCAModule{
private:
    /** Module Identifier */
    int32        moduleIdentifier;

    /** Number of Analogue Input channels for this module (Maximum 32)*/
    int32        numberOfAnalogueInputChannels;

    /** Number of Digital Input channels for this module  (1 or 0)    */
    int32        numberOfDigitalInputChannels;
        
    /** Number of Analogue Output channels                (Maximum 8) */
    int32        numberOfAnalogueOutputChannels;

    /** Number of Digital Output channels                 ()          */
    int32        numberOfDigitalOutputChannels;

    /** Output Map. Used to map the output to a specific physical
        output channel. Channels are identified from 1 to 8.          */
    int32        outputMap[8];

    /** Returns the sum of analogue and digital input channels */
    int32 NumberOfInputChannels(){
        return numberOfDigitalInputChannels + numberOfAnalogueInputChannels;
    }

    /** Returns the sum of analogue and digital output channels */
    int32 NumberOfOutputChannels(){
        return numberOfDigitalOutputChannels + numberOfAnalogueOutputChannels;
    }

    ////////////////////////////
    // Analogue Input Section //
    ////////////////////////////

    /** Is Master Board */
    bool             isMaster;
    
    /** If true synchronize on data arrival, if false return latest completed buffer */
    bool             synchronizing;

    /** Pointers to the DMA memory allocated for data acquisition.
        The number of buffers is fixed to 4.
    */
    int32            *dmaBuffers[DMA_BUFFS];

    /** Current DMA buffer index [0-3].  */
    static int32     currentDMABufferIndex;

    /** Estimated time of the next expected arrival in CPU Ticks of the acquired buffer.
        It is computed by adding a delay specified @param periodUsecSleep to
        the time of the previous completed acquisition. The system will sleep
        till this time elapses.
    */
    int64            nextExpectedAcquisitionCPUTicks;

    /** Specifies how long to sleep between acquisitions. It is specified in microseconds
        but it is internally converted in CPU ticks to avoid unecessary computations during
        realtime activities.
    */
    int64            periodCPUTicksSleep;

    /** Amount of time in microseconds after which the data stops waiting for data arrival 
        and reports an acquisition error. 
     */
    int64            dataAcquisitionUsecTimeOut;

    /** Length of a "Short Sleep" in seconds. It is used to monitor
        the data arrival on the master board and specifies a sleep time
        between checks of the data datagram arrival.
    */
    float            datagramArrivalFastMonitorSecSleep;

    /** Usec cycle Time during soft trigger. It is used to correct the
        time marking when in soft trigger mode. 
    */
    int32            usecCycleTimeForSoftTrigger;

    /** Software triggered acquisition. 
        0, which is the default value, means hardware trigger. 
    */
    int32            softwareTrigger;

    /** Find the currentDMABufferIndex and synchronize on data arrival
        
     */
    int32  CurrentBufferIndex();
    
    /** Find the latest completed buffer without synchronization*/
    int32 GetLatestBufferIndex();

public:

    SingleATCAModule();

    /** Initialises the SingleModule Parameter*/
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err = NULL);

    /** Reads  NumberOfInputChannels() from the DMA Buffer.
        The first module must be the master board to assure
        correct data transfer.
    */
    bool GetData(int32 *&buffer);

    bool WriteData(const int32 *&buffer);

    /** Copies the pointers to the DMA Buffers */
    bool InstallDMABuffers(int32 *dmaBufferPointers){
        if(dmaBufferPointers == NULL) return False;
        for(int32 i = 0; i < DMA_BUFFS; i++){
	    dmaBuffers[i]  = (int32 *)dmaBufferPointers[moduleIdentifier*DMA_BUFFS + i];
	}
        CStaticAssertErrorCondition(Information,"SingleATCAModule::InstallDMABuffers: dmaBuffers: %p %p %p %p", dmaBuffers[0], dmaBuffers[1], dmaBuffers[2], dmaBuffers[3]);

        return True;
    }

    /////////////////
    // Time Module //
    /////////////////

    int32             lastCycleUsecTime;

    int32             packetCounter;

    /** Returns the module Identifier */
    int32  BoardIdentifier(){return moduleIdentifier;}

    /** Returns the software identifier */
    bool  SoftwareTrigger(){
        if(isMaster)  return (softwareTrigger != 0);
        else          return False;
    }


};




OBJECT_DLL(ATCAadcDrv)
/** The high level driver for the ATCA ADC module */
class ATCAadcDrv:public GenericAcqModule{
private:

    /** Number of boards in the crate. Read during Initialisation */
    int32               numberOfBoards;

    /** Pointers to the ATCA modules */
    SingleATCAModule    *modules;

    /** Last cycle usec time */
    int32               lastCycleUsecTime;
    
    /** */
    bool                synchronizing;

public:
    // (De)Constructor
    ATCAadcDrv();

    virtual ~ATCAadcDrv(){
        if(modules != NULL) delete[] modules;
    }
		
    // Standard GAM methods
    /* Load setup from CDB.
       This IOGAM peculiar parameters are PeriodSleep_usec and FastSleep_usec. 
       @param info: CDB from which load data
       @param err: not used 
       @returns true if all ok*/
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
		
    /* Print internal GAM informations 
       @param s: StreamInterface in which print infos
       @param full: not used
       @param err: not used 
       @returns true if all ok*/
    virtual bool ObjectDescription(StreamInterface &s,bool full = False, StreamInterface *err=NULL);
		
    /* Saves the data into the DDB
       @param usecTime: not used
       @param buffer: pointer to the data buffer to be filled
       @returns 1 if all ok*/
    int32 GetData(uint32 usecTime, int32 *buffer);
		
    bool WriteData(uint32 usecTime, const int32 *buffer);
		
    // Set board used as input
    virtual bool SetInputBoardInUse(bool on = False){
        if(inputBoardInUse && on){
            AssertErrorCondition(InitialisationError, "ATCAadcDrv::SetInputBoardInUse: Board %s is already in use", Name());
            return False;
        }
        inputBoardInUse  = on;
        return True;
    }

    virtual bool SetOutputBoardInUse(bool on = False){
        if(outputBoardInUse && on){
            AssertErrorCondition(InitialisationError, "ATCAadcDrv::SetOutputBoardInUse: Board %s is already in use", Name());
            return False;
        }
        outputBoardInUse = on;
        return True;
    }

    virtual bool EnableAcquisition();

    virtual bool DisableAcquisition();

    //////////////////////
    // From Time Module //
    //////////////////////
    
    // Get the Time
    int64 GetUsecTime(){return lastCycleUsecTime;}

    //////////////////////////////////
    // Simulation Purpose Functions //
    //////////////////////////////////

    bool PulseStart(){    
        if(modules == NULL)return False;
        modules[0].packetCounter = 0; 
        return True;
    }

private:

    OBJECT_DLL_STUFF(ATCAadcDrv);
};
#endif /*ATCAADCDRV_H_*/
