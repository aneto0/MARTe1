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
 * The definition of the class HRTSynchronised.
 * A synchronised source of time base on the HRT.
 */
#if !defined (HRTSYNCHRONISED_H)
#define HRTSYNCHRONISED_H

#include "HRT.h"

class HRTSynchronised{

private:

    /** Reference time of the previous cal to Syncronise() */
    double oldTime;

    /** HRT value at the time of the previous call to Synchronise().*/
    int64 oldHrt;


    /** Current estimate of the HRT clock period. */
    double hrtPeriod;

    /** Offset used to calculate the time estimation. */
    double timeOffset;

    /** Offset used to calculate the time estimation. */
    double lastTimeOffset;

//    double period;
//    double error;

public:

    /** Constructor. */
    HRTSynchronised(){
//        period=0;
        hrtPeriod=-1;
        timeOffset=0;
        oldHrt=HRTRead64();
        oldTime=-1;
//        error=0;
    }

    /** To be called periodically to synchronise the time. This routine needs
        to be called at least 3 times before the class can be used.
        @param usecTime Is the reference time in microseconds. */
    void Synchronise(int64 usecTime){
        double time=(double)usecTime;
        time*=1e-6;

        volatile int64 N=HRTRead64();

        if (oldTime>0){

            volatile int64 dN=N-oldHrt;
            double deltaN=(double)dN;
            double deltaT = time-oldTime;
            double period = deltaT / (double)deltaN;
            if (hrtPeriod<=0){
                hrtPeriod=period;
            }
            else{
                hrtPeriod=hrtPeriod*0.9999+period*0.0001;
            }

            double k=1e-9;

            timeOffset+=hrtPeriod*deltaN;
            double  error=time-timeOffset;

            lastTimeOffset=timeOffset;
            timeOffset+=deltaN*error*k;
        }

        oldHrt=N;
        oldTime=time;

    }

     /** The estimated HRT period. The estimation is done using the external source. */
    double Period(){
        return hrtPeriod;
    }

    /** Returns the estimated time in seconds. */
    double CurrentTime(){
        int64 deltaT=HRTRead64()-oldHrt;
        double t=(double)deltaT;
        return lastTimeOffset+hrtPeriod*t;
    }

};

#endif
