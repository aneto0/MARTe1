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

/**
 * @file
 * @brief Single Input Single Output filter
 *
 * Ready to be used filter which can initialised from a CDB
 */
#include "Object.h"

class ConfigurationDataBase;

#if !defined (__FILTER_H)
#define __FILTER_H

class Filter;

extern "C"{
    bool FilterInit(Filter &f,ConfigurationDataBase &cdb);
    bool FilterResize(Filter &f,int inputSize,int outputSize);
}

OBJECT_DLL(Filter)

/** @file */

/** implements a single input single output filter. Initialisation is performed via CDB */
class Filter: public Object {
private:
OBJECT_DLL_STUFF(Filter)
private:

    /** */
    friend bool FilterInit(Filter &f,ConfigurationDataBase &cdb);

    /** */
    friend bool FilterResize(Filter &f,int inputSize,int outputSize);

    /** number of memories of input including current sample */
    int inputSize;

    /** number of memories of output including produced output which is at location 0 */
    int outputSize;

    /** */
    float *inputStatus;

    /** */
    float *outputStatus;

    /** */
    double *inputCoefficients;

    /** */
    double *outputCoefficients;

private:
    bool InitP(ConfigurationDataBase &cdb);
    bool Resize(int inputSize,int outputSize);

public:
    /** */
    ~Filter(){
        FilterResize(*this,0,0);
    }

    /** constructor */
    Filter(){
        inputSize           = 0;
        outputSize          = 0;
        inputStatus         = NULL;
        outputStatus        = NULL;
        inputCoefficients   = NULL;
        outputCoefficients  = NULL;
    }

    /**
        Numerator  = {a0 a1 a2....}
        Denominator= {b0 b1 ..... }

        OR

        0  = {a0 a1 a2....}  b0 assumed to be 1
        1  = {-b1 -b2..... }

        OR


        Poles  = {p0 p1 ... } =>      p0         p1
             which means            ------  *  ------
                                    S + p0     S + p1

        Zeros  = {z0 z1 ... }       S + z0     S + z1
            whic means              ------  *  ------
                                      z0         z1
        SamplingTime = 1e-3

        Gain = g  which multiplies the whole expression


        The conversion uses the following equation between S and Z
        uses     2 (1-Z^-1)
             S = ----------
                 T (1+Z^-1)

        The first two filter configuration are already expressed in z domain,
        The last one is expressed in s domain and the Filter class discretizes
        the transfer function using Tustin.
    */
    bool Init(ConfigurationDataBase &cdb){
        return FilterInit(*this,cdb);
    }

    /** */
    inline void Reset(float input=0.0,float output=0.0){
        int j;
        for (j = 0;j < inputSize;j++){
            inputStatus[j]  = input;
        }
        for (j = 0;j < outputSize;j++){
            outputStatus[j] = output;
        }
    }

#if !defined(_CINT)
    /** */
    inline float Process(float input){

        int j = 0;
        // update inputs memory
        for (j=inputSize-1;j > 0;j--){
            inputStatus[j] = inputStatus[j-1];
        }
        inputStatus[0] = input;

        // calculate the filter
        float output = 0.0;
        const float *filterStatus = inputStatus;
        const double *filterCoeffs = inputCoefficients;

        for (j = 0;j < inputSize;j++)
            output += filterStatus[j] * filterCoeffs[j];

        filterStatus = outputStatus;
        filterCoeffs = outputCoefficients;

        // update outputs memory
        for (j = outputSize-1;j > 0;j--){
            outputStatus[j] = outputStatus[j-1];
        }

        for (j = 1;j < outputSize;j++)
            output += filterStatus[j] * filterCoeffs[j];

        outputStatus[0] = output;

        return output;
    }
#endif
};



#endif
