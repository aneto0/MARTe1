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

#ifndef _WEBSTATISTIC_GAM_
#define _WEBSTATISTIC_GAM_

#include "GAM.h"
#include "System.h"

class DDBInputInterface;
class DDBOutputInterface;

/**
 * Holds information on statistical data on a signal.
 */
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


/**
 * The WebStatistic GAM.
 * Calculates statistical informations about a signal.
 */
OBJECT_DLL(WebStatisticGAM)
class WebStatisticGAM: public GAM, public HttpInterface {
private:
    /** Input interface to the DDB. */
    DDBInputInterface                              *inputData;
    /** Output interface to the DDB. */
    DDBOutputInterface                             *statistics;
    /** Current sample number */
    uint32                                         sampleNumber;
    /** Number of signal for which the GAM is keeping statistical data. */
    uint32                                         numberOfSignals;
    /** Pointer to the first StatSignalInfo class */
    StatSignalInfo                                 *statInfo;
    /** True if output on logger is desired at the end of the pulse. */
    bool					                       EOPOutput;
    /** True if output of memory information is desired at the end of the pulse. */
    bool                                           EOPMemInfo;
    /** Number of cycles before starting to keep statistical information. Used to discard the first wrong samples. */
    int64					                       voidCycles;
    bool					                       voidCyclesDone;
    /** The last time the data was updated*/
    int64                                          lastDataUpdateTime;

public:

    /** Constructor. */
    WebStatisticGAM(){
        inputData       	= NULL;
        statistics      	= NULL;
        statInfo        	= NULL;
        sampleNumber    	= 0;
        numberOfSignals 	= 0;
        EOPOutput           = True;
        EOPMemInfo          = True;
        voidCycles          = 0;
        voidCyclesDone      = False;
        lastDataUpdateTime  = 0;
    };

    /** Deconstructor. */
    virtual ~WebStatisticGAM(){
      if(statInfo != NULL) delete[] statInfo;
    };

    /**
    * Loads GAM parameters from a CDB
    * @param cdbData the CDB
    * @return True if the initialisation went ok, False otherwise
    */
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /**
    * GAM main body
    * @param functionNumber The current state of MARTe
    * @return False on error, True otherwise
    */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /**
    * Saves parameters to a CDB
    * @param info the CDB to save to
    * @return True
    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){return True;};

    /**
     * Builds the webpage with the statistical info.
     * @param hStream The HttpStream to write to.
     * @return False on error, True otherwise.
     */
    virtual bool ProcessHttpMessage(HttpStream &hStream);

    /**
     * Builds a text page with the statistical info.
     * @param hStream The HttpStream to write to.
     * @return False on error, True otherwise.
     */ 
    bool ProcessHttpMessageTextMode(HttpStream &hStream);

    OBJECT_DLL_STUFF(WebStatisticGAM)
};


#endif


