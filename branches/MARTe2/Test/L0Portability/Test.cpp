/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
   permissions and limitations under the Licence. 
 *
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

#include <stdio.h>
#include "Processor.h"
#include "FastMath.h"
#include "HighResolutionTimer.h"
#include "Endianity.h"
#include "EndianityTest.h"
#include "MutexSem.h"
#include "TimerTest.h"

void Success(bool ret, const char *file, uint32 line, const char *command){
    if(!ret){
        printf("***************************\n");
        printf("File %s Line %i\n",file,line);
        printf("***** %s failed\n",command);
        printf("***************************\n");
    }
    else{
        printf("%s OK\n",command);
    }
}

#define ExpectSuccess(x)       Success((x),__FILE__,__LINE__,#x);

int main(int argc, char **argv){
    Processor p;
    printf("Processor\n");
    printf("Vendor=%s\n", p.VendorId());
    printf("Family=%u\n", p.Family());
    printf("Model=%u\n", p.Model());
    printf("#Cores=%u\n", p.Available());

    HighResolutionTimer hrt; 
    printf("\n\n\nHighResolutionTimer\n");
    printf("Frequency=%lld\n", hrt.Frequency());
    printf("Period=%e\n", hrt.Period());

    printf("\n\n\nEndianity = %d\n", Endianity::Type());
    EndianityTest<uint16> uint16Test(0xAABB);
    ExpectSuccess(uint16Test.All());

    EndianityTest<int16> int16Test(0xAABB);
    ExpectSuccess(int16Test.All());

    EndianityTest<uint32> uint32Test(0xFFEEDDCC);
    ExpectSuccess(uint32Test.All());

    EndianityTest<int32> int32Test(0xAABBCCDD);
    ExpectSuccess(int32Test.All());

    EndianityTest<uint64> uint64Test(0xFFEEDDCCBBAA1122);
    ExpectSuccess(uint64Test.All());

    EndianityTest<int64> int64Test(0xAABBCCDDEEFF3344);
    ExpectSuccess(int64Test.All());

    EndianityTest<float> floatTest(0.12345f);
    ExpectSuccess(floatTest.All());

    EndianityTest<double> doubleTest(0.123456789);
    ExpectSuccess(doubleTest.All());

    //Missing test classes
    printf("sin(pi/2) = %f cos(pi/2) = %f\n", FastMath::Sin(FastMath::PI_2), FastMath::Cos(FastMath::PI_2));
    printf("sin(pi/4) = %f cos(pi/4) = %f\n", FastMath::Sin(FastMath::PI_4), FastMath::Cos(FastMath::PI_4));

    //Missing test classes
    MutexSem mux;

    TimerTest tt;
    ExpectSuccess(tt.ConfigAndStartTimerTest());
}

