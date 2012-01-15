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

#if !defined(_STATISTIC_GAM_)
#define _STATISTIC_GAM_

#include "GAM.h"
#include "System.h"

class DDBInputInterface;
class DDBOutputInterface;

class StatSignalInfo{
private:
	
    double         sum;
	
    double         sumPower2;
	
    float          maxi;
	
    float          mini;

public:

StatSignalInfo():sum(0.0),sumPower2(0.0), maxi(-1e16), mini(1e16){};
    
    void Update(float sample){
    	sum       += sample;
    	sumPower2 += sample*sample;
    	if(maxi < sample)maxi = sample;
    	if(mini > sample)mini = sample;
    }
    
    void Reset(){
    	sum = 0.0;
    	sumPower2 = 0.0;
    	maxi = -1e16;
    	mini = +1e16;	
    }
	
    /* Remove the maximum from the sum */
    float Mean(int sampleNumber){
        if(sampleNumber < 2) return 0.0;
        return (sum- maxi)/(sampleNumber - 1);
    }
    
    /* Remove the maxi from the sumPower2*/
    float Variance(int sampleNumber){
        if(sampleNumber < 2)return 0.0;
        float mean = Mean(sampleNumber); 
        float var  = ((sumPower2 - maxi*maxi)/(sampleNumber - 1) - mean*mean);
        return var;
    }
	
    float Min(){return mini;}
	
    float Max(){return maxi;}
	
};


OBJECT_DLL(StatisticGAM)
class StatisticGAM: public GAM{
private:

    /** Jpf Data Collection */
    DDBInputInterface                              *inputData;

    /** Jpf Data Collection */
    DDBOutputInterface                             *statistics;
    
    /** SampleNumber */
    uint32                                         sampleNumber;
    
    /** */
    uint32                                         numberOfSignals;
                                   
    /**Statistic Information */
    StatSignalInfo                                 *statInfo;

    /**Verbose flag */
    bool					   verbose;

    /**Frequency of Verbose messages */
    int32					   frequencyOfVerbose;

public:

    /** */
    StatisticGAM(){
        inputData       	= NULL;
        statistics      	= NULL;
        statInfo        	= NULL;
        sampleNumber    	= 0;
        numberOfSignals 	= 0;	
        verbose			    = False;
        frequencyOfVerbose 	= 1;
    };
    
    /** */
    virtual ~StatisticGAM(){
        if(statInfo != NULL) delete[] statInfo;
    };

    // Initialise the module
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /** Implements the Saving of the parameters to Configuration Data Base */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){return True;};

    OBJECT_DLL_STUFF(StatisticGAM)
};


#endif


