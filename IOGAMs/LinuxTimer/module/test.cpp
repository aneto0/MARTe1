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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "System.h"
#include "HRT.h"

int main(int argc, char *argv[]) {
    char buffer[1];
    memset(buffer, 0, 1);
	
    int f = open("/dev/LinuxTimer",O_RDONLY);
    int64 tStart  = 0;
    int64 tStop   = 0;
    int64 tDelta  = 0;
    int64 min     = 10000000;
    int64 max     = 0;
    int64 diff    = 0;
    int   counter = 0;
    while(counter++ < 60000){
        tStart = HRTRead64();
        read(f,buffer,1);
        tStop  = HRTRead64();
	diff   = tStop - tStart; 
        tDelta += diff;
	if(diff > max){
		max = diff;
	}
	if(counter > 2 && diff < min){
		min = diff;
	}
    }
    double time = (tDelta*HRTClockCycle())/counter;
    printf("Average Wait : %e \n", time);
    printf("Maximum Wait : %e \n", (max * HRTClockCycle()));
    printf("Minimum Wait : %e \n", (min * HRTClockCycle()));
    return 1;
}
