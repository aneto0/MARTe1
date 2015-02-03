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
 * $Id: Processor.cpp 43 2012-02-08 17:07:58Z astephen $
 *
**/

#ifndef HIGH_RESOLUTION_TIMER_CALIBRATOR_OS_H
#define HIGH_RESOLUTION_TIMER_CALIBRATOR_OS_H

#include<stdio.h>
#include<GeneralDefinitions.h>

uint32 Processor_mSecTics;
uint64 Processor_HRTFrequency;
double Processor_HRTPeriod;

class HighResolutionTimerCalibratorOS{
public:
    HighResolutionTimerCalibratorOS(){
#define LINUX_CPUINFO_BUFFER_SIZE 1023
        Processor_HRTFrequency = 0;
        Processor_HRTPeriod    = 0;

        char buffer[LINUX_CPUINFO_BUFFER_SIZE + 1];

        FILE *f;
        f=fopen("/proc/cpuinfo","r");
        uint32 size = LINUX_CPUINFO_BUFFER_SIZE;
        size = fread(buffer,size,1,f);
        fclose(f);

        const char *pattern = "MHz";
        char *p = strstr(buffer,pattern);
        if (p != NULL){
            p = strstr(p,":");
            p++;
            double f = atof(p);
            if(f != 0){
                f *= 1.0e6;
                Processor_HRTFrequency = (int64)f;
                Processor_HRTPeriod    = 1.0 / f;
            }
        }
        Processor_mSecTics = Processor_HRTFrequency / 1000;
    }
};
#endif

