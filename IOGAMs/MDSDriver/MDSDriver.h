
#ifndef MDSDRIVER_H_
#define MDSDRIVER_H_

// framework includes
#include "System.h"
#include "MutexSem.h"
#include "GenericAcqModule.h"
#include "MDSInterface.h"

// framework object registration macro
OBJECT_DLL(MDSDriver)

class MDSDriver: public GenericAcqModule 
{
// framework object registration macro
private:
    int deviceIdx;
    int prevTime;
// Reference Waveform settings
    char **refWaveformNames;
    int numRefWaveforms;
    int *refWaveformIdxs;
//Output Signals settings
    char **outSignalNames;
    char **outSignalDescriptions;
    int numOutSignals;
    int *outSignalIdxs; 
    float *currInputs;
    float *currOutputs;
    void freeData()
    {
	if(numRefWaveforms > 0)
	{
	    for(int i = 0; i < numRefWaveforms; i++)
		delete [] refWaveformNames[i];
	    delete [] refWaveformNames;
	    delete [] refWaveformIdxs;
	    delete [] currInputs;
	}
	if(numOutSignals > 0)
	{
	    for(int i = 0; i < numOutSignals; i++)
	    {
		delete [] outSignalNames[i];
		if(outSignalDescriptions)
		    delete [] outSignalDescriptions[i];
	    }
	    delete []outSignalNames;
	    delete [] outSignalIdxs;
	    delete [] currOutputs;
	}
    }
    
public:
    /** Constructor */
    MDSDriver()
    {
	numRefWaveforms = numOutSignals = 0;
	deviceIdx = -1;
    prevTime = -1;
    }
    /** Destructor */
    ~MDSDriver()
    {
	freeData();
    }

    /** ObjectLoadSetup 
     * Configuration parameters are:
     * DeviceIdx: index of the target MDSplus device
     * Reference waveform name array: names of the reference waveforms. Must fit the declared waveforms in the device
     * Out Signal name array: output signal names. This is dynamic and not declared in advance in the pulse file.
     */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
    /** Save parameter */
    virtual bool ObjectDescription(StreamInterface &s, bool full=False, StreamInterface *err=NULL)
    { return True; }
    // ------------------------------------------------------------------------

    
    /** Set board used as input */
    virtual bool SetInputBoardInUse(bool on = True)
    {
	if(on) printf("SET INPUT\n");
        if(inputBoardInUse && on) {
            AssertErrorCondition(InitialisationError,
            		"MDSDriver::SetInputBoardInUse: Board %s is already in use", Name());
            return False;
        }
        inputBoardInUse  = on;
        return True;
    }
    /** Set board used as output */
    virtual bool SetOutputBoardInUse(bool on = True)
    {
	if(on) printf("SET OUTPUT\n");
        if(outputBoardInUse && on) 
	{
            AssertErrorCondition(FatalError,
            		"MDSDriver::SetOutputBoardInUse: Board %s can only be set as output", Name());
            return False;
	}
	return True;
    }
    // ------------------------------------------------------------------------

    
    virtual bool WriteData(uint32 usecTime, const int32 *buffer);

    /** GetData
     * Copy local input buffer in destination buffer:
     * return  0 if data not ready
     * return <0 if error
     * return >0 if OK (returns the number of bytes copied)
     * DataFormat: Time, trigger (always 1), samples
     */
    virtual int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0);
     /**
      * Polling method
      * Will returns true when data are available. Blocks when data is finished
      */
    virtual bool Poll(){return True;}
    virtual bool EnableAcquisition(){ printf("ENABLE ACQUISITION\n"); return True;}

    virtual bool DisableAcquisition(){ printf("ENABLE ACQUISITION\n"); return True;}

    /** return current time in mucroseconds */
    int64 GetUsecTime();
    
     /**
      * Output an HTML page with the current setting
      */
     virtual bool ProcessHttpMessage(HttpStream &hStream);
     /**
      */
     virtual bool PulseStart();
private:     
     OBJECT_DLL_STUFF(MDSDriver)

};


#endif /*MDSDRIVER_H_*/
