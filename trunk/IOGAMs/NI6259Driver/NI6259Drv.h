//******************************************************************************
//    $Log: NI6259Drv.h,v $
//
//******************************************************************************

/*
 * NI6259 Linux User-Space GenericAcqModule (GACQM)for NI 6259.
 * author: Gabriele Manduchi
 * The NI6259 GACQM provides support for all the functionality provided by the 
 * National Instruments NI6259 device, namely Analog Input (ADC), Analog Output(DAC)  
 * and Digital Input/output (DIO).
 * The configuration field IsAnalog which can  be either "True" or "False". When True
 * the GACQM instance can work as either ADC (NumberOfInputs > 0, NumberOfOutputs == 0) or
 * DAC (NumberOfInputs == 0, NumberOfOutputs > 0) but not both. 
 * When the GACQM instance is used as ADC, up do 32 channels can be read and the following 
 * configuration parameters are considered:
 * - InputRange (default "10V") which can be either 
 *     "10V"
 *     "5V"
 *     "2V"
 *     "1V"
 *     "500mV"
 *     "200mV"
 *     "100mV"
 * - Polarity (default "Bipolar") which can be either 
 *    "Unipolar"
 *    "Bipolar"
 * - InputMode (default "RSE") which can be either
 *    "Differential"
 *    "NRSE"
 *    "RSE"
 * - Frequency (default external) indicating the internal clock frequency in Hz. -1 means external clock
 *   which must be provided in PFI 0 input.
 *
 * When the GACQM instance is used as DAC, up to 4 output channels are defined. In this case no further 
 * configuration parameter is needed.
 * 
 * When the GACQM instance is used as DIO, the 32 bits of the P0 group can be used as either 
 * input or output. Input/Output bit direction is defined by configuration parameter DigitalMask (int32).
 * A bit set to 0 in DigitalMask means that the corresponding bit is an input. The default is 
 * DigitalMask = 0, i.e. all bits as inputs. Unlike the analog configuration, a single GACQM instance
 * must  be defined for input and output, possibily connecting the same intance to an InputGAM and
 * an OutputGAM.
 * When configuration parameter NumberOfInputs is greater than 0, i.e. the GACQM is used by an InputGAM
 * the first output is the time, followed by the actual inputs. For example, if 4 analog input channels 
 * are used, NumberOfInputs must be equal to 5. If the GACQM is used for digfital input, NumberOfInputs
 * will be always 2, i.e. time and the input 32 bit word (the bits actually used in input will be defined
 * in DigitalMask parameter)
 */
 
#ifndef NI6259DRV_H_
#define NI6259DRV_H_

#include "System.h"
#include "GenericAcqModule.h"
#include <pxi-6259-lib.h>
#include <pxi6259-enums.h>


#define NI6259_DEFAULT_ADDRESS "/dev/pxi6259"

OBJECT_DLL(NI6259Drv)

#define IS_NI6259_ADC 1
#define IS_NI6259_DAC 2
#define IS_NI6259_DIO 3
#define IS_NI6259_UNDEF 4

class NI6259Drv:public GenericAcqModule {
private:
//Configuration structures 
	pxi6259_ai_conf_t adcConf;
    pxi6259_dio_conf_t dioConfig;
    pxi6259_ao_conf_t dacConf;
	int adcFd;
	int dacFd;
	int dioFd;
	int chanAdcFd[32];
	int chanDacFd[4];
	int chanDioFd;
//Board ID
	int boardId;
// ADC/DAC mode
	int mode;
//Digital /analog switch. When analog input true, the input is ADC and output is DAC. 
//When false the device is Digital I/O and dioMask specifies input and output channels. 
	bool isAnalog;
//Digital IO mask. Bit 1 means digital out
    unsigned int dioMask;

// Configuration loaded flag
	bool configLoaded;	
//Typical input is performed by Poll(), which stores actual samples in dataBuffer
	bool dataAvailable;
	float dataBuffer[32];
//Previoud DAC clock value
	int prevDacClock;
//Sample Count
	int sampleCount;
//Acquisition enabled flag
	bool acquisitionEnabled;
///////////////////////////////////////////////////////
//ADC CONFIGURATION ///////////////////////////////////
	//number of input channels (max 32 if common mode, 16 if differential)
	int numInputChannels;
	//Input gain
	int gain;
	//Input mode (Differential, RSE, NRSE)
	ai_channel_type_t inputMode;
	//Polarity (Unipolar/Bipolar)
	ai_polarity_t polarity;
	//Internal clock frequency. If negative the clock is derived from PFI0 digital input 
	float frequency;
///////////////////////////////////////////////////////
//DAC CONFIGURATION
	//Number of output channels (Max 4)
	int numOutputChannels;
	//Index of sycnhronizing digital input (PO.0-31)
	int synchIdx;
	//Previous DAC samples
	float prevDacSamples[4];
// Generic
	int64 lastCycleUsecTime;
	// 
	// HttpInterface extension
    	const char *css;

public:
    /** Constructor */
    NI6259Drv();
    /** Deconstructor */
    virtual ~NI6259Drv();

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
    virtual bool EnableAcquisition();
    virtual bool DisableAcquisition();
 		
     // Set board used as input
    virtual bool SetInputBoardInUse(bool on = True)
    {
	    printf("NI6259::SetInputBoardInUse: %d\n", on);
        if(inputBoardInUse && on){
             AssertErrorCondition(InitialisationError, "NI6259Drv::SetInputBoardInUse: Board %s is already in use", Name());
             return False;
        }
        if(isAnalog)
        {
            if(on)
	            mode = IS_NI6259_ADC;	
	        else
	            mode = IS_NI6259_DAC;
        }
        else
            mode = IS_NI6259_DIO;	
        EnableAcquisition();
        return True;
    }


    virtual bool SetOutputBoardInUse(bool on = True)
    {
	    printf("NI6259::SetOutputBoardInUse: %d\n", on);
        if(outputBoardInUse && on){
             AssertErrorCondition(InitialisationError, "NI6259Drv::SetOutputBoardInUse: Board %s is already in use", Name());
             return False;
        }
        if(isAnalog)
        {
            if(on)
	            mode = IS_NI6259_DAC;
	        else
	            mode = IS_NI6259_ADC;
        }
        else
            mode = IS_NI6259_DIO;	
        EnableAcquisition();
        return True;
    }


    // *** NOT IMPLEMENTED ***
    int64 GetUsecTime(){return lastCycleUsecTime;}
    
     /**
      * Output an HTML page with the current value in mV of the acquired signals
      */
     virtual bool ProcessHttpMessage(HttpStream &hStream);
     /**
      * Polling method
      */
     virtual bool Poll();
     
     /**
      * Pulse start
      */
     virtual bool PulseStart();
     
private:
	OBJECT_DLL_STUFF(NI6259Drv)
};

#endif /*NI6259DRV_H_*/
