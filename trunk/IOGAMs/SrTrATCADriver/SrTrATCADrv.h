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

#if !defined (_SR_TR_ATCA_DRV)
#define _SR_TR_ATCA_DRV

/**
 * @file GenericAcqModule for the SrTr ATCA boards developed by IST
 * RAM acquisition is enabled and disabled using messages with expected content:
 * START_ACQUISITION
 * STOP_ACQUISITION
 * STORE_ACQUISITION
 */

#include "System.h"
#include "MessageHandler.h"
#include "File.h"
#include "EventSem.h"
#include "MutexSem.h"
#include "GenericAcqModule.h"
#include "pcieSrTr_ioctl.h"
#include <sys/ioctl.h>

/**
 * Each board has 4 physical channels
 */
#define N_CHANNELS 4

/**
 * Possible messages that can be sent
 */
#define MSG_START_ACQUISITION "START_ACQUISITION"
#define MSG_STOP_ACQUISITION  "STOP_ACQUISITION"
#define MSG_ABORT_ACQUISITION "ABORT_ACQUISITION"
#define MSG_STORE_ACQUISITION "STORE_ACQUISITION"
#define MSG_RELATIVE_DIR_CODE 0xAAFF

/**
 * The number of energies per channel
 */
#define N_ENERGIES_PER_CHANNEL 247

/**
 * Translates header counts to time
 */
#define HEADER_COUNTS_TO_TIME 4

/**
 * The RT packet structure
 */
struct RTDataPacket{
    uint32  usecTime;
    uint16  ch1Counts;
    uint16  ch1PileUp;
    uint16  ch2Counts;
    uint16  ch2PileUp;
    uint16  ch3Counts;
    uint16  ch3PileUp;
    uint16  ch4Counts;
    uint16  ch4PileUp;
    //organised as E1_Ch1|E2_Ch2|E3_Ch3|E4_Ch4...|E_N_ENERGIES_PER_CHANNEL_Ch1|...|E_N_ENERGIES_PER_CHANNEL_Ch4
    uint16  energies[N_CHANNELS * N_ENERGIES_PER_CHANNEL];
    //padding
    uint8   padding[52];
};

/**
 * The filter options
 */
enum SrTrFilter{
    NONE = 0,
    FIR  = 1
};

/**
 * Possible acquisition types. See acquisitionType
 */
enum SrTrAcqType{
    SINGLE        = 0,
    INTERLEAVED_2 = 1,
    INTERLEAVED_4 = 3
};

/**
 * Possible acquisition modes. See acquisitionMode
 */
enum SrTrAcqMode{
    RAW         = 0,
    PROCESSED   = 1,
    SEGMENTED   = 2,
    CALIBRATION = 3
};

/**
 * Possible trigger types. See triggerType
 */
enum SrTrTriggerType{
    SOFTWARE,
    HARDWARE
};

/**
 * Possible trigger modes. See triggerMode
 */
enum SrTrTriggerMode{
    LEVEL = 0,
    FLANK = 1
};

/**
 * Possible frequencies
 */
enum SrTrFrequency{
    FREQUENCY_250_400 = 1,
    FREQUENCY_200     = 3,
    FREQUENCY_125_160 = 5,
    FREQUENCY_100     = 7,
    FREQUENCY_62_5_80 = 9,
    FREQUENCY_50      = 11
};

OBJECT_DLL(SrTrATCADrv)
// Callback declaration for the thread responsible for downloading data from 
// the memory
void MemoryDownloadThreadCallback(void *userData);
// Callback declaration for the thread responsible for providing real-time data
void RTDataThreadCallback(void *userData);

// SrTrATCADrv
class SrTrATCADrv : public GenericAcqModule , public MessageHandler{
OBJECT_DLL_STUFF(SrTrATCADrv)
///CallBack
friend void MemoryDownloadThreadCallback(void *userData);
friend void RTDataThreadCallback(void *userData);

private:
    /**   
     * File descriptor for the device driver
     */ 
    int32 device;
    /*
     * The id of the thread responsible for downloading the data
     */
    TID   memoryDownloadThreadId;
    /**
     * CPU mask where the thread responsible for downloading the data lives on
     */
    int32 memoryDownloadThreadCpuMask;
    /** 
     * Memory download thread priority within real time class 
     */
    int32 memoryDownloadThreadPriority;

    /*
     * The id of the thread responsible for acquiring the RT data
     */
    TID   rtThreadId;
    /**
     * CPU mask where the thread responsible for the RT data lives on
     */
    int32 rtThreadCpuMask;
    /** 
     * RT data thread priority within real time class 
     */
    int32 rtThreadPriority;

    /**
     * This semaphore is to be posted every time a data download is to be performed
     */
    EventSem memoryDownloadEventSem;

    /**
     * Posted when the real-time data path should be enabled
     */
    EventSem rtThreadEventSem;

    /**
     * Only one thread (memoryDownloadThread and rtThread) can access the dma data at the time
     */
    MutexSem dmaPathMux;

    /**
     * Set to true when an acquisition is in progress (true even with realtimeEnabled = False)
     */
    bool acquisitionInProgress;

    /**
     * Set to true while a RT acquisition is in progress
     */
    bool rtAcquisitionInProgress;

    /**
     * Flag to control the memory download thread
     */
    bool  memoryDownloadThreadKeepRunning;

    /**
     * Flag to control the rt thread
     */
    bool  rtThreadKeepRunning;

    /**
     * Real-time access required? It will depend on the firmware as well
     */
    bool realtimeEnabled;

    /**
     * The board slot number
     */
    int32 slotNumber;

    /**
     * The type of acquisition. The module can acquire each acquisition channel separately
     * or a programmable number of simultaneous channels per module block.
     * It defines the number of ADCs which are grouped together. SINGLE means no interleave
     */
    SrTrAcqType acquisitionType;

    /**
     * Acquire raw, segmented, processed or calibration mode
     */
    SrTrAcqMode acquisitionMode;

    /**
     * The acquisition frequency
     */
    SrTrFrequency frequency;

    /**
     * The type of trigger to use: software or hardware
     */
    SrTrTriggerType triggerType;

    /**
     * The trigger mode: level or flank
     */
    SrTrTriggerMode triggerMode;

    /**
     * An external file with calibration parameters
     */
    FString externalCalibrationFile;

    /**
     * Number of samples to store before a trigger (only for segmented mode)
     */
    int32 preTriggerSamples;

    /**
     * The width of the pulse in samples (only for segmented mode)
     */
    int32 pulseWidth;

    /**
     * The accuracy of the trigger (only for segmented mode)
     */
    int32 triggerAccuracy;

    /**
     * Programmable delay after trigger occurrence
     */
    uint32 triggerDelay;

    /**
     * Rise time of the trapezoidal shape given in number samples, for the processed mode
     */
    int32 processK;

    /**
     * The duration of the flat top of the trapezoidal shape
     */
    int32 processL;

    /**
     * This parameter is the pole zero cancellation and depends
     * only on the decay time constant of the exponential pulse and the
     * sampling period, T, of the TRP-400 module
     */
    int32 processM;

    /**
     * Trigger level is a function of a discrete parameter (n).
     */
    int32 processT;

    /**
     * Calculate the trigger from filtered data
     */
    SrTrFilter triggerFilter;

    /**
     * Apply a filter on the input data
     */
    SrTrFilter inputFilter;

    /**
     * Invert the input signal
     */
    int32 invertInputSignal;
    
    /**
     * Number of bytes to acquire. This number, divided by the number of connected channels,
     * must be a multiple of readByteSize
     */
    uint32 acquisitionByteSize;

    /**
     * Number of bytes to transfer in each read. Must be a multiple of dmaByteSize
     */
    uint32 readByteSize;

    /**
     * Number of bytes to be sent on each DMA transfer 
     */
    uint32 dmaByteSize;

    /**
     * If a channel is connected
     */
    bool channelConnected[N_CHANNELS];

    /**
     * Short cut to store the number of connected channels
     */
    uint32 numberOfConnectedChannels;

    /**
     * Name of the channels where ram data is going to be stored
     */
    FString channelRamDataFilename[N_CHANNELS];

    /**
     * The base directory where to store the ram data
     */
    FString baseDirectoryRamData;

    /**
     * An optional relative (to baseDirectoryRamData) directory.
     * Not a very elegant parameter, but can be useful if only this location changes in a regular basis
     */
    FString relativeDirectoryRamData;

    /**
     * If the acquisition was aborted do not download data
     */
    bool    acquisitionAborted;

    /**
     * A copy of the latest RT packet received
     */
    RTDataPacket latestRTDataPacket;

    /**
     * The expected rt period of the board. New data should be sent at this rate
     */
    int32 rtUsecPeriod;

    /**
     * Number of packet size mismatches
     */
    int32 sizeMismatchErrorCounter;
    
    /**
     * Number of lost packets
     */
    int32 lostPacketErrorCounter;

    /**
     * Usec time of the last received packet
     */
    int32 lastPacketUsecTime;

    /**
     * When in offline, provide a non real-time millisecond tick just to make the rtthread go
     */
    int32 offlineMSecTick;

    /**
     * Frequency factor to convert from us to FPGA ticks in the post
     */
    uint32 postFrequencyToTicksFactor;
 
public:

    /** 
     *  The callback for the memory download thread
     */
    void MemoryDownloadCallback(void* arg);

    /**
     * The callback for the thread which retrieves the real-time data
     */
    void RTDataCallback(void *arg);

    /** 
     * Starts an acquisition 
     * @return True if the acquisition is successfully started
     */ 
    bool StartAcquisition();

    /** 
     * Stops an acquisition 
     * @return True if the acquisition is successfully stopped
     */ 
    bool StopAcquisition();

    /**
     * Stores the data collected in the board ram into the filenames
     * @param directory name. If empty it will be stored directly in the baseDirectoryRamData
     * @return True if data is successfully stored
     */
    bool StoreRamData();
    

public:

    /** Enable the System Acquisition */
    bool EnableAcquisition();

    /** Disable the System Acquisition */
    bool DisableAcquisition();

    /** Gets Data From the Module to the DDB
        @param usecTime Microseconds Time
        @return -1 on Error, 1 on success
    */
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);

public:

    /** Not available in the sr-tr-atca board*/
    bool WriteData(uint32 usecTime, const int32 *buffer){
        return False;
    }

///////////////////////////////////////////////////////////////////////////////
//                           Init & Mixed routine                            //
///////////////////////////////////////////////////////////////////////////////
private:    

    /** Copy constructors (since it is defined private it won't allow a public use!!) */
    SrTrATCADrv(const SrTrATCADrv&){};

    /** Operator=  (since it is defined private it won't allow a public use!!) */
    SrTrATCADrv& operator=(const SrTrATCADrv&){};

public:

    /** Constructor */
    SrTrATCADrv();

    /** Deconstructor */
    ~SrTrATCADrv();

    /** Load Object Parameters from the ConfigurationDataBase */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** Saves Object Parameters to the ConfigurationDataBase */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err){
        return True;
    };

    /** Object Description */
    virtual bool ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
        return True;
    }

    /** Set board used as input */
    virtual bool SetInputBoardInUse(bool on = True) {
        if(on && inputBoardInUse) {
            AssertErrorCondition(InitialisationError, "SrTrATCADrv::SetInputBoardInUse: Board %s is already in use", Name());
            return False;
        }
        inputBoardInUse  = on;
        return True;
    }

    /** Set board used as output */
    virtual bool SetOutputBoardInUse(bool on = True) {
        AssertErrorCondition(InitialisationError, "SrTrATCADrv::SetOutputBoardInUse: Board %s does not support output", Name());
        return False;
    }

public:

    // Get the Time. To be implemented when the FPGA real-time extension is available
    int64  GetUsecTime(){
        return latestRTDataPacket.usecTime;
    }

    // Serve webpage
    bool   ProcessHttpMessage(HttpStream &hStream);

    /**
     * Process the messages to handle the board acquisition
     * The message is analysed first by checking if the content is any of:
     * "START_ACQUISITION" = Start a new acquisition (assumes the board is set-up already)
     * "STOP_ACQUISITION"  = Stops an ongoing acquisition
     * "STORE_ACQUISITION" = Stores data relative an acquisition by posting on the data downloading thread
     * If the content is not equal to any of these, it checks to see if the message code is equal to 0xAAFF
     * and if it is, it will set the relativeDirectoryRamData equal to the message content.
     * Otherwise it will issue an error
     * @param envelope the message envelope
     * @return True if the message code or content are as expected
     */
    bool   ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

    /**
     * Called at pulse start
     */
    bool PulseStart();
};

#endif
