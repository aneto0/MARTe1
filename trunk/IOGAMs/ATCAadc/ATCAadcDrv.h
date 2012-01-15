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
#include "module/pcieAdcIoctl.h"
#ifdef _LINUX
#include <sys/mman.h>
#endif

class StatSignalInfo{
private:
    /** Internal counter. Used to reset the relative statistics after maxSamples. */
    int                 counter;
    /** Sum of values of a signal. It decays with a rate decayRate. */
    double              sum;
    /** Sum squared of values of a signal. It decays with a rate decayRate. */
    double              sumPower2;
    /** Absolute maximum of a signal. */
    float               absmax;
    /** Absolute minimum of a signal. */
    float               absmin;
    /** Relative maximum of a signal. It is reset every maxSamples. */
    float               relmax;
    /** Relative minimum of a signal. It is reset every maxSamples. */
    float               relmin;
    /** The last value*/
    float               lastValue;

public:
    /** The last value of an integer sample*/
    int32               lastIntSample;
    /** The last value of an int64 sample*/
    int64               lastInt64Sample;
    /** Rate at which past samples are "forgotten". */
    float               decayRate;
    /** Samples after which the relative statistics are reset. */
    int32               maxSamples;
    /** The signal type*/
    BasicTypeDescriptor signalType;


    /** Constructor */
    StatSignalInfo():sum(0.0),sumPower2(0.0), absmax(-1e16), absmin(1e16), relmax(-1e16), relmin(1e16), counter(0), lastIntSample(0), lastInt64Sample(0), lastValue(0){
        signalType = BTDInt32;
    };

    /** 
     * Updates the lastIntSample value
     */
    void Update(int32 sample){
        lastIntSample = sample;
    }

    /** 
     * Updates the lastInt64Sample value
     */
    void Update(int64 sample){
        lastInt64Sample = sample;
    }

    /**
     * Updates statistics. To be called during the online and offline phase.
     * @param sample Value of the signal.
     */
    void Update(float sample){
        // if maxSample == -1 we are not using relative max/min
    	if (maxSamples != -1) {
            if (counter == maxSamples){
	            Reset();
	        }
        }

        lastValue                 = sample;
        sum                       = (sum * decayRate) + sample*(1-decayRate);
    	sumPower2                 = (sumPower2 * decayRate) + sample*sample*(1-decayRate);
    	if(absmax < sample)absmax = sample;
    	if(absmin > sample)absmin = sample;
        if(relmax < sample)relmax = sample;
    	if(relmin > sample)relmin = sample;
        counter++;
    }

    /** Resets the relative statistics */
    void Reset(){
    	relmax = -1e16;
    	relmin = +1e16;
        counter = 0;
    }

    /** Initialises the class. To be called in the prepulse phase. */
    void Init(){
        Reset();
    	sum = 0.0;
    	sumPower2 = 0.0;
        absmax = -1e16;
	    absmin = 1e16;
    }

    /**
     * Returns the mean of the signal.
     * @param sampleNumber Number of samples added with Update.
     */
    float Mean(int sampleNumber){
        if(sampleNumber < 2) return 0.0;
        return sum;
    }

    /**
     * Returns the variance of the signal.
     * @param sampleNumber Number of samples added with Update.
     */
    float Variance(int sampleNumber){
        if(sampleNumber < 2)return 0.0;
        float mean = Mean(sampleNumber);
        float var  = ((sumPower2) - mean*mean);
        return var;
    }

    /** Returns the absolute minimum of the signal. */
    float AbsMin(){return absmin;}
    /** Returns the relative minimum of the signal. */
    float RelMin(){return relmin;}
    /** Returns the absolute maximum of the signal. */
    float AbsMax(){return absmax;}
    /** Returns the relative maximum of the signal. */
    float RelMax(){return relmax;}
    /** Returns the last value*/
    float LastValue(){return lastValue;}
    /** 
     * Prints the last acquired integer value in binary
     */
    void PrintLastIntSampleAsBinary(HttpStream &hStream) {
        unsigned int i;
        i = 1<<(sizeof(lastIntSample) * 8 - 1);
	    while (i > 0) {
	        if (lastIntSample & i){
	            hStream.Printf("<td>1</td>");
    	    }
	        else{
	            hStream.Printf("<td>0</td>");
    	    }
	        i >>= 1;
	    }
    }
    /** 
     * Prints the last acquired integer 64 value in binary
     */
    void PrintLastInt64SampleAsBinary(HttpStream &hStream) {
        uint64 i;
        //Unfortunately this has to be performed in two steps due to the operator <<
        i = 1<<31;
        i = i<<32;
	    while (i > 0) {
	        if (lastInt64Sample & i){
	            hStream.Printf("<td>1</td>");
    	    }
	        else{
	            hStream.Printf("<td>0</td>");
    	    }
	        i >>= 1;
	    }
    }
};

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


    ////////////////////////////
    // Analogue Input Section //
    ////////////////////////////

    /** If true synchronize on data arrival, if false return latest completed buffer */
    bool             synchronizing;

    /** Pointers to the DMA memory allocated for data acquisition.
        The number of buffers is fixed to 4.
    */
    int32            *dmaBuffers[DMA_BUFFS];

    /** Current DMA buffer index [0-3].  */
    static int32     currentDMABufferIndex;

    /** The current master header (must be the same in all the boards) */
    static int32     currentMasterHeader;

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
    int64            boardInternalCycleTicks;

    /** Amount of time in microseconds after which the data stops waiting for data arrival 
        and reports an acquisition error. 
     */
    int64            dataAcquisitionUsecTimeOut;

    /** Length of a "Short Sleep" in seconds. It is used to monitor
        the data arrival on the master board and specifies a sleep time
        between checks of the data datagram arrival.
    */
    float            datagramArrivalFastMonitorSecSleep;

    /** The number of micro seconds incremented by the board in each cycle. It gives the board acquisition frequency. 
    */
    int32            boardInternalCycleTime;

    /**
     * The statistics info for these channels
     */
    StatSignalInfo  *channelStatistics;
    
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
#ifdef _LINUX
    bool InstallDMABuffers(int32 *mappedDmaMemoryLocation);
#else
    /** Copies the pointers to the DMA Buffers */
    bool InstallDMABuffers();
#endif
    
    /////////////////
    // Time Module //
    /////////////////

    int64             lastCycleUsecTime;

    int32             packetCounter;
    
    /** Is Master Board */
    bool             isMaster;


    /** Returns the sum of analogue and digital input channels */
    int32 NumberOfInputChannels(){
        return numberOfDigitalInputChannels + numberOfAnalogueInputChannels;
    }

    /** Returns the sum of analogue and digital output channels */
    int32 NumberOfOutputChannels(){
        return numberOfDigitalOutputChannels + numberOfAnalogueOutputChannels;
    }
    
    /** Returns the module Identifier */
    int32  BoardIdentifier(){return moduleIdentifier;}
    
    /**
     * Output an HTML table with the current value in mV of the acquired signals for this board
     */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
    /**
     * Resets the statistics
     */
    bool ResetStatistics();
    /**
     * Polling method
     */
    virtual bool Poll();
    /**
     * Allow sleeping, when enough time is available, before start polling
     */
    bool allowPollSleeping;
    /**
     * Time to sleep before hard polling: pollSleepTime = CycleTime - time left to cycle time - pollSleepTimeWakeBeforeUs - worstPollSleepJitter;
     */
    float pollSleepTime;
    /**
     * Actually we want to start polling some us before reaching the cycle time
     */
    float pollSleepTimeWakeBeforeUs;
    /**
     * The worst jitter calculated in realtime of the actual time slept before polling and time meant to sleep
     */
    float worstPollSleepJitter;
    /**
     * The worst jitter will try to be recovered with a certain rate
     */
    float worstPollSleepJitterDecayRate;
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
    int64               lastCycleUsecTime;
    
    /** */
    bool                synchronizing;

    /** Software triggered acquisition. 
        0, which is the default value, means hardware trigger. 
    */
    int32               softwareTrigger;

	/** The master board index
     */
    int32               masterBoardIdx;

    /** If set to true the a software trigger will be sent every
     * AutoSoftwareTriggerAfterUs microseconds. For this to be 
     * true the value of AutoSoftwareTriggerAfterUs in the configuration file
     * must be > 1
     */
    bool                autoSoftwareTrigger;
    int32               autoSoftwareTriggerAfterUs;

#ifdef _LINUX
    /** Used only in Linux. The mmapped memory location.*/
    int32               *mappedDmaMemoryLocation;
    int32                mappedDmaMemorySize;
#endif
    /** The css for this page
     */
    const char *css;

public:

#ifdef _LINUX
    /**The page size*/
    static int32 pageSize;
    
    /**The file descriptor to access the driver*/
    static int32 fileDescriptor;
#endif
    
    // (De)Constructor
    ATCAadcDrv();

    virtual ~ATCAadcDrv(){
        if(modules != NULL) delete[] modules;
#ifdef _LINUX
	munmap(mappedDmaMemoryLocation, mappedDmaMemorySize);
        close(fileDescriptor);
#endif
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
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNum = 0);
		
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

    bool SoftwareTrigger(){    
        if(modules == NULL)return False;
#ifdef _RTAI
        SendSoftwareTrigger();
#elif defined(_LINUX)
        int ret = ioctl(fileDescriptor, PCIE_ATCA_ADC_IOCT_SEND_SOFT_TRG);
        if(ret != 0){
            AssertErrorCondition(InitialisationError,"ATCAadcDrv::PulseStart: Could send software trigger. ioctl returned : %d",ret);
            return False;
        }
#endif
        return True;
    }


    //////////////////////////////////
    // Simulation Purpose Functions //
    //////////////////////////////////

    bool PulseStart(){    
        if(modules == NULL)return False;
        if(softwareTrigger == 1){
            return SoftwareTrigger();
        }
        return True;
    }

    /**
     * Output an HTML page with the current value in mV of the acquired signals
     */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
    /**
     * Polling method
     */
    virtual bool Poll();
private:

    OBJECT_DLL_STUFF(ATCAadcDrv);
};
#endif /*ATCAADCDRV_H_*/
